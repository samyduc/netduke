#pragma once

#include "netdef.h"
#include "peer.h"
#include "serializer.h"
#include "serializerLess.h"

#include "serializerpool.h"

#include <map>


namespace NetDuke
{

class Channel;

typedef std::map<Peer, Channel*> channels_t;

class Listener
{
	public:
		explicit				Listener(const Peer& _peer);
								~Listener();

		void					Push(const Serializer& _ser, const Peer& _peer);
		bool					Pull(SerializerLess& _ser, Peer& _peer);

		const Peer&				GetPeer() const {return m_peer;}

		bool					IsBind() const {return m_isBind;}

		void					Flush();
		void					FlushSend();
		void					FlushRecv();

		SOCKET					GetSocket() const { assert(m_isBind); return m_socket; };

	private:

		void					CreateAndBind();
		Channel&				GetSendChannel(const Peer& _peer);
		Channel&				GetRecvChannel(const Peer& _peer);
		Channel&				GetChannel(channels_t& channels, const Peer& _peer);
		void					SendTo(Serializer& _ser, const Peer& _peer);
		void					RecvFrom();

		void					Pack(const Serializer& _ser, Channel& _channel);
		void					UnPack(Serializer& _ser, const Peer& _peer);


	private:
		SerializerPool	m_pool;

		Peer			m_peer;
		SOCKET			m_socket;
		bool			m_isBind;

		channels_t		m_sendChannels;
		channels_t		m_recvChannels;
};


};