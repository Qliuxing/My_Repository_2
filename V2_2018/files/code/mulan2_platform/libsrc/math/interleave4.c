/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Math library
 *
 */

#include <typelib.h>
#include <mathlib.h>

/* interleave 4 bits of x and y, 
   bits of x are in even positions and bits from y in the odd positions

   Note: bit0 is LSB, bit7 is MSB

   see also
   http://graphics.stanford.edu/~seander/bithacks.html#InterleaveTableObvious
*/

#if 0
uint8 interleave4 (uint8 x, uint8 y)
{
  uint8 z;

  uint8 x8 = x & 0x0F; /* safety, required as compiler will not guarantee */
  uint8 y8 = y & 0x0F; /* safety, required as compiler will not guarantee */

  x8 = (x8 | (x8 << 2)) & 0x33;
  x8 = (x8 | (x8 << 1)) & 0x55;

  y8 = (y8 | (y8 << 2)) & 0x33;
  y8 = (y8 | (y8 << 1)) & 0x55;

  z = x8 | (y8 << 1);

  return z;
}
#else

/* abcd -> 0a0b0c0d */

static const uint8 i4_t16[16] = 
  {
    0x00, /* 0000b -> 00000000b */
    0x01, /* 0001b -> 00000001b */
    0x04, /* 0010b -> 00000100b */
    0x05, /* 0011b -> 00000101b */
    0x10, /* 0100b -> 00010000b */
    0x11, /* 0101b -> 00010001b */
    0x14, /* 0110b -> 00010100b */
    0x15, /* 0111b -> 00010101b */
    0x40, /* 1000b -> 01000000b */
    0x41, /* 1001b -> 01000001b */
    0x44, /* 1010b -> 01000100b */
    0x45, /* 1011b -> 01000101b */
    0x50, /* 1100b -> 01010000b */
    0x51, /* 1101b -> 01010001b */
    0x54, /* 1110b -> 01010100b */
    0x55  /* 1111b -> 01010101b */
  };

uint8 interleave4 (uint8 x, uint8 y)
{
  return (i4_t16[x & 0xF] | (i4_t16[y & 0xF] << 1));
}
#endif

/* EOF */
