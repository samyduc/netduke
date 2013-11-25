#include "pingpong.h"

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
	: PingService(nullptr)
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
}

void PingPongServer::DeInit()
{
}


void PingPongServer::Tick()
{
}

PingPongClient::PingPongClient(const NetDuke::netChar* _addr, NetDuke::netU16 _port)
	: PingService(nullptr)
{
	m_netduke = new NetDuke::NetDuke();
	m_netduke->Init();

	NetDuke::Peer peer;
	peer.SetPort(0);
	peer.SetIPv4Addr("0.0.0.0");
	m_netduke->GetTransport().Listen(peer);
	//m_netduke->GetTransport().InitTCPStack(peer);

	m_peer.SetIPv4Addr(_addr);
	m_peer.SetPort(_port);
	m_netduke->RegisterObserver(&m_observer);
}

PingPongClient::~PingPongClient()
{
}

void PingPongClient::Init()
{
	m_netduke->EnableRPC(true);
}

void PingPongClient::DeInit()
{
}

void PingPongClient::Tick()
{
	if(m_pingRPC.IsComplete())
	{
		if(m_pingRPC.IsSuccess())
		{
			m_lastPingMs = NetDuke::Time::GetMsTime() - m_pingRPC.m_out.m_time;

			std::lock_guard<std::mutex> lock_guard(g_globalLock);
			printf("ping: %llu seq:%d\n", GetLastPingMs(), m_seq);
			assert(m_pingRPC.m_in.m_seq == m_seq-1);

			SendPing(m_peer);
		}
		else
		{
			assert(false);
		}
	}
	else if(m_pingRPC.GetState() == NetDuke::RPC::eState::STATE_UNDEFINED)
	{
		// first send
		SendPing(m_peer);

		// debug rpc
		m_superprc.m_in.m_time = 69;
		NetDuke::RPCService* service = m_netduke->GetRPCService();
		service->Send(m_superprc, m_peer);
	}

	if(m_superprc.IsComplete())
	{
		if(m_superprc.IsSuccess())
		{
			NetDuke::RPCService* service = m_netduke->GetRPCService();
			service->Send(m_superprc, m_peer);
		}
		else
		{
			assert(false);
		}

	}
}




};