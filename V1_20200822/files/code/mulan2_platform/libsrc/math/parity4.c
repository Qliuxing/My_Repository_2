/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Math Library
 *
 */

#include <typelib.h>
#include <mathlib.h>

/*
  Parity check of v, i.e. 
  returns true (1) if odd number of set bits in v, false (0) otherwise
  
  see also
  http://graphics.stanford.edu/~seander/bithacks.html
*/

/* total code size can be reduced when all in same file and using assembler labels at the various entry points
*/

/* also ok for v > 16, no need to mask with 0xF for safety ; see parity4_test */
/* 8 insn */
uint16 parity4 (uint8 v)
{
#if 1
  v ^= v >> 2;
  v ^= v >> 1;

  return v & 0x1;
#else
  v &= 0xf;

  return (0x6996 >> v) & 1; /* 16 bit lookup table */
#endif
}

/* EOF */
