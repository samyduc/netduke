#pragma once

#include "netdef.h"
#include "layer.h"

#include "observer.h"

#include "serializerless.h"
#include "peer.h"

#include <map>
#include <functional>

namespace NetDuke
{

class NetDuke;
class SerializerLess;
class Peer;


class Service : public IObserver
{
public:

	friend class NetDuke;

	explicit Service(NetDuke* _netduke) : m_netduke(_netduke) {}
	virtual ~Service() {}

	virtual netU32	GetType() const = 0;
	virtual void	Tick() = 0;

protected:
	virtual void	Init() = 0;
	virtual void	DeInit() = 0;

	virtual netBool	RecvHandler(SerializerLess& _ser, Peer& _peer) { (void)_ser; (void)_peer; return false; }

protected:
	NetDuke* m_netduke;

};





}