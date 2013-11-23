
#include "netduke.h"

#include "server.h"

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

const char *g_addr = "127.0.0.1";
//const char *g_addr = "88.191.134.22";

int main(void)
{
	NetDukeSample::PingPongServer server(15002);
	NetDuke::NetDuke &netduke = server.GetNetDuke();
	netduke.RegisterService(server);
	
	while(1)
	{
		netduke.Tick();
	}
	

	return 0;
}
