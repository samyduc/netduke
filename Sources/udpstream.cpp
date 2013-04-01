#include "udpstream.h"

#include "layer.h"
#include "listener.h"
#include "crc.h"

namespace NetDuke
{


UDPStream::UDPStream(const Peer& _peer)
	: m_peer(_peer)
	, m_isValid(false)
	, m_socket(0)
	, m_opt_compression(false)
	, m_opt_encryption(false)
	, m_pool(50)
{

}

void UDPStream::Tick()
{
	FlushSend();
	FlushRecv();
}

void UDPStream::Flush()
{
	for(channels_t::iterator it=m_sendChannels.begin(); it != m_sendChannels.end(); ++it)
	{
		delete (*it).second;
	}
	m_sendChannels.clear();

	for(listeners_t::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener *listener = (*it);
		listener->UnRegisterStream(*this);
	}
	m_listeners.clear();
}

void UDPStream::FlushSend()
{
	// TODO : congestion control here
	for(channels_t::iterator it=m_sendChannels.begin(); it != m_sendChannels.end(); ++it)
	{
		Channel& channel = *(*it).second;
		const Peer &peer = channel.GetPeer();

		if(!channel.IsEmpty())
		{
			SerializerLess ser;
			channel.Pop(ser);
			SendTo(ser, peer);
		}
	}
}

void UDPStream::FlushRecv()
{
	struct timeval time_val;
	time_val.tv_sec = 0;
	time_val.tv_usec = 0;

	fd_set rset;
	FD_ZERO(&rset);
	FD_SET(m_socket, &rset);

	if(select(static_cast<int>(m_socket + 1), &rset, NULL, NULL, &time_val) >= 0)
	{
		if(FD_ISSET(m_socket, &rset) > 0)
		{
			RecvFrom();
		}
	}	
}

void UDPStream::SendTo(Serializer& _ser, const Peer& _peer)
{
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

void UDPStream::RecvFrom()
{
	Peer peer;
	socklen_t fromlen = sizeof(peer.GetNativeStruct());

	Serializer& ser = m_pool.GetSerializer();

	int len = recvfrom(
					m_socket,
					reinterpret_cast<char*>(ser.GetRawBuffer()),
					static_cast<int>(ser.GetBufferSize()),
					0,
					(SOCKADDR *)&peer.GetNativeStruct(),
					&fromlen
				);

	if(len > 0)
	{
		SerializerLess serLess(ser);
		ser.SetCursor(len);
		
		UnPack(serLess, peer);
	}
}

void UDPStream::CreateAndBind()
{
	assert(IsValid() == false);

	const SOCKADDR_IN &native_addr = m_peer.GetNativeStruct();
	m_socket = socket(native_addr.sin_family, SOCK_DGRAM, 0);

	assert(m_socket > 0);
	// no blocking socket
	//u_long iMode=1;
	//ioctlsocket(m_socket, FIONBIO, &iMode);

	int err = bind(m_socket, (struct sockaddr*)&native_addr, sizeof(native_addr));

	assert(err == 0);

	// overwrite port (if automatic attribution)
	socklen_t native_size = sizeof(native_addr);
	err = getsockname(m_socket, (struct sockaddr *)&native_addr, &native_size);

	m_isValid = (m_socket != 0);
}

size_t UDPStream::GetHeaderSize() const
{
	// serializer + stream
	return Serializer::GetHeaderSize() + sizeof(netU32);
}

netBool UDPStream::Pull(SerializerLess &_ser, Peer& _peer)
{
	(void)_ser;
	(void)_peer;
	return false;
}

netBool UDPStream::Push(SerializerLess &_ser, const Peer& _peer)
{
	assert(m_isValid);

	Pack(_ser, _peer);

	return true;
}

netBool UDPStream::Pack(SerializerLess& _ser, const Peer& _peer)
{
	Channel& channel = GetChannel(m_sendChannels, _peer);

	_ser.ResetCursor();

	// read upper layer size, compute crc
	//_ser.Read(s_typeListener);
	SerializerLess upper_ser(_ser, GetHeaderSize(), _ser.GetBufferSize());
	netU32 crc = CRC32::Compute(_ser.GetBuffer() + GetHeaderSize(), upper_ser.GetSize() - GetHeaderSize());

	_ser.Write(s_typeUDPStream);
	_ser << crc;
	_ser.SetCursor(GetHeaderSize() + upper_ser.GetSize());
	_ser.Close();

	if(m_opt_compression)
	{
		// TODO : add compression
	}

	if(m_opt_encryption)
	{
		// TODO : add encryption
	}

	channel.Push(_ser);

	return true;
}

netBool UDPStream::UnPack(SerializerLess& _ser, const Peer& _peer)
{
	if(_ser.Read(s_typeUDPStream))
	{
		// check frame consistency (even if the type is correct)
		if(_ser.GetSize() <= _ser.GetBufferSize())
		{
			// Packet seems well formed, check data crc for consitency now
			netU32 crc_ser;
			static_cast<Serializer&>(_ser) >> crc_ser;

			SerializerLess upper_ser(_ser, GetHeaderSize(), _ser.GetBufferSize());
			netU32 crc_cmp = CRC32::Compute(_ser.GetBuffer() + GetHeaderSize(), upper_ser.GetSize() - GetHeaderSize());

			if(crc_cmp == crc_ser)
			{
				PushToStream(upper_ser, _peer);
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

	return true;
}

void UDPStream::PushToStream(SerializerLess& _ser, const Peer& _peer)
{
	for(listeners_t::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener* listener = (*it);
		if(listener->PullFromStream(_ser, _peer))
		{
			break;
		}
	}
}

Channel& UDPStream::GetChannel(channels_t& channels, const Peer& _peer)
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

netBool UDPStream::AttachListener(Listener& _listener)
{
	netBool is_valid = true;
	// avoid double type
	for(listeners_t::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener *listener = (*it);
		if(listener->GetType() == _listener.GetType())
		{
			is_valid = false;
			break;
		}
	}

	if(is_valid)
	{
		m_listeners.push_back(&_listener);
		_listener.RegisterStream(*this);
	}
	
	return is_valid;
}

netBool UDPStream::DetachListener(Listener& _listener)
{
	netBool is_valid = false;

	for(listeners_t::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener *listener = (*it);
		if(listener->GetType() == _listener.GetType())
		{
			is_valid = true;
			m_listeners.erase(it);
			listener->UnRegisterStream(*this);
			break;
		}
	}

	return is_valid;
}



};
