/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Math library
 *
 */

#include <typelib.h>
#include <mathlib.h>

/* interleave 16 bits of x and y, 
   bits of x are in even positions and bits from y in the odd positions

   Note: bit0 is LSB, bit31 is MSB
*/

uint32 interleave16 (uint16 x, uint16 y)
{
  uint16 lo = interleave8 (x, y); /* & 0x00FF safety not needed, can rely on compiler and function prototype */
  uint16 hi = interleave8 (x >> 8, y >> 8);

  uint32 lo32 = lo;
  uint32 hi32 = hi;

  return lo32 | (hi32 << 16);
}

/* EOF */
