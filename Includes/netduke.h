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

typedef std::list<Service*> services_t;

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
	services_t& GetServices() { return m_services; }

	// helper
	void		EnableRPC(netBool _state);

	// observer
	void		RegisterObserver(IObserver* _observer);
	IObserver*	GetObserver() { return m_publicObserver; }

private:
	IObserver*	m_publicObserver;
	IObserver*	m_privateObserver;

	Transport	m_transport;
	RPCService*	m_rpcService;

	services_t	m_services;


};


}
