#include "timeplatform.h"

#include <time.h>
#include <sys/time.h>

#if defined(EMSCRIPTEN_TARGET)
#include <emscripten/emscripten.h>
#include <math.h>
#endif

namespace NetDuke
{


timer_t Time::GetMsTime()
{
#if !defined(EMSCRIPTEN_TARGET)
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);

	return tp.tv_sec*1000 + tp.tv_nsec / 1000000;
#else
	return floor(emscripten_get_now());
#endif
}








};
