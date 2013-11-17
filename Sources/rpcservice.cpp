#include "rpcservice.h"

#include "transport.h"
#include "peer.h"
#include "netduke.h"

namespace NetDuke
{

RPCService::RPCService(NetDuke* _netduke)
	: Service(_netduke)
	, m_seq(0)
{
}

void RPCService::Init()
{

}

void RPCService::DeInit()
{
	for(rpcs_t::iterator it = m_rpcs.begin(); it != m_rpcs.end(); ++it)
	{
		RPC& rpc = *(*it);
		rpc.ChangeState(RPC::eState::STATE_DONE);
		rpc.ChangeError(RPC::eError::ERROR_CANCEL);
	}

	m_rpcs.clear();
}

RPCService::~RPCService()
{
	DeInit();
}

void RPCService::Tick()
{
	rpcs_t::iterator it = m_rpcs.begin();
	while(it != m_rpcs.end())
	{
		RPC& rpc = *(*it);
		if(rpc.GetState() == RPC::eState::STATE_DONE || CheckTimeOut(rpc))
		{
			// detach rpc
			it = m_rpcs.erase(it);
		}
		else
		{
			++it;
		}
	}
}

netBool RPCService::CheckTimeOut(RPC& _rpc)
{
	netBool is_timeout = _rpc.IsTimeOut();
	if(is_timeout)
	{
		_rpc.ChangeState(RPC::eState::STATE_DONE);
		_rpc.ChangeError(RPC::eError::ERROR_TIME_OUT);
	}

	return is_timeout;
}

netU8 RPCService::GetNextSequence()
{
	if(m_seq == 0xFF)
	{
		m_seq = 0;
	}
	else
	{
		++m_seq;
	}

	return m_seq;
}

void RPCService::Send(RPC& _rpc, const Peer& _peer)
{
	netU8 seq = GetNextSequence();
	_rpc.Serialize(_rpc.In(), seq);
	_rpc.ChangeState(RPC::eState::STATE_SENDING);
	_rpc.SetStartTime(Time::GetMsTime());

	m_rpcs.push_back(&_rpc);

	Transport& transport = m_netduke->GetTransport();

	if(transport.IsTCPEnabled())
	{
		m_netduke->GetTransport().Send(_rpc.GetSerializer(), _peer, s_typeUnreliableListener);
	}
	else
	{
		m_netduke->GetTransport().Send(_rpc.GetSerializer(), _peer, s_typeReliableListener);
	}
}

netBool RPCService::Recv(SerializerLess& _ser, const Peer& _peer)
{
	(void)_peer;

	netBool ret = false;

	// check if response
	for(rpcs_t::iterator it = m_rpcs.begin(); it != m_rpcs.end(); ++it)
	{
		RPC& rpc = *(*it);

		//if(rpc.Out().GetType() == _ser.GetType())
		if(rpc.GetType() == _ser.GetType())
		{
			RecvOut(rpc, _ser);
			ret = true;
			break;
		}
	}

	return ret;
}

void RPCService::RecvOut(RPC& _rpc, SerializerLess& _ser)
{
	_rpc.ChangeState(RPC::eState::STATE_DONE);

	netBool ret = _rpc.UnSerialize(_rpc.Out(), _ser);
	if(ret)
	{
		_rpc.ChangeError(RPC::eError::ERROR_OK);
	}
	else
	{
		_rpc.ChangeError(RPC::eError::ERROR_KO);
	}
}




}