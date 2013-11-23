#pragma once

#include "netduke.h"
#include "pingservice.h"

#include "server.h"

#include <ctime>

namespace NetDukeSample
{

class Observer : public NetDuke::IObserver
{
public:
	void OnUnregisteredMessage(NetDuke::SerializerLess& _ser, NetDuke::Peer& _peer)
	{
		printf("unknown packet !\n");
	}

	void OnPeerRemoved(const NetDuke::Peer& _peer)
	{
		printf("peer removed\n");
	}

};

class PingPongServer : public NetDuke::PingService
{

public:
	explicit PingPongServer(NetDuke::netU16 _port);
	~PingPongServer();

	void Init();
	void DeInit();

	void Tick();

	NetDuke::NetDuke&	GetNetDuke() { return *m_netduke; }

	virtual NetDuke::netBool	OnRecvPing(NetDuke::Peer& _peer);

private:
	Observer	m_observer;

};



};