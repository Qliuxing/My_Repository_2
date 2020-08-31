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

uint16 tan3rad_helper (uint16 x)
{
  uint32 x2;
  uint32 x23;
  uint16 x3;

  x2 = ((uint32) x) * x;

  x3 = (uint16) (x2 >> 16);
  x23 = ((uint32) x3) * U16(1/3.0);
  x3 = (uint16) (x23 >> 16);

  x2 = ((uint32) x3) * x;
  x3 = (uint16) (x2 >> 16);

  x3 += x;

  return x3;
}

uint16 tan5rad_helper (uint16 x)
{
  uint32 x2, x23, x4;
  uint16 x3, x45, x5;

  x2 = (uint32) x * x;
  x3 = (uint16) (x2 >> 16);

  x23 = (uint32) x3 * U16(1/3.0);
  
  x4 = (uint32) x3 * x3;
  x45 = (uint16) (x4 >> 16);
  x23 += (uint32) x45 * U16(2/15.0);

  x5 = (uint16) (x23 >> 16);
  x23 = (uint32) x5 * x;
  x5 = (uint16) (x23 >> 16);
  
  x5 += x;

  return x5;
}

uint16 tan7rad_helper (uint16 x)
{
  uint32 x2, x23, x4, x5;
  uint16 x3, x45, x55, x7;

  x2 = (uint32) x * x;
  x3 = (uint16) (x2 >> 16);

  x23 = (uint32) x3 * U16(1/3.0);
  
  x4 = (uint32) x3 * x3;
  x45 = (uint16) (x4 >> 16);
  x23 += (uint32) x45 * U16(2/15.0);

  x5 = (uint32) x45 * x3;
  x55 = (uint16) (x5 >> 16);
  x23 += (uint32) x55 * U16(17/315.0);

  x7 = (uint16) (x23 >> 16);
  x23 = (uint32) x7 * x;
  x7 = (uint16) (x23 >> 16);
  
  x7 += x;

  return x7;
}

/* EOF */
