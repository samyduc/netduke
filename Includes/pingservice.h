#pragma once

#include "netdef.h"

#include "service.h"
#include "crc.h"
#include "serializer.h"

namespace NetDuke
{

static const netU32 s_typePingService = CRC32::Compute("PingService");


class Transport;
class SerializerLess;

class PingService : public Service
{
public:
	

							PingService();
							PingService(Transport* _transport);
	virtual					~PingService();

	void					SetTransport(Transport* _transport) { m_transport = _transport; }

	virtual	void			Tick();
	virtual netU32			GetType() const { return s_typePingService; }

	virtual inline size_t	GetHeaderSize() const { return 0; };

	virtual netBool			Pull(SerializerLess &_ser, Peer& _peer);
	virtual netBool			Push(SerializerLess &_ser, const Peer& _peer);

	void					SendPing(const Peer& _peer);

	netU64					GetLastPingMs() { return m_lastPingMs; } 

	// overload for custom use
	virtual void			OnRecvPing(netU32 _type, netU64 _timestamp, Peer _peer) { (void)_type; (void)_timestamp; (void)_peer; };

protected:

	virtual netBool			Pack(SerializerLess& _ser, const Peer& _peer);
	virtual netBool			UnPack(SerializerLess& _ser, const Peer& _peer);

protected:

	enum requestType
	{
		TYPE_REQUEST = 0,
		TYPE_RESPONSE = 1,
	};

	Serializer m_request;
	netU64 m_lastPingMs;
	Transport* m_transport;
};






}
