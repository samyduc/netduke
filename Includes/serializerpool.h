#pragma once
#include "netdef.h"

#include <vector>

namespace NetDuke
{

class Serializer;

class SerializerPool
{
public: 
	explicit		SerializerPool(size_t _size);
					~SerializerPool();

	Serializer&		GetSerializer();

private:
	void			Switch();

private:
	typedef std::vector<Serializer*> serializers_t;
	serializers_t	m_serializers;

	size_t			m_internalCursor;

};






};