/*
 * Copyright (C) 2007-2009 Melexis N.V.
 *
 * Math library
 *
 */

#include <typelib.h>
#include <mathlib.h>

uint16 bitrev16(uint16 x)
{
  	/* swap each nibble */
	x = ((x & 0xaaaa) >> 1) | ((x & 0x5555) << 1);
	x = ((x & 0xcccc) >> 2) | ((x & 0x3333) << 2);
	/* swap nibbles in each byte */
	x = ((x & 0xf0f0) >> 4) | ((x & 0x0f0f) << 4);
	/* swap bytes in word */
	x = (x >> 8) | (x << 8);

	return x;
}

/* EOF */
