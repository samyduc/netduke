#pragma once

#include "netduke.h"

#include "pingservice.h"

#include <ctime>

namespace NetDukeSample
{



class SuperRPC : public NetDuke::RPC
{
public:

	NetDuke::netU32		GetType() const { return 12; }

	NetDuke::Dataset&	In() { return m_in; }
	NetDuke::Dataset&	Out() { return m_out; }

	NetDuke::PingRPCIn	m_in;
	NetDuke::PingRPCOut	m_out;

};

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
	Observer	m_observer;

	NetDuke::Peer	m_peer;
	SuperRPC		m_superprc;
};



};