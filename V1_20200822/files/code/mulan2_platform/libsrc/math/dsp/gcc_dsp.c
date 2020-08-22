/*
 * Copyright (C) 2007-2010 Melexis N.V.
 *
 * Math Library - dsp module
 *
 */

#include "typelib.h"
#include "mathlib.h"
#include "dsp.h"

/* note: abs and labs defined as returning (signed) int */
#include <stdlib.h> /* abs, labs */

uint32 vecsumU32_U162(const uint16 *a)
{
  uint32 sum;
  
  sum = (uint32) a[0] + a[1];

  return sum;
}

uint32 vecsumU32_U322(const uint32 *a)
{
  uint32 sum;
  
  sum = a[0] + a[1];

  return sum;
}

uint32 vecsumU48_U322(const uint32 *a, uint16 *msw)
{
  uint64 sum;
  
  sum = (uint64) a[0] + (uint64) a[1];

  *msw = (uint16) (sum >> 32);

  return (uint32) sum;
}

uint32 dotproductU32_U16byU162 (const uint16 *a, const uint16 *b)
{
  uint32 sum;

  sum  = (uint32) (*a++) * (*b++);
  sum += (uint32) (*a) * (*b);
  
  return sum;
}

uint32 dotproductU48_U16byU162 (const uint16 *a, const uint16 *b, uint16 *msw)
{
  uint64 sum;

  sum  = (uint64) ((uint32) (*a++) * *b++);
  sum += (uint32) (*a) * (*b);
  
  *msw = (uint16) (sum >> 32);

  return (uint32) sum;
}


uint8 vecsumU8_U8(const uint8 *a, uint16 n)
{
  unsigned int i;
  uint8 sum = 0;

  for (i = 0; i < n; i++) {
    sum += *a++;
  }

  return sum;
}

int8 vecsumI8_I8(const int8 *a, uint16 n)
{
  unsigned int i;
  int8 sum = 0;

  for (i = 0; i < n; i++) {
    sum += *a++;
  }

  return sum;
}

uint16 vecsumU16_U8(const uint8 *a, uint16 n)
{
  unsigned int i;
  uint16 sum = 0;

  for (i = 0; i < n; i++) {
    sum += *a++;
  }

  return sum;
}

int16 vecsumI16_I8(const int8 *a, uint16 n)
{
  unsigned int i;
  int16 sum = 0;

  for (i = 0; i < n; i++) {
    sum += *a++;
  }

  return sum;
}

uint16 vecsumU16_U16(const uint16 *a, uint16 n)
{
  unsigned int i;
  uint16 sum = 0;

  for (i = 0; i < n; i++) {
    sum += *a++;
  }

  return sum;
}

int16 vecsumI16_I16(const int16 *a, uint16 n)
{
  unsigned int i;
  uint16 sum = 0;

  for (i = 0; i < n; i++) {
    sum += *a++;
  }

  return sum;
}

int32 vecsumI32_I16(const int16 *a, uint16 n)
{
  unsigned int i;
  int32 sum = 0;

  for (i = 0; i < n; i++) {
    sum += *a++;
  }

  return sum;
}


int32 vecsumI32_I32(const int32 *a, uint16 n)
{
  unsigned int i;
  int32 sum = 0;

  for (i = 0; i < n; i++) {
    sum += *a++;
  }

  return sum;
}


/*u*/int32 vecsumI48_I32(const int32 *a, uint16 n, int16 *msw)
{
  unsigned int i;
  int64 sum = 0;

  for (i = 0; i < n; i++) {
    sum += (uint64) (*a++);
  }
  *msw = (int16) (sum >> 32);

  return (/*u*/int32) sum;
}

uint16 vecmaxU16_U16 (uint16 *v, uint16 n)
{
  uint16 i;
  uint16 m;

  m = 0;
  for (i = 0; i < n; i++) {
    if (m < *v++) {
      m = *--v;
      v++;
    }
  }
  return m;
}

#ifdef  HAS_MLX16_COPROCESSOR
/* difference in stack usage */
uint32 vecmaxU32_U32 (uint32 *v, uint16 n)
{
  uint16 i;
  uint32 m;

  m = 0;
  for (i = 0; i < n; i++) {
    if (m < *v++) {
      m = *--v;
      v++;
    }
  }
  return m;
}
#else
uint32 vecmaxU32_U32 (uint32 *v, uint16 n)
{
  uint16 i;
  uint32 m;

  m = 0;
  for (i = 0; i < n; i++) {
    if (m < *v) {
      m = *v;
    }
    v++;
  }
  return m;
}
#endif /* HAS_MLX16_COPROCESSOR */

uint16 normmaxvectorU16_I16 (int16 *v, uint16 n)
{
  uint16 i;
  uint16 m;

  m = 0;
  for (i = 0; i < n; i++) {
    uint16 val = abs (*v++);
    if (m < val) {
      m = val;
    }
  }
  return m;
}

uint32 normmaxvectorU32_I32 (int32 *v, uint16 n)
{
  uint16 i;
  uint32 m;

  m = 0;
  for (i = 0; i < n; i++) {
    uint32 val = abs (*v++);
    if (m < val) {
      m = val;
    }
  }
  return m;
}

uint32 norm1vectorU32_I16 (const int16 *a, uint16 n)
{
  unsigned int i;
  uint32 sum = 0;

  for (i = 0; i < n; i++) {
    sum += (uint32) (abs(*a)); /* LDRA does not likes: (uint32) abs(*a) */
    a++;
  }
  return sum;
}


uint32 norm1vectorU32_I32(const int32 *a, uint16 n)
{
  unsigned int i;
  uint32 sum = 0;

  for (i = 0; i < n; i++) {
    sum += (uint32) (labs(*a)); /* LDRA does not like (uint32) labs(*a) */
    a++;
  }

  return sum;
}

uint32 norm1vectorU48_I32(const int32 *a, uint16 n, uint16 *msw)
{
  unsigned int i;
  uint64 sum = 0;

  for (i = 0; i < n; i++) {
    sum += (uint64) (labs(*a)); /* LDRA does not like (uint64) labs(*a) */
    a++;
  }
  *msw = (uint16) (sum >> 32);

  return (uint32) sum;
}


uint32 norm2U32_I16byI16 (int16 a, int16 b)
{
  uint32 a2 = (((int32) a) * a);
  uint32 b2 = (((int32) b) * b);

  return a2 + b2;
}


uint32 norm2U48_I16byI16 (int16 a, int16 b, uint16 *msw)
{
  uint32 a2 = (((int32) a) * a);
  uint32 b2 = (((int32) b) * b);
  uint64 sum;

  sum = (uint64) a2 + (uint64) b2;

  *msw = (uint16) (sum >> 32);

  return (uint32) sum;
}


uint32 norm2vectorU32_I16byI16 (const int16 *a, uint16 n)
{
  unsigned int i;
  uint32 sum = 0;

  for (i = 0; i < n; i++) {
    sum += (uint32) (((int32) (*a)) * (*a));
    a++;
  }
  return sum;
}


uint32 norm2vectorU48_I16byI16 (const int16 *a, uint16 n, uint16 *msw)
{
  unsigned int i;
  uint64 sum = 0;

  for (i = 0; i < n; i++) {
    sum += (uint64) (((int32) (*a)) * (*a));
    a++;
  }
  *msw = (uint16) (sum >> 32);
  return (uint32) sum;
}


int32 dotproductI32_I16byI16 (const int16 *a, const int16 *b, uint16 n)
{
  unsigned int i;
  int32 sum = 0;

  for (i = 0; i < n; i++) {
    sum += (((int32) (*a++)) * (*b++));
  }

  return sum;
}


/*u*/int32 dotproductI48_I16byI16 (const int16 *a, const int16 *b, uint16 n, int16 *msw)
{
  unsigned int i;
  int64 sum = 0;

  for (i = 0; i < n; i++) {
    sum += (int64) (((int32) (*a++)) * (*b++));
  }
  *msw = (int16) (sum >> 32);

  return (/*u*/int32) sum;
}


