#pragma once

#include "netdef.h"
#include "peer.h"
#include "serializerless.h"

#include <queue>

namespace NetDuke
{

class Serializer;

class Channel
{
public:
	explicit Channel(const Peer& _peer);
	~Channel() {}

	void				Push(Serializer& _serializer);
	void				Push(SerializerLess& _serializer);
	netBool				Pop(SerializerLess& _ser);

	SerializerLess*		Front();
	SerializerLess*		Back();

	size_t				FrontSize() const;
	size_t				BackSize() const;

	bool				IsEmpty() const {return m_serializers.empty();}
	const Peer&			GetPeer() const {return m_peer;}

private:
	Peer				m_peer;

	typedef std::queue<SerializerLess> serializers_t;
	serializers_t		m_serializers;


};

};