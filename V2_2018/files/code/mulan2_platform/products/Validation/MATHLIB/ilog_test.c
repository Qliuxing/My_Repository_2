/*
 * Copyright (C) 2008-2010 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>

#define	ERROR()		__asm__("jmp .")

void ilog_test(void);

/* forward declarations */
void iexp2_U16_test(void);
void iexp2_U32_test(void);
void ilog2_U16_test(void);
void ilog2_U32_test(void);

void ilog_test(void)
{
	iexp2_U16_test();
	iexp2_U32_test();

	ilog2_U16_test();
	ilog2_U32_test();

}	/* ilog_test() */

/* not to rely on iexp2/ilog2 themselves */
const uint16 power2_16[16] = {
  0x0001, /* 1 <<  0 */
  0x0002, /* 1 <<  1 */
  0x0004, /* 1 <<  2 */
  0x0008, /* 1 <<  3 */
  0x0010, /* 1 <<  4 */
  0x0020, /* 1 <<  5 */
  0x0040, /* 1 <<  6 */
  0x0080, /* 1 <<  7 */
  0x0100, /* 1 <<  8 */
  0x0200, /* 1 <<  9 */
  0x0400, /* 1 << 10 */
  0x0800, /* 1 << 11 */
  0x1000, /* 1 << 12 */
  0x2000, /* 1 << 13 */
  0x4000, /* 1 << 14 */
  0x8000  /* 1 << 15 */
};

const uint32 power2_32[32] = {
  0x00000001, /* 1 <<  0 */
  0x00000002, /* 1 <<  1 */
  0x00000004, /* 1 <<  2 */
  0x00000008, /* 1 <<  3 */
  0x00000010, /* 1 <<  4 */
  0x00000020, /* 1 <<  5 */
  0x00000040, /* 1 <<  6 */
  0x00000080, /* 1 <<  7 */
  0x00000100, /* 1 <<  8 */
  0x00000200, /* 1 <<  9 */
  0x00000400, /* 1 << 10 */
  0x00000800, /* 1 << 11 */
  0x00001000, /* 1 << 12 */
  0x00002000, /* 1 << 13 */
  0x00004000, /* 1 << 14 */
  0x00008000,  /* 1 << 15 */
  0x00010000, /* 1 << 160 */
  0x00020000, /* 1 << 17 */
  0x00040000, /* 1 << 18 */
  0x00080000, /* 1 << 19 */
  0x00100000, /* 1 << 20 */
  0x00200000, /* 1 << 21 */
  0x00400000, /* 1 << 22 */
  0x00800000, /* 1 << 23 */
  0x01000000, /* 1 << 24 */
  0x02000000, /* 1 << 25 */
  0x04000000, /* 1 << 26 */
  0x08000000, /* 1 << 27 */
  0x10000000, /* 1 << 28 */
  0x20000000, /* 1 << 29 */
  0x40000000, /* 1 << 30 */
  0x80000000  /* 1 << 31 */
};

/*
 *  16 bit input
 */

void ilog2_U16_test (void)
{
	uint16 i;
	uint16 n;
	
	n = ilog2_U16 (0);
	if (0xFFFF != n) {
	  ERROR ();
	}
	for (i = 1; i < 65535; i++) {
	  n = ilog2_U16 (i);

	  /* 2^n <= x and x < 2^(n+1) */
	  if (n >= 16) {
	    ERROR ();
	  } else if (n == 15) {
	    if (! (power2_16[n] <= i)) {
	      ERROR ();
	    }
	  } else if (! (power2_16[n] <= i) && (i < power2_16[n+1])) {
	    ERROR ();
	  }
	}

	/* last entry, as otherwise infinite loop */
	i = 65535U; /* not to rely on for-loop */
	if (15 != ilog2_U16 (i)) {
	  ERROR ();
	}
}	/* ilog2_U16_test */

/*
 *  32 bit input
 */

void ilog2_U32_test (void)
{
	uint16 i;
	uint16 n;
	
	n = ilog2_U32 (0L);
	if (0xFFFF != n) {
	  ERROR ();
	}
	i = 65535U;
	if (15 != ilog2_U32 ((uint32) i)) {
	  ERROR ();
	}
	i = 1;
	if (16 != ilog2_U32 (((uint32) i) << 16)) {
	  ERROR ();
	}
	for (i = 1; i < 65535; i++) {
	  uint32 x = (uint32) i;
	  n = ilog2_U32 (x);

	  /* 2^n <= x and x < 2^(n+1) */
	  if (n >= 16) {
	    ERROR ();
	  } else if (! (power2_32[n] <= x) && (x < power2_32[n+1])) {
	    ERROR ();
	  } /* else ok */
	}
	i = 65535U;
	if (15 != ilog2_U32 ((uint32) i)) {
	  ERROR ();
	}
	i = 1U;
	if (16 != ilog2_U32 (((uint32) i) << 16)) {
	  ERROR ();
	}
	/* can't do all 32bit values */
	for (i = 1; i < 65535; i++) {
	  uint32 x = (((uint32) i) << 16) + (uint16) rand32(i);
	  n = ilog2_U32 (x);

	  /* 2^n <= x and x < 2^(n+1) */
	  if (n >= 32) {
	    ERROR ();
	  } else if (n == 31) {
	    if (! (power2_32[n] <= x)) {
	      ERROR ();
	    }
	  } else if (! (power2_32[n] <= x) && (x < power2_32[n+1])) {
	    ERROR ();
	  } /* else ok */
	}
	i = 65534U;
	if (31 != ilog2_U32 (((uint32) i) << 16)) {
	  ERROR ();
	}
	i = 65535U; /* not to rely on for-loop */
	if (31 != ilog2_U32 (((uint32) i) << 16)) {
	  ERROR ();
	}
	i = 65535U;
	if (31 != ilog2_U32 ((((uint32) i) << 16) + (uint16) rand32 (i))) {
	  ERROR ();
	}
}	/* ilog2_U32_test */

/*
 *  16 bit input
 */

void iexp2_U16_test (void)
{
	uint16 i;
	
	for (i = 0; i < 16; i++) {
	  uint16 n = iexp2_U16 (i);

	  if (n != power2_16[i]) {
	    ERROR ();
	  }
	}

#if 0
	/* not formal part of the test as unspecified input */
	/* MLX16-x8, MLX16-8 -> sfb (i & 0x0F)
	   MLX16 -> power2_8[i & 0x7] << 8
	*/
	for (i = 16; i < 64; i++) {
	  if (0 != iexp2_U16 (i)) {
	    ERROR ();
	  }
	}
#endif

}	/* iexp2_16_test */


/*
 *  32 bit input
 */

void iexp2_U32_test (void)
{
	uint16 i;
	
	for (i = 0; i < 32; i++) {
	  uint32 n = iexp2_U32 (i);

	  if (n != power2_32[i]) {
	    ERROR ();
	  }
	}

#if 0
	/* not formal part of the test as unspecified input */
	/* MLX16-x8, MLX16-8 -> sfb (i & 0xF0) TBC
	   MLX16 -> power2_16[i & 0xF] << 16
	*/
	for (i = 32; i < 64; i++) {
	  if (0L != iexp2_U32 (i)) {
	    ERROR ();
	  }
	}
#endif

}	/* iexp2_32_test */


