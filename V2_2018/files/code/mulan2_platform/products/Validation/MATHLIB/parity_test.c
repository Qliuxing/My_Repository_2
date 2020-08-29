/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>

#define	ERROR()		__asm__("jmp .")

void parity_test (void);

void parity4_test  (void);
void parity8_test  (void);
void parity16_test (void);
void parity32_test (void);

static uint16 parity (uint16 v)
{
  uint16 parity = 0; /* bool */

  while (v) {
    parity = !parity;
    v = v & (v - 1);
  }

  return parity;
}

void parity4_test (void)
{
  uint16 i;

  for (i = 0; i < 16; i++) {
    if (parity (i) != parity4 (i)) {
      ERROR ();
    }
  }
  /* check if also ok if high nibble not masked */
  for (i = 0; i < 256; i++) {
    if (parity4 (i&0xF) != parity4 (i)) {
      ERROR ();
    }
  }
}

void parity8_test (void)
{
  uint16 i;

  for (i = 0; i < 256; i++) {
    if (parity (i) != parity8 (i)) {
      ERROR ();
    }
  }
}

void parity16_test (void)
{
  uint16 i;

  for (i = 0; i < 65535; i++) {
    if (parity (i) != parity16 (i)) {
      ERROR ();
    }
  }
  /* last value */
  i = 65535U;
  if (parity (i) != parity16 (i)) {
    ERROR ();
  }
}

void parity32_test (void)
{
  uint16 i;

  /* can't do all 32bit values */
  for (i = 1; i < 65535; i++) {
    uint32 x = (((uint32) i) << 16) + (uint16) rand32(i);
    if ((parity (x) ^ parity (x>>16)) != parity32 (x)) {
      ERROR ();
    }
  }
}

void parity_test (void)
{
  parity4_test  ();
  parity8_test  ();
  parity16_test ();
  parity32_test ();
}
