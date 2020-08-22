/*
 * Copyright (C) 2008-2013 Melexis N.V.
 *
 * Math Library
 *
 */

#include "typelib.h"
#include "mathlib.h"

extern uint16 isqrt32_helper (uint16 idx, uint32 op);
uint16 isqrt32 (uint32 x)
{
  uint32 op;
  uint16 idx;

  op = x;

  /* "one" starts at the highest power of four <= than the argument. */
#if defined (HAS_MLX16_FSB_SFB_INSTRUCTIONS)
  if (op == 0) {
    return 0;
  } else {
    /* unsigned int idx; */
    
    idx = (uint16) _fsb32 (op);
    idx = (2 * (idx>>1));
    /* one = _sfb32 (2 * (idx / 2)); */
  }
#else
  {
    uint32 one;
    one = 1UL << 30;  /* second-to-top bit set */
    idx = 30;
    /* split in two loops: MSW and LSW? */
    while (one > op) {
      one >>= 2;
      idx -= 2;
    }
  }
#endif /* HAS_MLX16_FSB_SFB_INSTRUCTIONS */

  return isqrt32_helper (idx, op);
}

/* EOF */
