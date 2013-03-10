#pragma once

#include "netdef.h"

#include <algorithm>
#include <assert.h>

namespace NetDuke
{

enum eSerializerState
{
	E_IDLE,
	E_WRITE,
	E_READ
};

// we are little endian
static const netS32		s_OrderedInt		= 0x01020304;
static const netS8		*s_orderedChar		= reinterpret_cast<const netS8*>(&s_OrderedInt);
static const netBool	s_IsBigEndian		= s_orderedChar[0] == 0x01;

static const netU32		s_header_pos_length	= 0;
static const netU32		s_header_pos_type	= s_header_pos_length + sizeof(netU32);
static const netU32		s_header_pos		= s_header_pos_length + s_header_pos_type + sizeof(netU32);

class SerializerLess;

class Serializer
{
public:

	friend	class SerializerLess;

	explicit		Serializer(size_t);
	explicit		Serializer(const Serializer&);
	virtual			~Serializer();

	static size_t   GetHeaderSize() { return s_header_pos; }

	size_t			GetSize() const;
	netU32			GetType() const;
	size_t			GetBufferSize() const { return m_buffer_size; }
	const netU8*	GetBuffer() const { return m_buffer; }
	netU8*			GetRawBuffer() const { return m_buffer; }
	void			SetCursor(size_t _pos);
	size_t			GetCursor() const { return m_cursor_pos; }
	void			ResetCursor();

	void			Copy(const netU8* const _copy, size_t _copy_size);

	netBool			Write(netU32 _type);
	netBool			Write(netChar* _type);

	netBool			Read(netU32 _type);
	netBool			Read(netChar* _type);

	void			Close();

	virtual Serializer&	operator=(const Serializer &_copy);

	Serializer& operator <<(netU8);
	Serializer& operator <<(netS8);
	Serializer& operator <<(netChar);
	Serializer& operator <<(netU16);
	Serializer& operator <<(netS16);
	Serializer& operator <<(netU32);
	Serializer& operator <<(netS32);
	Serializer& operator <<(netU64);
	Serializer& operator <<(netS64);
	Serializer& operator <<(netF32);
	Serializer& operator <<(netF64);
	Serializer& operator <<(const Serializer&);

	Serializer& operator >>(netU8&);
	Serializer& operator >>(netS8&);
	Serializer& operator >>(netChar&);
	Serializer& operator >>(netU16&);
	Serializer& operator >>(netS16&);
	Serializer& operator >>(netU32&);
	Serializer& operator >>(netS32&);
	Serializer& operator >>(netU64&);
	Serializer& operator >>(netS64&);
	Serializer& operator >>(netF32&);
	Serializer& operator >>(netF64&);
	Serializer& operator >>(Serializer&);
	Serializer& operator >>(SerializerLess&);

	// ref counting
	virtual void		IncRef() { ++m_ref_nb; }
	virtual void		DecRef() { assert(m_ref_nb > 0); --m_ref_nb; /*if(m_ref_nb == 0) delete this;*/}
	virtual size_t		GetRef() { return m_ref_nb; }

	virtual Serializer*	GetSerializer() { return this; }

public:
	const static size_t MTU = 1400;


protected:
			Serializer();
	void	SetRef(Serializer* _ref);

protected:

	eSerializerState	m_state;
	netU8*				m_buffer;
	netBool				m_owner;

	size_t				m_buffer_size;
	size_t				m_cursor_pos;

	size_t				m_ref_nb;

};

}