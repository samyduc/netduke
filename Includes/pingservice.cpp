#include "pingservice.h"

#include "transport.h"
#include "timeplatform.h"

#include "serializerless.h"

namespace NetDuke
{


PingService::PingService()
	: m_request(200)
	, m_lastPingMs(0)
{
}

PingService::PingService(Transport* _transport)
	: m_request(200)
	, m_lastPingMs(0)
{
	SetTransport(_transport);
}

PingService::~PingService()
{
}

void PingService::Tick()
{

}

void PingService::SendPing(const Peer& _peer)
{
	SerializerLess ser(m_request);

	Pack(ser, _peer);
}

netBool PingService::Pack(SerializerLess& _ser, const Peer& _peer)
{
	m_request.ResetCursor();

	m_request.Write(GetType());
	m_request << TYPE_REQUEST;
	m_request << Time::GetMsTime();
	m_request.Close();

	m_transport->Send(_ser, _peer, s_typeReliableListener);

	return true;
}

netBool	PingService::UnPack(SerializerLess& _ser, const Peer& _peer)
{
	_ser.Read(GetType());

	netU32 type;
	static_cast<Serializer&>(_ser) >> type;

	netU64 timestamp;
	static_cast<Serializer&>(_ser) >> timestamp;

	_ser.Close();

	if(type == TYPE_REQUEST)
	{
		// respond
		m_request.Write(GetType());
		m_request << TYPE_RESPONSE;
		m_request << timestamp;
		m_request.Close();

		OnRecvPing(type, timestamp, _peer);

		m_transport->Send(m_request, _peer, s_typeReliableListener);
	}
	else if(type == TYPE_RESPONSE)
	{
		m_lastPingMs = Time::GetMsTime() - timestamp;
		OnRecvPing(type, timestamp, _peer);
	}
	else
	{
		assert(false);
	}

	return true;
}

netBool PingService::Pull(SerializerLess &_ser, Peer& _peer)
{
	(void)_ser;
	(void)_peer;

	return true;
}

netBool	PingService::Push(SerializerLess &_ser, const Peer& _peer)
{
	return UnPack(_ser, _peer);
}











}