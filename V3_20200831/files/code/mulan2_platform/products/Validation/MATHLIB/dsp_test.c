/*
 * Copyright (C) 2008-2012 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>

#define	ERROR()		__asm__("jmp .")

extern int16  ar[128];
extern uint16 br[128];
extern const uint32 ar32[128];
extern const uint16 br16[16];

void uavg_test(void);
void iavg_test(void);

void vecsumU32_U16_test(void);
void vecsumU32_U32_test(void);
void vecsumU48_U32_test(void);

void norm2U32_U16byU16_test(void);
void norm2U48_U16byU16_test(void);

void norm2vectorU32_U16byU16_test(void);
void norm2vectorU48_U16byU16_test(void);

void dotproductU32_U16byU16_test(void);
void dotproductU48_U16byU16_test(void);

static void init_vectors (void)
{
  int i;

  for (i = 0; i < 128; i++) {
    ar[i] = i;
    br[i] = 1024-i;
  }
}

void dsp_test(void)
{
  	init_vectors ();

	uavg_test();
	iavg_test();

	vecsumU32_U16_test();
	vecsumU32_U32_test();
	vecsumU48_U32_test();

	norm2U32_U16byU16_test();
	norm2U48_U16byU16_test();
	norm2vectorU32_U16byU16_test();
	norm2vectorU48_U16byU16_test();

	dotproductU32_U16byU16_test();
	dotproductU48_U16byU16_test();

}	/* dsp_test() */

/*
 *  uavg
 */

void uavg_test(void)
{
  uint16 i;
  uint16 j;

  i = 0U;
  j = 65535U;
  if (32767U != _uavg (i, j)) {
    ERROR ();
  }

  i = 65535U;
  j = 1U;
  if (32768U != _uavg (i, j)) {
    ERROR ();
  }

  i = 65535U;
  j = 65535U;
  if (65535U != _uavg (i, j)) {
    ERROR ();
  }

  i = 65534U;
  j = 65535U;
  if (65534U != _uavg (i, j)) {
    ERROR ();
  }

  i = 65535U;
  j = 32768U;
  if (49151U != _uavg (i, j)) {
    ERROR ();
  }

  i = 32767U;
  j = 65535U;
  if (49151U != _uavg (i, j)) {
    ERROR ();
  }

  i = 12345U;
  j = 54321U;
  if (33333U != _uavg (i, j)) {
    ERROR ();
  }

}	/* uavg_test */


/*
 *  u\iavg
 */

void iavg_test(void)
{
  int16 i;
  int16 j;

  i = 0;
  j = -1;
  if (-1 != _iavg (i, j)) {
    ERROR ();
  }

  i = -1;
  j = 1;
  if (0 != _iavg (i, j)) {
    ERROR ();
  }

  i = -1;
  j = -1;
  if (-1 != _iavg (i, j)) {
    ERROR ();
  }

  i = -2;
  j = -1;
  if (-2 != _iavg (i, j)) {
    ERROR ();
  }

  i = -1;
  j = -32768;
  if (-16385 != _iavg (i, j)) {
    ERROR ();
  }

  i = 32767;
  j = -1;
  if (16383 != _iavg (i, j)) {
    ERROR ();
  }

  i = 0x2345;
  j = 0x4321;
  if (0x3333 != _iavg (i, j)) {
    ERROR ();
  }

}	/* iavg_test */


/*
 *  vector sum
 */

void vecsumU32_U16_test(void)
{
  	uint16 i;
	uint32 sum;

  	sum = 0;
	for (i = 1; i <= 16; i++) {
	  if (sum != vecsumU32_U16 ((uint16 *)ar, i)) {
	    ERROR ();
	  }
	  sum += i;
	}

	/* for MLX16x8 also do for size 17, 24, 32, 128 */
#if defined (HAS_MLX16_COPROCESSOR)
	sum = 136;
	if (sum != vecsumU32_U16 ((uint16 *)ar, 17)) {
	  ERROR ();
	}
	sum = 276;
	if (sum != vecsumU32_U16 ((uint16 *)ar, 24)) {
	  ERROR ();
	}
	sum = 496;
	if (sum != vecsumU32_U16 ((uint16 *)ar, 32)) {
	  ERROR ();
	}
	sum = 8128;
	if (sum != vecsumU32_U16 ((uint16 *)ar, 128)) {
	  ERROR ();
	}
#endif  /* HAS_MLX16_COPROCESSOR */

}	/* vecsumU32_U16_test */

void vecsumU32_U32_test(void)
{
  	uint16 i;
	uint32 sum;

  	sum = 0;
	for (i = 1; i <= 16; i++) {
	  sum += i + 0x80000000;
	  if (sum != vecsumU32_U32 (ar32, i)) {
	    ERROR ();
	  }
	}

	/* for MLX16x8 also do for size 17, 24, 32, 128 */
#if defined (HAS_MLX16_COPROCESSOR)
	sum = 153;
	sum += 0x80000000;
	if (sum != vecsumU32_U32 (ar32, 17)) {
	  ERROR ();
	}
	sum = 300;
	if (sum != vecsumU32_U32 (ar32, 24)) {
	  ERROR ();
	}
	sum = 528;
	if (sum != vecsumU32_U32 (ar32, 32)) {
	  ERROR ();
	}
	sum = 8256;
	if (sum != vecsumU32_U32 (ar32, 128)) {
	  ERROR ();
	}
#endif  /* HAS_MLX16_COPROCESSOR */

}	/* vecsumU32_U32_test */


void vecsumU48_U32_test(void)
{
  	uint16 i;
	uint32 sum;
	uint16 msb48;

  	sum = 0;
	for (i = 1; i <= 16; i++) {
	  sum += i + 0x80000000;
	  if (sum != vecsumU48_U32 (ar32, i, &msb48)) {
	    ERROR ();
	  }
	  if (msb48 != (i / 2)) {
	    ERROR ();
	  }
	}

	/* for MLX16x8 also do for size 17, 24, 32, 128 */
#if defined (HAS_MLX16_COPROCESSOR)
	sum = 153 + 0x80000000;
	if (sum != vecsumU48_U32 (ar32, 17, &msb48)) {
	  ERROR ();
	}
	if (msb48 != 17/2) {
	  ERROR ();
	}
	sum = 300;
	if (sum != vecsumU48_U32 (ar32, 24, &msb48)) {
	  ERROR ();
	}
	if (msb48 != 24/2) {
	  ERROR ();
	}
	sum = 528;
	if (sum != vecsumU48_U32 (ar32, 32, &msb48)) {
	  ERROR ();
	}
	if (msb48 != 32/2) {
	  ERROR ();
	}
	sum = 8256;
	if (sum != vecsumU48_U32 (ar32, 128, &msb48)) {
	  ERROR ();
	}
	if (msb48 != 128/2) {
	  ERROR ();
	}
#endif  /* HAS_MLX16_COPROCESSOR */

}	/* vecsumU48_U32_test */


/*
 *  norm
 */

void norm2U32_U16byU16_test(void)
{
  uint16 i;
  uint16 j;
  uint32 norm;

  i = 0U;
  j = 0U;
  norm = 0U;
  if (norm != norm2U32_U16byU16 (i, j)) {
    ERROR ();
  }

  i = 0U;
  j = 1U;
  norm = 1U;
  if (norm != norm2U32_U16byU16 (i, j)) {
    ERROR ();
  }

  i = 2U;
  j = 1U;
  norm = 5U;
  if (norm != norm2U32_U16byU16 (i, j)) {
    ERROR ();
  }

  i = 32767U;
  j = 32768U;
  norm = 0x7FFF0001U;
  if (norm != norm2U32_U16byU16 (i, j)) {
    ERROR ();
  }

  i = 65535U;
  j = 0U;
  norm = 0xFFFE0001U;
  if (norm != norm2U32_U16byU16 (i, j)) {
    ERROR ();
  }

  i = 1U;
  j = 65535U;
  norm = 0xFFFE0002U;
  if (norm != norm2U32_U16byU16 (i, j)) {
    ERROR ();
  }

  i = 32766U;
  j = 65534U;
  norm = 0x3FFA0008U; 	/* with overflow */
  if (norm != norm2U32_U16byU16 (i, j)) {
    ERROR ();
  }

  i = 12345U;
  j = 54321U;
  norm = 3103170066U; /* 0xB8F69E12 */ 	/* no overflow */
  if (norm != norm2U32_U16byU16 (i, j)) {
    ERROR ();
  }

}	/* norm2U32_U16byU16_test */


void norm2U48_U16byU16_test(void)
{
  uint16 i;
  uint16 j;
  uint32 norm;
  uint16 msw;

  i = 0U;
  j = 0U;
  norm = 0U;
  if (norm != norm2U48_U16byU16 (i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  i = 0U;
  j = 1U;
  norm = 1U;
  if (norm != norm2U48_U16byU16 (i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  i = 2U;
  j = 1U;
  norm = 5U;
  if (norm != norm2U48_U16byU16 (i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  i = 32767U;
  j = 32768U;
  norm = 0x7FFF0001U;
  if (norm != norm2U48_U16byU16 (i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  i = 65535U;
  j = 0U;
  norm = 0xFFFE0001U;
  if (norm != norm2U48_U16byU16 (i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  i = 1U;
  j = 65535U;
  norm = 0xFFFE0002U;
  if (norm != norm2U48_U16byU16 (i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  i = 32766U;
  j = 65534U;
  norm = 0x3FFA0008U; 	/* with overflow */
  if (norm != norm2U48_U16byU16 (i, j, &msw)) {
    ERROR ();
  }
  if (1 != msw) {
    ERROR ();
  }

  i = 12345U;
  j = 54321U;
  norm = 3103170066U; /* 0xB8F69E12 */ 	/* no overflow */
  if (norm != norm2U48_U16byU16 (i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

}	/* norm2U48_U16byU16_test */


void norm2vectorU32_U16byU16_test(void)
{
  	uint16 i;
	uint32 norm;

  	norm = 0;
	for (i = 1; i <= 16; i++) {
	  if (norm != norm2vectorU32_U16byU16 ((uint16 *)ar, i)) {
	    ERROR ();
	  }
	  norm += (uint32) i * i;
	}

	/* for all CPU targets also do for size 17, 24, 32, 128 */
	norm = 1496;
	if (norm != norm2vectorU32_U16byU16 ((uint16 *)ar, 17)) {
	  ERROR ();
	}
	norm = 4324;
	if (norm != norm2vectorU32_U16byU16 ((uint16 *)ar, 24)) {
	  ERROR ();
	}
	norm = 10416;
	if (norm != norm2vectorU32_U16byU16 ((uint16 *)ar, 32)) {
	  ERROR ();
	}
	norm = 690880;
	if (norm != norm2vectorU32_U16byU16 ((uint16 *)ar, 128)) {
	  ERROR ();
	}

}	/* norm2vectorU32_U16_test */

void norm2vectorU48_U16byU16_test(void)
{
  	uint16 i;
	uint32 norm;
	uint16 msb48;

  	norm = 0;
	for (i = 1; i <= 16; i++) {
	  if ((i%2) == 1) {
	    norm += (uint32) (i/2+1) * (i/2+1);
	  } else {
	    norm += 0x40000000 + (uint32) (i/2) * 65536 + (uint32) (i/2) * (i/2);
	  }
	  if (norm != norm2vectorU48_U16byU16 (br16, i, &msb48)) {
	    ERROR ();
	  }
	  if (msb48 != (i / 8)) {
	    ERROR ();
	  }
	}

}	/* norm2vectorU48_U16_test */

/*
 *  dotproduct
 */

void dotproductU32_U16byU16_test(void)
{
  	uint16 i;
	uint32 sum;

	sum = 0;
	for (i = 1; i <= 16; i++) {
	  if (sum != dotproductU32_U16byU16 ((uint16 *)ar, br, i)) {
	    ERROR ();
	  }
	  sum += (uint32) i * (1024-i);
	}

}	/* dotproductU32_U16byU16_test */

void dotproductU48_U16byU16_test(void)
{
  	uint16 i;
	uint32 sum;
	uint16 msb48;

	sum = 0;
	for (i = 1; i <= 16; i++) {
	  if ((i%2) == 1) {
	    sum += (uint32) (i/2+1) * (i/2+1);
	  } else {
	    sum += 0x40000000 + (uint32) (i/2) * 65536 + (uint32) (i/2) * (i/2);
	  }
	  if (sum != dotproductU48_U16byU16 (br16, br16, i, &msb48)) {
	    ERROR ();
	  }
	  if (msb48 != (i / 8)) {
	    ERROR ();
	  }
	}
}	/* dotproductU48_U16byU16_test */

/* EOF */
