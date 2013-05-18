#include "tcpstream.h"

namespace NetDuke
{

TCPStream::~TCPStream()
{
	Flush();
	close(m_socket);
}



};
