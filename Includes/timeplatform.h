#pragma once

#include "netdef.h"

#if !defined(_WIN32)
	#include <sys/time.h>
#endif

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
#else
		struct timespec tp;
		clock_gettime(CLOCK_MONOTONIC, &tp);

		return tp.tv_sec*1000 + tp.tv_nsec / 1000000;
#endif
	}




};



};
