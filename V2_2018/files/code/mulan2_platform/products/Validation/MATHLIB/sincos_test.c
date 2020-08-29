/*
 * Copyright (C) 2007-2012 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>

#include <stdio.h> /* temp */

#define	ERROR()		__asm__("jmp .")

void sincos_test (void);


#ifdef TRIG_LOG_TESTS

void sinU16_log (void);

#ifdef EXTENDED_TESTS
void sinI16_log (void);
void cosU16_log (void);
void cosI16_log (void);
#endif /* EXTENDED_TESTS */

#endif /* TRIG_LOG_TESTS */

/* not needed if comparing with offline table
   only for overall closeness
   and to check properties than an application may want to rely on
*/
void sinU16_continuity_test (void);
void sinU16_symmetry_test (void);
void cosU16_continuity_test (void);

#ifdef EXTENDED_TESTS
uint16 sin2pluscos2_test (void);
uint16 sin2x_test (void);
#endif /* EXTENDED_TESTS */


void sincos_test (void)
{

#ifdef TRIG_LOG_TESTS
	sinU16_log ();
#ifdef EXTENDED_TESTS
	cosU16_log ();

	sinI16_log ();
	cosI16_log ();
#endif /* EXTENDED_TESTS */
#endif /* TRIG_LOG_TESTS */
	sinU16_continuity_test ();
	cosU16_continuity_test ();

	sinU16_symmetry_test ();

#ifdef EXTENDED_TESTS
  	(void) sin2pluscos2_test ();
	(void) sin2x_test ();
#endif /* EXTENDED_TESTS */
}


#ifdef TRIG_LOG_TESTS

/*
 *  sin16
 */

void sinU16_log (void)
{
	uint16 i;

	/* to check for (non-)monotonic increase per quadrant */
	/* to check for corner values, e.g. no sign change, 0 etc
	   corner cases: 
	   	- octant cross-overs
		- any internal cross-overs in approximation
		(sin1, sin3, sin5, sin7, ...)
	*/
	/* offline check to check correct values of sin from table
	   e.g. allow +/- 1-2LSB EXCEPT for the corner and cross-over cases
	   or to have a (reduced) table in ROM/flash
	*/

	printf ("i\tsin\n");
	for (i = 0; i < 65535; i++) {
	  int16 s = sinU16 (i);
	  printf ("%u\t%d\n", i, s);
	}
	/* last entry, as otherwise infinite loop */
	{
	  int16 s;

	  i = 65535U; /* not to rely on for-loop */
	  s = sinU16 (i);
	  printf ("%u\t%d\n", i, s);
	}

}	/* sinU16_log */

#endif /* TRIG_LOG_TESTS */

void sinU16_continuity_test (void)
{
	uint16 i;
	int16  s;

	/* to check for (non-)monotonic increase per quadrant */
	/* to check for corner values, e.g. no sign change, 0 etc
	   corner cases: 
	   	- octant cross-overs
		- any internal cross-overs in approximation
		(sin1, sin3, sin5, sin7, ...)
	*/

	i = 0;
	s = sinU16 (i);
	if (0 != s) {
	  ERROR ();
	}

	/* first quadrant */
	for (i = 1; i < 16384; i++) {
	  int16 s2 = sinU16 (i);

	  if (! (s2 >= s)) {
	    ERROR ();
	  }
	  if (! (s2 > 0)) {
	    ERROR ();
	  }
	  s = s2;
	}

	i = 16384; /* pi/2 */
	s = sinU16 (i);
	if (32767 != s) { /* "1" */
	  ERROR ();
	}

	/* second quadrant */
	for (i = 16385; i < 32768; i++) {
	  int16 s2 = sinU16 (i);

	  if (! (s2 <= s)) {
	    ERROR ();
	  }
	  if (! (s2 > 0)) {
	    ERROR ();
	  }
	  s = s2;
	}

	i = 32768; /* pi */
	s = sinU16 (i);
	if (0 != s) {
	  ERROR ();
	}

	/* third quadrant */
	for (i = 32769; i < 49152; i++) {
	  int16 s2 = sinU16 (i);

	  if (! (s2 <= s)) {
	    ERROR ();
	  }
	  if (! (s2 < 0)) {
	    ERROR ();
	  }
	  s = s2;
	}

	i = 49152; /* 3pi/2 */
	s = sinU16 (i);
	if (-32767 != s) { /* "-1", symmetry */
	  ERROR ();
	}

	/* fourth quadrant */
	for (i = 49153; i < 65535; i++) {
	  int16 s2 = sinU16 (i);

	  if (! (s <= s2)) {
	    ERROR ();
	  }
	  if (! (s2 < 0)) {
	    ERROR ();
	  }
	  s = s2;
	}

	/* last entry, as otherwise infinite loop */
	i = 65535U; 
	s = sinU16 (i);
	if (! (sinU16 (65534U) <= s)) {
	    ERROR ();
	  }
	if (! (s < 0)) {
	  ERROR ();
	}
}	/* sinU16_continuity_test */

/* symmetry test
   could be included in the other tests 
*/
/* not ok for i = 8192; differs 1 */
void sinU16_symmetry_test (void)
{
	uint16 i;

	for (i = 0; i <= 16384; i++) {
	  int16 s = sinU16 (i);

	  if (-s != sinU16(-i)) { /* sin (-x) = - sin(x) */
	    ERROR ();
	  }
	  if (s != sinU16 (32768 - i)) { /* sin (pi-x) = sin(x) */
	    ERROR ();
	  }
	  if (-s != sinU16 (32768 + i)) { /* sin (pi + x) = - sin(x) */
	    ERROR ();
	  }

	  if (s != cosU16 (16384 - i)) { /* cos (pi/2 - x) = sin (x) */
	    ERROR ();
	  }
	}

}	/* sinU16_symmetry_test */


#if defined (TRIG_LOG_TESTS) && defined (EXTENDED_TESTS)

void cosU16_log (void)
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

	printf ("i\tcos\n");
	for (i = 0; i < 65535; i++) {
	  int16 c = cosU16 (i);
	  printf ("%u\t%d\n", i, c);
	}
	/* last entry, as otherwise infinite loop */
	{
	  int16 c;

	  i = 65535U; /* not to rely on for-loop */
	  c = cosU16 (i);
	  printf ("%u\t%d\n", i, c);
	}

}	/* cosU16_log */

#endif /* TRIG_LOG_TESTS && EXTENDED_TESTS */

void cosU16_continuity_test (void)
{
	uint16 i;
	int16  c;

	/* to check for (non-)monotonic increase per quadrant */
	/* to check for corner values, e.g. no sign change, 0 etc
	   corner cases: 
	   	- octant cross-overs
		- any internal cross-overs in approximation
		(cos2, cos4, cos6, ...)
	*/

	i = 0;
	c = cosU16 (i);
	if (32767 != c) { /* "1" */
	  ERROR ();
	}

	/* first quadrant */
	for (i = 1; i < 16384; i++) {
	  int16 c2 = cosU16 (i);

	  if (! (c >= c2)) {
	    ERROR ();
	  }
	  if (! (c2 > 0)) {
	    ERROR ();
	  }
	  c = c2;
	}

	i = 16384; /* pi/2 */
	c = cosU16 (i);
	if (0 != c) {
	  ERROR ();
	}

	/* second quadrant */
	for (i = 16385; i < 32768; i++) {
	  int16 c2 = cosU16 (i);

	  if (! (c >= c2)) {
	    ERROR ();
	  }
	  if (! (c2 < 0)) {
	    ERROR ();
	  }
	  c = c2;
	}

	i = 32768; /* pi */
	c = cosU16 (i);
	if (-32767 != c) { /* "-1 */
	  ERROR ();
	}

	/* third quadrant */
	for (i = 32769; i < 49152; i++) {
	  int16 c2 = cosU16 (i);

	  if (! (c <= c2)) {
	    ERROR ();
	  }
	  if (! (c2 < 0)) {
	    ERROR ();
	  }
	  c = c2;
	}

	i = 49152; /* 3pi/2 */
	c = cosU16 (i);
	if (0 != c) {
	  ERROR ();
	}

	/* fourth quadrant */
	for (i = 49153; i < 65535; i++) {
	  int16 c2 = cosU16 (i);

	  if (! (c <= c2)) {
	    ERROR ();
	  }
	  if (! (c2 > 0)) {
	    ERROR ();
	  }
	  c = c2;
	}

	/* last entry, as otherwise infinite loop */
	i = 65535U; 
	c = cosU16 (i);
	if (! (cosU16 (65534U) <= c)) {
	    ERROR ();
	  }
	if (! (c > 0)) {
	  ERROR ();
	}
}	/* cosU16_continuity_test */


#if defined (TRIG_LOG_TESTS) && defined (EXTENDED_TESTS)

void sinI16_log (void)
{
	int16 i;

	/* to check for (non-)monotonic increase per quadrant */
	/* to check for corner values, e.g. no sign change, 0 etc
	   corner cases: 
	   	- octant cross-overs
		- any internal cross-overs in approximation
		(sin1, sin3, sin5, sin7, ...)
	*/
	/* offline check to check correct values of sin from table
	   e.g. allow +/- 1-2LSB EXCEPT for the corner and cross-over cases
	   or to have a (reduced) table in ROM/flash
	*/

	printf ("i\tsin\n");
	for (i = -32768; i < 32767; i++) {
	  int16 s = sinI16 (i);
	  printf ("%d\t%d\n", i, s);
	}
	/* last entry, as otherwise infinite loop */
	{
	  int16 s;

	  i = 32767; /* not to rely on for-loop */
	  s = sinI16 (i);
	  printf ("%d\t%d\n", i, s);
	}

}	/* sinI16_log */


void cosI16_log (void)
{
	int16 i;

	/* to check for (non-)monotonic increase per quadrant */
	/* to check for corner values, e.g. no sign change, 0 etc
	   corner cases: 
	   	- octant cross-overs
		- any internal cross-overs in approximation
		(sin1, sin3, sin5, sin7, ...)
	*/
	/* offline check to check correct values of sin from table
	   e.g. allow +/- 1-2LSB EXCEPT for the corner and cross-over cases
	   or to have a (reduced) table in ROM/flash
	*/

	printf ("i\tcos\n");
	for (i = -32768; i < 32767; i++) {
	  int16 c = cosI16 (i);
	  printf ("%d\t%d\n", i, c);
	}
	/* last entry, as otherwise infinite loop */
	{
	  int16 c;

	  i = 32767; /* not to rely on for-loop */
	  c = cosI16 (i);
	  printf ("%d\t%d\n", i, c);
	}

}	/* cosI16_log */

#endif /* TRIG_LOG_TESTS && EXTENDED_TESTS */

#ifdef EXTENDED_TESTS

/* tests identity: sin^2 + cos^2 = 1 */
uint16 sin2pluscos2_test (void)
{
	uint16 i;
	uint16 diff1cnt = 0;
#if defined (__MLX16__) && !defined (HAS_MLX16_COPROCESSOR)
	uint16 diff2cnt = 0;
	uint16 diff3cnt = 0;
#endif  /* HAS_MLX16_COPROCESSOR */
	for (i = 0; i < 65535; i++) {
	  int16 s = sinU16 (i);
	  int16 c = cosU16 (i);
	  int32 one32;
	  int16 one;

	  one32 = mulI32_I16byI16 (s, s) + mulI32_I16byI16 (c, c);
	  one = one32 >> 16;

	  if ((0x4000 != one) && (0x3FFF != one)) 
	  {
	    if (0x3FFE == one) {
	      diff1cnt++;
	    } 
#if defined (__MLX16__) && !defined (HAS_MLX16_COPROCESSOR)
	      else if (0x3FFD == one) {
		diff2cnt++;
	      }
	      else if (0x3FFC == one) {
		diff3cnt++;
	      }
#endif  /* HAS_MLX16_COPROCESSOR */
	      else {
	      ERROR();
	    }
	  }
	}
	/* last entry, as otherwise infinite loop */
	{
	  int16 s;
	  int16 c;
	  int32 one32;
	  int16 one;

	  i = 65535U; /* not to rely on for-loop */
	  s = sinU16 (i);
	  c = cosU16 (i);

	  one32 = mulI32_I16byI16 (s, s) + mulI32_I16byI16 (c, c);
	  one = one32 >> 16;

	  if ((0x4000 != one) && (0x3FFF != one)) 
	  {
	    if (0x3FFE == one) {
	      diff1cnt++;
#if defined (HAS_MLX16_COPROCESSOR)
	      ERROR ();
#endif
	    } else {
	      ERROR();
	    }
	  }
	}

	return diff1cnt;
}


/* tests identity: sin(2x) = sin cos + cos sin = 2 sin cos */
/* cos (2x) = cos^2 - sin^2 = 1 - 2 sin^2 = 2 cos^2 - 1 */
uint16 sin2x_test (void)
{
	uint16 i;
	uint16 pdiffcnt = 0; /* number of +1 LSB differences */
	uint16 mdiffcnt = 0; /* number of -1 LSB differences */

	for (i = 0; i < 65535; i++) {
	  int16 s = sinU16 (i);
	  int16 s2 = sinU16 (2*i);
	  int16 c = cosU16 (i);
	  int32 eq;

	  eq = mulI32_I16byI16 (s, c) << 1; /*+ mulI32_I16byI16 (c, s);*/
	  
	  switch (s2 - (eq>>15)) { /* signed multiplication only yields 30bits */
	  case 0:
	    break;
	  case +1:
	  case +2:
	  case +3:
	  case +4:
#if defined (__MLX16__) && !defined (HAS_MLX16_COPROCESSOR)
	  case +5:
#endif
	    /* 1 bit difference: ok */
	    pdiffcnt++;

	    break;
	  case -1:
	  case -2:
	  case -3:
#if defined (__MLX16__) && !defined (HAS_MLX16_COPROCESSOR)
	  case -4:
#endif
	    /* 1 bit difference: ok */
	    mdiffcnt++;
	    break;
	  default:
	    ERROR ();
	    break;
	  }
	  /* check for sign reversal */
	  if (! (   ((eq >= 0) && (s2 >= 0))
		 || ((eq < 0) && (s2 < 0)))) {
	    ERROR ();
	  }
	}
	/* last entry, as otherwise infinite loop */
	{
	  int16 s;
	  int16 s2;
	  int16 c;
	  int32 eq;

	  i = 65535U; 

	  s = sinU16 (i);
	  s2 = sinU16 (2*i);
	  c = cosU16 (i);

	  eq = mulI32_I16byI16 (s, c) << 1; /*+ mulI32_I16byI16 (c, s);*/
	  
	  switch (s2 - (eq>>15)) { /* signed multiplication only yields 30bits */
	  case 0:
	    break;
	  case +1:
	  case +2:
	  case +3:
	  case +4:
	    /* 1 bit difference: ok */
	    pdiffcnt++;
	    break;
	  case -1:
	  case -2:
	  case -3:
	    /* 1 bit difference: ok */
	    mdiffcnt++;
	    break;
	  default:
	    ERROR ();
	    break;
	  }
	  /* check for sign reversal */
	  if (! (   ((eq >= 0) && (s2 >= 0))
		 || ((eq < 0) && (s2 < 0)))) {
	    ERROR ();
	  }
	}

	return pdiffcnt + mdiffcnt;
}

#endif /* EXTENDED_TESTS */
