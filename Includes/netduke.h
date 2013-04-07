#pragma once

// unique incude header

#include "netdef.h"
#include "peer.h"
#include "serializer.h"
#include "serializerless.h"

#include "transport.h"
#include "service.h"

#include <map>

namespace NetDuke
{

class NetDuke
{
public:
	NetDuke() {}
	~NetDuke();

	void Init();

	void Tick();

	Transport& GetTransport() { return m_transport; }

	void RegisterService(Service& _service);
	void UnRegisterService(Service& _service);

private:
	Transport m_transport;

	typedef std::map<netU32, Service*> services_t;
	services_t m_services;


};

}
