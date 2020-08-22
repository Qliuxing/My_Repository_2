/*
 * Copyright (C) 2008-2013 Melexis N.V.
 *
 * Math Library
 *
 */

#include "typelib.h"
#include "mathlib.h"

/* on MLX16 (-O3), min 8*3+9=33, max 8*14+9=121 instructions */
/* can optimize on MLX16-x8 by using fsb instruction to find first scaling */
uint16 isqrt16 (uint16 x)
{
  uint16 one, op, res;

  op = x;

  /* "one" starts at the highest power of four <= than the argument. */
#if defined(HAS_MLX16_FSB_SFB_INSTRUCTIONS)
  if (op == 0) {
    return 0;
  } else {
    unsigned int idx;
    
    idx = (uint16) _fsb (op);
    one = _sfb (2 * (idx >> 1));
  }
#else
  one = 1 << 14;  /* second-to-top bit set */
  /* could use binary tree search, but probably not really beneficial */
  while (one > op) {
    one >>= 2;
  }
#endif /* HAS_MLX16_FSB_SFB_INSTRUCTIONS */

  res = 0;
  while (one != 0) {
    /* can eliminate stack usage for values, if re-calculating res 
       from addition, e.g.
       res = res + 2 * one = (res + one) + one
       res = res + one - one
    */
#if 0
    if (op >= res + one) {
      op = op - (res + one);
      res = res +  2 * one;
    }
#else
    res += one;
    if (op >= res) {
      op = op - res;
      res += one;
    } else {
      res -= one;
    }
#endif
    res >>= 1;
    one >>= 2;
  }

  return (res);
}

/* EOF */
