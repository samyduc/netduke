#include "netduke.h"

#include "service.h"
#include "rpcservice.h"
#include "peer.h"
#include "serializer.h"
#include "serializerless.h"
#include "observer.h"
#include "privateobserver.h"

namespace NetDuke
{

NetDuke::NetDuke()
	: m_rpcService(nullptr)
	, m_publicObserver(nullptr)
{
	// safe 
	m_privateObserver = new PrivateObserver(this);
}

NetDuke::~NetDuke()
{
	assert(m_services.size() == 0);
}

void NetDuke::Init()
{
	m_transport.RegisterObserver(m_privateObserver);
	m_transport.InitPlatform();
}

void NetDuke::DeInit()
{
	// delete helpers
	EnableRPC(false);

	// delete services
	for(services_t::iterator it=m_services.begin(); it!=m_services.end(); ++it)
	{
		Service& service = *(*it);
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
			Service& service = *(*it);

			if(service.RecvHandler(ser, peer))
			{
				isFound = true;
				break;
			}
		}

		if(!isFound)
		{
			m_privateObserver->OnUnregisteredMessage(ser, peer);
		}
	}

	// tick service logic
	for(services_t::iterator it=m_services.begin(); it!=m_services.end(); ++it)
	{
		Service* service = (*it);
		service->Tick();
	}
}

void NetDuke::RegisterService(Service& _service)
{
	_service.Init();
	m_services.push_back(&_service);
}

void NetDuke::UnRegisterService(Service& _service)
{
	for(services_t::iterator it = m_services.begin(); it != m_services.end(); ++it)
	{
		Service* serv = (*it);

		if(serv->GetType() == _service.GetType())
		{
			_service.DeInit();
			m_services.erase(it);
			break;
		}
	}
}

Service* NetDuke::GetService(netU32 _type)
{
	Service* service = nullptr;

	for(services_t::iterator it = m_services.begin(); it != m_services.end(); ++it)
	{
		Service* serv = (*it);

		if(serv->GetType() == _type)
		{
			service = serv;
			break;
		}
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

			// rpc service must be first in lane
			m_rpcService = new RPCService(this);
			m_services.push_front(m_rpcService);
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
	assert(m_publicObserver == nullptr);
	m_publicObserver = _observer;
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
