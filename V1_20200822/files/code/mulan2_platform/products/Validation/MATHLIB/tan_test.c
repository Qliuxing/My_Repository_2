/*
 * Copyright (C) 2007-2012 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>


#define	ERROR()		__asm__("jmp .")

void tan_test (void);

#ifdef TRIG_LOG_TESTS
void tanU16_log (void);

#ifdef EXTENDED_TESTS
void tanI16_log (void);
#endif /* EXTENDED_TESTS */
#endif /* TRIG_LOG_TESTS */

void tanU16_continuity_test (void);
void tanU16_symmetry_test (void);


void tan_test (void)
{
#ifdef TRIG_LOG_TESTS
	tanU16_log ();
#ifdef EXTENDED_TESTS
	tanI16_log ();
#endif /* EXTENDED_TESTS */
#endif /* TRIG_LOG_TESTS */
	tanU16_continuity_test ();
	tanU16_symmetry_test ();
}

#ifdef TRIG_LOG_TESTS

void tanU16_log (void)
{
	uint16 i;

	/* to check for (non-)monotonic increase per quadrant */
	/* to check for corner values, e.g. no sign change, 0 etc
	   corner cases: 
	   	- octant cross-overs
		- any internal cross-overs in approximation
		(tan1, tan3, tan5, ...)
	*/
	/* offline check to check correct values of cos from table
	   e.g. allow +/- 1-2LSB EXCEPT for the corner and cross-over cases
	   or to have a (reduced) table in ROM/flash
	*/

	printf ("i\ttan\n");
	for (i = 0; i < 65535; i++) {
	  int32 t = tanU16 (i);
	  /* need full version to support 32 bit values */
	  printf_full ("%u\t%ld\n", i, t);
	}
	/* last entry, as otherwise infinite loop */
	{
	  int32 t;

	  i = 65535U; /* not to rely on for-loop */
	  t = tanU16 (i);
	  printf_full ("%u\t%ld\n", i, t);
	}

}	/* tanU16_log */

#ifdef EXTENDED_TESTS

void tanI16_log (void)
{
  	int16 i;

	/* to check for (non-)monotonic increase per quadrant */
	/* to check for corner values, e.g. no sign change, 0 etc
	   corner cases: 
	   	- octant cross-overs
		- any internal cross-overs in approximation
		(tan1, tan3, tan5, ...)
	*/
	/* offline check to check correct values of cos from table
	   e.g. allow +/- 1-2LSB EXCEPT for the corner and cross-over cases
	   or to have a (reduced) table in ROM/flash
	*/

	printf ("i\ttan\n");
	for (i = -32768; i < 32767; i++) {
	  int32 t = tanI16 (i);
	  /* need full version to support 32 bit values */
	  printf_full ("%d\t%ld\n", i, t);
	}
	/* last entry, as otherwise infinite loop */
	{
	  int32 t;

	  i = 32767; /* not to rely on for-loop */
	  t = tanI16 (i);
	  printf_full ("%d\t%ld\n", i, t);
	}

}	/* tanI16_log */

#endif /* EXTENDED_TESTS */
#endif /* TRIG_LOG_TESTS */

void tanU16_continuity_test (void)
{
	uint16 i;
	int32  t;

	/* to check for (non-)monotonic increase per quadrant */
	/* to check for corner values, e.g. no sign change, 0 etc
	   corner cases: 
	   	- octant cross-overs
		- any internal cross-overs in approximation
		(tan1, tan3, tan5, tan7, ...)
	*/

	i = 0;
	t = tanU16 (i);
	if (0 != t) {
	  ERROR ();
	}

	/* first quadrant */
	for (i = 1; i < 16384; i++) {
	  int32 t2 = tanU16 (i);

	  if (! (t2 >= t)) {
	    ERROR ();
	  }
	  if (! (t2 > 0)) {
	    ERROR ();
	  }

	  if (i < 8192) {
	    if (! (t2 < 65536)) {
	      ERROR ();
	    }
	  } else {
	    if (! (t2 >= 65536)) {
	      ERROR ();
	    }
	  }

	  t = t2;
	}

	i = 8192; /* pi/4 */
	t = tanU16 (i);
	if (t != 65536UL) { 
	  ERROR ();
	}

	i = 16383; /* ~pi/2 */
	t = tanU16 (i);
	if (! ((uint32)t > (1UL << 29))) { /* "inf", symmetry*/
	  ERROR ();
	}

	i = 24576; /* 3pi/4 */
	t = tanU16 (i);
	if (t != -65536L) { 
	  ERROR ();
	}

	i = 16384; /* pi/2 */
	t = tanU16 (i);
	if (t != -0x7FFFFFFF) { /* "-inf", symmetry*/
	  ERROR ();
	}

	/* second quadrant */
	for (i = 16385; i < 32768; i++) {
	  int32 t2 = tanU16 (i);

	  if (! (t2 >= t)) {
	    ERROR ();
	  }
	  if (! (t2 < 0)) {
	    ERROR ();
	  }

	  if (i > 24576) {
	    if (! (t2 > -65536)) {
	      ERROR ();
	    }
	  } else {
	    if (! (t2 <= -65536)) {
	      ERROR ();
	    }
	  }

	  t = t2;
	}

	i = 32768; /* pi */
	t = tanU16 (i);
	if (0 != t) {
	  ERROR ();
	}

	/* third quadrant */
	for (i = 32769; i < 49152; i++) {
	  int32 t2 = tanU16 (i);

	  if (! (t <= t2)) {
	    ERROR ();
	  }
	  if (! (t2 > 0)) {
	    ERROR ();
	  }

	  if (i < 40960) {
	    if (! (t2 < 65536)) {
	      ERROR ();
	    }
	  } else {
	    if (! (t2 >= 65536)) {
	      ERROR ();
	    }
	  }

	  t = t2;
	}

	i = 40960; /* 5pi/4 */
	t = tanU16 (i);
	if (t != 65536UL) { 
	  ERROR ();
	}

	i = 49151; /* ~3pi/2 */
	t = tanU16 (i);
	if (! ((uint32)t > (1UL << 29))) { /* "inf", symmetry */
	  ERROR ();
	}

	i = 49152; /* 3pi/2 */
	t = tanU16 (i);
	if (t != -0x7FFFFFFF) { /* "-inf", symmetry */
	  ERROR ();
	}

	/* fourth quadrant */
	for (i = 49153; i < 65535; i++) {
	  int32 t2 = tanU16 (i);

	  if (! (t <= t2)) {
	    ERROR ();
	  }
	  if (! (t2 < 0)) {
	    ERROR ();
	  }

	  if (i > 57344) {
	    if (! (t2 > -65536)) {
	      ERROR ();
	    }
	  } else {
	    if (! (t2 <= -65536)) {
	      ERROR ();
	    }
	  }

	  t = t2;
	}

	i = 57344; /* 7pi/4 */
	t = tanU16 (i);
	if (t != -65536L) { 
	  ERROR ();
	}

	/* last entry, as otherwise infinite loop */
	i = 65535U; 
	t = tanU16 (i);
	if (! (tanU16 (65534U) <= t)) {
	    ERROR ();
	  }
	if (! (t < 0)) {
	  ERROR ();
	}
}	/* tanU16_continuity_test */


/* symmetry test
   could be included in the other tests 
*/
void tanU16_symmetry_test (void)
{
	uint16 i;

	for (i = 0; i < 16384; i++) {
	  int32 t = tanU16 (i);

	  if (-t != tanU16(-i)) { /* tan (-x) = - tan(x) */
	    ERROR ();
	  }
	  if (-t != tanU16 (32768 - i)) { /* tan (pi-x) = -tan(x) */
	    ERROR ();
	  }
	  if (t != tanU16 (32768 + i)) { /* tan (pi + x) = tan(x) */
	    ERROR ();
	  }

	}

}	/* tanU16_symmetry_test */
