#include "unreliablelistener.h"


#include "stream.h"

#include "channel.h"
#include "serializerless.h"

#include <assert.h>


namespace NetDuke
{


UnreliableListener::UnreliableListener()
	: m_pool(50)
	, m_stream(nullptr)
{

}

UnreliableListener::~UnreliableListener()
{
	Flush();
}

size_t UnreliableListener::GetHeaderSize() const
{
	return m_stream->GetHeaderSize() + Serializer::GetHeaderSize();
}

void UnreliableListener::Flush()
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

void UnreliableListener::Tick()
{
	FlushSend();
}

void UnreliableListener::FlushSend()
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
			m_stream->Push(ser, peer);
		}
	}
}

void UnreliableListener::RegisterStream(Stream& _stream)
{
	assert(m_stream == nullptr);

	m_stream = &_stream;
}

void UnreliableListener::UnRegisterStream(const Stream& _stream)
{
	assert(m_stream != nullptr);
	assert(m_stream->GetType() == _stream.GetType());

	m_stream = nullptr;
}

netBool UnreliableListener::Pull(SerializerLess &_ser, Peer& _peer)
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

netBool UnreliableListener::Push(SerializerLess &_ser, const Peer& _peer)
{
	assert(m_stream != nullptr);

	Channel& channel = GetChannel(m_sendChannels, _peer);

	SerializerLess *back = channel.Back();

	// bundling
	if(channel.IsEmpty() || back->GetSize() + _ser.GetSize() > back->GetBufferSize())
	{
		SerializerLess ser(m_pool.GetSerializer());
		// prepare serializer
		ser.Write(m_stream->GetType());

		SerializerLess stream_ser(ser, m_stream->GetHeaderSize(), ser.GetBufferSize());
		stream_ser.Write(GetType());
		stream_ser.Close();

		ser.SetCursor(ser.GetCursor() + stream_ser.GetSize());
		ser.Close();
		channel.Push(ser);
	}

	return Pack(_ser, _peer);
}


netBool UnreliableListener::Pack(SerializerLess& _ser, const Peer& _peer)
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

netBool UnreliableListener::UnPack(SerializerLess& _ser, const Peer& _peer)
{
	netBool is_valid = true;
	if(_ser.Read(GetType()))
	{
		Channel& channel = GetChannel(m_recvChannels, _peer);

		// no header in udp -> unbundling
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

netBool UnreliableListener::PullFromStream(SerializerLess& _ser, const Peer& _peer)
{
	// read serializer type
	netBool is_valid = _ser.Read(GetType());

	if(is_valid)
	{
		_ser.Close();
		netBool ret = UnPack(_ser, _peer);
		assert(ret);
	}

	return is_valid;
}

Channel& UnreliableListener::GetChannel(channels_t& channels, const Peer& _peer)
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

void UnreliableListener::DeletePeer(const Peer& _peer)
{
	channels_t::iterator it_channel = m_sendChannels.find(_peer);
	if(it_channel != m_sendChannels.end())
	{
		Channel *channel = (*it_channel).second;
		delete channel;
		m_sendChannels.erase(it_channel);
	}

	channels_t::iterator it_channel_rcv = m_recvChannels.find(_peer);
	if(it_channel_rcv != m_recvChannels.end())
	{
		Channel *channel = (*it_channel_rcv).second;
		delete channel;
		m_recvChannels.erase(it_channel_rcv);
	}
}





};