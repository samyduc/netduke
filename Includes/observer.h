#pragma once

#include "netdef.h"


namespace NetDuke
{

class SerializerLess;
class Peer;

class IObserver 
{
public:

	virtual				~IObserver() {}

	virtual	void		OnUnregisteredMessage(SerializerLess& _ser, Peer& _peer) = 0;
	virtual void		OnPeerRemoved(const Peer& _peer) = 0;

protected:
	

};


};