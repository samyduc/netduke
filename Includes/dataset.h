#pragma once

#include "netdef.h"



namespace NetDuke
{

class Serializer;

class Dataset
{
public:
	Dataset()
	: m_isDirty(false)
	{
	}

	netBool			IsDirty() const { return m_isDirty; }
	void			SetDirty() { m_isDirty = true; }
	void			SetClean() { m_isDirty = false; }

	virtual netU32	GetType() const = 0;
	virtual void	Write(Serializer& _ser) = 0;
	virtual void	Read(Serializer& _ser) = 0;

private:
	netBool m_isDirty; 

};


}