#include "reliablelistener.h"


#include "stream.h"

#include "channel.h"




namespace NetDuke
{


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

			// push serializer to ack wait
			PlayerReliableInfo_t& reliableInfo = GetReliableInfo(peer);
			netU16 sequence = reliableInfo.AcquireSequence();

			// write ack number
			SerializerLess stream_ser(ser, m_stream->GetHeaderSize(), ser.GetBufferSize());
			size_t stream_size = stream_ser.GetSize();
			stream_ser.Write(GetType());
			static_cast<Serializer&>(stream_ser) << sequence;
			static_cast<Serializer&>(stream_ser) << reliableInfo.AcquireAck();
			stream_ser.SetCursor(stream_size);
			stream_ser.Close();

			// compute new size
			ser.Write(m_stream->GetType());
			ser.SetCursor(m_stream->GetHeaderSize() + stream_size);
			ser.Close();

			reliableInfo.m_send.push_back(struct ReliableSendInfo(sequence, ser));

			m_stream->Push(ser, peer);
		}
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

netBool ReliableListener::CheckTimeout(clock_t _start)
{
	clock_t time = clock() - _start;

	return time > m_maxClock;
}

void ReliableListener::FlushTimeout()
{
	for(reliableInfo_t::iterator it = m_reliableInfo.begin(); it != m_reliableInfo.end(); ++it)
	{
		PlayerReliableInfo_t &reliableInfo = (*it).second;
		// check ack timeout
		if(reliableInfo.m_ackWaitTime != 0 && CheckTimeout(reliableInfo.m_ackWaitTime + m_maxClock / 2))
		{
			// force sending packet if needed for ack
			Channel& channel = GetChannel(m_sendChannels, reliableInfo.m_peer);

			if(channel.IsEmpty())
			{
				CreateBundle(channel);
			}
			// reset timer
			reliableInfo.m_ackWaitTime = clock();
		}

		// check expired packet
		std::list<ReliableSendInfo>::iterator iter = reliableInfo.m_send.begin();
		while( iter != reliableInfo.m_send.end())
		{
			struct ReliableSendInfo& sendInfo = (*iter);

			if(CheckTimeout(sendInfo.m_emissionTime))
			{
				// send back
				Push(sendInfo.m_ser, reliableInfo.m_peer);
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
	stream_ser << static_cast<netU16>(0);
	stream_ser << static_cast<netU16>(0);
	stream_ser.Close();

	ser.SetCursor(ser.GetCursor() + stream_ser.GetSize());
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
	SerializerLess listener_ser(ser, m_stream->GetHeaderSize(), ser.GetBufferSize());
	size_t listener_size = listener_ser.GetSize();

	listener_ser.Write(GetType());
	listener_ser.SetCursor(listener_size);

	listener_ser << _ser;
	listener_ser.Close();

	return true;
}

netBool ReliableListener::PreUnpack(SerializerLess& _ser, const Peer& _peer)
{
	netBool is_valid = true;
	if(_ser.Read(GetType()))
	{
		PlayerReliableInfo_t& reliableInfo = GetReliableInfo(_peer);

		// unpack header first
		netU16 sequence;
		netU16 ack;

		static_cast<Serializer&>(_ser) >> sequence;
		static_cast<Serializer&>(_ser) >> ack;

		RecvAck(reliableInfo, ack);
		RecvSequence(reliableInfo, _ser, sequence);
	}
	else
	{
		is_valid = false;
	}

	return is_valid;
}

netBool ReliableListener::UnPack(SerializerLess& _ser, const Peer& _peer)
{
	netBool is_valid = true;
	if(_ser.Read(GetType()))
	{
		Channel& channel = GetChannel(m_recvChannels, _peer);

		// unpack header first
		netU16 sequence;
		netU16 ack;

		static_cast<Serializer&>(_ser) >> sequence;
		static_cast<Serializer&>(_ser) >> ack;

		while(_ser.GetCursor() != _ser.GetSize())
		{
			SerializerLess body;
			_ser >> body;

			channel.Push(body);
		}

		_ser.Close();
	}
	else
	{
		is_valid = false;
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

netBool ReliableListener::CompareSequence(PlayerReliableInfo_t &_reliableInfo, netU16 _sequence)
{
	netBool is_ordered = false;

	// warning in case of cycling number !
	if(_reliableInfo.m_lastAck == 0xFFFF && _sequence == 1)
	{
		is_ordered = true;
		_reliableInfo.IncAck();
	}
	else if(_reliableInfo.m_lastAck+1 == _sequence)
	{
		is_ordered = true;
		_reliableInfo.IncAck();
	}

	return is_ordered;
}

netBool ReliableListener::RecvSequence(PlayerReliableInfo_t &_reliableInfo, SerializerLess& _ser, netU16 _sequence)
{
	netBool is_ordered = CompareSequence(_reliableInfo, _sequence);
	
	if(is_ordered)
	{
		// unpack
		_ser.Close();
		UnPack(_ser, _reliableInfo.m_peer);

		// check if we fill a hole
		_reliableInfo.m_recv.sort();
		std::list<ReliableRecvInfo>::iterator iter = _reliableInfo.m_recv.begin();
		while(iter != _reliableInfo.m_recv.end())
		{
			struct ReliableRecvInfo& cmp = (*iter);
			if(CompareSequence(_reliableInfo, cmp.m_sequence))
			{
				UnPack(cmp.m_ser, _reliableInfo.m_peer);
				iter = _reliableInfo.m_recv.erase(iter);
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		// special case, if pure ack, do not wait an additional case for it
		if(_ser.GetCursor() != _ser.GetSize())
		{
			_ser.Close();
			_reliableInfo.m_recv.push_front(struct ReliableRecvInfo(_sequence, _ser));
		}
	}

	return is_ordered;
}

void ReliableListener::RecvAck(PlayerReliableInfo_t &_reliableInfo, netU16 _ack)
{
	if(_ack != 0)
	{
		std::list<ReliableSendInfo>::iterator it = _reliableInfo.m_send.begin();
		while(it != _reliableInfo.m_send.end())
		{
			struct ReliableSendInfo& sendInfo = (*it);
			if(sendInfo.m_sequence <= _ack)
			{
				// remove
				it = _reliableInfo.m_send.erase(it);
			}
			else
			{
				++it;
			}
		}
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

void ReliableListener::SetTimeOut(netU32 _ms)
{
	m_maxClock = (_ms/1000) * CLOCKS_PER_SEC;
}




};