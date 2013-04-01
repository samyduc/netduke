#include "udpstream.h"


namespace NetDuke
{

UDPStream::~UDPStream()
{
	Flush();
	closesocket(m_socket);
}



};