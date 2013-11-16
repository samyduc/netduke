#pragma once

#include "netdef.h"
#include "netduke.h"

#include "service.h"
#include "servicehandler.h"
#include "rpc.h"
#include "crc.h"
#include "serializer.h"
#include "serializerless.h"
#include "dataset.h"

namespace NetDuke
{

static const netU32 s_typePingService = CRC32::Compute("PingService");
static const netU32 s_datasetRPCPingIn = CRC32::Compute("s_datasetRPCPingIn");
static const netU32 s_datasetRPCPingOut = CRC32::Compute("s_datasetRPCPingOut");

class Transport;
class SerializerLess;

struct PingRPCIn : public Dataset
{
	netU64 m_time;
	netU32 m_seq;

	netU32 GetType() const { return s_datasetRPCPingIn; }

	void Write(Serializer& _ser)
	{
		_ser << m_time;
		_ser << m_seq;
	}

	void Read(Serializer& _ser)
	{
		_ser >> m_time;
		_ser >> m_seq;
	}
};

struct PingRPCOut : public Dataset
{
	netU64 m_time;
	netU32 m_seq;

	netU32 GetType() const { return s_datasetRPCPingOut; }

	void Write(Serializer& _ser)
	{
		_ser << m_time;
		_ser << m_seq;
	}

	void Read(Serializer& _ser)
	{
		_ser >> m_time;
		_ser >> m_seq;
	}
};

static const netU32 s_PingRPC = CRC32::Compute("NetDuke::PingRPC");

class PingRPC : public RPC
{
public:

	netU32		GetType() const { return s_PingRPC; }

	Dataset&	In() { return m_in; }
	Dataset&	Out() { return m_out; }

	PingRPCIn	m_in;
	PingRPCOut	m_out;

};

class PingService : public ServiceHandler<PingService>
{
public:
	

							PingService(NetDuke* _netDuke);
	virtual					~PingService();

	virtual void			Init() {}
	virtual void			DeInit() {}

	virtual	void			Tick();
	virtual netU32			GetType() const { return s_typePingService; }

	void					SendPing(const Peer& _peer);
	netU64					GetLastPingMs() { return m_lastPingMs; } 

	// overload for custom use
	virtual netBool			OnRecvPing(Peer& _peer);

protected:

protected:

	enum requestType
	{
		TYPE_REQUEST = 0,
		TYPE_RESPONSE = 1,
	};

	PingRPC		m_pingRPC;
	netU64		m_lastPingMs;
	netU32		m_seq;
};






}
