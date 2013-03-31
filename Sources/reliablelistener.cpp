#include "reliablelistener.h"


#include "stream.h"

#include "channel.h"




namespace NetDuke
{

static const ack_t s_fakeWindow = 10;

ReliableListener::ReliableListener()
	: m_pool(50)
	, m_stream(nullptr)
{
	SetTimeOut(5000);
}

ReliableListener::~ReliableListener()
{
	Flush();
}

size_t ReliableListener::GetHeaderSize() const
{
	return m_stream->GetHeaderSize() + Serializer::GetHeaderSize() + sizeof(seq_t) + sizeof(ack_t);
}

void ReliableListener::Flush()
{
	for(channels_t::iterator it=m_sendChannels.begin(); it != m_sendChannels.end(); ++it)
	{
		delete (*it).second;
	}
	m_sendChannels.clear();

	for(channels_t::iterator it=m_recvChannels.begin(); it != m_recvChannels.end(); ++it)
	{
		delete (*it).second;
	}
	m_recvChannels.clear();

	if(m_stream)
	{
		m_stream->DetachListener(*this);
		UnRegisterStream(*m_stream);
	}
}

void ReliableListener::Tick()
{
	FlushSend();

	FlushTimeout();
}

void ReliableListener::FlushSend()
{
	assert(m_stream != nullptr);

	for(channels_t::iterator it=m_sendChannels.begin(); it != m_sendChannels.end(); ++it)
	{
		Channel& channel = *(*it).second;
		const Peer &peer = channel.GetPeer();

		if(!channel.IsEmpty())
		{
			SerializerLess ser;
			channel.Pop(ser);

			SendToStream(ser, peer);
		}
	}
}

void ReliableListener::SendToStream(SerializerLess& _ser, const Peer &_peer)
{
		SerializerLess stream_ser(_ser, m_stream->GetHeaderSize(), _ser.GetBufferSize());
		size_t stream_size = stream_ser.GetSize();

		seq_t sequence;
		ack_t ack;

		netBool is_ok = stream_ser.Read(GetType());
		assert(is_ok);
		static_cast<Serializer&>(stream_ser) >> sequence;
		static_cast<Serializer&>(stream_ser) >> ack;
		stream_ser.Close();

		PlayerReliableInfo_t& reliableInfo = GetReliableInfo(_peer);

		is_ok = stream_ser.Write(GetType());
		assert(is_ok);
		// if it is a retransmission, do not change the sequence number
		if(sequence == 0 && _ser.GetSize() != GetHeaderSize())
		{
			sequence = reliableInfo.AcquireSequence();	
		}

		static_cast<Serializer&>(stream_ser) << sequence;
		static_cast<Serializer&>(stream_ser) << reliableInfo.AcquireAck();
		stream_ser.SetCursor(stream_size);
		stream_ser.Close();
			
		// compute new size
		_ser.Write(m_stream->GetType());
		_ser.SetCursor(m_stream->GetHeaderSize() + stream_size);
		_ser.Close();

		m_stream->Push(_ser, _peer);

		if(_ser.GetSize() != GetHeaderSize())
		{
			// only wait ack if not a pure ack packet
			struct ReliableSendInfo sendInfo(sequence, _ser);
			reliableInfo.m_send.push_back(sendInfo);
		}
}

void ReliableListener::RegisterStream(Stream& _stream)
{
	assert(m_stream == nullptr);

	m_stream = &_stream;
}

void ReliableListener::UnRegisterStream(const Stream& _stream)
{
	assert(m_stream != nullptr);
	assert(m_stream->GetType() == _stream.GetType());

	m_stream = nullptr;
}

netBool ReliableListener::CheckTimeout(netU64 _start)
{
	netU64 time = Time::GetMsTime() - _start;

	return time > m_maxClock;
}

void ReliableListener::FlushTimeout()
{
	for(reliableInfo_t::iterator it = m_reliableInfo.begin(); it != m_reliableInfo.end(); ++it)
	{
		PlayerReliableInfo_t &reliableInfo = (*it).second;
		// check ack timeout
		if(reliableInfo.m_ackRecvTime != 0 && CheckTimeout(reliableInfo.m_ackRecvTime - m_maxClock / 2))
		{
			// force sending packet if needed for ack
			Channel& channel = GetChannel(m_sendChannels, reliableInfo.m_peer);

			if(channel.IsEmpty())
			{
				CreateBundle(channel);
			}

			// reset timer
			reliableInfo.m_ackRecvTime = 0;
		}

		// check expired packet
		std::list<ReliableSendInfo>::iterator iter = reliableInfo.m_send.begin();
		while( iter != reliableInfo.m_send.end())
		{
			struct ReliableSendInfo& sendInfo = (*iter);

			if(CheckTimeout(sendInfo.m_emissionTime))
			{
				// send back
				//Push(sendInfo.m_ser, reliableInfo.m_peer);

				// direct mode, no bundling
				SendToStream(sendInfo.m_ser, reliableInfo.m_peer);
				iter = reliableInfo.m_send.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	}
}

netBool ReliableListener::Pull(SerializerLess &_ser, Peer& _peer)
{
	bool is_pull = false;
	for(channels_t::iterator it = m_recvChannels.begin(); it != m_recvChannels.end(); ++it)
	{
		Channel& channel = *(*it).second;
		if(!channel.IsEmpty())
		{
			channel.Pop(_ser);
			_peer = (*it).first;
			is_pull = true;
			break;
		}
	}

	return is_pull;
}

void ReliableListener::CreateBundle(Channel& _channel)
{
	SerializerLess ser(m_pool.GetSerializer());
	// prepare serializer
	ser.Write(m_stream->GetType());

	SerializerLess stream_ser(ser, m_stream->GetHeaderSize(), ser.GetBufferSize());
	stream_ser.Write(GetType());
	// sequence number, ack number
	stream_ser << static_cast<seq_t>(0);
	stream_ser << static_cast<ack_t>(0);
	stream_ser.Close();

	ser.SetCursor(m_stream->GetHeaderSize() + stream_ser.GetSize());
	ser.Close();
	_channel.Push(ser);
}

netBool ReliableListener::Push(SerializerLess &_ser, const Peer& _peer)
{
	assert(m_stream != nullptr);

	Channel& channel = GetChannel(m_sendChannels, _peer);

	SerializerLess *back = channel.Back();

	// bundling
	if(channel.IsEmpty() || back->GetSize() + _ser.GetSize() > back->GetBufferSize())
	{
		CreateBundle(channel);
	}

	return Pack(_ser, _peer);
}


netBool ReliableListener::Pack(SerializerLess& _ser, const Peer& _peer)
{
	Channel& channel = GetChannel(m_sendChannels, _peer);

	assert(!channel.IsEmpty());
	SerializerLess& ser = *channel.Front();

	ser.Write(m_stream->GetType());

	SerializerLess listener_ser(ser, m_stream->GetHeaderSize(), ser.GetBufferSize());
	size_t listener_size = listener_ser.GetSize();

	listener_ser.Write(GetType());
	listener_ser.SetCursor(listener_size);

	listener_ser << _ser;
	listener_ser.Close();

	ser.SetCursor(m_stream->GetHeaderSize() + listener_ser.GetSize());
	ser.Close();

	return true;
}

netBool ReliableListener::PreUnpack(SerializerLess& _ser, const Peer& _peer)
{
	netBool is_valid = false;
	if(_ser.Read(GetType()))
	{
		PlayerReliableInfo_t& reliableInfo = GetReliableInfo(_peer);

		// unpack header first
		seq_t sequence;
		ack_t ack;

		static_cast<Serializer&>(_ser) >> sequence;
		static_cast<Serializer&>(_ser) >> ack;

		RecvAck(reliableInfo, ack);
		RecvSequence(reliableInfo, _ser, sequence);

		is_valid = true;
	}

	return is_valid;
}

netBool ReliableListener::UnPack(SerializerLess& _ser, const Peer& _peer)
{
	netBool is_valid = false;
	if(_ser.Read(GetType()))
	{
		Channel& channel = GetChannel(m_recvChannels, _peer);

		// unpack header first
		seq_t sequence;
		ack_t ack;

		static_cast<Serializer&>(_ser) >> sequence;
		static_cast<Serializer&>(_ser) >> ack;

		while(_ser.GetCursor() != _ser.GetSize())
		{
			SerializerLess body;
			_ser >> body;

			channel.Push(body);
		}

		_ser.Close();
		is_valid = true;
	}

	return is_valid;
}

netBool ReliableListener::PullFromStream(SerializerLess& _ser, const Peer& _peer)
{
	// read serializer type
	netBool is_valid = _ser.Read(GetType());

	if(is_valid)
	{
		_ser.Close();
		netBool ret = PreUnpack(_ser, _peer);
		assert(ret);
	}

	return is_valid;
}

netBool ReliableListener::CompareSequence(PlayerReliableInfo_t &_reliableInfo, seq_t _sequence)
{
	// note : a _sequence = 0 means pure ack, so it cannot be ordered
	netBool is_ordered = false;

	// warning in case of cycling number !
	if(_reliableInfo.m_recvAck == 0xFFFF && _sequence == 1)
	{
		is_ordered = true;
		_reliableInfo.IncAck();
	}
	else if(_reliableInfo.m_recvAck+1 == _sequence)
	{
		is_ordered = true;
		_reliableInfo.IncAck();
	}

	return is_ordered;
}

netBool ReliableListener::RecvSequence(PlayerReliableInfo_t &_reliableInfo, SerializerLess& _ser, seq_t _sequence)
{
	netBool is_ordered = CompareSequence(_reliableInfo, _sequence);
	
	if(is_ordered)
	{
		// unpack
		_ser.Close();
		UnPack(_ser, _reliableInfo.m_peer);

		// check if we fill a hole
		_reliableInfo.m_recv.sort();
		netBool is_contiguous = true;
		std::list<ReliableRecvInfo>::iterator iter = _reliableInfo.m_recv.begin();
		while(is_contiguous && iter != _reliableInfo.m_recv.end())
		{
			struct ReliableRecvInfo& cmp = (*iter);
			is_contiguous = CompareSequence(_reliableInfo, cmp.m_sequence);
			if(is_contiguous)
			{
				UnPack(cmp.m_ser, _reliableInfo.m_peer);
				iter = _reliableInfo.m_recv.erase(iter);
			}
		}
	}
	else
	{
		// special case, if not pure ack, wait an additional time for it
		if(_sequence != 0)
		{
			_ser.Close();
			struct ReliableRecvInfo recvInfo(_sequence, _ser);
			_reliableInfo.m_recv.push_front(recvInfo);
		}
	}

	return is_ordered;
}

netBool ReliableListener::CompareAck(seq_t _seq, ack_t _lastAck)
{
	netBool is_ack = false;
	//sendInfo.m_sequence <= _ack

	if(_lastAck <= s_fakeWindow)
	{
		ack_t diff = 0xFFFF - s_fakeWindow + _lastAck;

		if( _seq <= _lastAck || _seq >= diff)
		{
			is_ack = true;
		}
	}
	else if(_seq <= _lastAck)
	{
		is_ack = true;
	}

	return is_ack;
}

void ReliableListener::RecvAck(PlayerReliableInfo_t &_reliableInfo, ack_t _ack)
{
	if(_ack != 0)
	{
		std::list<ReliableSendInfo>::iterator it = _reliableInfo.m_send.begin();
		while(it != _reliableInfo.m_send.end())
		{
			struct ReliableSendInfo& sendInfo = (*it);
			if(CompareAck(sendInfo.m_sequence, _ack))
			{
				it = _reliableInfo.m_send.erase(it);
			}
			else
			{
				++it;
			}
		}

		// if no more ack to receive -> equivalent to _reliableInfo.m_lastAck-1 == _ack
		/*if(_reliableInfo.m_send.empty())
		{
			_reliableInfo.m_ackRecvTime = 0;
		}*/
	}
}

Channel& ReliableListener::GetChannel(channels_t& channels, const Peer& _peer)
{
	channels_t::iterator it = channels.find(_peer);
	Channel *channel_out = nullptr;

	if(it != channels.end())
	{
		channel_out = (*it).second;
	}
	else
	{
		channel_out = new Channel(_peer);
		channels[_peer] = channel_out;
	}

	return *channel_out;
}

PlayerReliableInfo_t& ReliableListener::GetReliableInfo(const Peer& _peer)
{
	reliableInfo_t::iterator it = m_reliableInfo.find(_peer);
	struct PlayerReliableInfo *reliableInfo_out = nullptr;

	if(it != m_reliableInfo.end())
	{
		reliableInfo_out = &((*it).second);
	}
	else
	{
		m_reliableInfo.insert(std::make_pair(_peer, PlayerReliableInfo_t(_peer)));

		// TODO : not sure
		struct PlayerReliableInfo& reliable = m_reliableInfo[_peer];
		reliableInfo_out = &reliable;
	}

	return *reliableInfo_out;
}

void ReliableListener::SetTimeOut(timer_t _ms)
{
	m_maxClock = _ms;
}




};
