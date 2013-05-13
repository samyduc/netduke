#pragma once

//#include <cstddef> 
#include <stddef.h>
#include <stdint.h>
namespace NetDuke
{

#if defined(_MSC_VER)
	typedef			 bool		netBool;

	typedef unsigned char		netU8;
	typedef unsigned __int16	netU16;
	typedef unsigned __int32	netU32;
	typedef unsigned __int64	netU64;

	typedef	signed	 char		netS8;
	typedef			 char		netChar;
	typedef			 wchar_t	netWChar;
	typedef			 __int16	netS16;
	typedef			 __int32	netS32;
	typedef			 __int64	netS64;

	typedef			 float		netF32;
	typedef			 double		netF64;
#elif defined(__GNUC__)

	typedef			 bool		netBool;

	typedef unsigned char		netU8;
	typedef			 uint16_t	netU16;
	typedef			 uint32_t	netU32;
	typedef			 uint64_t	netU64;

	typedef	signed	 char		netS8;
	typedef			 char		netChar;
	typedef			 wchar_t	netWChar;
	typedef			 int16_t	netS16;
	typedef			 int32_t	netS32;
	typedef			 int64_t	netS64;

	typedef			 float		netF32;
	typedef			 double		netF64;

#else
#error "Compiler not defined"
#endif



// platform declaration
#if defined(_WIN32)

#elif defined(__gnu_linux__)

#else
#error "Platform not defined"
#endif	

}
