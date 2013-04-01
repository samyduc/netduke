#pragma once

#include "netduke.h"

#include <ctime>

namespace NetDukeSample
{

class PingPongServer
{

public:
	explicit PingPongServer(NetDuke::netU16 _port);


	void Tick();

private:
	NetDuke::Transport m_transport;


};

class PingPongClient
{
public:
	explicit PingPongClient(NetDuke::netChar* _addr, NetDuke::netU16 _port);

	void Tick();

private:
	clock_t m_clock;
	NetDuke::Peer m_peer;
	NetDuke::Serializer m_serializer;
	NetDuke::Transport m_transport;

	NetDuke::netU32 m_round;

	bool m_state;
};



};