#include "tcpstream.h"



namespace NetDuke
{

TCPStream::~TCPStream()
{
	Flush();
	closesocket(m_socket);
}


};