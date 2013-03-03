#pragma once

#include "netdef.h"
#include "crc.h"

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
class Peer;


class Transport
{
private:
	typedef std::list<Listener*> listeners_t;
	listeners_t m_listeners;
public:
						Transport() {}
						~Transport() {}

	void				InitPlatform();
	void				DesInitPlatform();

	void				Tick();
	Listener*			Listen(const Peer &_peer);

	Listener*			GetListener(const Peer &_peer) const;
	const listeners_t&	GetListeners() const;

	void				Send(const Serializer& _ser, const Peer& _peer);
	void				Send(const Serializer& _ser, const Peer& _peer, Listener& _listener);
	bool				Pull(SerializerLess &_ser, Peer& _peer);

public:
	
	
private:
	// TODO : refactor -> doublon with listener
	Channel&			GetChannel(const Peer& _peer);
	void				Recv();
	void				Unpack(SerializerLess& _ser, const Peer& _peer);

	fd_set m_readfs;

	channels_t m_recvChannels;

#if defined(_WIN32)
	bool				InitPlatformPrivate();
	bool				DesInitPlatformPrivate();
#endif

};

};
