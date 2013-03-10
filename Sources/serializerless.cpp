#include "serializerless.h"
#include "serializer.h"

namespace NetDuke
{

SerializerLess::SerializerLess()
	: Serializer()
	, m_ref(nullptr)
{

}

SerializerLess::SerializerLess(Serializer& _ser)
	: Serializer()
	, m_ref(nullptr)
{
	//m_buffer = _ser.m_buffer + _ser.GetCursor();
	//m_buffer_size = _ser.GetBufferSize() - _ser.GetCursor();

	SliceBuffer(_ser, _ser.GetCursor(), _ser.GetBufferSize());
}

SerializerLess::SerializerLess(const SerializerLess& _ser)
	: Serializer()
{
	Copy(_ser);
}

SerializerLess::SerializerLess(Serializer& _ser, size_t _size)
	: Serializer()
	, m_ref(nullptr)
{
	SliceBuffer(_ser, _size);
}

SerializerLess::SerializerLess(Serializer& _ser, size_t _sizeA, size_t _sizeB)
	: Serializer()
	, m_ref(nullptr)
{
	SliceBuffer(_ser, _sizeA, _sizeB);
}

SerializerLess::~SerializerLess()
{
	if(m_ref)
	{
		DecRef();
	}
}

void SerializerLess::SliceBuffer(Serializer& _ser, size_t _size)
{
	assert(_ser.GetBufferSize() - _ser.GetCursor() >= _size);

	m_buffer = _ser.m_buffer + _ser.GetCursor();
	m_buffer_size = _size;

	assert(m_ref == nullptr);
	m_ref = _ser.GetSerializer();
	IncRef();
}

void SerializerLess::SliceBuffer(Serializer& _ser, size_t _sizeA, size_t _sizeB)
{
	assert(_sizeB > _sizeA);
	assert(_ser.GetBufferSize() >= _sizeB - _sizeA);

	m_buffer = _ser.m_buffer + _sizeA;
	m_buffer_size = _sizeB - _sizeA;

	assert(m_ref == nullptr);
	m_ref = _ser.GetSerializer();
	IncRef();
}

SerializerLess&	SerializerLess::operator=(const SerializerLess &_copy)
{
	Copy(_copy);
	return *this;
}

SerializerLess& SerializerLess::operator >>(SerializerLess& _ser)
{
	_ser.ResetCursor();

	// read type and length
	netU32 length;
	netU32 type;

	size_t old_cursor = GetCursor();

	static_cast<Serializer&>(*this) >> length;
	static_cast<Serializer&>(*this) >> type;

	SetCursor(old_cursor);

	_ser.SliceBuffer(*this, length);
	SetCursor(GetCursor() + length);

	_ser.m_ref = m_ref;
	//_ser.IncRef(); // IncRef done in slice buffers

	return *this;
}

void SerializerLess::Copy(const SerializerLess &_copy)
{
	m_ref = _copy.m_ref;
	m_buffer_size = _copy.m_buffer_size;
	m_cursor_pos = _copy.m_cursor_pos;
	m_buffer = _copy.m_buffer;
	m_owner = _copy.m_owner;
	m_ref_nb = _copy.m_ref_nb;
	m_state = _copy.m_state;

	IncRef();
}

void SerializerLess::IncRef()
{ 
	m_ref->IncRef(); 
}

void SerializerLess::DecRef()
{
	m_ref->DecRef();
}
size_t SerializerLess::GetRef()
{ 
	return m_ref->GetRef();
}

};