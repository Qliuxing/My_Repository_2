/*
 * Copyright (C) 2007-2012 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>

#include <stdio.h> /* temp */
#include <stdlib.h> /* abs */

#define	ERROR()		__asm__("jmp .")

void atan_test (void);

void atan2U16_test (void);
void atan2I16_test (void);

#ifdef TRIG_LOG_TESTS
void atanI16_log   (void);
void atanU16_log   (void);
#endif

void atanU16_continuity_test (void);
void atanI16_continuity_test (void);
void atanI16_symmetry_test   (void);

#ifdef EXTENDED_TESTS
void atan2_id_test (void);
#endif /* EXTENDED_TESTS */


void atan_test (void)
{
  	atan2U16_test ();
  	atan2I16_test ();

#ifdef TRIG_LOG_TESTS
	atanU16_log ();
  	atanI16_log ();
#endif /* TRIG_LOG_TESTS */

	/* not needed if comparing with offline table
	   only for overall closeness
	   and to check properties than an application may want to rely on
	*/

#ifdef EXTENDED_TESTS
	atanU16_continuity_test ();
	atanI16_continuity_test ();
	atanI16_symmetry_test ();
#endif /* EXTENDED_TESTS */

#ifdef EXTENDED_TESTS
	atan2_id_test ();
#endif /* EXTENDED_TESTS */

}	/* atan_test() */


void atan2U16_test (void) 
{
  if (0 != atan2U16(0,0)) {
    ERROR ();
  }

  if (0x2000 != atan2U16(1,1)) {
    ERROR ();
  }

  if (0 != atan2U16(0,1)) {
    ERROR ();
  }

  if (0 != atan2U16(0,8191)) {
    ERROR ();
  }

  if (0 != atan2U16(0,8192)) {
    ERROR ();
  }

  if (0 != atan2U16(0,8193)) {
    ERROR ();
  }

  if (0 != atan2U16(0,0x8000)) {
    ERROR ();
  }

  if (0 != atan2U16(0,0x9000)) {
    ERROR ();
  }

  /* pi/2 */
  if (0x4000 != atan2U16(1,0)) {
    ERROR ();
  }

  if (0x4000 != atan2U16(8191,0)) {
    ERROR ();
  }

  if (0x4000 != atan2U16(8192,0)) {
    ERROR ();
  }

  if (0x4000 != atan2U16(8193,0)) {
    ERROR ();
  }

  if (0x4000 != atan2U16(0x1000,0)) {
    ERROR ();
  }

  if (0x4000 != atan2U16(0x8000,0)) {
    ERROR ();
  }

  if (0x4000 != atan2U16(0x9000,0)) {
    ERROR ();
  }

  /* pi/4 */
  if (0x2000 != atan2U16(0x8000,0x8000)) {
    ERROR ();
  }

  /* pi/4 */
  if (0x2000 != atan2U16(0xFFFF,0xFFFF)) {
    ERROR ();
  }
  if (0x1FFF != atan2U16(0xFFFE,0xFFFF)) {
    ERROR ();
  }

  if (11548 != atan2U16 (0x8000, 0x4000)) {
    ERROR ();
  }

#if 0
  if (11548 != atan2U16(2000,1000)) {
    ERROR ();
  }

  if (11547 != atan2U16(0x8000,0xFFFF)) {
    ERROR ();
  }

#else
  if ((abs (11547 - atan2U16(2000,1000))) > 1) {
    ERROR ();
  }

  if ((abs (4836 - atan2U16(1000,2000))) > 1) {
    ERROR ();
  }

#endif

  if (4836 != atan2U16(0x4000,0x7FFF)) {
    ERROR ();
  }

  if (4835 != atan2U16(0x4000,0x8001)) {
    ERROR ();
  }

}	/* atan2U16_test */

void atan2I16_test (void) 
{
  if (0 != atan2I16(0,0)) {
    ERROR ();
  }

  if (0x2000 != atan2I16(1,1)) {
    ERROR ();
  }

  if (0 != atan2I16(0,1)) {
    ERROR ();
  }

  if (0 != atan2I16(0,8191)) {
    ERROR ();
  }

  if (0 != atan2I16(0,8192)) {
    ERROR ();
  }

  if (0 != atan2I16(0,8193)) {
    ERROR ();
  }

  /* -pi for negative x */
  if ((int16)0x8000 != atan2I16(0,0x8000)) {
    ERROR ();
  }

  if ((int16)0x8000 != atan2I16(0,0x9000)) {
    ERROR ();
  }

  /* pi/2 */
  if (0x4000 != atan2I16(1,0)) {
    ERROR ();
  }

  if (0x4000 != atan2I16(8191,0)) {
    ERROR ();
  }

  if (0x4000 != atan2I16(8192,0)) {
    ERROR ();
  }

  if (0x4000 != atan2I16(8193,0)) {
    ERROR ();
  }

  if (0x4000 != atan2I16(0x1000,0)) {
    ERROR ();
  }
  /* -pi/2 */
  if ((int16)0xC000 != atan2I16(0x8000,0)) {
    ERROR ();
  }
  if ((int16)0xC000 != atan2I16(0x9000,0)) {
    ERROR ();
  }

  /* -3pi/4 */
  if (-0x6000 != atan2I16(0x8000,0x8000)) {
    ERROR ();
  }
  if (-0x6000 != atan2I16(0x8001,0x8001)) {
    ERROR ();
  }
  if (-0x6000 != atan2I16(0xFFFF,0xFFFF)) {
    ERROR ();
  }
  /* atan2(-2,-1) */
  if (-21220 != atan2I16(0xFFFE,0xFFFF)) {
    ERROR ();
  }

  if (-11548 != atan2I16 (0x8000, 0x4000)) {
    ERROR ();
  }
  if (-11548 != atan2I16 (0x8002, 0x4001)) {
    ERROR ();
  }

#if 0
  if (11548 != atan2I16(2000,1000)) {
    ERROR ();
  }

  if (-11548 != atan2I16(-2000,1000)) {
    ERROR ();
  }

  if (21220 != atan2I16(2000,-1000)) {
    ERROR ();
  }

  if (-21220 != atan2I16(-2000,-1000)) {
    ERROR ();
  }

#else
  if ((abs (11547 - atan2I16(2000,1000))) > 1) {
    ERROR ();
  }

  if ((abs (-11547 - atan2I16(-2000,1000))) > 1) {
    ERROR ();
  }

  if ((abs (21220 - atan2I16(2000,-1000))) > 1) {
    ERROR ();
  }

  if ((abs (-21220 - atan2I16(-2000,-1000))) > 1) {
    ERROR ();
  }

  if ((abs (4836 - atan2I16(1000,2000))) > 1) {
    ERROR ();
  }

  if ((abs (-4836 - atan2I16(-1000,2000))) > 1) {
    ERROR ();
  }

  if ((abs (27931 - atan2I16(1000,-2000))) > 1) {
    ERROR ();
  }

  if ((abs (-27932 - atan2I16(-1000,-2000))) > 1) {
    ERROR ();
  }
#endif

  if (4836 != atan2I16(0x4000,0x7FFF)) {
    ERROR ();
  }

  if (27932 != atan2I16(0x4000,0x8001)) {
    ERROR ();
  }

  if (-27932 != atan2I16(0xC000,0x8001)) {
    ERROR ();
  }

}	/* atan2I16_test */

static int16 atanU16(uint16 y)
{
  /* better: atan2_lookup (-y, -65536)? */
  return atan2U16 (y, 65535U);
}

/* atan for values -1(-32768)..1 (32767)
   i.e. result in range -1/8 (-8192) .. 1/8(8192) i.e. -pi/4 .. +pi/4

   use other divisor to get other range
   could change return type 

   atan results in -pi/2 .. pi/2, so could change output type of atan_lut
*/
static int16 atanI16 (int16 y)
{
  /* better: atan2_lookup (-y, -32768)? */
  return atan2I16 (y, 32767U);
  /* return 0x8000 - atan2I16 (y, -32768); */
}


#ifdef TRIG_LOG_TESTS

void atanU16_log (void)
{
	uint16 i;

	/* to check for (non-)monotonic increase per quadrant */
	/* to check for corner values, e.g. no sign change, 0 etc
	   corner cases: 
	   	- octant cross-overs
		- any internal cross-overs in approximation
		(cos2, cos4, cos6, ...)
	*/
	/* offline check to check correct values of cos from table
	   e.g. allow +/- 1-2LSB EXCEPT for the corner and cross-over cases
	   or to have a (reduced) table in ROM/flash
	*/

	printf ("i\tatan\n");
	for (i = 0; i < 65535; i++) {
	  int16 a = atanU16 (i);
	  printf ("%u\t%d\n", i, a);
	}
	/* last entry, as otherwise infinite loop */
	{
	  int16 a;

	  i = 65535U; /* not to rely on for-loop */
	  a = atanU16 (i);
	  printf ("%u\t%d\n", i, a);
	}

}	/* atanU16_log */

void atanI16_log (void)
{
	int16 i;

	/* to check for (non-)monotonic increase per quadrant */
	/* to check for corner values, e.g. no sign change, 0 etc
	   corner cases: 
	   	- octant cross-overs
		- any internal cross-overs in approximation
		(cos2, cos4, cos6, ...)
	*/
	/* offline check to check correct values of cos from table
	   e.g. allow +/- 1-2LSB EXCEPT for the corner and cross-over cases
	   or to have a (reduced) table in ROM/flash
	*/

	printf ("i\tatan\n");
	for (i = -32768; i < 32767; i++) {
	  int16 a = atanI16 (i);
	  printf ("%d\t%d\n", i, a);
	}
	/* last entry, as otherwise infinite loop */
	{
	  int16 a;

	  i = 32767; /* not to rely on for-loop */
	  a = atanI16 (i);
	  printf ("%d\t%d\n", i, a);
	}

}	/* atanI16_log */

#endif /* TRIG_LOG_TESTS */

uint16 diffcnt = 0;

void atanU16_continuity_test (void)
{
	uint16 i;
	int16  a;

	i = 0;
	a = atanU16 (i);
	if (0 != a) {
	  ERROR ();
	}

	for (i = 1; i < 65535; i++) {
	  int16 a2 = atanU16 (i);

	  if (! (a <= a2)) {
	    ERROR ();
	  }

	  /* pi / 4 */
	  if (a2 >= 0x2000) {
	    ERROR ();
	  }
	}

	/* last entry */
	i = 65535;
	if (0x2000 != atanU16 (i)) {
	  ERROR ();
	}
}	/* atanU16_continuity_test */

void atanI16_continuity_test (void)
{
	int16 i;
	int16 a;

	i = 0;
	a = atanI16 (i);
	if (0 != a) {
	  ERROR ();
	}

	for (i = 1; i < 32767; i++) {
	  int16 a2 = atanI16 (i);

	  if (! (a <= a2)) {
	    ERROR ();
	  }

	  /* pi / 4 */
	  if (a2 >= 0x2000) {
	    ERROR ();
	  }
	}

	/* last entry */
	i = 32767;
	if (0x2000 != atanI16 (i)) {
	  ERROR ();
	}


	i = -32768;
	a = atanI16 (i);
	/* fails: returns 0xDFFF, since atanI16 defines as atan2(i/32767) */
	if (-0x2000 != a) {
	  ERROR ();
	}

	for (i = -32767; i < 0; i++) {
	  int16 a2 = atanI16 (i);

	  if (! (a2 >= a)) {
	    ERROR ();
	  }

	  /* = 0 for -3, -2, -1, 0 */
	  if (a2 > 0) {
	    ERROR ();
	  }
	}

}	/* atanI16_continuity_test */

void atanI16_symmetry_test (void)
{
	uint16 i;

	for (i = 0; i < 32768; i++) {
	  int16 x = atanI16 ((int16) i);

	  if (-x != atanI16 (-(int16) i)) {
	    ERROR ();
	  }
	}

}	/* atanI16_symmetry_test */

/* check for atan2 (sin(x), cos(x)) = x */
void atan2_id_test (void)
{
	uint16 i;
	uint16  xp;

	xp = 0;
	for (i = 0; i < 65535; i++) {
	  int16 s = sinU16(i);
	  int16 c = cosU16(i);
	  int16 x = atan2I16 (s, c);

	  /* printf ("%u\t%u\n", i, x); */

	  /* needs to be (non-)monotonic increasing */
	  if ((uint16)x < xp) {
	    ERROR ();
	  }
	  xp = x;

	  if ((uint16)x != i) {
	    /* due to truncation etc, allow one bit diff */

	    /* result should be in same octant */ 
	    /* sign inversions for the following values:
	       16383 -> 16384
	       32767 -> -32768
	       49151 (-16385) -> -16384
	    */
	    if ((x & 0xE000) != (i & 0xE000)) {
	      ERROR ();
	    }
	    if (((x - i) != 1) && ((i - x) != 1)) {
	      ERROR ();
	    }
	  }
	}
	/* last entry, as otherwise infinite loop */
	{
	  int16 s;
	  int16 c;
	  int16 x;

	  i = 65535U; /* not to rely on for-loop */

	  s = sinU16(i);
	  c = cosU16(i);
	  x = atan2I16 (s, c);
	  if ((uint16)x != i) {
	    /* due to truncation etc, allow one bit diff */
	    /* TBD check for sign inversion */
	    if ((x - i) != 1) {
	      ERROR ();
	    }
	  }

	}

}	/* atan2_id_test */

