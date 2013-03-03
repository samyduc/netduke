#pragma once

namespace NetDuke
{

#ifdef _MSC_VER
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

	typedef			float		netF32;
	typedef			double		netF64;
#else
#error "Compiler not defined"
#endif



// platform declaration
#if defined(_WIN32)

#else
#error "Platform not defined"
#endif	

}