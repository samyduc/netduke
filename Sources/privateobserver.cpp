#include "privateobserver.h"
#include "netduke.h"
#include "service.h"

namespace NetDuke
{

PrivateObserver::PrivateObserver(NetDuke* _netduke)
	: m_netduke(_netduke)
{
	assert(m_netduke != nullptr);
}


void PrivateObserver::OnUnregisteredMessage(SerializerLess& _ser, Peer& _peer)
{
	services_t& services = m_netduke->GetServices();

	for(services_t::iterator it = services.begin(); it != services.end(); ++it)
	{
		Service* service = (*it);
		service->OnUnregisteredMessage(_ser, _peer);
	}

	IObserver* observer = m_netduke->GetObserver();
	if(observer)
	{
		observer->OnUnregisteredMessage(_ser, _peer);
	}
}

void PrivateObserver::OnPeerAdded(const Peer& _peer)
{
	services_t& services = m_netduke->GetServices();

	for(services_t::iterator it = services.begin(); it != services.end(); ++it)
	{
		Service* service = (*it);
		service->OnPeerAdded(_peer);
	}

	IObserver* observer = m_netduke->GetObserver();
	if(observer)
	{
		observer->OnPeerAdded(_peer);
	}
}

void PrivateObserver::OnPeerRemoved(const Peer& _peer)
{
	services_t& services = m_netduke->GetServices();

	for(services_t::iterator it = services.begin(); it != services.end(); ++it)
	{
		Service* service = (*it);
		service->OnPeerRemoved(_peer);
	}

	IObserver* observer = m_netduke->GetObserver();
	if(observer)
	{
		observer->OnPeerRemoved(_peer);
	}
}


};