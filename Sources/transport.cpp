#include "transport.h"

#include "stream.h"
#include "listener.h"
#include "unreliablelistener.h"
#include "reliablelistener.h"
#include "udpstream.h"
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
	for(streams_t::iterator it=m_streams.begin(); it != m_streams.end(); ++it)
	{
		Stream *stream = (*it);
		stream->Tick();
	}

	for(listeners_t::iterator it=m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener *listener = (*it);
		listener->Tick();
	}
}

Listener* Transport::GetListener(netU32 _type) const
{
	Listener* listener = nullptr;

	for(listeners_t::const_iterator it=m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener *localListener = (*it);
		if(localListener->GetType() == _type)
		{
			listener = localListener;
			break;
		}
	}

	return listener;
}

netBool Transport::Pack(SerializerLess& _ser, const Peer& _peer)
{
	(void)_ser;
	(void)_peer;
	return false;
}

netBool Transport::UnPack(SerializerLess& _ser, const Peer& _peer)
{
	(void)_ser;
	(void)_peer;
	return false;
}

void Transport::Listen(const Peer &_peer)
{
	assert(s_IsPlatformInit);
	
	// look for existing stream
	netBool is_new = true;
	for(streams_t::iterator it=m_streams.begin(); it != m_streams.end(); ++it)
	{
		Stream* stream = (*it);
		if(stream->GetPeer() == _peer)
		{
			is_new = false;
			break;
		}
	}

	if(is_new)
	{
		// create stream
		UDPStream *udpstream = new UDPStream(_peer);
		udpstream->CreateAndBind();
		m_streams.push_back(udpstream);

		// create listener
		UnreliableListener *unreliablelistener = new UnreliableListener();
		m_listeners.push_back(unreliablelistener);

		ReliableListener *reliablelistener = new ReliableListener();
		m_listeners.push_back(reliablelistener);
	
		// attach
		udpstream->AttachListener(*unreliablelistener);
		udpstream->AttachListener(*reliablelistener);
	}
}

void Transport::Send(Serializer& _ser, const Peer& _peer, netU32 _type)
{
	Listener *listener = GetListener(_type);

	if(listener)
	{
		_ser.ResetCursor();
		SerializerLess serless(_ser);
		listener->Push(serless, _peer);
	}
}

netBool Transport::Push(SerializerLess &_ser, const Peer& _peer)
{
	(void)_ser;
	(void)_peer;
	return false;
}

netBool Transport::Pull(SerializerLess &_ser, Peer& _peer)
{
	netBool isFound = false;

	for(listeners_t::iterator it=m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener *listener = (*it);
		if(listener->Pull(_ser, _peer))
		{
			isFound = true;
			break;
		}
	}

	return isFound;
}

void Transport::InitPlatform()
{
	if(!s_IsPlatformInit)
	{
		//FD_ZERO(&m_readfs);
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

		for(streams_t::iterator it=m_streams.begin(); it != m_streams.end(); ++it)
		{
			delete (*it);
		}
		m_listeners.clear();
		

		//FD_ZERO(&m_readfs);
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