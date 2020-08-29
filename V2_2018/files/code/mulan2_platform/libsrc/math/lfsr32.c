/*
 * Copyright (C) 2010-2012 Melexis N.V.
 *
 * Math Library
 *
 */

#include "typelib.h"
#include "mathlib.h"

/* 32 bit Linear Feedback shift register implementation
   e.g. for use with pseudo random number generation
   
   See http://en.wikipedia.org/wiki/Linear_feedback_shift_register
   Galois LFSR
   http://en.wikipedia.org/wiki/Scrambler
*/

/* 32bit polynomial for LFSR with period 2^32-1: B400

   Other polynomials:
   http://en.wikipedia.org/wiki/Linear_feedback_shift_register
   http://www.xilinx.com/support/documentation/application_notes/xapp052.pdf
*/   

#ifndef LFSR_POLYNOME
#define LFSR_POLYNOME 0x80200003UL
#endif

/* the LFSR state variable */
/* do we need a pragma space? */
static uint32 lfsr_32 = 0x1; /* should be non-zero */

/* initialization of the LFSR */
void init_lfsr32 (uint32 seed)
{
  /* assert (0 != seed); */

  lfsr_32 = seed;
}


/* implements one iteration of 16 bit Linear Feedback shift register 
   e.g. for use with pseudo random number generation
*/
#if defined (HAS_MLX16_COPROCESSOR)
uint32 lfsr32 (void)
{
  /* e.g. taps: 32 22 2 1
     characteristic polynomial: x^32 + x^22 + x^2 + x^1 + 1 
     0x80200003
  */
  /* LDRA: negation used for optimization purpose */
  lfsr_32 = (lfsr_32 >> 1) ^ ( ((-(lfsr_32 & (uint32)0x00000001))) & LFSR_POLYNOME); 

  /* formally output is lsb of lfsr *before* the round */
  // seed = (seed << 1) | (lfsr_32 & (uint32) 0x00000001);

  return lfsr_32 /* & (uint32) 0x00000001 */;
}
#else
/* slightly more optimal for MLX16 CPU target (3 insn) */
/* slightly more code size than above for MLX16-x8 CPU target
   1 insn more in bit==1 case for MLX16-x8 CPU target
   2 or 3 insn less in bit==0 case for MLX16-x8 CPU target
*/
uint32 lfsr32 (void)
{
  /* e.g. taps: 32 22 2 1
     characteristic polynomial: x^32 + x^22 + x^2 + x^1 + 1 
     0x80200003
  */
  uint16 bit;

  bit = lfsr_32 & 0x00000001UL;
  lfsr_32 >>=  1;
  if (0 != bit) {
    lfsr_32 ^= LFSR_POLYNOME; /* polynomial */
  }

  return lfsr_32;
}
#endif /* HAS_MLX16_COPROCESSOR */

/* EOF */
