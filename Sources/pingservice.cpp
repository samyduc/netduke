#include "pingservice.h"

#include "transport.h"
#include "timeplatform.h"
#include "rpcservice.h"
#include "serializerless.h"

#include "dataset.h"
#include "rpc.h"

namespace NetDuke
{



PingService::PingService(NetDuke* _netduke)
	: ServiceHandler<PingService>(_netduke, this)
	, m_pingRPC()
	, m_lastPingMs(0)
	, m_seq(0)
{
	RegisterHandler(m_pingRPC, &PingService::OnRecvPing);
}

PingService::~PingService()
{
}

void PingService::Tick()
{
	if(m_pingRPC.IsComplete())
	{
		if(m_pingRPC.IsSuccess())
		{
			m_lastPingMs = Time::GetMsTime() - m_pingRPC.m_out.m_time;
		}
	}
}

void PingService::SendPing(const Peer& _peer)
{
	assert(m_netduke != nullptr);

	m_pingRPC.Reset();
	m_pingRPC.m_in.m_seq = m_seq;
	m_pingRPC.m_in.m_time = Time::GetMsTime();

	RPCService* service = m_netduke->GetRPCService();
	service->Send(m_pingRPC, _peer);
	
	++m_seq;
}

netBool PingService::OnRecvPing(Peer& _peer)
{
	(void)_peer;

	m_pingRPC.m_out.m_time = m_pingRPC.m_in.m_time;
	m_pingRPC.m_out.m_seq = m_pingRPC.m_in.m_seq;

	return true;
}

}