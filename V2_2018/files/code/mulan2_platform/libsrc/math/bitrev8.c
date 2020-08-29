/*
 * Copyright (C) 2007-2010 Melexis N.V.
 *
 * Math library
 *
 */

#include <typelib.h>
#include <mathlib.h>

extern const uint8 bitrev4_16[16];

uint8 bitrev8(uint8 x)
{
  /* function prototype assures no need to add a mask for x >> 4 */
  return (bitrev4_16[(x >> 4) & 0xF]) | (bitrev4_16[x & 0xF] << 4);
}

/* EOF */
