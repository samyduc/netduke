#include "transport.h"
#include "serializer.h"
#include "serializerLess.h"
#include "peer.h"
#include "channel.h"


namespace NetDuke
{

volatile static bool s_IsPlatformInit = false;

static std::mutex s_lock;

void Transport::Tick()
{
	for(listeners_t::iterator it=m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener *listener = (*it);

		listener->FlushSend();
	}

	// can be multi threaded
	Recv();
}

Listener* Transport::GetListener(const Peer &_peer) const
{
	Listener* listener = nullptr;

	for(listeners_t::const_iterator it=m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		if(_peer == (*it)->GetPeer())
		{
			listener = (*it);
			break;
		}
	}

	return listener;
}

const Transport::listeners_t& Transport::GetListeners() const
{
	return m_listeners;
}

Listener* Transport::Listen(const Peer &_peer)
{
	assert(s_IsPlatformInit);
	Listener *listener = nullptr;

	if(GetListener(_peer) == nullptr)
	{

		listener = new Listener(_peer);
		if(listener->IsBind())
		{
			m_listeners.push_back(listener);
		}
		else
		{
			// can fail if multiple instance of the stack are instaciated
			delete listener;
			listener = nullptr;
		}
	}

	return listener;
}

void Transport::Send(const Serializer& _ser, const Peer& _peer)
{
	assert(m_listeners.size() != 0);
	Listener& listener = *m_listeners.front();
	listener.Push(_ser, _peer);
}

void Transport::Send(const Serializer& _ser, const Peer& _peer, Listener& _listener)
{
	_listener.Push(_ser, _peer);
}

void Transport::Recv()
{
	struct timeval time_val;
	time_val.tv_sec = 0;
	time_val.tv_usec = 0;

	// TODO : can be optimized !!!
	for(listeners_t::iterator it=m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener *listener = (*it);
		SOCKET socket = listener->GetSocket();
		
		FD_ZERO(&m_readfs);
		FD_SET(socket, &m_readfs);
		if(select(static_cast<int>(socket + 1), &m_readfs, NULL, NULL, &time_val) >= 0)
		{
			if(FD_ISSET(socket, &m_readfs) > 0)
			{
				listener->FlushRecv();

				// pull the received packet
				Peer peer;
				SerializerLess ser;

				if(listener->Pull(ser, peer))
				{
					Unpack(ser, peer);
					//delete ser;
				}
			}
		}
	}
}

void Transport::Unpack(SerializerLess& _ser, const Peer& _peer)
{
	Channel& channel = GetChannel(_peer);

	// TODO : check reliable
	bool result = _ser.Read(s_typeTransport);
	assert(result);

	// there is only serializer here
	while(_ser.GetCursor() != _ser.GetSize())
	{
		SerializerLess payload;
		_ser >> payload;

		// replace by pool
		channel.Push(payload);
	}

	_ser.Close();
}

bool Transport::Pull(SerializerLess &_ser, Peer& _peer)
{
	bool isFound = false;

	for(channels_t::iterator it = m_recvChannels.begin(); it != m_recvChannels.end(); ++it)
	{
		Channel& channel = *(*it).second;
		if(!channel.IsEmpty())
		{
			SerializerLess ser;
			channel.Pop(ser);
			ser.ResetCursor();
			_ser = ser;
			_peer = (*it).first;
			isFound = true;
			break;
		}
	}

	return isFound;
}

Channel& Transport::GetChannel(const Peer& _peer)
{
	channels_t::iterator it = m_recvChannels.find(_peer);
	Channel *channel_out = nullptr;

	if(it != m_recvChannels.end())
	{
		channel_out = (*it).second;
	}
	else
	{
		channel_out = new Channel(_peer);
		m_recvChannels[_peer] = channel_out;
	}

	return *channel_out;
}

void Transport::InitPlatform()
{
	if(!s_IsPlatformInit)
	{
		FD_ZERO(&m_readfs);
		s_IsPlatformInit = InitPlatformPrivate();
	}
}

void Transport::DesInitPlatform()
{
	if(s_IsPlatformInit)
	{
		// remember to kill thread first
		for(listeners_t::iterator it=m_listeners.begin(); it != m_listeners.end(); ++it)
		{
			delete (*it);
		}
		m_listeners.clear();

		FD_ZERO(&m_readfs);
		s_IsPlatformInit = DesInitPlatformPrivate();
	}
}

// platform specific
#if defined(_WIN32)
bool Transport::InitPlatformPrivate()
{
	WSADATA wsa;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa);

	return result == 0;
}

bool Transport::DesInitPlatformPrivate()
{
	WSACleanup();
	return true;
}
#endif

};