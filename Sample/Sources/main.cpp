
#include "netduke.h"

#include "pingpong.h"

#include <iostream>
#include <thread>
#include <list>


#if !defined(_WIN32)
	#include <unistd.h>
	#define Sleep sleep
#endif

#if defined(EMSCRIPTEN_TARGET)
#include <emscripten.h>
#endif

void threaded_server()
{
	NetDukeSample::PingPongServer server(15002);
	NetDuke::NetDuke & net_duke = server.GetNetDuke();

	net_duke.RegisterService(server);

	bool bCond = true;
	while(bCond)
	{
		net_duke.Tick();
		Sleep(1);
	}

	net_duke.UnRegisterService(server);
}

void threaded_client()
{
	NetDukeSample::PingPongClient client("127.0.0.1", 15002);
	NetDuke::NetDuke & net_duke = client.GetNetDuke();

	net_duke.RegisterService(client);

	bool bCond = true;
	while(bCond)
	{
		net_duke.Tick();
		Sleep(1);
	}

	net_duke.UnRegisterService(client);
}

NetDuke::NetDuke *g_duke;
void one_iter();

const char *g_addr = "127.0.0.1";
//const char *g_addr = "88.191.134.22";

int main(void)
{
	// test serializer
	/*NetDuke::Serializer ser(1400);
	NetDuke::Serializer to_write(200);
	to_write.Write("tata");
	to_write << 3;
	to_write.Close();

	ser.Write("gerard");
	ser << 32;
	ser << 2;
	ser << to_write;
	ser.Close();

	int a;

	std::cout << ser.Read("gerard") << std::endl;
	ser >> a;
	std::cout << a << std::endl;
	ser >> a;
	std::cout << a << std::endl;
	ser >> to_write;

	ser.Close();

	to_write.Read("tata");
	to_write >> a;
	std::cout << a << std::endl;
	to_write.Close();

	std::cout << ser.GetType() << std::endl;

	// test transport
	NetDuke::Transport transport;
	transport.InitPlatform();

	// test : 1 -> 2
	NetDuke::Peer peer_listen1;
	peer_listen1.SetPort(15000);
	peer_listen1.SetIPv4Addr("0.0.0.0");

	NetDuke::Peer peer_listen2;
	peer_listen2.SetPort(15001);
	peer_listen2.SetIPv4Addr("127.0.0.1");

	NetDuke::Peer peer;
	peer.SetPort(15001);
	peer.SetIPv4Addr("127.0.0.1");

	NetDuke::Listener *listen1 = transport.Listen(peer_listen1);
	NetDuke::Listener *listen2 = transport.Listen(peer_listen2);

	transport.Send(ser, peer, *listen1);

	while(true)
	{
		transport.Tick();

		
	}*/

	// thread
	std::thread t1(threaded_server);
	Sleep(50);

	std::list<std::thread> threads;
	for(size_t i = 0; i<5; ++i)
	{
		threads.push_back(std::thread(threaded_client));
	}

	//t1.join();
	threads.front().join();

	printf("helloworld");

	// a la mano
	/*printf("start\n");

	NetDuke::NetDuke netduke;
	netduke.Init();

	NetDuke::Transport& transport = netduke.GetTransport();

	NetDuke::Peer peer("0.0.0.0", 0);
	transport.InitTCPStack(peer);
	//transport.InitUDPStack(peer);

	printf("init tcp stack\n");

	NetDuke::Peer peer_to(g_addr, 8090);

	printf("write serializer\n");

	NetDuke::Serializer ser(NetDuke::Serializer::MTU);
	ser.Write(12);
	ser << 0;
	ser << NetDuke::Time::GetMsTime();
	ser.Close();

	printf("end write serializer\n");

	transport.Send(ser, peer_to, NetDuke::s_typeUnreliableListener);

	printf("send data\n");

	netduke.Tick();
	netduke.Tick();

	g_duke = &netduke;

#if defined(EMSCRIPTEN_TARGET)
	emscripten_set_main_loop(one_iter, 1000000, true);
#else
	while(true)
	{
		one_iter();
		Sleep(1);
	}
#endif
	*/
	

	return 0;
}

NetDuke::timer_t last_call = 0;

void one_iter()
{

	//printf("iteration: %lld\n", NetDuke::Time::GetMsTime() - last_call);
	//last_call = NetDuke::Time::GetMsTime();

	for(size_t i = 0; i<25; ++i)
	{

	NetDuke::Peer peer_to(g_addr, 8090);
	g_duke->Tick();


	NetDuke::Transport &transport = g_duke->GetTransport();

	NetDuke::SerializerLess ser;
	NetDuke::Peer peer;

	static NetDuke::timer_t start_time = NetDuke::Time::GetMsTime();
	
	while(transport.Pull(ser, peer))
	{
		static NetDuke::timer_t timer;
		ser.Read(12);
		int seq;
		static_cast<NetDuke::Serializer&>(ser) >> seq;
		static_cast<NetDuke::Serializer&>(ser) >> timer;
		ser.Close();

		if(seq % 100 == 0)
		{
			NetDuke::timer_t end_time = NetDuke::Time::GetMsTime() - start_time;
			printf("bench %f\n", end_time / double(seq));
			start_time = NetDuke::Time::GetMsTime();
			seq = 0;
		}

		NetDuke::Serializer serSeq(NetDuke::Serializer::MTU);
		serSeq.Write(12);
		serSeq << ++seq;
		serSeq << NetDuke::Time::GetMsTime();
		serSeq.Close();

		//printf("%lld\n", NetDuke::Time::GetMsTime());
		//printf("pong %d, %lld\n", seq, NetDuke::Time::GetMsTime() - timer);

		transport.Send(serSeq, peer_to, NetDuke::s_typeUnreliableListener);
	}

	}
}
