#pragma once

#include "netdef.h"
#include "layer.h"

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#endif

namespace NetDuke
{

class Peer;
class Listener;

class Stream : public Layer
{
public:

	virtual void			CreateAndBind() = 0;

	virtual netBool			IsValid() const = 0;
	virtual const Peer&		GetPeer() const = 0;
	virtual SOCKET			GetSocket() const = 0;

	virtual netBool			AttachListener(Listener& _listener) = 0;
	virtual netBool			DetachListener(Listener& _listener) = 0;
};




};