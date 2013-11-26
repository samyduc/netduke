#pragma once

#include "netdef.h"
#include "service.h"
#include "serializerless.h"
#include "peer.h"
#include "rpc.h"
#include "event.h"

#include <map>

namespace NetDuke
{




template<class TUP>
class ServiceHandler : public Service
{
public:
	typedef netBool (TUP::*rpchandler_t)(RPC&, Peer&);
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
	typedef std::map<netU32, struct RPCHandler> handlersRPC_t;

	typedef netBool (TUP::*eventhandler_t)(Event&, Peer&);
	struct EventHandler
	{
		EventHandler()
		{
		}

		EventHandler(Event& _event, eventhandler_t _handler)
			: m_event(&_event), m_handler(_handler)
		{
		}

		eventhandler_t	m_handler;
		Event*			m_event;
	};
	typedef std::map<netU32, struct EventHandler> handlersEvent_t;

public:
	explicit ServiceHandler(NetDuke* _netduke, TUP* _this) : Service(_netduke), m_this(_this) {}
	virtual ~ServiceHandler() {}

protected:

	void RegisterHandler(Event& _event, eventhandler_t _handler_func)
	{
		//netU32 type = _rpc.In().GetType();
		netU32 type = _event.GetType();
		typename handlersEvent_t::iterator it = m_handlersEvent.find(type);

		if(it == m_handlersEvent.end())
		{
			m_handlersEvent[type] = EventHandler(_event, _handler_func);
		}
		else
		{
			// try to register the same function twice
			assert(false);
		}
	}

	void RegisterHandler(RPC& _rpc, rpchandler_t _handler_func)
	{
		//netU32 type = _rpc.In().GetType();
		netU32 type = _rpc.GetType();
		typename handlersRPC_t::iterator it = m_handlersRPC.find(type);

		if(it == m_handlersRPC.end())
		{
			m_handlersRPC[type] = RPCHandler(_rpc, _handler_func);
		}
		else
		{
			// try to register the same function twice
			assert(false);
		}
	}

	void UnregisterHandler(RPC& _rpc)
	{
		typename handlersRPC_t::iterator it = m_handlersRPC.find(_rpc.GetType());

		if(it != m_handlersRPC.end())
		{
			m_handlersRPC.erase(it);
		}
	}

	virtual netBool	RecvHandler(SerializerLess& _ser, Peer& _peer)
	{ 
		netU32 type = _ser.GetType();
		netBool ret = false;
		typename handlersRPC_t::iterator it = m_handlersRPC.find(type);

		if(it != m_handlersRPC.end())
		{
			SerializerLess response;
			struct RPCHandler &handler = it->second;
			handler.m_rpc->ChangeState(RPC::eState::STATE_RECEIVING);

			ret = handler.m_rpc->UnSerialize(handler.m_rpc->In(), _ser);
			if(ret)
			{
				handler.m_rpc->ChangeError(RPC::eError::ERROR_OK);
				ret = ((*m_this).*(handler.m_handler))(*handler.m_rpc, _peer);
			}
			else
			{
				handler.m_rpc->ChangeError(RPC::eError::ERROR_KO);
			}

			if(ret)
			{
				// send response
				handler.m_rpc->Serialize(handler.m_rpc->Out());

				Transport& transport = m_netduke->GetTransport();

				if(transport.IsTCPEnabled())
				{
					transport.Send(handler.m_rpc->GetSerializer(), _peer, s_typeUnreliableListener);
				}
				else
				{
					transport.Send(handler.m_rpc->GetSerializer(), _peer, s_typeReliableListener);
				}
			}
		}
		else
		{
			// check event after rpc !
			typename handlersEvent_t::iterator it = m_handlersEvent.find(type);

			if(it != m_handlersEvent.end())
			{
				struct EventHandler &handler = it->second;

				ret = handler.m_event->UnSerialize(handler.m_event->In(), _ser);
				if(ret)
				{
					ret = ((*m_this).*(handler.m_handler))(*handler.m_event, _peer);
				}
			}
		}

		return ret; 
	}

protected:
	handlersRPC_t	m_handlersRPC;
	handlersEvent_t m_handlersEvent;
	TUP*			m_this;

};

}
