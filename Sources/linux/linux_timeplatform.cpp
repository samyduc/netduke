#include "timeplatform.h"

#include <time.h>
#include <sys/time.h>


namespace NetDuke
{


timer_t Time::GetMsTime()
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);

	return tp.tv_sec*1000 + tp.tv_nsec / 1000000;
}








};
