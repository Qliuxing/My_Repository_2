/*
 * Copyright (C) 2008 Melexis N.V.
 *
 * Math Library
 *
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

#endif /* ! WIN32 */


#define U16(f) ((uint16) ((f) * 65536))

uint16 sin3rad_helper (uint16 x)
{
  uint32 x2;
  uint32 x26;
  uint16 x3;

  x2 = ((uint32) x) * x;

  x3 = (uint16) (x2 >> 16);
  (uint16) (  x26 = ((uint32) x3) * U16(1/6.0));
  x26 = -x26;
  x3 = (uint16) (x26 >> 16);

  x2 = ((uint32) x3) * x;
  x3 = (uint16) (x2 >> 16);

  return x3;
}

uint16 sin5rad_helper (uint16 x)
{
  uint32 x2;
  uint32 x20;
  uint32 x26;
  uint16 x3;
  uint16 x4;

  x2 = ((uint32) x) * x;

  x3 = (uint16) (x2 >> 16);
  x20 = ((uint32) x3) * U16(1/20.0);
  x20 = -x20;
  x3 = (uint16) (x20 >> 16);

  x4 = (uint16) (x2 >> 16);
  x26 = ((uint32) x4) * x3;
  x4 = (uint16) (x26 >> 16);
  x2 = ((uint32) x4) * U16(1/6.0);
  x2 = -x2;
  x4 = (uint16) (x2 >> 16);

  x2 = ((uint32) x4) * x;
  x4 = (uint16) (x2 >> 16);

  return x4;
}

uint16 sin7rad_helper (uint16 x)
{
  uint32 x2;
  uint32 x20;
  uint32 x6;
  uint32 x42;
  uint16 x3;
  uint16 x4;

  x2 = ((uint32) x) * x;

  x3 = (uint16) (x2 >> 16);
  x42 = ((uint32) x3) * U16(1/42.0);
  x42 = -x42;
  x3 = (uint16) (x42 >> 16);

  x4 = (uint16) (x2 >> 16);
  x20 = ((uint32) x4) * x3;
  x4 = (uint16) (x20 >> 16);
  x20 = ((uint32) x4) * U16(1/20.0);
  x20 = -x20;
  x4 = (uint16) (x20 >> 16);

  x3 = (uint16) (x2 >> 16);
  x6 = ((uint32) x3) * U16(1/6.0);
  x3 = (uint16) (x6 >> 16);
  x6 = ((uint32) x3) * x4;
  x6 = -x6;
  x3 = (uint16) (x6 >> 16);

  x2 = ((uint32) x3) * x;
  x3 = (uint16) (x2 >> 16);

  return x3;
}

uint16 cos2rad_helper (uint16 x)
{
  uint32 x2;
  uint16 x3;

  x2 = ((uint32) x) * x;
  x2 /= 2;

  x2 = -x2;
  x3 = (uint16) (x2 >> 16);

  return x3;
}

uint16 cos4rad_helper (uint16 x)
{
  /* 1- x^2/2 + x^4/24 =
     1 - x^2/2 * (1 - x^2 / 12)
  */

  uint32 x2;
  uint32 x12;
  uint32 x22;
  uint16 x3;
  uint16 x4;

  x2 = (uint32) x * x;

  x3 = (uint16) (x2 >> 16);
  x12 = ((uint32) x3) * U16(1/12.0);
  x12 = -x12;
  x3 = (uint16) (x12 >> 16);

  x4 = (uint16) (x2 >> 16);
  x22 = ((uint32) x4) * x3;
  x4 = (uint16) (x22 >> 16);
  x2 = ((uint32) x4) * U16(1/2.0);
  x2 = -x2;
  x4 = (uint16) (x2 >> 16);

  return x4;
}


uint16 cos6rad_helper (uint16 x)
{
  /* 1-x^2/2 + x^4/24 - x^6/720 =
     1-x^2/2*(1-x^2/12*(1-x^2/30))
  */

  uint32 x2;
  uint32 x230;
  uint32 x212;
  uint32 x22;
  uint16 x3;
  uint16 x4;

  x2 = (uint32) x * x;

  x3 = (uint16) (x2 >> 16);
  x230 = ((uint32) x3) * U16(1/30.0);
  x230 = -x230;
  x3 = (uint16) (x230 >> 16);

  x4 = (uint16) (x2 >> 16);
  x212 = ((uint32) x4) * x3;
  x4 = (uint16) (x212 >> 16);
  x212 = ((uint32) x4) * U16(1/12.0);
  x212 = -x212;
  x4 = (uint16) (x212 >> 16);

  x3 = (uint16) (x2 >> 16);
  x22 = ((uint32) x3) * x4;
  x3 = (uint16) (x22 >> 16);
  x22 = ((uint32) x3) * U16(1/2.0);
  x22 = -x22;
  x3 = (uint16) (x22 >> 16);

  return x3;
}

/* EOF */
