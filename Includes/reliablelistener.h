#pragma once

#include "netdef.h"
#include "layer.h"
#include "peer.h"
#include "listener.h"
#include "crc.h"
#include "serializerless.h"
#include "serializerpool.h"

#include <list>
#include <ctime>

#include <assert.h>

namespace NetDuke
{

class Channel;
class Stream;

struct ReliableSendInfo
{
	ReliableSendInfo(netU16 _sequence, SerializerLess& _ser)
		: m_sequence(_sequence)
		, m_ser(_ser)
	{
		m_emissionTime = clock();
	}

	netU16			m_sequence;
	clock_t			m_emissionTime;
	SerializerLess	m_ser;
};

struct ReliableRecvInfo
{
	ReliableRecvInfo(netU16 _sequence, SerializerLess& _ser)
		: m_ser(_ser)
		, m_sequence(_sequence)
	{
	}

	bool operator<(struct ReliableRecvInfo& _cmp) const
	{
		return m_sequence < _cmp.m_sequence;
	}

	netU16			m_sequence;
	SerializerLess	m_ser;
};

struct PlayerReliableInfo
{
	PlayerReliableInfo() { assert(false); }
	PlayerReliableInfo(const Peer& _peer)
		: m_peer(_peer)
		, m_currentSequence(1)
		, m_currentAck(0)
		, m_lastAck(0)
		, m_ackWaitTime(0)
	{
	}

	netU16 AcquireSequence()
	{
		netU16 sequence = m_currentSequence;
		m_currentSequence++;

		if(m_currentSequence == 0xFFFF)
		{
			m_currentSequence = 1;
		}

		return sequence;
	}

	netU16 AcquireAck()
	{
		netU16 ack = m_lastAck;
		m_currentAck = 0;
		m_ackWaitTime = 0;

		return ack;
	}

	void IncAck()
	{
		if(m_lastAck == 0xFFFF)
		{
			m_lastAck = 1;
			m_currentAck = 1;
		}
		else
		{
			++m_lastAck;
			++m_currentAck;
		}

		if(m_ackWaitTime == 0)
		{
			m_ackWaitTime = clock();
		}
	}

	Peer						m_peer;
	netU16						m_currentSequence;
	netU16						m_currentAck;
	netU16						m_lastAck;
	clock_t						m_ackWaitTime;
	std::list<ReliableSendInfo>	m_send;
	std::list<ReliableRecvInfo>	m_recv;
};
typedef struct PlayerReliableInfo PlayerReliableInfo_t;

typedef std::map<Peer, struct PlayerReliableInfo> reliableInfo_t;

class ReliableListener : public Listener
{
public:

							ReliableListener();
							~ReliableListener();

	void					Tick();
	netU32					GetType() const { return s_typeReliableListener; } 

	netBool					Pull(SerializerLess &_ser, Peer& _peer);
	netBool					Push(SerializerLess &_ser, const Peer& _peer);

	netBool					PullFromStream(SerializerLess& _ser, const Peer& _peer);

	void					RegisterStream(Stream& _stream);
	void					UnRegisterStream(const Stream& _stream);

	void					SetTimeOut(netU32 _ms);

protected:
	netBool					Pack(SerializerLess& _ser, const Peer& _peer);
	netBool					UnPack(SerializerLess& _ser, const Peer& _peer);

	Channel&				GetChannel(channels_t& channels, const Peer& _peer);

	void					FlushSend();
	void					FlushTimeout();
	void					Flush();

	PlayerReliableInfo_t&	GetReliableInfo(const Peer& _peer);
	void					RecvAck(PlayerReliableInfo_t &_reliableInfo, netU16 _ack);
	netBool					PreUnpack(SerializerLess& _ser, const Peer& _peer);
	netBool					CompareSequence(PlayerReliableInfo_t &_reliableInfo, netU16 _sequence);
	netBool					RecvSequence(PlayerReliableInfo_t &_reliableInfo, SerializerLess& _less, netU16 _sequence);
	netBool					CheckTimeout(clock_t _start);

	// helper
	void					CreateBundle(Channel& _channel);

private:
	Stream*			m_stream;

	channels_t		m_sendChannels;
	channels_t		m_recvChannels;

	reliableInfo_t	m_reliableInfo;

	clock_t			m_maxClock;
	SerializerPool	m_pool;

};




};