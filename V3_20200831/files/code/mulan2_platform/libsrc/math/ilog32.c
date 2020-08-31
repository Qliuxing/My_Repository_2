/*
 * Copyright (C) 2007-2010 Melexis N.V.
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

uint16 ilog2_U32 (uint32 v)
{
  uint16 result;

  /* ilog2 (0) will return 0xFFFF */
  if (v == 0) {
    result = 0xFFFF;
  } else {
    result = (uint16) _fsb32 (v);
  }

  return result;
}

/* EOF */
