/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>
void   init_lfsr16_B400(uint16 seed);
void   init_lfsr32_0000000C(uint32 seed);
void   init_lfsr32_80200003(uint32 seed);
uint16 lfsr16_B400(void);
uint32 lfsr32_80200003(void);
uint32 lfsr32_0000000C(void);

#ifdef LFSR_LOG_TESTS
#include <stdio.h> /* temp */
#endif /* LFSR_LOG_TESTS */

#define	ERROR()		__asm__("jmp .")

void lfsr_test(void);

/* requires known seed */
static uint16 t[10]= {
  27556,
  13778,
  6889,
  47476,
  23738,
  11869,
  41774,
  20887,
  40139,
  64101
};

static uint32 t32[10]= {
  27556UL,
  13778UL,
  6889UL,
  2149584247UL,
  3224372920UL,
  1612186460UL,
  806093230UL,
  403046615UL,
  2351104104UL,
  1175552052UL
};

#define T_SEED 55112UL
/* command line can set seed and logging count (32 bit number) 
   for log and extended tests 
*/
#ifndef SEED
#define SEED T_SEED
#endif
/* command line can set seed */
#ifndef LFSR_LOG_COUNT
#define LFSR_LOG_COUNT 65536UL
#endif


void lfsr16_value_test  (void);
void lfsr32_value_test  (void);
#ifdef LFSR_LOG_TESTS
void lfsr16_log         (uint32 n);
void lfsr32_log         (uint32 n);
#endif /* LFSR_LOG_TESTS */
#ifdef EXTENDED_TESTS
void lfsr16_period_test (void);
void lfsr32_period_test (void);
#endif /* EXTENDED_TESTS */

/* lfsr16 */

/* check first 10 values */
void lfsr16_value_test (void)
{
  uint16 x = 0;
  int i;

  init_lfsr16 ((uint16) T_SEED);
  
  for(i = 0; i < 10; ++i) {
    x = lfsr16 ();
    if(t[i] != x) {
      ERROR();
    }
  }
}

void lfsr32_value_test (void)
{
  uint32 x = 0;
  int i;

  init_lfsr32 (T_SEED);
  
  for(i = 0; i < 10; ++i) {
    x = lfsr32 ();
    if(t32[i] != x) {
      ERROR();
    }
  }
}

#ifdef LFSR_LOG_TESTS
void lfsr16_log   (uint32 n)
{
  uint32 i;

  init_lfsr16 ((uint16) SEED);

  printf ("i\tlfsr16\n");
  for (i = 0; i < n; i++) {
    uint16 l = lfsr16 ();
    /* need full version to support 32 bit values */
    printf_full ("%lu\t%u\n", i, l);
  }

}

void lfsr32_log   (uint32 n)
{
  uint32 i;

  init_lfsr32 (SEED);

  printf ("i\tlfsr32\n");
  for (i = 0; i < n; i++) {
    uint32 l = lfsr32 ();
    /* need full version to support 32 bit values */
    printf_full ("%lu\t%lu\n", i, l);
  }

}
#endif /* LFSR_LOG_TESTS */

#ifdef EXTENDED_TESTS
/* to check periodicity of LFSR; dependent on polynomial */
void lfsr16_period_test (void)
{
  uint16 period = 1;

  init_lfsr16_B400 ((uint16) SEED);

  /* should not overflow or infinite loop */
  while (((uint16) SEED) != lfsr16_B400 ()) {
    period++;
  }

  if (65535U != period) {
    ERROR ();
  }
}

/* to check periodicity of LFSR; dependent on polynomial */
/* standard polynomial (0x80200003 has period of 2^32-1 .. ) */
void lfsr32_period_test (void)
{
  uint16 period = 1;

  init_lfsr32_0000000C (SEED);

  /* should not overflow or infinite loop */
  while (SEED != lfsr32_0000000C ()) {
    period++;
  }

  if (15 != period) {
    ERROR ();
  }
}
#endif /* EXTENDED_TESTS */


void lfsr_test(void)
{
  lfsr16_value_test ();
  lfsr32_value_test ();
#ifdef LFSR_LOG_TESTS
  //  lfsr16_log (LFSR_LOG_COUNT);
  lfsr32_log (LFSR_LOG_COUNT);
#endif /* LFSR_LOG_TESTS */
#ifdef EXTENDED_TESTS
  lfsr16_period_test ();
  lfsr32_period_test ();
#endif /* EXTENDED_TESTS */
}
