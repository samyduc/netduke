#pragma once

#include "netdef.h"

#include "peer.h"

#include <map>
#include <list>

namespace NetDuke
{

class Listener;
class SerializerLess;
class Channel;
class Stream;

typedef std::map<Peer, Channel*> channels_t;
typedef std::list<Listener*> listeners_t;
typedef std::list<Stream*> streams_t;

class Layer
{
public:

	virtual void		Tick() = 0;
	virtual netU32		GetType() const = 0;

	virtual netBool		Pull(SerializerLess &_ser, Peer& _peer) = 0;
	virtual netBool		Push(SerializerLess &_ser, const Peer& _peer) = 0;

protected:
	virtual netBool		Pack(SerializerLess& _ser, const Peer& _peer) = 0;
	virtual netBool		UnPack(SerializerLess& _ser, const Peer& _peer) = 0;

	

};




};