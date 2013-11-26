#include "eventservice.h"

#include "transport.h"
#include "netduke.h"

#include "event.h"

namespace NetDuke
{




EventService::EventService(NetDuke* _netduke)
	: Service(_netduke)
{

}

EventService::~EventService()
{

}

void EventService::Init()
{

}

void EventService::DeInit()
{

}

void EventService::Tick()
{

}

void EventService::SendReliable(Event& _event)
{
	for(peers_t::iterator it = m_peers.begin(); it != m_peers.end(); ++it)
	{
		Peer& peer = (*it);
		SendReliable(_event, peer);
	}
}

void EventService::SendReliable(Event& _event, const Peer& _peer)
{
	Transport& transport = m_netduke->GetTransport();
	_event.Serialize(_event.In());

	if(!transport.IsTCPEnabled())
	{
		transport.Send(_event.GetSerializer(), _peer, s_typeReliableListener);
	}
	else
	{
		transport.Send(_event.GetSerializer(), _peer, s_typeUnreliableListener);
	}
}

void EventService::SendReliable(Event& _event, peers_t& _peerList)
{
	for(peers_t::iterator it = _peerList.begin(); it != _peerList.end(); ++it)
	{
		Peer& peer = (*it);
		SendReliable(_event, peer);
	}
}

void EventService::SendUnReliable(Event& _event)
{
	for(peers_t::iterator it = m_peers.begin(); it != m_peers.end(); ++it)
	{
		Peer& peer = (*it);
		SendUnReliable(_event, peer);
	}
}

void EventService::SendUnReliable(Event& _event, const Peer& _peer)
{
	Transport& transport = m_netduke->GetTransport();
	_event.Serialize(_event.In());

	transport.Send(_event.GetSerializer(), _peer, s_typeUnreliableListener);
}

void EventService::SendUnReliable(Event& _event, peers_t& _peerList)
{
	for(peers_t::iterator it = _peerList.begin(); it != _peerList.end(); ++it)
	{
		Peer& peer = (*it);
		SendUnReliable(_event, peer);
	}
}

// observer
void EventService::OnPeerAdded(const Peer& _peer)
{
	m_peers.push_back(_peer);
}

void EventService::OnPeerRemoved(const Peer& _peer)
{
	for(peers_t::iterator it = m_peers.begin(); it != m_peers.end(); ++it)
	{
		Peer& peer = (*it);

		if(peer == _peer)
		{
			m_peers.erase(it);
			break;
		}
	}

}

};