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
static const uint8 power2_8[8] = {
  0x01, /* 1 <<  0 */
  0x02, /* 1 <<  1 */
  0x04, /* 1 <<  2 */
  0x08, /* 1 <<  3 */
  0x10, /* 1 <<  4 */
  0x20, /* 1 <<  5 */
  0x40, /* 1 <<  6 */
  0x80, /* 1 <<  7 */
};
#endif /* HAS_MLX16_FSB_SFB_INSTRUCTIONS */

uint16 iexp2_U16 (uint16 v)
{
#if defined (HAS_MLX16_FSB_SFB_INSTRUCTIONS)
  return _sfb(v);
#else
  /* MLX16 */
  
  /* sfb: 52 bytes, 12+ insn */

  /* 28 bytes + 8 bytes (table) = 36 bytes */
  /* 8 insn */
  if (v < 8) {
    uint16 p = (uint16) power2_8[v & 0x7];

    return p;
  } else {
    uint16 p = (uint16) power2_8[v & 0x7];

    return p << 8;
  }
#endif /* HAS_MLX16_FSB_SFB_INSTRUCTIONS */
}

/* EOF */
