/*
 * Copyright (C) 2008-2014 Melexis N.V.
 *
 * Math Library
 *
 */

/* beware of compiling with -O3 vs -Os
   some versions of twopi compile bad/good when inlining
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

#endif /* ! WIN32 */

#if defined(__MLX16__) && !defined(HAS_MLX16_COPROCESSOR)
#define USE_TRIGONOMETRIC_LOOKUPTABLES_
#endif 

#ifndef USE_TRIGONOMETRIC_LOOKUPTABLES_
/* temporary - the asm supporting functions */
extern uint16 sin3rad_helper (uint16 x);
extern uint16 sin5rad_helper (uint16 x);
extern uint16 sin7rad_helper (uint16 x);
extern uint16 cos2rad_helper (uint16 x);
extern uint16 cos4rad_helper (uint16 x);
extern uint16 cos6rad_helper (uint16 x);


#if 0
/* below constants are for MLX16 asm; 
   win32 gcc seem to be different ... 
*/
uint16 sin3rad (uint16 x)
{
  return (x < 256 ? x : sin3rad_helper (x));
}

uint16 sin5rad (uint16 x)
{
  return (x < 363 ? x : sin5rad_helper (x));
}

uint16 sin7rad (uint16 x)
{
  return (x < 363 ? x : sin7rad_helper (x));
}

uint16 cos2rad (uint16 x)
{
  return (x < 2 ? 65535 : cos2rad_helper (x));
}

uint16 cos4rad (uint16 x)
{
  return (x < 256 ? 65535 : cos4rad_helper (x));
}

uint16 cos6rad (uint16 x)
{
  return (x < 363 ? 65535 : cos6rad_helper (x));
}
#endif /* 0 */

/* 0 <= x <= pi/4 * 2^16 */
uint16 sinrad_helper (uint16 x)
{
  return (x < 2950 ? x : sin5rad_helper (x));
}

/* 0 <= x <= pi/4 * 2^16 */
uint16 cosrad_helper (uint16 x)
{
  /* can be upto 443 or 512? */
  return (x < 362 ? 65535 : cos6rad_helper (x));
}

#define U16(f) ((uint16) ((f) * 65536))

static inline uint16 twopi (uint16 x)
{
  return (uint16) (((uint32) x * U16(3.1415926 / 4)) >> 13);
}
#endif /* ! USE_TRIGONOMETRIC_LOOKUPTABLES_ */

#ifdef USE_TRIGONOMETRIC_LOOKUPTABLES_
/* forward declaration */
int16 sin_lookup (uint16 x);
#endif /* ! USE_TRIGONOMETRIC_LOOKUPTABLES_ */

/* 0 <= x <= 1/8 * 2^16 */
int16 sin_helper (uint16 x)
{
#ifdef USE_TRIGONOMETRIC_LOOKUPTABLES_
  /* alternatively also return uint16 for more precision for quadrant/octant */
  return sin_lookup (x);
#else
  return (int16) (sinrad_helper (twopi (x)) >> 1);
#endif  /* USE_TRIGONOMETRIC_LOOKUPTABLES_ */
}

/* 0 <= x <= pi/4 * 2^16 */
int16 cos_helper (uint16 x)
{
#ifdef USE_TRIGONOMETRIC_LOOKUPTABLES_
  /* alternatively also return uint16 for more precision for quadrant/octant */
  return sin_lookup (0x4000U - x);
#else
#if 1
  return (int16) (cosrad_helper (twopi (x)) >> 1);
#else
  /* 362 (443?, 512?) / 2pi */
  if (x < 58) {
    return (int16) (65535 >> 1);
  }
  return (int16) (cosrad_helper (twopi(x)) >> 1);
#endif
#endif  /* USE_TRIGONOMETRIC_LOOKUPTABLES_ */
}

/* 0 < x < 65536=2pi */
int16 sinU16 (uint16 x)
{
  int16 result;

  /* horizontal mirror */
  if ( ( x & 0x4000 ) != 0 ) {
    /* 90 < x < 180 */
    /* -90 < x < 0 , 270 < x < 360 */
    x = 0x8000 - x; /* 180 - x */
  }
  if ((x & 0x8000) != 0) {
    /* vertical mirror */
    x &= ~0x8000;
    if (x < 0x2000) {
      result = - sin_helper (x);
    }
    else {
      result = - cos_helper (0x4000U - x);
    }
  } else {
    if (x < 0x2000) {
      result = sin_helper (x);
    }
    else {
      result = cos_helper (0x4000U - x);
    }
  }

  return result;
}

/* 0 < x < 65536=2pi */
int16 cosU16 (uint16 x)
{
  return sinU16 (0x4000U-x);
}

/* 32768(-1->-pi) < x < 32768(1->pi) */
/* exactly same code as for sinU16 */
#ifdef __MLX16__ /* GNUC? */
/* 32768(-1->-pi) < x < 32768(1->pi) */
/* exactly same code as for sinU16 */
int16 sinI16 (int16 x) __attribute__((alias("sinU16")));
#else
/* 32768(-1->-pi) < x < 32768(1->pi) */
/* exactly same code as for sinU16 */
int16 sinI16 (int16 x) 
{
  return sinU16 ((uint16) x);
}
#endif /* __MLX16__ */


#ifdef __MLX16__ /* GNUC? */
/* 32768(-1->-pi) < x < 32768(1->pi) */
/* exactly same code as for cosU16 */
int16 cosI16 (int16 x) __attribute__((alias("cosU16")));
#else
/* 32768(-1->-pi) < x < 32768(1->pi) */
/* exactly same code as for cosU16 */
int16 cosI16 (int16 x) 
{
  return cosU16 ((uint16) x);
}
#endif /* __MLX16__ */


#ifdef USE_TRIGONOMETRIC_LOOKUPTABLES_

#ifndef LOOKUPTABLESIZE
#define LOOKUPTABLESIZE 256
#endif /* ! LOOKUPTABLESIZE */

#if LOOKUPTABLESIZE == 256
#define LUT_VAL8(v1, v2, v3, v4, v5, v6, v7, v8) v1, v2, v3, v4, v5, v6, v7, v8
#define LUT_FINAL_VAL8(v1, v2, v3, v4, v5, v6, v7, v8) v1
#elif LOOKUPTABLESIZE == 128
#define LUT_VAL8(v1, v2, v3, v4, v5, v6, v7, v8) v1,     v3,     v5,     v7
#define LUT_FINAL_VAL8(v1, v2, v3, v4, v5, v6, v7, v8) v2
#elif LOOKUPTABLESIZE == 64
#define LUT_VAL8(v1, v2, v3, v4, v5, v6, v7, v8) v1,             v5
#define LUT_FINAL_VAL8(v1, v2, v3, v4, v5, v6, v7, v8) v4
#elif LOOKUPTABLESIZE == 32
#define LUT_VAL8(v1, v2, v3, v4, v5, v6, v7, v8) v1
#define LUT_FINAL_VAL8(v1, v2, v3, v4, v5, v6, v7, v8) v8
#else
#error "Supported LOOKUPTABLESIZE values 32, 64, 128, 256"
#endif

/* one quadrant */
/* S16, to be updated for U16 */
static const uint16 SIN_LUT[ LOOKUPTABLESIZE+1 ] =
  {
    LUT_VAL8(      0,    201,    402,    603,    804,   1005,   1206,   1406),
    LUT_VAL8(   1607,   1808,   2009,   2209,   2410,   2610,   2811,   3011),
    LUT_VAL8(   3211,   3411,   3611,   3811,   4011,   4210,   4409,   4609),
    LUT_VAL8(   4807,   5006,   5205,   5403,   5601,   5799,   5997,   6195),
    LUT_VAL8(   6392,   6589,   6786,   6983,   7179,   7375,   7571,   7766),
    LUT_VAL8(   7961,   8156,   8351,   8545,   8739,   8933,   9126,   9319),
    LUT_VAL8(   9511,   9704,   9895,  10087,  10278,  10469,  10659,  10849),
    LUT_VAL8(  11038,  11228,  11416,  11605,  11792,  11980,  12167,  12353),
    LUT_VAL8(  12539,  12725,  12910,  13094,  13278,  13462,  13645,  13827),
    LUT_VAL8(  14009,  14191,  14372,  14552,  14732,  14911,  15090,  15268),
    LUT_VAL8(  15446,  15623,  15799,  15975,  16150,  16325,  16499,  16673),
    LUT_VAL8(  16845,  17017,  17189,  17360,  17530,  17700,  17868,  18037),
    LUT_VAL8(  18204,  18371,  18537,  18702,  18867,  19031,  19195,  19357),
    LUT_VAL8(  19519,  19680,  19841,  20000,  20159,  20317,  20475,  20631),
    LUT_VAL8(  20787,  20942,  21096,  21250,  21402,  21554,  21705,  21855),
    LUT_VAL8(  22005,  22153,  22301,  22448,  22594,  22739,  22883,  23027),
    LUT_VAL8(  23170,  23311,  23452,  23592,  23731,  23869,  24007,  24143),
    LUT_VAL8(  24279,  24413,  24547,  24679,  24811,  24942,  25072,  25201),
    LUT_VAL8(  25329,  25456,  25582,  25707,  25832,  25955,  26077,  26198),
    LUT_VAL8(  26319,  26438,  26556,  26673,  26790,  26905,  27019,  27132),
    LUT_VAL8(  27245,  27356,  27466,  27575,  27683,  27790,  27896,  28001),
    LUT_VAL8(  28105,  28208,  28310,  28410,  28510,  28609,  28706,  28802),
    LUT_VAL8(  28898,  28992,  29085,  29177,  29268,  29358,  29447,  29534),
    LUT_VAL8(  29621,  29706,  29791,  29874,  29956,  30037,  30116,  30195),
    LUT_VAL8(  30273,  30349,  30424,  30498,  30571,  30643,  30714,  30783),
    LUT_VAL8(  30851,  30919,  30985,  31049,  31113,  31176,  31237,  31297),
    LUT_VAL8(  31356,  31414,  31470,  31526,  31580,  31633,  31685,  31735),
    LUT_VAL8(  31785,  31833,  31880,  31926,  31971,  32014,  32056,  32097),
    LUT_VAL8(  32137,  32176,  32213,  32249,  32284,  32318,  32351,  32382),
    LUT_VAL8(  32412,  32441,  32469,  32495,  32520,  32544,  32567,  32589),
    LUT_VAL8(  32609,  32628,  32646,  32663,  32678,  32692,  32705,  32717),
    LUT_VAL8(  32727,  32737,  32745,  32751,  32757,  32761,  32764,  32766),
    32767
  } ;

/* x in range -32768..32767 (-1..1) -> -pi..pi 
   or 0..65535 [0..1)
*/
/* only called for first quadrant 0 .. 16384 (incl) */
int16 sin_lookup (uint16 x)
{
  uint16 interp ;
  uint16 index ;

  /* assert (x <= 0x4000) */

  if (x == 0x4000) {
    return 32767;
  }

  /* interpolation:
     MSW: lookup table index
     LSW: interpolation value
  */
  /* lut size -> index 
     256 6
     128 7
     64  8
     32  9
     16  10
   */
#define LUT_FACTOR (65536/4/LOOKUPTABLESIZE)
  /* ensure that below is compiled into simple shifts 
     and also not additional 32 bits.. 
  */
  interp = x % LUT_FACTOR;
  /* index = hidden masking bit15,b14 (0x3FFF) */
  index = (x / LUT_FACTOR) & 0x00FF;

  /* beware of overflow due to SIN_LUT element size and interp value */
  /* for LOOKUPTABLESIZE=256 and 128 and SIN_LUT element difference < 512, could use *
        for MLX16-x8 this is even better as than single insn instead of a call
	for MLX16(-8) could use mulU16_U8byU8 to save a few CPU insn 
  */
  
  return  (int16) (SIN_LUT[ index ] +

  /* #if LUT_FACTOR <= .. */
#if LOOKUPTABLESIZE >= 128
		   (((SIN_LUT[index + 1] - SIN_LUT[index] ) * interp) / ((uint16) LUT_FACTOR))

#else
		   ((uint16) (mulU32_U16byU16 ((SIN_LUT[index + 1] - SIN_LUT[index] ), interp) / ((uint16) LUT_FACTOR)))
#endif
		   );
} 
#endif /* USE_TRIGONOMETRIC_LOOKUPTABLES_ */
/* EOF */
