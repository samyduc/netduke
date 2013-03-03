#include "serializer.h"
#include "serializerLess.h"
#include "crc.h"

namespace NetDuke
{

#ifdef _MSC_VER
	#define SWAP_16(x) _byteswap_ushort(x)
	#define SWAP_32(x) _byteswap_ulong(x)
	#define SWAP_64(x) _byteswap_uint64(x)
#else
	#define SWAP_16(x) ((x & 0xFF00) >> 8 | (x & 0x00FF) << 8)

	#define SWAP_32(x) (((x & 0xFF000000) >> 24)| \
                       ((x & 0x00FF0000) >>  8) | \
                       ((x & 0x0000FF00) <<  8) | \
                       ((x & 0x000000FF) << 24)) 

	#define SWAP_64(x)	(((x & 0x00000000000000FF) << 56) | \
					    ((x &  0x000000000000FF00) << 40) | \
					    ((x &  0x0000000000FF0000) << 24) | \
					    ((x &  0x00000000FF000000) << 8) | \
					    ((x &  0x000000FF00000000) >> 8) | \
					    ((x &  0x0000FF0000000000) >> 24)| \
					    ((x &  0x00FF000000000000) >> 40)| \
					    ((x &  0xFF00000000000000) >> 56))

#endif

// use only with inherited class
Serializer::Serializer()
 	: m_cursor_pos(0)
	, m_buffer_size(0)
	, m_state(E_IDLE)
	, m_buffer(nullptr)
	, m_owner(false)
	, m_ref_nb(0)
{
}

Serializer::Serializer(size_t size)
	: m_cursor_pos(0)
	, m_buffer_size(size)
	, m_state(E_IDLE)
	, m_owner(true)
	, m_ref_nb(0)
{
	m_buffer = new netU8[size];
}

Serializer::Serializer(const Serializer& _serializer)
	: m_cursor_pos(0)
	, m_buffer_size(_serializer.GetSize())
	, m_state(E_IDLE)
	, m_owner(true)
	, m_ref_nb(0)
{
	m_buffer = new netU8[_serializer.GetSize()];

	Copy(_serializer.m_buffer, _serializer.GetSize());
}

Serializer::~Serializer()
{
	if(m_owner)
	{
		assert(m_ref_nb == 0);
		delete m_buffer;
	}
}

size_t Serializer::GetSize() const
{
	netU32 len;
	memcpy(&len, &m_buffer[s_header_pos_length], sizeof(netU32));

	if(s_IsBigEndian)
	{
		len = SWAP_32(len);
	}

	return static_cast<size_t>(len);
}

netU32 Serializer::GetType() const
{
	netU32 type;
	memcpy(&type, &m_buffer[s_header_pos_type], sizeof(netU32));

	if(s_IsBigEndian)
	{
		type = SWAP_32(type);
	}

	return type;
}

void Serializer::Copy(const netU8* const _copy, size_t _copy_size)
{
	assert(_copy_size <= m_buffer_size);

	ResetCursor();
	memcpy(m_buffer, _copy, _copy_size);
}

void Serializer::SetCursor(size_t _pos)
{
	m_cursor_pos = _pos;
}

void Serializer::ResetCursor()
{
	m_cursor_pos = 0;
}

netBool Serializer::Write(netU32 _type)
{
	assert(m_state == E_IDLE);

	ResetCursor();
	m_state = E_WRITE;

	*this << static_cast<netU32>(0);
	*this << _type;

	return m_state == E_WRITE;
}

netBool Serializer::Write(netChar* _type)
{
	return Write(CRC32::Compute(_type));
}

void Serializer::Close()
{
	assert(m_state == E_WRITE || m_state == E_READ);

	if(m_state == E_WRITE)
	{
		// compute csksum && length
		size_t save_cursor = GetCursor();
		ResetCursor();

		// cast size_t for x64
		*this << static_cast<netU32>(save_cursor);

		SetCursor(save_cursor);
	}

	m_state = E_IDLE;
}

netBool Serializer::Read(netU32 _type)
{
	assert(m_state == E_IDLE);

	ResetCursor();
	m_state = E_READ;

	netU32 length, type;

	*this >> length;
	*this >> type;

	if(type != _type || length > m_buffer_size)
	{
		Close();
	}
		
	return m_state == E_READ;
}

netBool Serializer::Read(netChar* _type)
{
	return Read(CRC32::Compute(_type));
}

Serializer&	Serializer::operator=(const Serializer &_copy)
{
	if(m_owner)
	{
		if(m_buffer != nullptr)
		{
			delete m_buffer;
		}

		m_buffer = new netU8[_copy.m_buffer_size];
		Copy(_copy.m_buffer, _copy.m_buffer_size);
	}

	return *this;
}

Serializer& Serializer::operator <<(const netU8 _val)
{
	assert(m_state == E_WRITE);

	m_buffer[m_cursor_pos] = _val;
	m_cursor_pos += sizeof(netU8);

	assert(m_cursor_pos <= m_buffer_size);

	return *this;
}

Serializer& Serializer::operator <<(netS8 _val)
{
	*this << static_cast<netU8>(_val);

	return *this;
}

Serializer& Serializer::operator <<(netChar _val)
{
	*this << static_cast<netU8>(_val);

	return *this;
}

Serializer& Serializer::operator <<(netU16 _val)
{
	assert(m_state == E_WRITE);

	if(s_IsBigEndian)
	{
		_val = SWAP_16(_val);
	}

	memcpy(&m_buffer[m_cursor_pos], &_val, sizeof(netU16));

	m_cursor_pos += sizeof(netU16);

	assert(m_cursor_pos <= m_buffer_size);

	return *this;
}

Serializer& Serializer::operator <<(netS16 _val)
{
	*this << static_cast<netU16>(_val);

	return *this;
}

Serializer& Serializer::operator <<(netU32 _val)
{
	assert(m_state == E_WRITE);

	if(s_IsBigEndian)
	{
		_val = SWAP_32(_val);
	}

	memcpy(&m_buffer[m_cursor_pos], &_val, sizeof(netU32));

	m_cursor_pos += sizeof(netU32);

	assert(m_cursor_pos <= m_buffer_size);

	return *this;
}

Serializer& Serializer::operator <<(netS32 _val)
{
	*this << static_cast<netU32>(_val);

	return *this;
}

Serializer& Serializer::operator <<(netU64 _val)
{
	assert(m_state == E_WRITE);

	if(s_IsBigEndian)
	{
		_val = SWAP_64(_val);
	}

	memcpy(&m_buffer[m_cursor_pos], &_val, sizeof(netU64));

	m_cursor_pos += sizeof(netU64);

	assert(m_cursor_pos <= m_buffer_size);

	return *this;
}

Serializer& Serializer::operator <<(netS64 _val)
{
	*this << static_cast<netU64>(_val);

	return *this;
}

Serializer& Serializer::operator <<(netF32 _val)
{
	// not implemented
	assert(false);

	(void)_val;

	return *this;
}

Serializer& Serializer::operator <<(netF64 _val)
{
	// not implemented
	assert(false);

	(void)_val;

	return *this;
}

Serializer& Serializer::operator <<(const Serializer& _copy)
{
	assert(m_state == E_WRITE);

	assert(m_cursor_pos + _copy.GetSize() <= m_buffer_size);
	
	memcpy(m_buffer + m_cursor_pos, _copy.GetBuffer(), _copy.GetSize());
	m_cursor_pos += _copy.GetSize();

	return *this;
}

Serializer& Serializer::operator >>(netU8& _val)
{
	assert(m_state == E_READ);

	_val = m_buffer[m_cursor_pos];
	m_cursor_pos += sizeof(netU8);

	return *this;
}

Serializer& Serializer::operator >>(netS8& _val)
{
	*this >> reinterpret_cast<netU8&>(_val);

	return *this;
}

Serializer& Serializer::operator >>(netChar& _val)
{
	*this >> reinterpret_cast<netU8&>(_val);

	return *this;
}

Serializer& Serializer::operator >>(netU16& _val)
{
	assert(m_state == E_READ);

	memcpy(&_val, &m_buffer[m_cursor_pos], sizeof(netU16));
	m_cursor_pos += sizeof(netU16);

	if(s_IsBigEndian)
	{
		_val = SWAP_16(_val);
	}

	return *this;
}

Serializer& Serializer::operator >>(netS16& _val)
{
	*this >> reinterpret_cast<netU16&>(_val);

	return *this;
}

Serializer& Serializer::operator >>(netU32& _val)
{
	assert(m_state == E_READ);

	memcpy(&_val, &m_buffer[m_cursor_pos], sizeof(netU32));
	m_cursor_pos += sizeof(netU32);

	if(s_IsBigEndian)
	{
		_val = SWAP_32(_val);
	}

	return *this;
}

Serializer& Serializer::operator >>(netS32& _val)
{
	*this >> reinterpret_cast<netU32&>(_val);

	return *this;
}

Serializer& Serializer::operator >>(netU64& _val)
{
	assert(m_state == E_READ);

	memcpy(&_val, &m_buffer[m_cursor_pos], sizeof(netU64));
	m_cursor_pos += sizeof(netU64);

	if(s_IsBigEndian)
	{
		_val = SWAP_64(_val);
	}

	return *this;
}

Serializer& Serializer::operator >>(netS64& _val)
{
	*this >> reinterpret_cast<netU64&>(_val);

	return *this;
}

Serializer& Serializer::operator >>(netF32& _val)
{
	// not implemented
	assert(false);

	(void)_val;

	return *this;
}

Serializer& Serializer::operator >>(netF64& _val)
{
	// not implemented
	assert(false);

	(void)_val;

	return *this;
}

Serializer& Serializer::operator >>(Serializer& _copy)
{
	assert(m_state == E_READ);

	_copy.ResetCursor();

	// read type and length
	netU32 length;
	netU32 type;

	size_t old_cursor = GetCursor();

	*this >> length;
	*this >> type;

	SetCursor(old_cursor);

	assert(length < _copy.GetBufferSize());

	memcpy(_copy.m_buffer, m_buffer + m_cursor_pos, length);
	SetCursor(GetCursor() + length);

	return *this;
}

Serializer& Serializer::operator >>(SerializerLess& _read)
{
	//assert(m_state == E_READ);

	_read.ResetCursor();

	// read type and length
	netU32 length;
	netU32 type;

	size_t old_cursor = GetCursor();

	*this >> length;
	*this >> type;

	SetCursor(old_cursor);

	_read.SliceBuffer(*this, length);
	SetCursor(GetCursor() + length);

	return *this;
}

};