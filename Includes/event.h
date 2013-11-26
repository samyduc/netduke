#pragma once


#include "netdef.h"
#include "serializer.h"
#include "serializerless.h"
#include "dataset.h"
#include "timeplatform.h"

namespace NetDuke
{

class Event
{
public:

	friend class EventService;

	Event()
		: m_ser(Serializer::MTU)
	{
	}

	virtual netU32		GetType() const = 0;

	virtual Dataset&	In() = 0;

	netBool				Serialize(Dataset& _data)
						{ 
							if(m_ser.Write(GetType()))
							{
								m_ser << _data.GetType();
								_data.Write(m_ser); 
								m_ser.Close();
								return true;
							}
							return false;
						}

	netBool				UnSerialize(Dataset& _data, SerializerLess& _ser)
						{ 
							netBool ret = false;
							if(_ser.Read(GetType()))
							{
								netU32 type;
								static_cast<Serializer&>(_ser) >> type;

								if(type == _data.GetType())
								{
									_data.Read(_ser); 
									ret = true;
								}
								_ser.Close();
							}

							return ret;
						}

	Serializer&			GetSerializer() { return m_ser; }

	void				Reset()
	{
		m_ser.ResetCursor();
	}

private:
	Serializer	m_ser;

};



}