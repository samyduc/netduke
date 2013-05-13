#pragma once

#include "netdef.h"
#include "service.h"
#include "serializerless.h"
#include "peer.h"
#include "rpc.h"

#include <map>

namespace NetDuke
{




template<class TUP>
class ServiceHandler : public Service
{
public:
	typedef netBool (TUP::*rpchandler_t)(Peer&);

	struct RPCHandler
	{
		RPCHandler()
		{
		}

		RPCHandler(RPC& _rpc, rpchandler_t _handler)
			: m_rpc(&_rpc), m_handler(_handler)
		{
		}

		rpchandler_t	m_handler;
		RPC*			m_rpc;
	};

	typedef std::map<netU32, struct RPCHandler> handlers_t;

public:
	explicit ServiceHandler(NetDuke* _netduke, TUP* _this) : Service(_netduke), m_this(_this) {}
	virtual ~ServiceHandler() {}

protected:

	void RegisterHandler(RPC& _rpc, rpchandler_t _handler_func)
	{
		netU32 type = _rpc.In().GetType();
		typename handlers_t::iterator it = m_handlers.find(type);

		if(it == m_handlers.end())
		{
			m_handlers[type] = RPCHandler(_rpc, _handler_func);
		}
		else
		{
			// try to register the same function twice
			assert(false);
		}
	}

	void UnregisterHandler(netU32 _type)
	{
		typename handlers_t::iterator it = m_handlers.find(_type);

		if(it != m_handlers.end())
		{
			m_handlers.erase(it);
		}
	}

	virtual netBool	RecvHandler(SerializerLess& _ser, Peer& _peer)
	{ 
		netU32 type = _ser.GetType();
		netBool ret = false;
		typename handlers_t::iterator it = m_handlers.find(type);

		if(it != m_handlers.end())
		{
			SerializerLess response;
			struct RPCHandler &handler = it->second;
			handler.m_rpc->ChangeState(RPC::eState::STATE_RECEIVING);

			ret = handler.m_rpc->UnSerialize(handler.m_rpc->In(), _ser);
			if(ret)
			{
				handler.m_rpc->ChangeError(RPC::eError::ERROR_OK);
				ret = ((*m_this).*(handler.m_handler))(_peer);
			}
			else
			{
				handler.m_rpc->ChangeError(RPC::eError::ERROR_KO);
			}

			if(ret)
			{
				// send response
				handler.m_rpc->Serialize(handler.m_rpc->Out(), handler.m_rpc->GetSequence());
				m_netduke->GetTransport().Send(handler.m_rpc->GetSerializer(), _peer, s_typeReliableListener);
			}
		}

		return ret; 
	}

protected:
	handlers_t	m_handlers;
	TUP*		m_this;

};

}
