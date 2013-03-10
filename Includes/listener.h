#pragma once

#include "netdef.h"

#include "layer.h"
#include "stream.h"

namespace NetDuke
{

class SerializerLess;
class Peer;
class Stream;

class Listener : public Layer
{
public:

	virtual netBool		PullFromStream(SerializerLess& _ser, const Peer& _peer) = 0;

	virtual void		RegisterStream(Stream& _stream) = 0;
	virtual void		UnRegisterStream(const Stream& _stream) = 0;
protected:
	

};


};