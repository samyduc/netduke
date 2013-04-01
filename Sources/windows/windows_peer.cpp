#include "peer.h"


namespace NetDuke
{


bool Peer::operator==(const Peer &_peer) const
{
	return m_addr.sin_addr.S_un.S_addr == _peer.m_addr.sin_addr.S_un.S_addr && m_addr.sin_port == _peer.m_addr.sin_port;
}

bool Peer::operator<(const Peer &_peer) const
{
	return m_addr.sin_addr.S_un.S_addr < _peer.m_addr.sin_addr.S_un.S_addr || (m_addr.sin_addr.S_un.S_addr == _peer.m_addr.sin_addr.S_un.S_addr && m_addr.sin_port < _peer.m_addr.sin_port);
}





};