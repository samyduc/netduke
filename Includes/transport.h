#pragma once

#include "netdef.h"
#include "crc.h"

#include "layer.h"
#include "listener.h"

#include <list>

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#endif

namespace NetDuke
{

class Channel;
class Serializer;
class SerializerLess;
class Peer;


class Transport : public Layer
{
public:
						Transport() {}
						~Transport() {}

	void				InitPlatform();
	void				DesInitPlatform();

	void				Tick();
	netU32				GetType() const { return s_typeTransport; }

	// helper
	void				Listen(const Peer &_peer);
	Listener*			GetListener(netU32 _type) const;
	void				Send(Serializer& _ser, const Peer& _peer, netU32 _type);

	netBool				Push(SerializerLess &_ser, const Peer& _peer);
	netBool				Pull(SerializerLess &_ser, Peer& _peer);

protected:
	netBool				Pack(SerializerLess& _ser, const Peer& _peer);
	netBool				UnPack(SerializerLess& _ser, const Peer& _peer);
	
private:

	listeners_t m_listeners;
	streams_t	m_streams;

#if defined(_WIN32)
	bool				InitPlatformPrivate();
	bool				DesInitPlatformPrivate();
#endif

};

};
