#pragma once

#include "netdef.h"
#include "crc.h"

#include "layer.h"
#include "listener.h"
#include "timeplatform.h"

#include <list>


namespace NetDuke
{

class Channel;
class Serializer;
class SerializerLess;
class Peer;
class IObserver;

// this is top level statistic not taking into account retry and dead peers
struct Activity
{
	struct Activity(const Peer& _peer)
	{
		m_creation = Time::GetMsTime();
		m_lastSend = 0;
		m_lastRecv = 0;
		m_nbSend = 0;
		m_nbRecv = 0;
		m_peer = _peer;
	}

	timer_t	m_creation;
	timer_t m_lastSend;
	timer_t m_lastRecv;
	size_t	m_nbSend;
	size_t	m_nbRecv;
	Peer	m_peer;
};

class Transport : public Layer
{
public:
						Transport() : m_timeoutPeer(30000) {}
	virtual				~Transport() {}

	void				InitPlatform();
	void				DesInitPlatform();

	void				Tick();
	netU32				GetType() const { return s_typeTransport; }

	inline size_t		GetHeaderSize() const { return 0; }

	// helper
	void				Listen(const Peer &_peer);
	Listener*			GetListener(netU32 _type) const;
	void				Send(Serializer& _ser, const Peer& _peer, netU32 _type);

	netBool				Push(SerializerLess &_ser, const Peer& _peer);
	netBool				Pull(SerializerLess &_ser, Peer& _peer);

	// helper
	void				InitTCPStack(const Peer &_peer);
	void				InitUDPStack(const Peer &_peer);

	netBool				IsTCPEnabled() { return m_tcpMode; }

	void				RegisterObserver(IObserver* _observer);

	void				DeletePeer(const Peer& _peer);
	void				SetTimeoutPeer(timer_t _timer) { m_timeoutPeer = _timer; }

protected:
	netBool				Pack(SerializerLess& _ser, const Peer& _peer);
	netBool				UnPack(SerializerLess& _ser, const Peer& _peer);

	void				CheckTimeOutClients();

	struct Activity*	GetActivity(const Peer&_peer);
	
private:
	timer_t				m_timeoutPeer;
	netBool				m_tcpMode;
	listeners_t			m_listeners;
	streams_t			m_streams;

	typedef std::map<Peer, struct Activity*> activities_t;
	activities_t		m_activities;

private:
	

	netBool				InitPlatformPrivate();
	netBool				DesInitPlatformPrivate();

};

};
