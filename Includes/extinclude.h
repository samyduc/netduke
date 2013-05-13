#pragma once

#if defined(WINDOWS_TARGET)

	// socket
	#include <winsock2.h>
	#include <ws2tcpip.h>
	//

#elif defined(LINUX_TARGET)

	// socket
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#define SOCKADDR struct sockaddr	
	#define SOCKADDR_IN struct sockaddr_in
	#define SOCKET int	
	//

#elif defined(EMSCRIPTEN_TARGET)

	#include <sys/types.h>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <net/arpa/inet.h>
	#include <net/netinet/in.h>
	#include <unistd.h>

	#define SOCKADDR struct sockaddr	
	#define SOCKADDR_IN struct sockaddr_in
	#define SOCKET int	

	#define INADDR_NONE             0xffffffff

#else
	#error "no include for this platform"
#endif