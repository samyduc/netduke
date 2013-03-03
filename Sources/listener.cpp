#include "listener.h"
#include "channel.h"
#include "serializer.h"
#include "crc.h"


#include <assert.h>

namespace NetDuke
{

Listener::Listener(const Peer &_peer)
	: m_peer(_peer)
	, m_isBind(false)
	, m_pool(50)
	, m_socket(0)
{

	CreateAndBind();
}

Listener::~Listener()
{
	Flush();
}

void Listener::CreateAndBind()
{
	const SOCKADDR_IN &native_addr = m_peer.GetNativeStruct();
	m_socket = socket(native_addr.sin_family, SOCK_DGRAM, 0);

	int err = bind(m_socket, (struct sockaddr*)&native_addr, sizeof(native_addr));

	assert(err == 0);
	// no blocking socket
	/*u_long iMode=1;
	ioctlsocket(m_socket, FIONBIO, &iMode);*/
	
	//FD_SET(m_socket, readfs);

	m_isBind = (m_socket != 0);
}

void Listener::Push(const Serializer& _ser, const Peer& _peer)
{
	Channel& channel = GetSendChannel(_peer);
	//Serializer& ref = m_pool.GetSerializer();

	// copy
	//ref = _ser;
	//channel.PushQueue(ref);

	SerializerLess *back = channel.Back();

	if(channel.IsEmpty() || back->GetSize() + _ser.GetSize() > back->GetBufferSize())
	{
		Serializer& ser = m_pool.GetSerializer();
		// prepare serializer
		ser.Write(s_typeListener);

		SerializerLess crc_ser(ser, Serializer::GetHeaderSize() + sizeof(netU32));
		ser.SetCursor(ser.GetCursor() + crc_ser.GetBufferSize());

		ser.Close();
		channel.Push(ser);
	}

	Pack(_ser, channel);
}

void Listener::SendTo(Serializer& _ser, const Peer& _peer)
{
	// compute crc
	_ser.Read(s_typeListener);
	SerializerLess crc_ser(_ser, Serializer::GetHeaderSize() + sizeof(netU32));

	netU32 crc = CRC32::Compute(_ser.GetBuffer() + Serializer::GetHeaderSize() + Serializer::GetHeaderSize() + sizeof(netU32), _ser.GetSize() - Serializer::GetHeaderSize() - sizeof(netU32) - Serializer::GetHeaderSize());
	
	crc_ser.Write(s_typeCrc);
	crc_ser << crc;
	crc_ser.Close();

	_ser.SetCursor(_ser.GetSize());
	_ser.Close();

	// send
	int len = sendto(
						m_socket,
						reinterpret_cast<const char*>(_ser.GetBuffer()),
						static_cast<int>(_ser.GetSize()),
						0,
						(SOCKADDR *)&_peer.GetNativeStruct(),
						sizeof(_peer.GetNativeStruct()));

	// TODO : handle this case
	assert(len == _ser.GetSize());
}

void Listener::RecvFrom()
{
	Peer peer;
	socklen_t fromlen = sizeof(peer.GetNativeStruct());

	Serializer& ser = m_pool.GetSerializer();

	int len = recvfrom(
					m_socket,
					reinterpret_cast<char*>(ser.m_buffer),
					static_cast<int>(ser.GetBufferSize()),
					0,
					(SOCKADDR *)&peer.GetNativeStruct(),
					&fromlen
				);

	if(len > 0)
	{
		ser.SetCursor(len);
		UnPack(ser, peer);
	}
}

void Listener::UnPack(Serializer& _ser, const Peer& _peer)
{
	if(_ser.Read(s_typeListener))
	{
		// check frame consistency (even if the type is correct)
		if(_ser.GetSize() <= _ser.GetBufferSize())
		{
			// Packet seems well formed, check data crc for consitency now
			SerializerLess crc_ser;
			_ser >> crc_ser;

			if(crc_ser.Read(s_typeCrc))
			{
				netU32 src_crc;
				static_cast<Serializer&>(crc_ser) >> src_crc;

				// compute crc
				netU32 cmp_crc = CRC32::Compute(_ser.GetBuffer() + Serializer::GetHeaderSize() + crc_ser.GetSize(), _ser.GetSize() - Serializer::GetHeaderSize() - crc_ser.GetSize());

				if(cmp_crc == src_crc)
				{
					SerializerLess unpack_ser(_ser, crc_ser.GetSize(), _ser.GetSize());
					unpack_ser.Write(s_typeTransport);
					unpack_ser.SetCursor(unpack_ser.GetBufferSize());
					unpack_ser.Close();

					Channel& channel = GetRecvChannel(_peer);
					channel.Push(unpack_ser);
				}
				else
				{
					assert(false);
				}
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			assert(false);
		}
		_ser.Close();
	}
}

void Listener::FlushSend()
{
	for(channels_t::iterator it=m_sendChannels.begin(); it != m_sendChannels.end(); ++it)
	{
		Channel& channel = *(*it).second;
		const Peer &peer = channel.GetPeer();

		if(!channel.IsEmpty())
		{
			//Serializer& ser = m_pool.GetSerializer();
			//Pack(ser, channel);
			SerializerLess ser;
			channel.Pop(ser);
			SendTo(ser, peer);
		}
	}
}

void Listener::Pack(const Serializer& _ser, Channel& _channel)
{
	assert(!_channel.IsEmpty());
	SerializerLess& ser = *_channel.Front();
	size_t size = ser.GetSize();

	ser.Write(s_typeListener);
	ser.SetCursor(size);

	ser << _ser;
	ser.Close();

}

bool Listener::Pull(SerializerLess &_ser, Peer& _peer)
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

void Listener::FlushRecv()
{
	RecvFrom();
}

Channel& Listener::GetSendChannel(const Peer& _peer)
{
	return GetChannel(m_sendChannels, _peer);
}

Channel& Listener::GetRecvChannel(const Peer& _peer)
{
	return GetChannel(m_recvChannels, _peer);
}

Channel& Listener::GetChannel(channels_t& channels, const Peer& _peer)
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

void Listener::Flush()
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
}

};
