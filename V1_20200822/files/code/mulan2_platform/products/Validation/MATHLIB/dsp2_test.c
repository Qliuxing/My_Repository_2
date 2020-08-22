/*
 * Copyright (C) 2008-2013 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>
#include <dsp.h>

#include <stdlib.h> /* labs */

#define	ERROR()		__asm__("jmp .")

extern int16  ar[128];
extern uint16 ar16[128];
extern uint8 ar8[128];
extern int8  ar8s[128];
extern uint16 br[128];
extern const uint32 ar32[128];
extern const uint16 br16[16];

void vecsumU8_U8_test(void);
void vecsumI8_I8_test(void);
void vecsumU16_U8_test(void);
void vecsumI16_I8_test(void);
void vecsumU16_U16_test(void);
void vecsumI16_I16_test(void);

void vecsumI32_I16_test(void);
void vecsumI32_I32_test(void);
void vecsumI48_I32_test(void);

void vecmaxU16_U16_test(void);
void vecmaxU32_U32_test(void);

void normmaxvectorU16_I16_test(void);
void normmaxvectorU32_I32_test(void);

void norm1vectorU32_I16_test(void);
void norm1vectorU32_I32_test(void);
void norm1vectorU48_I32_test(void);

void norm2U32_I16byI16_test(void);
void norm2U48_I16byI16_test(void);

void norm2vectorU32_I16byI16_test(void);
void norm2vectorU48_I16byI16_test(void);

void dotproductI32_I16byI16_test(void);
void dotproductI48_I16byI16_test(void);

/* used const vectors not to consume the entire 2K RAM of MelexCM */
/* static int16 ar16[5] = { -1, +2, -3, +4, -5}; */ /* 0xFFFF, 2, 0xFFFD, 4, 0xFFFB */
/*static int32 ar32[5] = { -1, +2, -3, +4, -5};*/
/* static int16 br16[5] = { 0x7FFF, +2, 0x7FFD, +4, 0x7FFB}; */
/*static int16 cr16[5] = { 0x7FFF, 0x8002, 0x7FFD, 0x8003, 0x7FFB};*/


static void init_vectors (void)
{
  int i;

  for (i = 0; i < 128; i++) {
    ar8[i] = i;
    ar16[i] = i;
    if (i%2 == 0) {
      ar[i] = i;
      ar8s[i] = i * 4;
    } else {
      ar[i] = -i;
      ar8s[i] = -(i * 4);
    }
    br[i] = i-1024;
  }
}

void dsp2_test(void)
{
  	init_vectors ();

	vecsumU8_U8_test();
	vecsumU16_U8_test();
	vecsumU16_U16_test();

	vecsumI8_I8_test();
	vecsumI16_I8_test();
	vecsumI16_I16_test();

	vecsumI32_I16_test();
	vecsumI32_I32_test();
	vecsumI48_I32_test();

	vecmaxU16_U16_test ();
	vecmaxU32_U32_test ();

	normmaxvectorU16_I16_test();
	normmaxvectorU32_I32_test();

	norm1vectorU32_I16_test();
	norm1vectorU32_I32_test();
	norm1vectorU48_I32_test();

	norm2U32_I16byI16_test();
	norm2U48_I16byI16_test();
	norm2vectorU32_I16byI16_test();
	norm2vectorU48_I16byI16_test();

	dotproductI32_I16byI16_test();
	dotproductI48_I16byI16_test();

}	/* dspI_test() */


/*
 *  vector sum
 */

void vecsumU8_U8_test(void)
{
  	uint16 i;
	uint8  sum;

  	sum = 0;
	for (i = 1; i <= 16; i++) {
	  if (sum != vecsumU8_U8 (ar8, i)) {
	    ERROR ();
	  }
	  sum += i;
	}

	sum = 136;
	if (sum != vecsumU8_U8 (ar8, 17)) {
	  ERROR ();
	}
	sum = (uint8) 276;
	if (sum != vecsumU8_U8 (ar8, 24)) {
	  ERROR ();
	}
	sum = (uint8) 496;
	if (sum != vecsumU8_U8 (ar8, 32)) {
	  ERROR ();
	}
	sum = (uint8) 8128;
	if (sum != vecsumU8_U8 (ar8, 128)) {
	  ERROR ();
	}

}	/* vecsumU8_U8_test */

void vecsumU16_U8_test(void)
{
  	uint16 i;
	uint16  sum;

  	sum = 0;
	for (i = 1; i <= 16; i++) {
	  if (sum != vecsumU16_U8 (ar8, i)) {
	    ERROR ();
	  }
	  sum += i;
	}

	sum = 136;
	if (sum != vecsumU16_U8 (ar8, 17)) {
	  ERROR ();
	}
	sum = 276;
	if (sum != vecsumU16_U8 (ar8, 24)) {
	  ERROR ();
	}
	sum = 496;
	if (sum != vecsumU16_U8 (ar8, 32)) {
	  ERROR ();
	}
	sum = 8128;
	if (sum != vecsumU16_U8 (ar8, 128)) {
	  ERROR ();
	}

}	/* vecsumU16_U8_test */

void vecsumU16_U16_test(void)
{
  	uint16 i;
	uint16  sum;

  	sum = 0;
	for (i = 1; i <= 16; i++) {
	  if (sum != vecsumU16_U16 (ar16, i)) {
	    ERROR ();
	  }
	  sum += i;
	}

	sum = 136;
	if (sum != vecsumU16_U16 (ar16, 17)) {
	  ERROR ();
	}
	sum = 276;
	if (sum != vecsumU16_U16 (ar16, 24)) {
	  ERROR ();
	}
	sum = 496;
	if (sum != vecsumU16_U16 (ar16, 32)) {
	  ERROR ();
	}
	sum = 8128;
	if (sum != vecsumU16_U16 (ar16, 128)) {
	  ERROR ();
	}

}	/* vecsumU16_U16_test */

void vecsumI8_I8_test(void)
{
  	uint16 i;
	int8 sum;

  	sum = 0;
	for (i = 1; i <= 16; i++) {
	  if (sum != vecsumI8_I8 (ar8s, i)) {
	    ERROR ();
	  }
	  if (i%2 == 0) {
	    sum += i << 2;
	  } else {
	    sum -= i << 2;
	  }
	}

	sum = 8 << 2; 
	if (sum != vecsumI8_I8 (ar8s, 17)) {
	  ERROR ();
	}
	sum = -(12 << 2);
	if (sum != vecsumI8_I8 (ar8s, 24)) {
	  ERROR ();
	}
	sum = -(16 << 2);
	if (sum != vecsumI8_I8 (ar8s, 32)) {
	  ERROR ();
	}
	sum = (int8) -(64 << 2);
	if (sum != vecsumI8_I8 (ar8s, 128)) {
	  ERROR ();
	}
}	/* vecsumI8_I8_test */

void vecsumI16_I8_test(void)
{
  	uint16 i;
	int16 sum;

  	sum = 0;
	for (i = 1; i <= 16; i++) {
	  if (sum != vecsumI16_I8 (ar8s, i)) {
	    ERROR ();
	  }
	  if (i%2 == 0) {
	    sum += i << 2;
	  } else {
	    sum -= i << 2;
	  }
	}

	sum = 8 << 2; 
	if (sum != vecsumI16_I8 (ar8s, 17)) {
	  ERROR ();
	}
	sum = -(12 << 2);
	if (sum != vecsumI16_I8 (ar8s, 24)) {
	  ERROR ();
	}
	sum = -(16 << 2);
	if (sum != vecsumI16_I8 (ar8s, 32)) {
	  ERROR ();
	}
	sum = -(64 << 2);
	if (sum != vecsumI16_I8 (ar8s, 128)) {
	  ERROR ();
	}
}	/* vecsumI16_I8_test */

void vecsumI16_I16_test(void)
{
  	uint16 i;
	int16 sum;

  	sum = 0;
	for (i = 1; i <= 16; i++) {
	  if (sum != vecsumI16_I16 (ar, i)) {
	    ERROR ();
	  }
	  if (i%2 == 0) {
	    sum += i;
	  } else {
	    sum -= i;
	  }
	}

	sum = 8; 
	if (sum != vecsumI16_I16 (ar, 17)) {
	  ERROR ();
	}
	sum = -12;
	if (sum != vecsumI16_I16 (ar, 24)) {
	  ERROR ();
	}
	sum = -16;
	if (sum != vecsumI16_I16 (ar, 32)) {
	  ERROR ();
	}
	sum = -64;
	if (sum != vecsumI16_I16 (ar, 128)) {
	  ERROR ();
	}
}	/* vecsumI16_I16_test */

void vecsumI32_I16_test(void)
{
  	uint16 i;
	int32 sum;

  	sum = 0;
	for (i = 1; i <= 16; i++) {
	  if (sum != vecsumI32_I16 (ar, i)) {
	    ERROR ();
	  }
	  if (i%2 == 0) {
	    sum += i;
	  } else {
	    sum -= i;
	  }
	}

	/* for MLX16x8 also do for size 17, 24, 32, 128 */
#if defined (HAS_MLX16_COPROCESSOR)
	sum = 8; 
	if (sum != vecsumI32_I16 (ar, 17)) {
	  ERROR ();
	}
	sum = -12;
	if (sum != vecsumI32_I16 (ar, 24)) {
	  ERROR ();
	}
	sum = -16;
	if (sum != vecsumI32_I16 (ar, 32)) {
	  ERROR ();
	}
	sum = -64;
	if (sum != vecsumI32_I16 (ar, 128)) {
	  ERROR ();
	}
#endif  /* HAS_MLX16_COPROCESSOR */
}	/* vecsumI32_I16_test */

void vecsumI32_I32_test(void)
{
  	uint16 i;
	int32 sum;

  	sum = 0;
	for (i = 1; i <= 16; i++) {
	  sum += i + 0x80000000;
	  if (sum != vecsumI32_I32 ((int32 *)ar32, i)) {
	    ERROR ();
	  }
	}

	/* for MLX16x8 also do for size 17, 24, 32, 128 */
#if defined (HAS_MLX16_COPROCESSOR)
	sum = 153;
	sum += 0x80000000;
	if (sum != vecsumI32_I32 ((int32 *)ar32, 17)) {
	  ERROR ();
	}
	sum = 300;
	if (sum != vecsumI32_I32 ((int32 *)ar32, 24)) {
	  ERROR ();
	}
	sum = 528;
	if (sum != vecsumI32_I32 ((int32 *)ar32, 32)) {
	  ERROR ();
	}
	sum = 8256;
	if (sum != vecsumI32_I32 ((int32 *)ar32, 128)) {
	  ERROR ();
	}
#endif  /* HAS_MLX16_COPROCESSOR */

}	/* vecsumI32_I32_test */

void vecsumI48_I32_test(void)
{
  	uint16 i;
	int32 sum;
	int16 msb48;

  	sum = 0;
	for (i = 1; i <= 16; i++) {
	  sum += i + 0x80000000;
	  if (sum != vecsumI48_I32 ((int32 *)ar32, i, &msb48)) {
	    ERROR ();
	  }
	  if (msb48 != (int16)( -((i + 1) / 2))) {
	    ERROR ();
	  }
	}

	/* for MLX16x8 also do for size 17, 24, 32, 128 */
#if defined (HAS_MLX16_COPROCESSOR)
	sum = 153 + 0x80000000;
	if (sum != vecsumI48_I32 ((int32 *)ar32, 17, &msb48)) {
	  ERROR ();
	}
	if (msb48 != -(17+1)/2) {
	  ERROR ();
	}
	sum = 300;
	if (sum != vecsumI48_I32 ((int32 *)ar32, 24, &msb48)) {
	  ERROR ();
	}
	if (msb48 != -(24+1)/2) {
	  ERROR ();
	}
	sum = 528;
	if (sum != vecsumI48_I32 ((int32 *)ar32, 32, &msb48)) {
	  ERROR ();
	}
	if (msb48 != -(32+1)/2) {
	  ERROR ();
	}
	sum = 8256;
	if (sum != vecsumI48_I32 ((int32 *)ar32, 128, &msb48)) {
	  ERROR ();
	}
	if (msb48 != -(128+1)/2) {
	  ERROR ();
	}
#endif  /* HAS_MLX16_COPROCESSOR */

}	/* vecsumI48_I32_test */

/*
 *  vector max
 */

void vecmaxU16_U16_test (void)
{
  	uint16 i;
	uint16  max;

  	max = 0;
	for (i = 1; i <= 16; i++) {
	  if (max != vecmaxU16_U16 (ar16, i)) {
	    ERROR ();
	  }
	  max = i;
	}
  	i = 17;
	max = 16;
	if (max != vecmaxU16_U16 (ar16, i)) {
	  ERROR ();
	}
  	i = 24;
	max = 23;
	if (max != vecmaxU16_U16 (ar16, i)) {
	  ERROR ();
	}
  	i = 32;
	max = 31;
	if (max != vecmaxU16_U16 (ar16, i)) {
	  ERROR ();
	}
  	i = 128;
	max = 127;
	if (max != vecmaxU16_U16 (ar16, i)) {
	  ERROR ();
	}

	max = 2;
	i = 1;
	if (max != vecmaxU16_U16 ((uint16 *)&ar[2], i)) {
	  ERROR ();
	}
	max = -(i+2);
	for (i = 2; i <= 16; i++) {
	  if (max != vecmaxU16_U16 ((uint16 *)&ar[2], i)) {
	    ERROR ();
	  }
	}
	i = 17;
	if (max != vecmaxU16_U16 ((uint16 *)&ar[2], i)) {
	    ERROR ();
	}
	i = 24;
	if (max != vecmaxU16_U16 ((uint16 *)&ar[2], i)) {
	    ERROR ();
	}
	i = 32;
	if (max != vecmaxU16_U16 ((uint16 *)&ar[2], i)) {
	    ERROR ();
	}
	i = 128;
	if (max != vecmaxU16_U16 ((uint16 *)&ar[2], i)) {
	    ERROR ();
	}

	max = 1;
	i = 1;
	if (max != vecmaxU16_U16 (br16, i)) {
	  ERROR ();
	}
  	max += 0x8000;
	for (i = 2; i <= 16; i++) {
	  if (max != vecmaxU16_U16 (br16, i)) {
	    ERROR ();
	  }
	  if (i%2 == 1) {
	    max++;
	  }
	}
	/* br16 has only 16 elements */
}

void vecmaxU32_U32_test (void)
{
  	uint16 i;
	uint32  max;
	const uint32 br32[5] = { 0, -1, 2, -2, 3 };

  	max = 0x80000001;
	for (i = 1; i <= 16; i++) {
	  if (max != vecmaxU32_U32 (ar32, i)) {
	    ERROR ();
	  }
	  max++;
	}
#if defined (HAS_MLX16_COPROCESSOR)
  	i = 17;
	max = 0x80000000 + 17;
	if (max != vecmaxU32_U32 (ar32, i)) {
	  ERROR ();
	}
  	i = 24;
	max = 0x80000000 + 24;
	if (max != vecmaxU32_U32 (ar32, i)) {
	  ERROR ();
	}
  	i = 32;
	max = 0x80000000 + 32;
	if (max != vecmaxU32_U32 (ar32, i)) {
	  ERROR ();
	}
  	i = 128;
	max = 0x80000000 + 128;
	if (max != vecmaxU32_U32 (ar32, i)) {
	  ERROR ();
	}
#endif  /* HAS_MLX16_COPROCESSOR */

	/* to ensure 100% code coverage */
	i = 1;
	max = 2;
	if (max != vecmaxU32_U32 (&br32[2], i)) {
	  ERROR ();
	}
	max = 0xFFFFFFFE;
	for (i = 2; i <= 5-2; i++) {
	  if (max != vecmaxU32_U32 (&br32[2], i)) {
	    ERROR ();
	  }
	}
}

/*
 *  norm max vector
 */

const int16 mvec[18] = {
  2,
  -1, /* 1, 4,4,6,6,8,8,10,10,12,12,14,14,16,16,18,18*/
  4,
  3,
  -6,
  5,
  8,
  7,
  -10,
  9,
  12,
  11,
  14,
  -13,
  16,
  15,
  -18,
  17
};

const int32 mvec32[18] = {
  0x7FFF0008 + 2,
  0x8000FFF8 - 1, /* 1, 4,4,6,6,8,8,10,10,12,12,14,14,16,16,18,18*/
  0x7FFF0008 + 4,
  0x8000FFF8 - 3,
  0x8000FFF8 - 6,
  0x7FFF0008 + 5,
  0x7FFF0008 + 8,
  0x8000FFF8 - 7,
  0x8000FFF8 - 10,
  0x7FFF0008 + 9,
  0x7FFF0008 + 12,
  0x7FFF0008 + 11,
  0x7FFF0008 + 14,
  0x8000FFF8 - 13,
  0x7FFF0008 + 16,
  0x7FFF0008 + 15,
  0x8000FFF8 - 18,
  0x7FFF0008 + 17
};

void normmaxvectorU16_I16_test(void)
{
  uint16 i;

  for (i = 1; i <= 16; i++) {
    if ((((i+1)/2)*2) != normmaxvectorU16_I16 ((int16 *) mvec, i)) { 
      ERROR ();
    }
  }
#if defined (HAS_MLX16_COPROCESSOR)
  for (i = 17; i <= 18; i++) {
    if ((((i+1)/2)*2) != normmaxvectorU16_I16 ((int16 *) mvec, i)) { 
      ERROR ();
    }
  }
#endif  /* HAS_MLX16_COPROCESSOR */

  i = 1;
  if (1 != normmaxvectorU16_I16 ((int16 *) &mvec[1], i)) { 
    ERROR ();
  }
  for (i = 2; i <= 16; i++) {
    if ((((i+2)/2)*2) != normmaxvectorU16_I16 ((int16 *) &mvec[1], i)) { 
      ERROR ();
    }
  }
#if defined (HAS_MLX16_COPROCESSOR)
  for (i = 17; i <= 17; i++) {
    if ((((i+2)/2)*2) != normmaxvectorU16_I16 ((int16 *) &mvec[1], i)) { 
      ERROR ();
    }
  }
#endif  /* HAS_MLX16_COPROCESSOR */
}

void normmaxvectorU32_I32_test()
{
  uint16 i;

  for (i = 1; i <= 16; i++) {
    if (((((i+1)/2)*2) + 0x7FFF0008u) != normmaxvectorU32_I32 ((int32 *) mvec32, i)) {
      ERROR ();
    }
  }

  i = 1;
  if (0x7FFF0008+1 != normmaxvectorU32_I32 ((int32 *) &mvec32[1], i)) { 
    ERROR ();
  }
  for (i = 2; i <= 16; i++) {
    if (((((i+2)/2)*2)+0x7FFF0008u) != normmaxvectorU32_I32 ((int32 *) &mvec32[1], i)) {
      ERROR ();
    }
  }
}

/*
 *  norm1 vector
 */

void norm1vectorU32_I16_test(void)
{
  uint16 i;
  uint32 norm;

  norm = 0;
  for (i = 1; i <= 16; i++) {
    if (norm != norm1vectorU32_I16 (ar, i)) {
      ERROR ();
    }
    norm += i;
  }

  norm = 1024;
  if (norm != norm1vectorU32_I16 ((int16 *)br, 1)) {
    ERROR ();
  }
  norm = 2047;
  if (norm != norm1vectorU32_I16 ((int16 *)br, 2)) {
    ERROR ();
  }

	/* for MLX16x8 also do for size 17, 24, 32, 128 */
#if defined (HAS_MLX16_COPROCESSOR)
	norm = 136;
	if (norm != norm1vectorU32_I16 (ar, 17)) {
	  ERROR ();
	}
	norm = 276;
	if (norm != norm1vectorU32_I16 (ar, 24)) {
	  ERROR ();
	}
	norm = 496;
	if (norm != norm1vectorU32_I16 (ar, 32)) {
	  ERROR ();
	}
	norm = 8128;
	if (norm != norm1vectorU32_I16 (ar, 128)) {
	  ERROR ();
	}
#endif  /* HAS_MLX16_COPROCESSOR */

} 	/* norm1vectorU32_I16_test */

void norm1vectorU32_I32_test(void)
{

  uint16 i;
  uint32 norm;
  uint32 sum;

  sum = 0;
  for (i = 1; i <= 16; i++) {
#if 0
    sum += i;
    if ((i%2) == 0) {
      norm = -sum;
    } else {
      norm = 0x80000000 - sum;
    }
#else
    sum += labs (ar32[i-1]);
    norm = sum;
#endif
    if (norm != norm1vectorU32_I32 ((int32 *)ar32, i)) {
      ERROR ();
    }
  }

} 	/* norm1vectorU32_I32_test */

void norm1vectorU48_I32_test(void)
{

  uint16 i;
  uint32 norm;
  uint32 sum;
  uint16 msw;

  sum = 0;
  for (i = 1; i <= 16; i++) {
#if 0
    sum += i;
    if ((i%2) == 0) {
      norm = -sum;
    } else {
      norm = 0x80000000 - sum;
    }
#else
    sum += labs (ar32[i-1]);
    norm = sum;
#endif
    if (norm != norm1vectorU48_I32 ((int32 *)ar32, i, &msw)) {
      ERROR ();
    }
    if (msw != (i-1)/2 ) {
      ERROR ();
    }
  }

} 	/* norm1vectorU48_I32_test */

/*
 *  norm2
 */

void norm2U32_I16byI16_test(void)
{
  int16 i;
  int16 j;
  uint32 norm;

  i = 0;
  j = 0;
  norm = 0U;
  if (norm != norm2U32_I16byI16 (i, j)) {
    ERROR ();
  }

  i = 0;
  j = 1;
  norm = 1U;
  if (norm != norm2U32_I16byI16 (i, j)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (i, -j)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (j, i)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (-j, i)) {
    ERROR ();
  }

  i = 2;
  j = 1;
  norm = 5U;
  if (norm != norm2U32_I16byI16 (i, j)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (i, -j)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (-i, j)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (-i, -j)) {
    ERROR ();
  }

  i = 32767;
  j = 32767;
  norm = 0x7FFE0002U;
  if (norm != norm2U32_I16byI16 (i, j)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (i, -j)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (-i, j)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (-i, -j)) {
    ERROR ();
  }

  i = 32767;
  j = -32768;
  norm = 0x7FFF0001U;
  if (norm != norm2U32_I16byI16 (i, j)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (-i, j)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (j, i)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (j, -i)) {
    ERROR ();
  }

  i = 12345;
  j = -11215;
  norm = 278175250U; /* 0x10949E12 */ 	/* no overflow */
  if (norm != norm2U32_I16byI16 (i, j)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (i, -j)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (-i, j)) {
    ERROR ();
  }
  if (norm != norm2U32_I16byI16 (-i, -j)) {
    ERROR ();
  }

}	/* norm2U32_I16byI16_test */


void norm2U48_I16byI16_test(void)
{
  int16 i;
  int16 j;
  uint32 norm;
  uint16 msw;

  /* should never overflow */

  i = 0;
  j = 0;
  norm = 0U;
  if (norm != norm2U48_I16byI16 (i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  i = 0;
  j = 1;
  norm = 1U;
  if (norm != norm2U48_I16byI16 (i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (i, -j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (j, i, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (-j, i, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  i = 2;
  j = 1;
  norm = 5U;
  if (norm != norm2U48_I16byI16 (i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (i, -j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (-i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (-i, -j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  i = 32767;
  j = 32767;
  norm = 0x7FFE0002U;
  if (norm != norm2U48_I16byI16 (i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (i, -j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (-i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (-i, -j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  i = 32767;
  j = -32768;
  norm = 0x7FFF0001U;
  if (norm != norm2U48_I16byI16 (i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (-i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (j, i, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (j, -i, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  i = 12345;
  j = -11215;
  norm = 278175250U; /* 0x10949E12 */ 	/* no overflow */
  if (norm != norm2U48_I16byI16 (i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (i, -j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (-i, j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }
  if (norm != norm2U48_I16byI16 (-i, -j, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

}	/* norm2U48_I16byI16_test */


/*
 *  norm2 vector
 */

void norm2vectorU32_I16byI16_test(void)
{
  	uint16 i;
	uint32 norm;
#if 0
  if (0xFFEE0037 != norm2vectorU32_U16byU16 (ar16, 5)) {
    ERROR ();
  }
  if (0x00000037 != norm2vectorU32_I16byI16 (ar16, 5)) {
    ERROR ();
  }
#endif

  	norm = 0;
  	for (i = 1; i <= 16; i++) {
	  if (norm != norm2vectorU32_I16byI16 (ar, i)) {
	    ERROR ();
	  }
	  norm += (uint32) i * i;
	}

	/* for all CPU targets also do for size 17, 24, 32, 128 */
	norm = 1496;
	if (norm != norm2vectorU32_I16byI16 (ar, 17)) {
	  ERROR ();
	}
	norm = 4324;
	if (norm != norm2vectorU32_I16byI16 (ar, 24)) {
	  ERROR ();
	}
	norm = 10416;
	if (norm != norm2vectorU32_I16byI16 (ar, 32)) {
	  ERROR ();
	}
	norm = 690880;
	if (norm != norm2vectorU32_I16byI16 (ar, 128)) {
	  ERROR ();
	}

}	/* norm2vectorU32_I16byI16_test */


void norm2vectorU48_I16byI16_test(void)
{
#if 0
  int16  msw;
  uint16 msu;
static int16 ar16[5] = { -1, +2, -3, +4, -5}; /* 0xFFFF, 2, 0xFFFD, 4, 0xFFFB */
static int16 br16[5] = { 0x7FFF, +2, 0x7FFD, +4, 0x7FFB};

  if (0xFFEE0037 != norm2vectorU48_U16byU16 (ar16, 5, &msu)) {
    ERROR ();
  }
  if (2 != msu) {
    ERROR ();
  }

  if (0x00000037 != norm2vectorU48_I16byI16 (ar16, 5, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  if (0xBFF70037 != norm2vectorU48_U16byU16 (br16, 5, &msu)) {
    ERROR ();
  }
  if (0 != msu) {
    ERROR ();
  }

  if (0xBFF70037 != norm2vectorU48_I16byI16 (br16, 5, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  if (0x3FFC0030 /*0x3FFD0037*/ != norm2vectorU48_U16byU16 (cr16, 5, &msu)) {
    ERROR ();
  }
  if (1 != msu) {
    ERROR ();
  }

  if (0x3FF20030 /*0x3FFD0037*/ != norm2vectorU48_I16byI16 (cr16, 5, &msw)) {
    ERROR ();
  }
  if (1 != msw) {
    ERROR ();
  }
#endif

  	uint16 i;
	uint32 norm;
	uint16 msb48;

  	norm = 0;
	for (i = 1; i <= 16; i++) {
	  if ((i%2) == 1) {
	    norm += (uint32) (i/2+1) * (i/2+1);
	  } else {
	    norm += 0x40000000 - (uint32) (i/2) * 65536 + (uint32) (i/2) * (i/2);
	  }
	  if (norm != norm2vectorU48_I16byI16 ((int16 *)br16, i, &msb48)) {
	    ERROR ();
	  }
	  if (i >= 2) {
	    if (msb48 != (i-2) / 8) {
	      ERROR ();
	    }
	  } else if (msb48 != 0) {
	    ERROR ();
	  } else {
	    ; /* ok */
	  }
	}

}	/* norm2vectorU48_I16byI16_test */

/*
 * dotproduct
 */

void dotproductI32_I16byI16_test(void)
{
#if 0
  if (0xFFEE0037 != dotproductU32_U16byU16 (ar16, ar16, 5)) {
    ERROR ();
  }
  if (0x00000037 != dotproductI32_I16byI16 (ar16, ar16, 5)) {
    ERROR ();
  }
#endif

  	int16 i; /* as for i-1024 to have it signed, and also use signed multiplication*/
	int32 sum;

	sum = 0;
	for (i = 1; i <= 16; i++) {
	  if (sum != dotproductI32_I16byI16 ((int16 *) ar, (int16 *) br, (uint16) i)) {
	    ERROR ();
	  }
	  if ((i%2) == 0) {
	    /* GCC compiles * into mul32_16by16 but relies on X register not modified */
	    sum += /*(int32) i * (i-1024)*/ mulI32_I16byI16(i, i-1024);
	  } else {
	    sum -= /*(int32) i * (i-1024)*/ mulI32_I16byI16(i, i-1024);
	  }
	}

}	/* dotproductI32_I16byI16_test */


void dotproductI48_I16byI16_test(void)
{
#if 0
  int16  msw;
  uint16 msu;

  if (0xFFEE0037 != dotproductU48_U16byU16 (ar16, ar16, 5, &msu)) {
    ERROR ();
  }
  if (2 != msu) {
    ERROR ();
  }

  if (0x00000037 != dotproductI48_I16byI16 (ar16, ar16, 5, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  if (0xBFF70037 != dotproductU48_U16byU16 (br16, br16, 5, &msu)) {
    ERROR ();
  }
  if (0 != msu) {
    ERROR ();
  }

  if (0xBFF70037 != dotproductI48_I16byI16 (br16, br16, 5, &msw)) {
    ERROR ();
  }
  if (0 != msw) {
    ERROR ();
  }

  if (0x3FFC0030 /*0x3FFD0037*/ != dotproductU48_U16byU16 (cr16, cr16, 5, &msu)) {
    ERROR ();
  }
  if (1 != msu) {
    ERROR ();
  }

  if (0x3FF20030 /*0x3FFD0037*/ != dotproductI48_I16byI16 (cr16, cr16, 5, &msw)) {
    ERROR ();
  }
  if (1 != msw) {
    ERROR ();
  }
#endif

  	uint16 i;
	int32 sum;
	int16 msb48;

	sum = 0;
	for (i = 1; i <= 16; i++) {
	  if ((i%2) == 1) {
	    sum += (int32) (i/2+1) * (i/2+1);
	  } else {
	    sum += 0x40000000 - (int32) (i/2) * 65536 + (int32) (i/2) * (i/2);
	  }
	  if (sum != dotproductI48_I16byI16 ((int16 *)br16, (int16 *)br16, i, &msb48)) {
	    ERROR ();
	  }
	  if (i >= 2) {
	    if (msb48 != (int16)( (i-2) / 8) ) {
	      ERROR ();
	    }
	  } else if (msb48 != 0) {
	    ERROR ();
	  } else {
	    ; /* ok */
	  }
	}
}	/* dotproductI48_I16byI16_test */

/* EOF */
