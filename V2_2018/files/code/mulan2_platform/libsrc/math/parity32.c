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

/* 21 insn (MLX16-x8), 22 insn (MLX16); stack 4bytes but not used */
uint16 parity32 (uint32 v)
{
  uint16 v16;

  v16  = (uint16) v;
  v16 ^= (uint16) (v >> 16);

  v16 ^= v16 >> 8;
  v16 ^= v16 >> 4;
  v16 ^= v16 >> 2;
  v16 ^= v16 >> 1;

  return v16 & 0x1;
}

/* EOF */
