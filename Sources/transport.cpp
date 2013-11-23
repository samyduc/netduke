#include "transport.h"

#include "stream.h"
#include "listener.h"
#include "unreliablelistener.h"
#include "reliablelistener.h"
#include "udpstream.h"
#include "tcpstream.h"
#include "serializer.h"
#include "serializerless.h"
#include "peer.h"
#include "channel.h"

#include <mutex>

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

	CheckTimeOutClients();
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

void Transport::InitTCPStack(const Peer &_peer)
{
	assert(s_IsPlatformInit);

	m_tcpMode = true;
	// create stream
	TCPStream *tcpstream = new TCPStream(_peer);

	if(_peer.GetPort() != 0)
	{
		tcpstream->CreateAndBind();
	}
	else
	{
		tcpstream->Init();
	}
	//
	m_streams.push_back(tcpstream);

	// create listener - note: we do not need reliable listener on tcp
	UnreliableListener *unreliablelistener = new UnreliableListener();
	m_listeners.push_back(unreliablelistener);
	
	// attach
	tcpstream->AttachListener(*unreliablelistener);
}

void Transport::InitUDPStack(const Peer &_peer)
{
	assert(s_IsPlatformInit);

	m_tcpMode = false;
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
		// we use udp by default
		InitUDPStack(_peer);
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
	else
	{
		// unavailable transport
		assert(false);
	}

	// update stats
	struct Activity* activity = GetActivity(_peer);
	activity->m_lastSend = Time::GetMsTime();
	activity->m_nbSend++;
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

			// update stats
			struct Activity* activity = GetActivity(_peer);
			activity->m_lastRecv = Time::GetMsTime();
			activity->m_nbRecv++;

			break;
		}
	}

	return isFound;
}

void Transport::RegisterObserver(IObserver* _observer)
{
	Layer::RegisterObserver(_observer);

	for(listeners_t::iterator it=m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener* listener = (*it);
		listener->RegisterObserver(_observer);
	}

	for(streams_t::iterator it=m_streams.begin(); it != m_streams.end(); ++it)
	{
		Stream* stream = (*it);
		stream->RegisterObserver(_observer);
	}
}

void Transport::CheckTimeOutClients()
{
	activities_t::iterator it = m_activities.begin();
	timer_t now = Time::GetMsTime();

	while(it != m_activities.end())
	{
		struct Activity* activity = it->second;

		if((now - activity->m_lastRecv ) > m_timeoutPeer && (activity->m_creation + m_timeoutPeer) < now)
		{
			DeletePeer(activity->m_peer);

			if(m_observer)
			{
				m_observer->OnPeerRemoved(activity->m_peer);
			}

			delete activity;
			it = m_activities.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void Transport::DeletePeer(const Peer& _peer)
{
	for(listeners_t::iterator it=m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener* listener = (*it);
		listener->DeletePeer(_peer);
	}

	for(streams_t::iterator it=m_streams.begin(); it != m_streams.end(); ++it)
	{
		Stream* stream = (*it);
		stream->DeletePeer(_peer);
	}
}

void Transport::InitPlatform()
{
	if(!s_IsPlatformInit)
	{
		//FD_ZERO(&m_readfs);
		s_IsPlatformInit = InitPlatformPrivate();
	}
}

struct Activity* Transport::GetActivity(const Peer&_peer)
{
	struct Activity* activity = nullptr;
	activities_t::iterator it = m_activities.find(_peer);

	if(it != m_activities.end())
	{
		activity = it->second;
	}
	else
	{
		activity = new struct Activity(_peer);
		m_activities[_peer] = activity;
	}

	assert(activity);
	return activity;
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

		for(activities_t::iterator it=m_activities.begin(); it != m_activities.end(); ++it)
		{
			delete it->second;
		}
		m_activities.clear();
		

		//FD_ZERO(&m_readfs);
		s_IsPlatformInit = DesInitPlatformPrivate();
	}
}


};
