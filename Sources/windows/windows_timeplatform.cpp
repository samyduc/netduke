#include "timeplatform.h"

#include <Windows.h>


namespace NetDuke
{

timer_t Time::GetMsTime()
{
	return GetTickCount64();
}


};