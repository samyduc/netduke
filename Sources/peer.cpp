#include "peer.h"

namespace NetDuke
{

Peer::Peer()
{
}

Peer::Peer(netChar* _ipv4, netU16 _port)
{
	SetIPv4Addr(_ipv4);
	SetPort(_port);
}

Peer::Peer(const Peer& _peer)
{
	m_addr.sin_family = _peer.m_addr.sin_family;
	m_addr.sin_addr.s_addr = _peer.m_addr.sin_addr.s_addr;
	m_addr.sin_port = _peer.m_addr.sin_port;
}

Peer& Peer::operator=(const Peer &_peer)
{
	m_addr.sin_family = _peer.m_addr.sin_family;
	m_addr.sin_addr.s_addr = _peer.m_addr.sin_addr.s_addr;
	m_addr.sin_port = _peer.m_addr.sin_port;

	return *this;
}

const SOCKADDR_IN &Peer::GetNativeStruct() const
{
	return m_addr;
}

bool Peer::SetIPv4Addr(netU32 ipv4)
{
	m_addr.sin_family = AF_INET; 
	m_addr.sin_addr.s_addr = htonl(ipv4);

	return true;
}

bool Peer::SetIPv4Addr(netChar *ip_char)
{
	m_addr.sin_family = AF_INET; 
	m_addr.sin_addr.s_addr = inet_addr(ip_char);

	return m_addr.sin_addr.s_addr != INADDR_NONE;
}

netU32 Peer::GetIPv4Addr() const
{
	return ntohl(m_addr.sin_addr.s_addr);
}

void Peer::SetPort(netU16 _port)
{
	m_addr.sin_port = htons(_port);
}

netU16 Peer::GetPort() const
{
	return ntohs(m_addr.sin_port);
}

bool Peer::IsIPv4() const
{
	return m_addr.sin_family == AF_INET;
}

bool Peer::IsIPv6() const
{
	return m_addr.sin_family == AF_INET6;
}

};
