#pragma once

#include "netduke.h"

#include "pingservice.h"

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
	Observer	m_observer;

};

class PingPongClient : public NetDuke::PingService
{
public:
	explicit PingPongClient(const NetDuke::netChar* _addr, NetDuke::netU16 _port);
	~PingPongClient();

	void Init();
	void DeInit();

	void Tick();

	NetDuke::NetDuke&	GetNetDuke() { return *m_netduke; }

private:
	NetDuke::Peer m_peer;
};



};