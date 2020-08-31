/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Math library
 *
 */

#include <typelib.h>
#include <mathlib.h>

/* interleave 8 bits of x and y, 
   bits of x are in even positions and bits from y in the odd positions

   Note: bit0 is LSB, bit15 is MSB

   see also
   http://graphics.stanford.edu/~seander/bithacks.html#InterleaveTableObvious
*/

uint16 interleave8 (uint8 x, uint8 y)
{
  uint16 z;

  uint16 x16 = x; /* & 0x00FF safety not needed, can rely on compiler and function prototype */
  uint16 y16 = y; /* & 0x00FF safety not needed, can rely on compiler and function prototype */

  x16 = (x16 | (x16 << 8)) & 0x00FF; /* redundant but results in better optimized code */
  x16 = (x16 | (x16 << 4)) & 0x0F0F;
  x16 = (x16 | (x16 << 2)) & 0x3333;
  x16 = (x16 | (x16 << 1)) & 0x5555;

  y16 = (y16 | (y16 << 8)) & 0x00FF; /* redundant but results in better optimized code */
  y16 = (y16 | (y16 << 4)) & 0x0F0F;
  y16 = (y16 | (y16 << 2)) & 0x3333;
  y16 = (y16 | (y16 << 1)) & 0x5555;

  z = x16 | (y16 << 1);

  return z;
}

/* EOF */
