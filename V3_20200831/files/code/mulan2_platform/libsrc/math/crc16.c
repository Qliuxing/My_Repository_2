/*
 * Copyright (C) 2007-2009 Melexis N.V.
 *
 * Math library
 *
 */

#include <typelib.h>
#include <mathlib.h>

/* CCITT polynomial	: 1021 (reversed: 8408)
   http://www.ece.cmu.edu/~koopman/roses/dsn04/koopman04_crc_poly_embedded.pdf 

   Other polynomials:
   CRC-16 ANSI/IBM	: 8005 (reversed: A001)
*/

#ifndef CRC_POLYNOME
#define CRC_POLYNOME 0x1021
#endif


uint16 crc16 (uint8 c, uint16 crc)
{
	uint16 i;

	crc = crc ^ (((uint16) c) << 8);

	for (i = 0; i < 8; ++i)
	{
	        if ((crc & 0x8000) != 0)
		{
			crc = (crc << 1) ^ CRC_POLYNOME;
		}
		else
		{
			crc = crc << 1;
		}
	}

	return crc;
}

/* EOF */
