#pragma once

#include "netdef.h"
#include "serializerless.h"
#include "peer.h"

#include "observer.h"

namespace NetDuke
{

class NetDuke;

class PrivateObserver : public IObserver
{
public:

	explicit			PrivateObserver(NetDuke* _netduke);
	virtual				~PrivateObserver() {}

	virtual	void		OnUnregisteredMessage(SerializerLess& _ser, Peer& _peer);
	virtual void		OnPeerRemoved(const Peer& _peer);

protected:
	NetDuke* m_netduke;

};


};