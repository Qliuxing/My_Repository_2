/*
 * Copyright (C) 2008-2010 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>

#define	ERROR()		__asm__("jmp .")

void isqrt_test(void);

void isqrt16_test(void);
void isqrt32_test(void);

void isqrt_test(void)
{
	isqrt16_test();
	isqrt32_test();

}	/* isqrt_test() */

/*
 *  16 bit input
 */

void isqrt16_test (void)
{
	uint16 i;

	for (i = 0; i < 65535; i++) {
	  uint16 r = isqrt16 (i);

	  /* r*r <= i < (r+1)*(r+1) */

	  /* could use mulU16_U8byU8 to speed up on MLX16 */
	  uint16 sq  = r * r;

	  /* beware of r = 255 */
	  /* faster than (r+1)*(r+1) on MLX16 */
	  uint16 sq1 = sq + r + r + 1;

	  if (r < 255) {
	    if (! ((sq <= i) && (sq1 > i))) {
	      ERROR ();
	    }
	  } else {
	    if (! ((r == 255) && (sq <= i) && (sq1 == 0))) {
	      ERROR ();
	    }
	  }
	}
	/* last entry, as otherwise infinite loop */
	i = 65535U; /* not to rely on for-loop */
	if (255 != isqrt16 (i)) {
	  ERROR ();
	}
}

/*
 *  32 bit input
 */

void isqrt32_test (void)
{
	uint16 i;

	/* can't do all 32bit values */
	i = 0;
	if (0 != isqrt32 ((uint32) i)) {
	  ERROR ();
	}
	/* first verify that isqrt32 = isqrt16 for 16 bit values */
	for (i = 0; i < 65534; i++) {
	  uint32 x = ((uint32) i);
	  uint16 r = isqrt32 (x);

	  /* r*r <= i < (r+1)*(r+1) */

	  /* could use mulU16_U8byU8 to speed up on MLX16 */
	  uint32 sq  = ((uint32) r) * r;
	  /* beware of r = 65535 */
	  /* faster than (r+1)*(r+1) on MLX16 */
	  uint32 sq1 = sq + r + r + 1;

	  if (! ((sq <= x) && (sq1 > x))) {
	    ERROR ();
	  }
	}
	/* last entries, as otherwise infinite loop or overflow */
	i = 65534U;
	if (255 != isqrt32 ((uint32) i)) {
	  ERROR ();
	}
	/* last entries, as otherwise infinite loop or overflow */
	i = 65535U;
	if (255 != isqrt32 ((uint32) i)) {
	  ERROR ();
	}
	for (i = 0; i < 65534; i++) {
	  uint32 x = (((uint32) i) << 16) + (uint16) rand32 (i);
	  uint16 r = isqrt32 (x);

	  /* r*r <= i < (r+1)*(r+1) */

	  /* could use mulU16_U8byU8 to speed up on MLX16 */
	  uint32 sq  = ((uint32) r) * r;
	  /* beware of r = 65535 */
	  /* faster than (r+1)*(r+1) on MLX16 */
	  uint32 sq1 = sq + r + r + 1;

	  if (! ((sq <= x) && (sq1 > x))) {
	    ERROR ();
	  }
	}
	/* last entries, as otherwise infinite loop or overflow */
	i = 65534U;
	if (65534U != isqrt32 (((uint32) i) << 16)) {
	  ERROR ();
	}
	i = 65534U;
	if (65535U != isqrt32 ((((uint32) i) << 16) + (uint16) rand32(i))) {
	  ERROR ();
	}
	i = 65535U;
	if (65535U != isqrt32 (((uint32) i) << 16)) {
	  ERROR ();
	}
	i = 65535U;
	if (65535U != isqrt32 ((((uint32) i) << 16) + (uint16) rand32(i))) {
	  ERROR ();
	}
	i = 32768U;
	if (46340U != isqrt32 ((((uint32) i) << 16) + 0)) {
	  ERROR ();
	}
	i = 32768U;
	if (32768U != isqrt32 ((((uint32) i) << 15) + 0)) {
	  ERROR ();
	}
	i = 65535U;
	if (65535U != isqrt32 ((((uint32) i) << 16) + i)) {
	  ERROR ();
	}
}

/* EOF */
