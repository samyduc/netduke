#include "netduke.h"

#include "service.h"
#include "rpcservice.h"
#include "peer.h"
#include "serializer.h"
#include "serializerless.h"
#include "observer.h"

namespace NetDuke
{

NetDuke::NetDuke()
	: m_rpcService(nullptr)
	, m_observer(nullptr)
{

}

NetDuke::~NetDuke()
{
	assert(m_services.size() == 0);
}

void NetDuke::Init()
{
	m_transport.InitPlatform();
}

void NetDuke::DeInit()
{
	// delete helpers
	EnableRPC(false);

	// delete services
	for(services_t::iterator it=m_services.begin(); it!=m_services.end(); ++it)
	{
		Service& service = *(it->second);
		service.DeInit();
	}

	m_services.clear();
	m_transport.DesInitPlatform();

}

void NetDuke::Tick()
{
	m_transport.Tick();

	// poll message and distribute to services
	SerializerLess ser;
	Peer peer;
	netBool isFound;

	while(m_transport.Pull(ser, peer))
	{
		isFound = false;
		// rpc handler
		for(services_t::iterator it=m_services.begin(); it!=m_services.end(); ++it)
		{
			Service& service = *it->second;

			if(service.RecvHandler(ser, peer))
			{
				isFound = true;
				break;
			}
		}

		if(m_observer && isFound == false)
		{
			m_observer->OnUnregisteredMessage(ser, peer);
		}
	}

	// tick service logic
	for(services_t::iterator it=m_services.begin(); it!=m_services.end(); ++it)
	{
		Service* service = it->second;
		service->Tick();
	}
}

void NetDuke::RegisterService(Service& _service)
{
	_service.Init();
	m_services[_service.GetType()] = &_service;
}

void NetDuke::UnRegisterService(Service& _service)
{
	services_t::iterator it = m_services.find(_service.GetType());

	if(it != m_services.end())
	{
		Service& service = *(it->second);
		service.DeInit();
		m_services.erase(it);
	}
}

Service* NetDuke::GetService(netU32 _type)
{
	Service* service = nullptr;

	services_t::iterator it = m_services.find(_type);

	if(it != m_services.end())
	{
		service = it->second;
	}

	return service;
}

void NetDuke::EnableRPC(netBool _state)
{
	if(_state)
	{
		if(m_rpcService == nullptr)
		{
			// dummy check
			Service* service = GetService(s_RPCService);
			assert(service == nullptr);

			m_rpcService = new RPCService(this);
			RegisterService(*m_rpcService);
		}
	}
	else
	{
		if(m_rpcService)
		{
			Service* service = GetService(s_RPCService);
			assert(service != nullptr);

			UnRegisterService(*service);

			delete m_rpcService;
			m_rpcService = nullptr;
		}
	}
}

void NetDuke::RegisterObserver(IObserver* _observer)
{
	// check if already assigned
	assert(m_observer == nullptr);
	m_observer = _observer;

	m_transport.RegisterObserver(m_observer);
}

/*extern "C" {


int main()
{
	Peer listen_peer("127.0.0.1", 9000);
	Peer send_peer("127.0.0.1", 9001);

	Serializer ser(1024);
	ser.Write("toto");
	ser << 12;
	ser.Close();

	NetDuke netduke;

	netduke.Init();

	netduke.GetTransport().Listen(listen_peer);
	netduke.GetTransport().Send(ser, send_peer, s_typeUnreliableListener);

	//while(true)
	//{
	//	netduke.Tick();
	//}

	return 0;
}

}*/


}
