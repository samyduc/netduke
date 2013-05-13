#include "udpstream.h"

namespace NetDuke
{

UDPStream::~UDPStream()
{
	Flush();
	close(m_socket);
}



};
