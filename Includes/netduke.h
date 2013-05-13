#pragma once

// unique incude header

#include "netdef.h"
#include "peer.h"
#include "serializer.h"
#include "serializerless.h"

#include "transport.h"
#include "service.h"
#include "rpcservice.h"

#include <map>

namespace NetDuke
{

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

private:
	Transport m_transport;
	RPCService *m_rpcService;

	typedef std::map<netU32, Service*> services_t;
	services_t m_services;


};


}
