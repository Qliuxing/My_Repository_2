/*
 * Copyright (C) 2007-2012 Melexis N.V.
 *
 * Math Library
 *
 */

#include "typelib.h"
#include "mathlib.h"

#if ((__MLX16_GCC_MAJOR__ == 1) && (__MLX16_GCC_MINOR__ >= 8)) || (__MLX16_GCC_MAJOR__ > 1)
    /* ok , library routines have been renamed in MLX16-GCC v1.8 */
#else
#define	MLX16GCC_REV12_OR_ABOVE
#endif


#ifdef MLX16GCC_REV12_OR_ABOVE

#if defined (HAS_MLX16_COPROCESSOR)

#define GCC_mulI32_I16byI16
#define GCC_mulU32_U16byU16

#else

#endif /* HAS_MLX16_COPROCESSOR */

#else /* MLX16GCC_REV12_OR_ABOVE not defined */

#define GCC_mulI32_I16byI16
#define GCC_mulU32_U16byU16

#endif


/*
 *	Multiplication
 */

/* 32 = 16 x 16 */
#ifdef GCC_mulI32_I16byI16
int32 mulI32_I16byI16(int16 a, int16 b)
{
	return (int32)a * b;
}
#endif

#ifdef GCC_mulU32_U16byU16
uint32 mulU32_U16byU16(uint16 a, uint16 b)
{
	return (uint32)a * b;
}
#endif

int32 mulI32_I16byU16(int16 a, uint16 b)
{
	return (int32)a * b;
}

/* 16 = 16 x 16 */

int16 mulI16_I16byI16(int16 a, int16 b)
{
  	int32 acc = (int32)a * b;
	return (int16)(acc >> 16);
}

int16 mulI16_I16byU16(int16 a, uint16 b)
{
	int32 acc = (int32)a * b;
	return (int16)(acc >> 16);
}

uint16 mulU16_U16byU16(uint16 a, uint16 b)
{
	uint32 acc = (uint32)a * b;
	return (uint16)(acc >> 16);
}

/* Q15 = Q15 x Q15 */
int16 mulQ15_Q15byQ15(int16 a, int16 b)
{
	int32 acc = (int32)a * b;
	return (int16)(acc >> 15);
}

/* 32 = 32 x 16 */

int32 mulI32_I32byI16(int32 a, int16 b)
{
	int32 acc;

    	acc = (int32)b * ((int16)(a>>16));

	acc <<= 16;

	acc += (int32)b * ((uint16)a);

	return acc;
}

int32 mulI32_I32byU16(int32 a, uint16 b)
{
	int32 acc;

	acc = (int32)b * ((int16)(a>>16));

	acc <<= 16;

	acc += (int32)b * ((uint16)a);

	return acc;
}

uint32 mulU32_U32byU16(uint32 a, uint16 b)
{
	uint32 acc;

	acc = (uint32)b * ((uint16)(a>>16));

	acc <<= 16;

	acc += (uint32)b * ((uint16)a);

	return acc;
}

/* 32hi = 32 x 16 */

/* to be optimized */
int32 mulI32hi_I32byI16(int32 a, int16 b)
{
  	int64 acc = ((int64) a) * b;

	return (int32) (acc >> 16);
}

int32 mulI32hi_I32byU16(int32 a, uint16 b)
{
  	int64 acc = ((int64) a) * b;

	return (int32) (acc >> 16);
}

uint32 mulU32hi_U32byU16(uint32 a, uint16 b)
{
  	uint64 acc = ((uint64) a) * b;

	return (uint32) (acc >> 16);
}

/* 24 = 16 x 8 */

int32 mulI24_I16byI8(int16 a, int8 b)
{
	int32 acc = ((int32) a) * b;

	return acc;
}

int32 mulI24_I16byU8(int16 a, uint8 b)
{
  	int32 acc = ((int32) a) * b;

	return acc;
}

uint32 mulU24_U16byU8(uint16 a, uint8 b)
{
	uint32 acc = ((uint32) a) * b;

	return acc;
}

/* 24 (16hi) = 16 x 8 */

/* beware of truncation difference for negative results */
int16 mulI16hi_I16byI8(int16 a, int8 b)
{
  	int32 acc = ((int32) a) * b;

	return (int16) (acc >> 8);
}

/* beware of truncation difference for negative results */
int16 mulI16hi_I16byU8(int16 a, uint8 b)
{
  	int32 acc = ((int32) a) * b;

	return (int16) (acc >> 8);
}

uint16 mulU16hi_U16byU8(uint16 a, uint8 b)
{
	uint32 acc = ((uint32) a) * b;

	return (uint16) (acc >> 8);
}

/* 16 = 8 x 8 */

int16 mulI16_I8byI8(int8 a, int8 b)
{
  	int16 acc = ((int16) a) * b;

	return acc;
}

int16 mulI16_I8byU8(int8 a, uint8 b)
{
  	int16 acc = ((int16) a) * b;

	return acc;
}

uint16 mulU16_U8byU8(uint8 a, uint8 b)
{
	uint16 acc = ((uint16) a) * b;

	return acc;
}



/*
 *	Division
 */

uint32 divU32_U32byU16(uint32 n, uint16 d)
{
	int i;
	uint32 q, r, t;
	uint16 carry;

	q = 0;
	r = 0;

	for(i = 0; i < 32; ++i)
	{
		t = (r << 1) | (n >> 31);
		n <<= 1;

		carry = (t >= d) ? 1 : 0;

		if(carry != 0)
		{
			r = t - d;
		}
		else
		{
			r = t;
		}

		q = (q << 1) | carry;
	}

	return q;
}

int32 divI32_I32byU16(int32 n, uint16 d)
{
	int32 res;

	if(n < 0)
	{
	  	res = - (int32) divU32_U32byU16((uint32)(-n), d);
	}
	else
	{
	  res = (int32) divU32_U32byU16((uint32) n, d);
	}

	return res;
}

int32 divI32_I32byI16(int32 n, int16 d)
{
	int32 res;

	if(d < 0)
	{
	  	res = - (int32) divI32_I32byU16(n, (uint16)(-d));
	}
	else
	{
	  	res = (int32) divI32_I32byU16(n, d);
	}

	return res;
}

uint16 divU16_U32byU16(uint32 n, uint16 d)
{
	return (uint16)divU32_U32byU16(n, d);
}

int16 divI16_I32byU16(int32 n, uint16 d)
{
	int16 res;

	if(n < 0)
	{
	  	res = - (int16) divU16_U32byU16((uint32) (-n), d);
	}
	else
	{
	  res = (int16) divU16_U32byU16((uint32) n, d);
	}

	return res;
}

int16 divI16_I32byI16(int32 n, int16 d)
{
	int16 res;

	if(d < 0)
	{
	  	res = - (int16) divI16_I32byU16(n, (uint16) (-d));
	}
	else
	{
	  	res = (int16) divI16_I32byU16(n, d);
	}

	return res;
}

uint16 divU16_U16byU16(uint16 n, uint16 d)
{
	return (uint16)divU32_U32byU16(((uint32)n) << 16, d);
}

int16 divI16_I16byU16(int16 n, uint16 d)
{
	int16 res;

	if(n < 0)
	{
	  	res = - (int16) divU16_U16byU16((uint16)(-n), d);
	}
	else
	{
	  	res = (int16) divU16_U16byU16(n, d);
	}

	return res;
}

int16 divI16_I16byI16(int16 n, int16 d)
{
	int16 res;

	if(d < 0)
	{
	  	res = - (int16) divI16_I16byU16(n, (uint16) (-d));
	}
	else
	{
	  	res = (int16) divI16_I16byU16(n, d);
	}

	return res;
}

uint8 divU8_U8byU8(uint8 n, uint8 d)
{
  return (uint8) ((uint16) n / (uint16) d);
}

int8 divI8_I8byU8(int8 n, uint8 d)
{
  return (int8) ((int16) n / (int16) d);
}

int8 divI8_I8byI8(int8 n, int8 d)
{
  return (int8) ((int16) n / (int16) d);
}

uint8 divU8hi_U8byU8(uint8 n, uint8 d)
{
  return (uint8) ((uint16) n * 256 / (uint16) d);
}

#if 0 /* deprecated */
int8 divI8hi_I8byU8(int8 n, uint8 d)
{
  return (int8) ((int16) n * 256 / (int16) d);
}

int8 divI8hi_I8byI8(int8 n, int8 d)
{
  return (int8) ((int16) n * 256 / (int16) d);
}

#endif 

/* rand32 */

uint32 rand32(uint32 seed)
{
	seed = seed * 69609 + 12345;

	return seed;
}

/* EOF */
