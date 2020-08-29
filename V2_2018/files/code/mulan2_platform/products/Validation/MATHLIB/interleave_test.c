/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>

#define	ERROR()		__asm__("jmp .")

void interleave_test(void);

void interleave4_test (void);
void interleave8_test (void);
void interleave16_test(void);

uint8 i4 (uint8 x, uint8 y)
{
  int i;
  uint8 z;

  z = 0;
  for (i = 0; i < 4/*nibble*/; i++) {
    z |= ((x & (1U << i)) << i) | ((y & (1U << i)) << (i + 1));
  }

#ifdef WIN2
  printf ("i4: %x %x %x\n", x, y, z);
#endif
  return z;
}

uint16 i8 (uint8 x, uint8 y)
{
  int i;
  uint16 z;

  uint16 x16 = x;
  uint16 y16 = y;

  z = 0;
  for (i = 0; i < 8; i++) {
    z |= ((x16 & (1U << i)) << i) | ((y16 & (1U << i)) << (i + 1));
  }

#ifdef WIN2
  printf ("i8: %x %x %x\n", x, y, z);
#endif
  return z;
}

uint32 i16 (uint16 x, uint16 y)
{
  int i;
  uint32 z;

  uint32 x32 = x;
  uint32 y32 = y;

  z = 0;
  for (i = 0; i < 16; i++) {
    z |= ((x32 & (1UL << i)) << i) | ((y32 & (1UL << i)) << (i + 1));
  }

  return z;
}

void interleave4_test (void)
{
	if (i4(0xf, 0x0) != interleave4 (0xf, 0x0)) {
	  ERROR ();
	}
	if (0x55 != interleave4 (0xf, 0x0)) {
	  ERROR ();
	}
	if (i4(0x0, 0xf) != interleave4 (0x0, 0xf)) {
	  ERROR ();
	}
	if (0xAA != interleave4 (0x0, 0xf)) {
	  ERROR ();
	}
	
	if (i4(0xc, 0x0) != interleave4 (0xc, 0x0)) {
	  ERROR ();
	}
	if (0x50 != interleave4 (0xc, 0x0)) {
	  ERROR ();
	}
	
	if (i4(0xc, 0x3) != interleave4 (0xc, 0x3)) {
	  ERROR ();
	}
	if (0x5A != interleave4 (0xc, 0x3)) {
	  ERROR ();
	}
	
	if (i4(0x4, 0x1) != interleave4 (0x4, 0x1)) {
	  ERROR ();
	}
	if (0x12 != interleave4 (0x4, 0x1)) {
	  ERROR ();
	}
	
	if (i4(0x1, 0x8) != interleave4 (0x1, 0x8)) {
	  ERROR ();
	}
	if (0x81 != interleave4 (0x1, 0x8)) {
	  ERROR ();
	}
	
	if (i4(0x2, 0x9) != interleave4 (0x2, 0x9)) {
	  ERROR ();
	}
	if (0x86 != interleave4 (0x2, 0x9)) {
	  ERROR ();
	}
}

void interleave8_test (void)
{
	if (i8(0xff, 0x0) != interleave8 (0xff, 0x0)) {
	  ERROR ();
	}
	if (0x5555 != interleave8 (0xff, 0x0)) {
	  ERROR ();
	}
	if (i8(0x0, 0xff) != interleave8 (0x0, 0xff)) {
	  ERROR ();
	}
	if (0xAAAA != interleave8 (0x0, 0xff)) {
	  ERROR ();
	}
	
	if (i8(0xf0, 0x0) != interleave8 (0xf0, 0x0)) {
	  ERROR ();
	}
	if (0x5500 != interleave8 (0xf0, 0x0)) {
	  ERROR ();
	}
	
	if (i8(0xf0, 0x03) != interleave8 (0xf0, 0x03)) {
	  ERROR ();
	}
	if (0x550A != interleave8 (0xf0, 0x03)) {
	  ERROR ();
	}
	
	if (i8(0x10, 0x01) != interleave8 (0x10, 0x01)) {
	  ERROR ();
	}
	if (0x0102 != interleave8 (0x10, 0x01)) {
	  ERROR ();
	}
	
	if (i8(0x12, 0x89) != interleave8 (0x12, 0x89)) {
	  ERROR ();
	}
	if (0x8186 != interleave8 (0x12, 0x89)) {
	  ERROR ();
	}
}

void interleave16_test(void)
{
	if (i16(0xffff, 0x0) != interleave16 (0xffff, 0x0)) {
	  ERROR ();
	}
	if (0x55555555 != interleave16 (0xffff, 0x0)) {
	  ERROR ();
	}
	if (i16(0x0, 0xffff) != interleave16 (0x0, 0xffff)) {
	  ERROR ();
	}
	if (0xAAAAAAAA != interleave16 (0x0, 0xffff)) {
	  ERROR ();
	}
	
	if (i16(0xff00, 0x0) != interleave16 (0xff00, 0x0)) {
	  ERROR ();
	}
	if (0x55550000 != interleave16 (0xff00, 0x0)) {
	  ERROR ();
	}
	
	if (i16(0xff00, 0x0033) != interleave16 (0xff00, 0x0033)) {
	  ERROR ();
	}
	if (0x55550A0A != interleave16 (0xff00, 0x0033)) {
	  ERROR ();
	}
	
	if (i16(0xf000, 0x0003) != interleave16 (0xf000, 0x0003)) {
	  ERROR ();
	}
	if (0x5500000A != interleave16 (0xf000, 0x0003)) {
	  ERROR ();
	}
	
	if (i16(0x1000, 0x0001) != interleave16 (0x1000, 0x0001)) {
	  ERROR ();
	}
	if (0x01000002 != interleave16 (0x1000, 0x0001)) {
	  ERROR ();
	}
	
	if (i16(0x1234, 0x6789) != interleave16 (0x1234, 0x6789)) {
	  ERROR ();
	}
	if (0x292E8592 != interleave16 (0x1234, 0x6789)) {
	  ERROR ();
	}
}

void interleave_test(void)
{
	interleave4_test();
	interleave8_test();
	interleave16_test();
}
