/*
 * Copyright (C) 2008-2014 Melexis N.V.
 *
 * Math Library
 *
 */

/* Use of lookup table, as Taylor series does not converge very rapidly
   0 < x (ratio) < 1 (65536 -> pi/4 or 1/8th)
   atan = x - x^3 / 3 + x^5 / 5 - x^7 / 7 + ..
        = x (1 - x^2 / 3 (1 - 3 x^2 / 5 (1 - 15 x^2 / 7 (1 - ..))))
	note also: 15 / 7 > 1
*/

/* N.B. check for cornercases n*pi/4
   define right types for functions/arguments
   to add: atan functions = atan (y, 1);
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
static uint16 divU16_U16byU16 (uint16 x, uint16 y)
{
  return (U16(F16(x) / F16(y)));
}

uint32 mulU32_U16byU16 (uint16 a, uint16 b)
{
  return ((uint32) a ) * b;
}

#endif /* ! WIN32 */


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
#define LUT_VAL8(v1, v2, v3, v4, v5, v6, v7, v8) 0
#define LUT_FINAL_VAL8(v1, v2, v3, v4, v5, v6, v7, v8) 0
#endif

/* one "octant", i.e. for input values 0..1 (0..65536) */
/* S16, to be updated for U16 */
static const uint16 ATAN_LUT[ LOOKUPTABLESIZE+2 ] = 
{
  LUT_VAL8(      0,    163,    326,    489,    652,    815,    978,   1141),
  LUT_VAL8(   1303,   1466,   1629,   1792,   1954,   2117,   2279,   2442),
  LUT_VAL8(   2604,   2767,   2929,   3091,   3253,   3415,   3577,   3738),
  LUT_VAL8(   3900,   4061,   4223,   4384,   4545,   4706,   4867,   5028),
  LUT_VAL8(   5188,   5349,   5509,   5669,   5829,   5989,   6148,   6308),
  LUT_VAL8(   6467,   6626,   6784,   6943,   7101,   7260,   7418,   7575),
  LUT_VAL8(   7733,   7890,   8047,   8204,   8361,   8517,   8673,   8829),
  LUT_VAL8(   8985,   9140,   9296,   9450,   9605,   9759,   9914,  10067),
  LUT_VAL8(  10221,  10374,  10527,  10680,  10832,  10984,  11136,  11287),
  LUT_VAL8(  11439,  11590,  11740,  11890,  12040,  12190,  12339,  12488),
  LUT_VAL8(  12637,  12785,  12933,  13081,  13228,  13375,  13522,  13668),
  LUT_VAL8(  13814,  13959,  14105,  14249,  14394,  14538,  14682,  14825),
  LUT_VAL8(  14968,  15111,  15253,  15395,  15537,  15678,  15819,  15960),
  LUT_VAL8(  16100,  16239,  16379,  16518,  16656,  16794,  16932,  17069),
  LUT_VAL8(  17206,  17343,  17479,  17615,  17750,  17885,  18020,  18154),
  LUT_VAL8(  18288,  18421,  18554,  18687,  18819,  18951,  19083,  19213),
  LUT_VAL8(  19344,  19474,  19604,  19733,  19862,  19991,  20119,  20247),
  LUT_VAL8(  20374,  20501,  20627,  20753,  20879,  21004,  21129,  21254),
  LUT_VAL8(  21378,  21501,  21624,  21747,  21870,  21992,  22113,  22234),
  LUT_VAL8(  22355,  22475,  22595,  22714,  22834,  22952,  23070,  23188),
  LUT_VAL8(  23306,  23423,  23539,  23655,  23771,  23886,  24001,  24116),
  LUT_VAL8(  24230,  24344,  24457,  24570,  24682,  24795,  24906,  25017),
  LUT_VAL8(  25128,  25239,  25349,  25459,  25568,  25677,  25785,  25893),
  LUT_VAL8(  26001,  26108,  26215,  26321,  26427,  26533,  26638,  26743),
  LUT_VAL8(  26848,  26952,  27056,  27159,  27262,  27364,  27467,  27568),
  LUT_VAL8(  27670,  27771,  27871,  27972,  28072,  28171,  28270,  28369),
  LUT_VAL8(  28467,  28565,  28663,  28760,  28857,  28953,  29050,  29145),
  LUT_VAL8(  29241,  29336,  29430,  29525,  29619,  29712,  29805,  29898),
  LUT_VAL8(  29991,  30083,  30175,  30266,  30357,  30448,  30538,  30628),
  LUT_VAL8(  30718,  30807,  30896,  30985,  31073,  31161,  31248,  31336),
  LUT_VAL8(  31423,  31509,  31595,  31681,  31767,  31852,  31937,  32022),
  LUT_VAL8(  32106,  32190,  32273,  32357,  32439,  32522,  32604,  32686),
  32768,  
  LUT_FINAL_VAL8(32849, 32930, 33011, 33091, 33171, 33251, 33331, 33410) 
};

/* 
   Computes atan (y,x)
   0 <= y <= x < 65536
   
   Returns 0..8192, i.e. 0..pi/4 (45 deg)

   Uses look-up table for arctan
*/
int16 atan2_helper (uint16 y, uint16 x)
{
  uint16 ratio; /* y / x */

  /* interpolation:
     MSW: lookup table index
     LSW: interpolation value
  */
  uint16 index;
  uint16 interp;

  uint16 atan;

  /* divide y by x */
  if (x != 0) {
    /* y should be smaller than x as otherwise overflow */
    if (y < x) {
      ratio = divU16_U16byU16 (y, x);
    }
    else {
      return 0x2000; /* alt: interp = 0 and index = 256 */
    }
  } else {
    /* atan2(0,0) is specified as 0 */
    return 0; /* alt: ratio = 0 */
  }


  /* lut size -> index 
     256 8
     128 9
     64  10
     32  11
     16  12
   */
#define LUT_FACTOR (65536/LOOKUPTABLESIZE)
  /* ensure that below is compiled into simple shifts 
     and also not additional 32 bits.. 
  */
  interp = ratio % LUT_FACTOR;
  index  = ratio / LUT_FACTOR;

  /* atan in range of 0..8192, i.e. 0.. pi/4, 45 deg
     i.e. one octant
  */

  /* could use rounding for / 4; that helps atan2_id_test, but not atan_test */

  /* beware of overflow due to ATAN_LUT element size and interp value */
  /* for MLX16(-8) could use mulU16_U8byU8 to save a few CPU insn */
  /* for LOOKUPTABLESIZE=256 and ATAN_LUT element difference < 256, could use *
        for MLX16-x8 this is even better as than single insn instead of a call
	for MLX16(-8) could use mulU16_U8byU8 to save a few CPU insn 
  */
  atan = ( ATAN_LUT[ index ] +
#if LUT_FACTOR <= 256
	    (( ( ATAN_LUT[ index + 1 ] - ATAN_LUT[ index ] ) * interp ) / ((uint16) LUT_FACTOR))
#else
	   ((uint16) ((mulU32_U16byU16 (
					( ATAN_LUT[ index + 1 ] - ATAN_LUT[ index ] ),  interp )) / ((uint16) LUT_FACTOR)))
#endif
	   ) / 4;

  return (int16) atan;
}

/* 
   Computes atan (y/x)
   Result is an angle [-32768 .. 32768), i.e. [-1..1) (-pi..pi)

*/
static int16 atan2_lookup( uint16 y, uint16 x)
{
  uint16 result ;

  /* mirror to first octant, i.e. 0 < y / x < 1 */
  if (y <= x) {
    /* first octant already */
    result = (uint16) atan2_helper (y, x);
  } else {
    /* mirror to first octant */
    /* swap x and y */
    /* could use MLX16 swap asm instruction 
       if ensured that x and y are in Y/A 
    */
    result = (uint16) atan2_helper (x, y);
    result = 0x4000 - result; /* 90 - x */
  }

  return (int16) result ;
} 

int16 atan2U16( uint16 y, uint16 x)
{
  return atan2_lookup (y, x);
}

int16 atan2I16( int16 y, int16 x)
{
  int16 result;

  if (y < 0) {
    if (x < 0) {
      /*LDRA_INSPECTED 96 S */ /* LDRA ok with -(0x8000-atan2_lookup (-y,-x)) */
      result = atan2_lookup (-y, -x) - 0x8000; /* x - pi */
    } else {
      result = - atan2_lookup (-y, x);
    }
  } else {
    if (x < 0) {
      result = 0x8000 - atan2_lookup (y, -x); /* pi - x */
    } else {
      result = atan2_lookup (y, x);
    }
  }

  return result;
}

/* EOF */
