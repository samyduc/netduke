#include "peer.h"


namespace NetDuke
{


bool Peer::operator==(const Peer &_peer) const
{
	return m_addr.sin_addr.s_addr == _peer.m_addr.sin_addr.s_addr && m_addr.sin_port == _peer.m_addr.sin_port;
}

bool Peer::operator<(const Peer &_peer) const
{
	return m_addr.sin_addr.s_addr < _peer.m_addr.sin_addr.s_addr || (m_addr.sin_addr.s_addr == _peer.m_addr.sin_addr.s_addr && m_addr.sin_port < _peer.m_addr.sin_port);
}





};