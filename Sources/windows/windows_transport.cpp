#include "transport.h"



namespace NetDuke
{


bool Transport::InitPlatformPrivate()
{
	WSADATA wsa;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa);

	return result == 0;
}

bool Transport::DesInitPlatformPrivate()
{
	WSACleanup();
	return true;
}





};