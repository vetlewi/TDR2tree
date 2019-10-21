#include "Parameters/XIA_CFD.h"



#include <stdlib.h>
#include <stdint.h>

#define BIT15TO15	0x8000
#define BIT14TO14	0x4000
#define BIT15TO13	0xE000
#define BIT14TO0	0x7FFF
#define BIT13TO0	0x3FFF
#define BIT12TO0	0x1FFF

double XIA_CFD_Fraction_100MHz(uint16_t CFDvalue, bool &fail)
{
	double correction;
	uint32_t cfdfailbit, timecfd;

	cfdfailbit = ((CFDvalue & BIT15TO15) >> 15);
	timecfd = ((CFDvalue & BIT14TO0) >> 0);
	correction = (10*(double)timecfd)/32768.0;
    fail = (cfdfailbit > 0 || CFDvalue == 0);
	return correction;
}

double XIA_CFD_Fraction_250MHz(uint16_t CFDvalue, bool &fail)
{
	double correction;
	uint32_t cfdfailbit, cfdtrigsource, timecfd;

	cfdfailbit = ((CFDvalue & BIT15TO15) >> 15);
	cfdtrigsource = ((CFDvalue & BIT14TO14) >> 14);
	timecfd = ((CFDvalue & BIT13TO0) >> 0);
	correction = (((double)timecfd)/16384.0 - cfdtrigsource)*4.0;
    fail = (cfdfailbit > 0 || CFDvalue == 0);
	return correction;
}

double XIA_CFD_Fraction_500MHz(uint16_t CFDvalue, bool &fail)
{
	double correction;
	uint32_t cfdtrigsource, timecfd;

	cfdtrigsource = ((CFDvalue & BIT15TO13) >> 13);
	timecfd = ((CFDvalue & BIT12TO0) >> 0);
	correction = (((double)timecfd)/8192.0 + cfdtrigsource - 1.0)*2.0;
    fail = (cfdtrigsource >= 7);
	return correction;
}