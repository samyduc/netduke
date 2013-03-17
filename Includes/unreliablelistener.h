#pragma once

#include "netdef.h"
#include "layer.h"
#include "listener.h"
#include "crc.h"
#include "serializerpool.h"

#include <list>

namespace NetDuke
{

class Peer;
class Channel;
class SerializerLess;
class Stream;

class UnreliableListener : public Listener
{
public:

				UnreliableListener();
				~UnreliableListener();

	void		Tick();
	netU32		GetType() const { return s_typeUnreliableListener; } 

	netBool		Pull(SerializerLess &_ser, Peer& _peer);
	netBool		Push(SerializerLess &_ser, const Peer& _peer);

	netBool		PullFromStream(SerializerLess& _ser, const Peer& _peer);

	void		RegisterStream(Stream& _stream);
	void		UnRegisterStream(const Stream& _stream);

protected:
	netBool		Pack(SerializerLess& _ser, const Peer& _peer);
	netBool		UnPack(SerializerLess& _ser, const Peer& _peer);

	Channel&	GetChannel(channels_t& channels, const Peer& _peer);

	void		FlushSend();
	void		Flush();

private:
	Stream*			m_stream;

	channels_t		m_sendChannels;
	channels_t		m_recvChannels;

	SerializerPool	m_pool;

};




};