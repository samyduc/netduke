
#include "transport.h"
#include "serializer.h"
#include "peer.h"
#include "listener.h"

#include "pingpong.h"
#include <iostream>
#include <thread>


void threaded_server()
{
	NetDukeSample::PingPongServer server(15002);

	bool bCond = true;
	while(bCond)
	{
		server.Tick();
		Sleep(1);
	}
}

void threaded_client()
{
	NetDukeSample::PingPongClient client("127.0.0.1", 15002);

	bool bCond = true;
	while(bCond)
	{
		client.Tick();
		Sleep(1);
	}
}

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
	Sleep(150);
	std::thread t1(threaded_server);
	Sleep(50);
	std::thread t2(threaded_client);
	std::thread t3(threaded_client);
	std::thread t4(threaded_client);
	std::thread t5(threaded_client);

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();

	//threaded_server();
	//threaded_client();

	return 0;
}