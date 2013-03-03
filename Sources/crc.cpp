#include "crc.h"
#include <cstring>

namespace NetDuke
{

netU32 CRC32::Compute(const netU8* _buf, size_t _len)
{
	netU32 CRCVal = 0xffffffff;

	for(size_t i = 0; i < _len; ++i)
	{
		CRCVal = (CRCVal >> 8) ^ CRCTable[(CRCVal & 0xff) ^ _buf[i]];
	}
	CRCVal ^= 0xffffffff; 
	return CRCVal;
}

netU32 CRC32::Compute(const netChar* _str)
{
	return Compute(reinterpret_cast<const netU8*>(_str), strlen(_str));
}

};