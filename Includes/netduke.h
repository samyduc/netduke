#pragma once

// unique incude header

#include "netdef.h"
#include "transport.h"


#include <map>
#include <list>
#include <cassert>

namespace NetDuke
{

class IObserver;
class SerializerLess;
class Serializer;
class RPCService;
class Service;
class Peer;


class NetDuke
{
public:
				NetDuke();
				~NetDuke();

	void		Init();
	void		DeInit();

	void		Tick();

	Transport&	GetTransport() { return m_transport; }
	RPCService* GetRPCService() { assert(m_rpcService != nullptr); return m_rpcService; }

	void		RegisterService(Service& _service);
	void		UnRegisterService(Service& _service);

	Service*	GetService(netU32 _type);

	// helper
	void		EnableRPC(netBool _state);

	// observer
	void		RegisterObserver(IObserver* _observer);
	IObserver*	GetObserver() { return m_observer; }

private:
	IObserver*	m_observer;
	Transport	m_transport;
	RPCService*	m_rpcService;

	typedef std::list<Service*> services_t;
	services_t	m_services;


};


}
