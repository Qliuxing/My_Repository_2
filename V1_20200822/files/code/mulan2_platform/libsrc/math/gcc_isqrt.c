/*
 * Copyright (C) 2007-2008 Melexis N.V.
 *
 * Math Library
 *
 */

#include "typelib.h"
#include "mathlib.h"

uint16 isqrt32_helper (uint16 idx, uint32 op)
{
  uint32 res = 0;
  uint32 one = 1UL << idx;

  /* note: 
     3 32 bit variable require a lot of mov's back and forth the stack
     res + one = sfb32(res, idx)
     and if res in YA, do either sfb(Y-16) or sfb(A)
     then 'one'/idx can be stored in X register
     also (res + 2 * one) >> 1 = sfb32(res>>1, idx)

     wrt 32 bit values can also split while loop in 2:
     one high part, one low part
  */
  while (one != 0) {
    /* can eliminate stack usage for values, if re-calculating res 
       from addition, e.g.
       res = res + 2 * one = (res + one) + one
       res = res + one - one
    */
#if 1
    if (op >= res + one) {
      /*LDRA_INSPECTED 52 S */
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

  return (uint16) res;

}

/* EOF */
