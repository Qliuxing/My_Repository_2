/*
 * Copyright (C) 2007-2013 Melexis N.V.
 *
 * Math Library
 *
 */

#include "typelib.h"
#include "mathlib.h"

/* Note: 
   ilog2 = fsb
   iexp2 = sfb

   integer versions only, not (yet) fractional, i.e. log for 0 < x < 1(2^16)
*/

/* tables for use with optimize sfb for MLX16 CPU target */
#if defined (HAS_MLX16_FSB_SFB_INSTRUCTIONS)
    /* table is not needed */
#else
/* MLX16 */
static const uint16 power2_16[16] = {
  0x0001, /* 1 <<  0 */
  0x0002, /* 1 <<  1 */
  0x0004, /* 1 <<  2 */
  0x0008, /* 1 <<  3 */
  0x0010, /* 1 <<  4 */
  0x0020, /* 1 <<  5 */
  0x0040, /* 1 <<  6 */
  0x0080, /* 1 <<  7 */
  0x0100, /* 1 <<  8 */
  0x0200, /* 1 <<  9 */
  0x0400, /* 1 << 10 */
  0x0800, /* 1 << 11 */
  0x1000, /* 1 << 12 */
  0x2000, /* 1 << 13 */
  0x4000, /* 1 << 14 */
  0x8000  /* 1 << 15 */
};
#endif /* HAS_MLX16_FSB_SFB_INSTRUCTIONS */

uint32 iexp2_U32 (uint16 v)
{
#if defined (HAS_MLX16_FSB_SFB_INSTRUCTIONS)
  return _sfb32(v);
#else
  /* MLX16 */

  /* sfb32: 86 bytes, 14+ insn */

  /* 34 bytes + 16*2 bytes (table) = 66 bytes */
  /* 9 - 10 insn */
  if (v < 16) {
    uint16 p = power2_16[v & 0xF];
    return p;
  } else {
    uint16 p = power2_16[v & 0xF];
    return ((uint32) p) << 16;
  }
#endif /* HAS_MLX16_FSB_SFB_INSTRUCTIONS */
}

/* EOF */
