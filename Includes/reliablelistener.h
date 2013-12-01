#pragma once

#include "netdef.h"
#include "layer.h"
#include "peer.h"
#include "listener.h"
#include "crc.h"
#include "serializerless.h"
#include "serializerpool.h"

#include <forward_list>
#include "timeplatform.h"

#include <assert.h>

namespace NetDuke
{

class Channel;
class Stream;

typedef netU16 ack_t;
typedef netU16 seq_t;

struct ReliableSendInfo
{
	ReliableSendInfo(seq_t _sequence, SerializerLess& _ser)
		: m_sequence(_sequence)
		, m_ser(_ser)
	{
		m_emissionTime = Time::GetMsTime();
	}

	seq_t			m_sequence;
	timer_t			m_emissionTime;
	SerializerLess	m_ser;
};

struct ReliableRecvInfo
{
	ReliableRecvInfo(seq_t _sequence, SerializerLess& _ser)
		: m_ser(_ser)
		, m_sequence(_sequence)
	{
	}

	bool operator<(const struct ReliableRecvInfo& _cmp) const
	{
		return m_sequence < _cmp.m_sequence;
	}

	seq_t			m_sequence;
	SerializerLess	m_ser;
};

typedef std::list<ReliableSendInfo>	reliableSendInfoList_t;
typedef	std::list<ReliableRecvInfo>	reliableRecvInfoList_t;

struct PlayerReliableInfo
{
	PlayerReliableInfo() { assert(false); }
	PlayerReliableInfo(const Peer& _peer)
		: m_peer(_peer)
		, m_currentSequence(1)
		, m_recvAck(0)
		, m_ackRecvTime(0)
	{
	}

	netU16 AcquireSequence()
	{
		seq_t sequence = m_currentSequence;

		if(m_currentSequence == 0xFFFF)
		{
			m_currentSequence = 1;
		}
		else
		{
			++m_currentSequence;
		}

		return sequence;
	}

	netU16 AcquireAck()
	{
		ack_t ack = m_recvAck;

		return ack;
	}

	void IncAck()
	{
		if(m_recvAck == 0xFFFF)
		{
			m_recvAck = 1;
		}
		else
		{
			++m_recvAck;
		}

		if(m_ackRecvTime == 0)
		{
			m_ackRecvTime = Time::GetMsTime();
		}
	}

	Peer						m_peer;
	seq_t						m_currentSequence;
	ack_t						m_recvAck;
	timer_t						m_ackRecvTime;
	reliableSendInfoList_t		m_send;
	reliableRecvInfoList_t		m_recv;
};
typedef struct PlayerReliableInfo PlayerReliableInfo_t;

typedef std::map<Peer, struct PlayerReliableInfo> reliableInfo_t;

class ReliableListener : public Listener
{
public:

							ReliableListener();
	virtual					~ReliableListener();

	void					Tick();
	netU32					GetType() const { return s_typeReliableListener; } 

	inline size_t			GetHeaderSize() const;

	netBool					Pull(SerializerLess &_ser, Peer& _peer);
	netBool					Push(SerializerLess &_ser, const Peer& _peer);

	netBool					PullFromStream(SerializerLess& _ser, const Peer& _peer);

	void					RegisterStream(Stream& _stream);
	void					UnRegisterStream(const Stream& _stream);

	void					SetTimeOut(timer_t _ms);
	void					DeletePeer(const Peer& _peer);

protected:
	netBool					Pack(SerializerLess& _ser, const Peer& _peer);
	netBool					UnPack(SerializerLess& _ser, const Peer& _peer);

	Channel&				GetChannel(channels_t& channels, const Peer& _peer);

	void					FlushSend();
	void					FlushTimeout();
	void					Flush();

	void					SendToStream(SerializerLess& _ser, const Peer &_peer);

	PlayerReliableInfo_t&	GetReliableInfo(const Peer& _peer);
	void					RecvAck(PlayerReliableInfo_t &_reliableInfo, ack_t _ack);
	netBool					PreUnpack(SerializerLess& _ser, const Peer& _peer);
	netBool					CompareSequence(PlayerReliableInfo_t &_reliableInfo, seq_t _sequence);
	netBool					CompareAck(seq_t _seq, ack_t _lastAck);
	netBool					RecvSequence(PlayerReliableInfo_t &_reliableInfo, SerializerLess& _less, seq_t _sequence);
	netBool					CheckTimeout(timer_t _start);

	// helper
	void					CreateBundle(Channel& _channel);

private:
	Stream*			m_stream;

	channels_t		m_sendChannels;
	channels_t		m_recvChannels;

	reliableInfo_t	m_reliableInfo;

	timer_t			m_maxClock;
	SerializerPool	m_pool;

};




};