#pragma once

#include "netdef.h"

#include "service.h"
#include "crc.h"
#include "timeplatform.h"
#include "rpc.h"

#include <list>
#include <map>
#include <queue>

namespace NetDuke
{

class NetDuke;
class Peer;

static const netU32 s_RPCService = CRC32::Compute("RPCService");

typedef std::queue<RPC*> rpcsQueue_t;

struct RPCChannel
{
	RPCChannel(const Peer& _peer) : m_peer(_peer) {}
	Peer m_peer;
	rpcsQueue_t m_rpcs;
};

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
	netBool		RecvOut(RPC& _rpc, SerializerLess& _ser);

	struct RPCChannel* GetRPCChannel(const Peer& _peer);

private:
	
	typedef std::map<Peer, struct RPCChannel*> rpcChannel_t;
	rpcChannel_t m_rpcs;

};





}