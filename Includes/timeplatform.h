#pragma once

#include "netdef.h"

namespace NetDuke
{

typedef netU64 timer_t;

class Time
{

public:

	static timer_t GetMsTime()
	{
#ifdef _WIN32
		return GetTickCount64();
#endif
	}




};



};