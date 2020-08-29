/*
 * Copyright (C) 2008-2012 Melexis N.V.
 *
 * Math Library
 *
 */

/* NB: 
   1. beware of n * pi/4 values, e.g. for sign
   
   2. not fully symmetric
   e.g. for sin/cosI16, the negative values have a larger deviation, e.g. 3 vs 1 bits ..
*/

#ifndef WIN32

#include "typelib.h"
#include "mathlib.h"

#else

typedef unsigned long long 	uint64;
typedef unsigned int 	uint32;
typedef   signed int 	int32;
typedef unsigned short  uint16;
typedef   signed short  int16;

#include <math.h>

#define U16(f) ((uint16) ((f) * 65536))
#define F16(u) (((u) / 65536.0))
#define U32(f) ((uint16) ((f) * 65536 * 65536))
#define F32(u) (((u) / (65536 * 65536.0)))
static uint16 divU16_U16byU16 (uint16 x, uint16 y)
{
  return (U16(F16(x) / F16(y)));
}

static uint32 divU32_U32byU16 (uint32 x, uint16 y)
{
  return (uint32) ((x * 1.0) / y);
}

#endif /* ! WIN32 */

#if defined(__MLX16__) && !defined(HAS_MLX16_COPROCESSOR)
#define USE_TRIGONOMETRIC_LOOKUPTABLES_
#endif 

#ifndef USE_TRIGONOMETRIC_LOOKUPTABLES_
/* temporary - the asm supporting functions */
extern uint16 tan3rad_helper (uint16 x);
extern uint16 tan5rad_helper (uint16 x);
extern uint16 tan7rad_helper (uint16 x);
extern int16 sin_helper (uint16 x);
extern int16 cos_helper (uint16 x);

#if 0
/* below constants are TBC
   they can all be zero for MLX16
*/
uint16 tan3rad (uint16 x)
{
  return (x < 256 ? x : tan3rad_helper (x));
}

uint16 tan5rad (uint16 x)
{
  return (x < 363 ? x : tan5rad_helper (x));
}

uint16 tan7rad (uint16 x)
{
  return (x < 363 ? x : tan7rad_helper (x));
}
#endif /* 0 */

/* 0 <= x <= pi/4 * 2^16 */
uint16 tanrad_helper (uint16 x)
{
  return (x < 2950 ? x : tan7rad_helper (x));
}

/* note that (2)pi > 1 (=65536), so full circle can not be immediately represented */
/* also results is signed, so losing another bit of precision */

#define U16(f) ((uint16) ((f) * 65536))

static inline uint16 twopi (uint16 x)
{
  return (uint16) (((uint32) x * U16(3.1415926 / 4)) >> 13);
}

#else

/* could outcode some common code as known in first quadrant */
extern int16 sin_lookup (uint16 x);

#endif /* ! USE_TRIGONOMETRIC_LOOKUPTABLES_ */

/* 0 <= x <= 1/4 * 2^16 (0-90 degrees) */
/* NB: tan > 1 for x > 45degrees, hence 32bit (16 fractional bits) */
int32 tan_helper (uint16 x)
{
  int16 sin;
  int16 cos;

  int32 result;

#ifndef USE_TRIGONOMETRIC_LOOKUPTABLES_
  /* for small values of x, use Taylor series 
     as otherwise double number of instructions for sin+cos 
     for larger values calculate sin/cos 
     as no good conversion > pi/4 ...
  */

  /* 29739 / 2pi (26deg) */
  if (x < 4733) {
    return (int32) (tanrad_helper (twopi (x))/* >> 1*/);
  }

  /* to do: support full quadrant whereas sin/cos_helper only one octant */
  /* could outcode the * twopi in both sin/cos functions */
  /* sin_helper(0x2000) != cos_helper (0x2000) :-(
     would require use of sin/cos but that adds another 12+ instructions
  */
  if (x == 0x2000) { 
    /* alt: return (1UL << 16); */
    sin = sin_helper(x); 
    cos = sin;
  } else if (x > 0x2000) {
    /* 45 - 90 degrees */
    sin = cos_helper (0x4000U - x);
    cos = sin_helper (0x4000U - x);
  } else {
    /* first octant */
    sin = sin_helper (x);
    cos = cos_helper (x);
  }
#else /* USE_TRIGONOMETRIC_LOOKUPTABLES_ */
#if 0
  if (x < 469) {
    return (twopi (x));
  }
#endif /* 0 */

  sin = sin_lookup (x); /* sinU16 (x) */ 
  cos = sin_lookup (0x4000U - x); /* cosU16 (x) */ 
#endif /* ! USE_TRIGONOMETRIC_LOOKUPTABLES_ */

  /* NB: for divU16, x should be larger than y, otherwise overflow
     for sin/cos for 0 < x < pi/4 this should be the case, 
     but close to pi/4 and rounding in approximations, might not be the case ..
  */
  if (sin < cos) { /* no need to take abs as in first quadrant */
    result = (int32) (divU16_U16byU16 (sin, cos) /* >> 1*/);
  } else {
    /* very close to pi/4 if ensured in first octant */
    /* was: return 32767; */
    /* NB: divu(.., 0) returns 0xFFFF; same for divi(..,0) ... */
    if (0 == cos) {
      /* could hoist up to start of function to check for almost 90 degrees */
      result = (int32) 0x7FFFFFFF; /* max signed */
    } else {
      /* ok to use divU32 as in first quadrant */
      result = (int32) (divU32_U32byU16 (((uint32) sin) << 16, cos) /*>> 1*/);
    }
  }

  return result;
}

/* 0 < x < 65536(1->2pi) */
/* NB: tan > 1 for x > 45degrees, hence 32bit (16 fractional bits) */
int32 tanU16 (uint16 x)
{
  int32 result;

  /* get quadrant */
  switch (((uint16) x) >> 14) {
  case 0:
    /* 0 < x < 90 */
    result = tan_helper (x);
    break;
  case 1:
    /* 90 < x < 180 */
    result = - tan_helper (0x8000U - x); /* 180 - x */
    break;
  case 2:
    /* 180 < x < 270 */
    result = tan_helper (x - 0x8000U); /* x - 180 */
    break;
  case 3:
    /* 270 < x < 360 */
    result = - tan_helper (/* 0x10000 */ - x); /* 360 - x */
    break;
  default:
    /* error */
    result = 0;
    break;
  }

  return result;
}

/* 32768(-1->-pi) < x < 32768(1->pi) */
/* NB: tan > 1 for x > 45degrees, hence 32bit (16 fractional bits) */
/* exactly same code as for tanU16 */
#ifdef __MLX16__ /* GNUC? */
int32 tanI16 (int16 x) __attribute__((alias("tanU16")));
#else
/* 32768(-1->-pi) < x < 32768(1->pi) */
/* exactly same code as for sinU16 */
int32 tanI16 (int16 x) 
{
  return tanU16 ((uint16) x);
}
#endif /* __MLX16__ */

/* EOF */
