#include "server.h"

#include <thread>
#include <mutex>
#include <iostream>

#include "rpcservice.h"

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
	: ServiceHandler<PingPongServer>(nullptr, this)
{
	m_netduke = new NetDuke::NetDuke();
	m_netduke->Init();

	NetDuke::Peer peer;
	peer.SetPort(_port);
	peer.SetIPv4Addr("0.0.0.0");
	m_netduke->GetTransport().Listen(peer);
	//m_netduke->GetTransport().InitTCPStack(peer);
	m_netduke->RegisterObserver(&m_observer);
}

PingPongServer::~PingPongServer()
{
	
}

void PingPongServer::Init()
{
	m_netduke->EnableRPC(true);

	RegisterHandler(m_pingRPC, &PingPongServer::OnRecvPing);
	RegisterHandler(m_superRPC, &PingPongServer::OnRecvSuperRPC);
}

void PingPongServer::DeInit()
{
}


void PingPongServer::Tick()
{
}

NetDuke::netBool PingPongServer::OnRecvPing(NetDuke::Peer& _peer)
{
	printf("Rcv ping\n");
	m_pingRPC.m_out.m_seq = m_pingRPC.m_in.m_seq;
	m_pingRPC.m_out.m_time = m_pingRPC.m_in.m_time;

	return true;
}

NetDuke::netBool PingPongServer::OnRecvSuperRPC(NetDuke::Peer& _peer)
{
	printf("Rcv superrpc\n");

	return true;
}

};