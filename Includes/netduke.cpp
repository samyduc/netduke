#include "netduke.h"



namespace NetDuke
{

NetDuke::~NetDuke()
{
	assert(m_services.size() == 0);
}

void NetDuke::Init()
{
	m_transport.InitPlatform();
}

void NetDuke::Tick()
{
	m_transport.Tick();

	// poll message and distribute to services
	SerializerLess ser;
	Peer peer;

	while(m_transport.Pull(ser, peer))
	{
		netU32 type = ser.GetType();

		services_t::iterator it = m_services.find(type);

		if(it != m_services.end())
		{
			Service* service = it->second;
			service->Push(ser, peer);
		}
		else
		{
			// unknown packet, do something !
		}
	}

	// tick internal logic
	for(services_t::iterator it=m_services.begin(); it!=m_services.end(); ++it)
	{
		Service* service = it->second;
		service->Tick();
	}
}

void NetDuke::RegisterService(Service& _service)
{
	//size_t index = m_services.size();
	//_service.SetRegistrationId(index);

	m_services[_service.GetType()] = &_service;
}

void NetDuke::UnRegisterService(Service& _service)
{
	// find a use for this
	//size_t index = _service.GetRegistrationId();
	//(void)index;

	services_t::iterator it = m_services.find(_service.GetType());

	if(it != m_services.end())
	{
		m_services.erase(it);
	}
}








}