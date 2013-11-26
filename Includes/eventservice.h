#pragma once

#include "netdef.h"

#include "service.h"
#include "crc.h"
#include "timeplatform.h"

#include <list>


namespace NetDuke
{

class NetDuke;
class Peer;
class Event;

static const netU32 s_EventService = CRC32::Compute("EventService");

typedef std::list<Peer> peers_t;

class EventService : public Service
{
public:

	explicit	EventService(NetDuke* _netduke);
				~EventService();

	void		Init();
	void		DeInit();

	netU32		GetType() const { return s_EventService; }
	void		Tick();

	void		SendReliable(Event& _event);
	void		SendReliable(Event& _event, const Peer& _peer);
	void		SendReliable(Event& _event, peers_t& _peerList);

	void		SendUnReliable(Event& _event);
	void		SendUnReliable(Event& _event, const Peer& _peer);
	void		SendUnReliable(Event& _event, peers_t& _peerList);

	void		OnPeerAdded(const Peer& _peer);
	void		OnPeerRemoved(const Peer& _peer);


private:
	peers_t		m_peers;

};





}