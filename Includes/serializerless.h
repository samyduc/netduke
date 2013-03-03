#pragma once
#include "netdef.h"

#include "serializer.h"

namespace NetDuke
{

class Serializer;

class SerializerLess : public Serializer
{

public:

					SerializerLess();
					SerializerLess(const SerializerLess& _ser);
					SerializerLess(Serializer& _ser);
					SerializerLess(Serializer& _ser, size_t _size);
					SerializerLess(Serializer& _ser, size_t _sizeA, size_t _sizeB);

	virtual			~SerializerLess();

	SerializerLess&	operator=(const SerializerLess &_copy);
	SerializerLess& operator >>(SerializerLess& _ser);

	void			SliceBuffer(Serializer& _ser, size_t _size);
	void			SliceBuffer(Serializer& _ser, size_t _sizeA, size_t _sizeB);

	virtual void	IncRef();
	virtual void	DecRef();
	virtual size_t	GetRef();
	Serializer*		GetSerializer() { return m_ref; }

	void			Copy(const SerializerLess &_copy);

private:
	Serializer* m_ref;
};


};