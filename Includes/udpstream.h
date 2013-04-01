#pragma once

#include "netdef.h"
#include "crc.h"

#include "stream.h"

#include "channel.h"
#include "peer.h"
#include "serializer.h"
#include "serializerless.h"

#include "serializerpool.h"

#include <map>
#include <list>


namespace NetDuke
{

class Listener;

class UDPStream : public Stream
{
public:
	
	explicit		UDPStream(const Peer& _peer);
	virtual			~UDPStream(); // platform specific

	UDPStream&		operator=( const UDPStream& ) { assert(false); }

	void			Tick();
	netU32			GetType() const { return s_typeUDPStream; } 

	void			CreateAndBind();
	inline size_t	GetHeaderSize() const;

	netBool			Pull(SerializerLess &_ser, Peer& _peer);
	netBool			Push(SerializerLess &_ser, const Peer& _peer);

	void			Flush();
	void			FlushSend();
	void			FlushRecv();

	void			SendTo(Serializer& _ser, const Peer& _peer);
	void			RecvFrom();

	netBool			IsValid() const { return m_isValid; }
	const Peer&		GetPeer() const { return m_peer; }
	SOCKET			GetSocket() const { assert(m_socket != 0); return  m_socket; }

	netBool			AttachListener(Listener& _listener);
	netBool			DetachListener(Listener& _listener);

protected:
	netBool			Pack(SerializerLess& _ser, const Peer& _peer);
	netBool			UnPack(SerializerLess& _ser, const Peer& _peer);

	void			PushToStream(SerializerLess& _ser, const Peer& _peer);
	Channel&		GetChannel(channels_t& channels, const Peer& _peer);

private:

	netBool		m_isValid;
	netBool		m_opt_compression;
	netBool		m_opt_encryption;

	const Peer	m_peer;
	SOCKET		m_socket;

	channels_t	m_sendChannels;

	SerializerPool	m_pool;

	listeners_t m_listeners;
};

};	
