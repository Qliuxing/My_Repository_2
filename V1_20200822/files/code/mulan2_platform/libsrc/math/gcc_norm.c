/*
 * Copyright (C) 2007-2008 Melexis N.V.
 *
 * Math Library
 *
 */

#include "typelib.h"
#include "mathlib.h"


uint32 norm2U32_U16byU16 (uint16 a, uint16 b)
{
  uint32 a2 = (((uint32) a) * a);
  uint32 b2 = (((uint32) b) * b);

  return a2 + b2;
}

uint32 norm2U48_U16byU16 (uint16 a, uint16 b, uint16 *msw)
{
  uint32 a2 = (((uint32) a) * a);
  uint32 b2 = (((uint32) b) * b);
  uint64 sum;

  sum = (uint64) a2 + (uint64) b2;

  *msw = (uint16) (sum >> 32);

  return (uint32) sum;
}

uint32 norm2vectorU32_U16byU16 (const uint16 *a, uint16 n)
{
  unsigned int i;
  uint32 sum = 0;

  for (i = 0; i < n; i++) {
    sum += (((uint32) *a) * *a);
    a++;
  }
  return sum;
}

uint32 norm2vectorU48_U16byU16 (const uint16 *a, uint16 n, uint16 *msw)
{
  unsigned int i;
  uint64 sum = 0;

  for (i = 0; i < n; i++) {
    sum += (uint64) (((uint32) *a) * *a);
    a++;
  }
  *msw = (uint16) (sum >> 32);
  return (uint32) sum;
}

uint32 vecsumU32_U16(const uint16 *a, uint16 n)
{
  unsigned int i;
  uint32 sum = 0;

  for (i = 0; i < n; i++) {
    sum += *a++;
  }

  return sum;
}

uint32 vecsumU32_U32(const uint32 *a, uint16 n)
{
  unsigned int i;
  uint32 sum = 0;

  for (i = 0; i < n; i++) {
    sum += *a++;
  }

  return sum;
}

uint32 vecsumU48_U32(const uint32 *a, uint16 n, uint16 *msw)
{
  unsigned int i;
  uint64 sum = 0;

  for (i = 0; i < n; i++) {
    sum += (uint64) (*a++);
  }
  *msw = (uint16) (sum >> 32);

  return (uint32) sum;
}

uint32 dotproductU32_U16byU16 (const uint16 *a, const uint16 *b, uint16 n)
{
  unsigned int i;
  uint32 sum = 0;

  for (i = 0; i < n; i++) {
    sum += (((uint32) *a++) * *b++);
  }

  return sum;
}

uint32 dotproductU48_U16byU16 (const uint16 *a, const uint16 *b, uint16 n, uint16 *msw)
{
  unsigned int i;
  uint64 sum = 0;

  for (i = 0; i < n; i++) {
    sum += (uint64) (((uint32) *a++) * *b++);
  }
  *msw = (uint16) (sum >> 32);

  return (uint32) sum;
}

/* EOF */
