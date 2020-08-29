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

/* 16 insn */
uint16 parity16 (uint16 v)
{
  v ^= v >> 8;
  v ^= v >> 4;
  v ^= v >> 2;
  v ^= v >> 1;

  return v & 0x1;
}

/* EOF */
