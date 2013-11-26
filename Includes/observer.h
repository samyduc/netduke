#pragma once

#include "netdef.h"
#include "serializerless.h"
#include "peer.h"


namespace NetDuke
{



class IObserver 
{
public:

	virtual				~IObserver() {}

	virtual	void		OnUnregisteredMessage(SerializerLess& _ser, Peer& _peer) { (void)_ser; (void)_peer; };
	virtual void		OnPeerAdded(const Peer& _peer) { (void)_peer; };
	virtual void		OnPeerRemoved(const Peer& _peer) { (void)_peer; };

protected:
	

};


};