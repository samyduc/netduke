#pragma once

#include "netdef.h"
#include "extinclude.h"
#include "crc.h"
#include "channel.h"
#include "peer.h"
#include "serializer.h"
#include "serializerless.h"
#include "serializerpool.h"
#include "stream.h"

#include <map>

namespace NetDuke
{

class Listener;

struct ConnectionPacker
{
	ConnectionPacker(SOCKET _socket) : m_socket(_socket), m_ser(), m_written(0)
	{
	}

	SOCKET			m_socket;
	size_t			m_written;
	SerializerLess	m_ser;
};


class TCPStream : public Stream
{
public:
	explicit					TCPStream(const Peer& _peer);
	virtual						~TCPStream(); // platform specific

	TCPStream&					operator=( const TCPStream& ) { assert(false); }

	void						Init();
	void						Tick();
	netU32						GetType() const { return s_typeTCPStream; } 

	void						CreateAndBind();
	size_t						GetHeaderSize() const;

	netBool						Pull(SerializerLess &_ser, Peer& _peer);
	netBool						Push(SerializerLess &_ser, const Peer& _peer);

	void						Flush();
	void						FlushSend();
	void						FlushRecv();

	netBool						Connect(const Peer& _peer);
	void						Accept();
	void						SendTo(Serializer& _ser, const Peer& _peer);
	netBool						RecvFrom(struct ConnectionPacker& _connection, const Peer& _peer);

	netBool						IsValid() const { return m_isValid; }
	const Peer&					GetPeer() const { return m_peer; }
	SOCKET						GetSocket() const { assert(m_socket != 0); return  m_socket; }

	netBool						AttachListener(Listener& _listener);
	netBool						DetachListener(Listener& _listener);

protected:
	netBool						Pack(SerializerLess& _ser, const Peer& _peer);
	netBool						UnPack(SerializerLess& _ser, const Peer& _peer);

	void						PushToStream(SerializerLess& _ser, const Peer& _peer);
	Channel&					GetChannel(channels_t& channels, const Peer& _peer);

	struct ConnectionPacker*	GetOrCreateConnection(const Peer& _peer);
	struct ConnectionPacker*	GetConnection(const Peer& _peer);

	void						CheckIncomingPacket(size_t _len, ConnectionPacker& _connection, Serializer& _ser, const Peer& _peer);
	void						DeletePeer(const Peer& _peer);

private:

	netBool						m_isValid;
	netBool						m_opt_compression;
	netBool						m_opt_encryption;
	netBool						m_opt_skipCRC;

	const Peer					m_peer;
	SOCKET						m_socket;
	SOCKET						m_fdmax;

	channels_t					m_sendChannels;

	SerializerPool				m_pool;

	listeners_t					m_listeners;

	typedef std::map<Peer, struct ConnectionPacker*> connections_t;
	connections_t				m_connections;








};
}