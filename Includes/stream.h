#pragma once

#include "netdef.h"
#include "peer.h"

namespace NetDuke
{

class Stream
{
public:
	explicit	Stream(const Peer& _peer);
	virtual		~Stream();

	netBool		IsBind() const { return m_isBind; }


private:
	netBool m_isBind;

}






};