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

	void Init();
	void DeInit();

	void Tick();

	NetDuke::NetDuke&	GetNetDuke() { return *m_netduke; }


};

class PingPongClient : public NetDuke::PingService
{
public:
	explicit PingPongClient(NetDuke::netChar* _addr, NetDuke::netU16 _port);
	~PingPongClient();

	void Init();
	void DeInit();

	void Tick();

	NetDuke::NetDuke&	GetNetDuke() { return *m_netduke; }

private:
	NetDuke::Peer m_peer;
};



};