#include "pingpong.h"

#include <thread>
#include <mutex>
#include <iostream>

namespace NetDukeSample
{

static std::mutex g_globalLock;

void Log(char *fmt, ...)
{
	/*std::lock_guard<std::mutex> gLock(g_globalLock);

	va_list	va;
	va_start (va, fmt) ;
		//netVsprintf(s_logtxt,sizeof(s_logtxt)-1 , fmt, va) ;
		printf(fmt, va);
	va_end (va);*/
}

PingPongServer::PingPongServer(NetDuke::netU16 _port)
	: m_netduke()
{
	m_netduke.Init();

	NetDuke::Peer peer;
	peer.SetPort(_port);
	peer.SetIPv4Addr("0.0.0.0");
	m_netduke.GetTransport().Listen(peer);

	m_netduke.RegisterService(*this);
}

PingPongServer::~PingPongServer()
{
	m_netduke.UnRegisterService(*this);
}

void PingPongServer::OnRecvPing(NetDuke::netU32 _type, NetDuke::netU64 _timestamp, NetDuke::Peer _peer)
{
	(void)_type;
	(void)_timestamp;
	(void)_peer;
}

void PingPongServer::Tick()
{
	m_netduke.Tick();
}

PingPongClient::PingPongClient(NetDuke::netChar* _addr, NetDuke::netU16 _port)
	: m_netduke()
{
	m_netduke.Init();

	NetDuke::Peer peer;
	peer.SetPort(0);
	peer.SetIPv4Addr("0.0.0.0");
	m_netduke.GetTransport().Listen(peer);

	m_peer.SetIPv4Addr(_addr);
	m_peer.SetPort(_port);

	m_netduke.RegisterService(*this);
}

PingPongClient::~PingPongClient()
{
	m_netduke.UnRegisterService(*this);
}

void PingPongClient::Tick()
{
	static bool sent = false;

	if(!sent)
	{
		SendPing(m_peer);
		sent = true;
	}

	m_netduke.Tick();
}

void PingPongClient::OnRecvPing(NetDuke::netU32 _type, NetDuke::netU64 _timestamp, NetDuke::Peer _peer)
{
	// send back
	assert(_type == NetDuke::PingService::TYPE_RESPONSE);
	assert(m_peer.GetIPv4Addr() == _peer.GetIPv4Addr());
	assert(m_peer.GetPort() == _peer.GetPort());


	SendPing(_peer);
	std::lock_guard<std::mutex> lock_guard(g_globalLock);
	printf("ping: %llu\n", GetLastPingMs());
}









};