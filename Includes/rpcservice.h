#pragma once

#include "netdef.h"

#include "service.h"
#include "crc.h"
#include "timeplatform.h"
#include "rpc.h"

#include <list>

namespace NetDuke
{

class NetDuke;
class Peer;

static const netU32 s_RPCService = CRC32::Compute("RPCService");

class RPCService : public Service
{
public:

	explicit	RPCService(NetDuke* _netduke);
				~RPCService();

	void		Init();
	void		DeInit();

	netU32		GetType() const { return s_RPCService; }
	void		Tick();

	void		Send(RPC& _rpc, const Peer& _peer);
	netBool		Recv(SerializerLess& _ser, const Peer& _peer);
	netBool		RecvHandler(SerializerLess& _ser, Peer& _peer) { return Recv(_ser, _peer); }

private:
	netBool		CheckTimeOut(RPC& _rpc);
	netU8		GetNextSequence();

	netBool		RecvOut(RPC& _rpc, SerializerLess& _ser);

private:
	
	typedef std::list<RPC*> rpcs_t;
	rpcs_t m_rpcs;

	netU8	m_seq;

};





}