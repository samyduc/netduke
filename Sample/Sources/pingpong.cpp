#include "pingpong.h"

#include "serializerless.h"

#include <thread>
#include <mutex>
#include <iostream>

namespace NetDukeSample
{

static std::mutex g_globalLock;

void Log(char *fmt, ...)
{
	/*std::lock_guard<std::mutex> gLock(g_globalLock);

	va_list	va;
	va_start (va, fmt) ;
		//netVsprintf(s_logtxt,sizeof(s_logtxt)-1 , fmt, va) ;
		printf(fmt, va);
	va_end (va);*/
}

PingPongServer::PingPongServer(NetDuke::netU16 _port)
{
	m_transport.InitPlatform();

	NetDuke::Peer peer;
	peer.SetPort(_port);
	peer.SetIPv4Addr("0.0.0.0");
	m_transport.Listen(peer);
}

void PingPongServer::Tick()
{
	m_transport.Tick();

	NetDuke::Peer peer;

	// ping
	NetDuke::SerializerLess ser;
	
	while(m_transport.Pull(ser, peer))
	{
		// pong
		NetDuke::Serializer ser_copy(ser);
		m_transport.Send(ser_copy, peer, NetDuke::s_typeReliableListener);

		ser.ResetCursor();
	}
}

PingPongClient::PingPongClient(NetDuke::netChar* _addr, NetDuke::netU16 _port)
	: m_serializer(NetDuke::Serializer::MTU)
	, m_state(false)
	, m_round(0)
{
	m_transport.InitPlatform();

	NetDuke::Peer peer;
	peer.SetPort(0);
	peer.SetIPv4Addr("0.0.0.0");
	m_transport.Listen(peer);

	m_peer.SetIPv4Addr(_addr);
	m_peer.SetPort(_port);
}

void PingPongClient::Tick()
{
	m_transport.Tick();

	if(!m_state)
	{
		m_state = true;
		m_serializer.ResetCursor();
		m_serializer.Write("pingpong");
		m_serializer << m_round;
		m_serializer.Close();

		m_clock = clock();
		
		m_transport.Send(m_serializer, m_peer, NetDuke::s_typeReliableListener);
		// force bundling
		//NetDuke::Serializer serializer(m_serializer);
		//m_transport.Send(serializer, m_peer, NetDuke::s_typeReliableListener);
	}
	else
	{
		NetDuke::Peer peer;

		// ping
		NetDuke::SerializerLess ser;
		clock_t interval = clock() - m_clock;

		while(m_transport.Pull(ser, peer))
		{		
			m_state = false;

			NetDuke::netU32 round;
			NetDuke::netBool ret = m_serializer.Read("pingpong");
			assert(ret);

			m_serializer >> round;
			m_serializer.Close();

			assert(round == m_round);

			if(m_round == 0xFFFFFFFF)
			{
				m_round = 0;
			}
			else
			{
				++m_round;
			}
			// log recv
			std::lock_guard<std::mutex> gLock(g_globalLock);
			std::thread::id id = std::this_thread::get_id();
			printf("threadId:%llu, clicks :%llu clicks seconds :%f \n", id, interval, ((double)interval)/CLOCKS_PER_SEC);
		}

		/*if(interval > 1000)
		{
			std::lock_guard<std::mutex> gLock(g_globalLock);
			printf("threadId:%llu, retry \n", std::this_thread::get_id());
			m_state = false;
		}*/
	}
}



};