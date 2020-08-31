/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Math Library
 *
 */

#include "typelib.h"
#include "mathlib.h"

/* 16 bit Linear Feedback shift register implementation
   e.g. for use with pseudo random number generation
   
   See http://en.wikipedia.org/wiki/Linear_feedback_shift_register
   Galois LFSR
   http://en.wikipedia.org/wiki/Scrambler
*/

/* 16bit polynomial for LFSR with period 2^16-1:
   taps: 16 14 13 11
   x^16 + x^14 + x^13 + x^11 + 1  
   0xB400

   Other polynomials:
   http://en.wikipedia.org/wiki/Linear_feedback_shift_register
   http://www.xilinx.com/support/documentation/application_notes/xapp052.pdf
*/   

#ifndef LFSR_POLYNOME
#define LFSR_POLYNOME 0xB400
#endif

/* the LFSR state variable */
/* do we need a pragma space? */
static uint16 lfsr_16 = 0x1; /* should be non-zero */

/* initialization of the LFSR */
void init_lfsr16 (uint16 seed)
{
  /* assert (0 != seed); */

  lfsr_16 = seed;
}


/* implements one iteration of 16 bit Linear Feedback shift register 
   e.g. for use with pseudo random number generation
*/
uint16 lfsr16 (void)
{
  /* e.g. taps: 16 14 13 11; 
     characteristic polynomial: x^16 + x^14 + x^13 + x^11 + 1 
     0xB400
  */
  /* LDRA: negation used for optimization purpose */
  lfsr_16 = (lfsr_16 >> 1) ^ ( ((-(lfsr_16 & (uint16)0x0001))) & LFSR_POLYNOME); 

  /* formally output is lsb of lfsr *before* the round */
  // seed = (seed << 1) | (lfsr_16 & (uint16) 0x0001);

  return lfsr_16 /* & (uint16) 0x0001 */;
}

/* EOF */
