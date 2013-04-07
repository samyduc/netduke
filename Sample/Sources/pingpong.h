#pragma once

#include "netduke.h"

#include "pingservice.h"

#include <ctime>

namespace NetDukeSample
{

class PingPongServer : public NetDuke::PingService
{

public:
	explicit PingPongServer(NetDuke::netU16 _port);
	~PingPongServer();

	void Tick();
	void OnRecvPing(NetDuke::netU32 _type, NetDuke::netU64 _timestamp, NetDuke::Peer _peer);

private:
	NetDuke::NetDuke m_netduke;


};

class PingPongClient : public NetDuke::PingService
{
public:
	explicit PingPongClient(NetDuke::netChar* _addr, NetDuke::netU16 _port);
	~PingPongClient();

	void Tick();
	void OnRecvPing(NetDuke::netU32 _type, NetDuke::netU64 _timestamp, NetDuke::Peer _peer);

private:
	NetDuke::NetDuke m_netduke;
	NetDuke::Peer m_peer;

};



};