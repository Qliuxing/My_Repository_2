/*
 * Copyright (C) 2007-2014 Melexis N.V.
 *
 * Math Library
 *
 */

#ifndef MATHLIB_H_
#define MATHLIB_H_

#include <mlx16_cfg.h>

/* SW component version */
#define MLX_MATHLIB_SW_MAJOR_VERSION    2
#define MLX_MATHLIB_SW_MINOR_VERSION    4
#define MLX_MATHLIB_SW_PATCH_VERSION    0

/* Validate MLX16-GCC version */
#if ((__MLX16_GCC_MAJOR__ == 1) && (__MLX16_GCC_MINOR__ >= 8)) || (__MLX16_GCC_MAJOR__ > 1)
    /* ok */
#else
#warning "Math library requires MLX16-GCC release 1.8 or later"
#endif


#if !defined (__ASSEMBLER__)

#include "typelib.h"

/* Check MathLib INLINE_ASM availability;
   Choosing INLINE_ASM or ASM functions declaration */
#if defined (HAS_MLX16_COPROCESSOR) && !defined (__MATHLIB_NON_INLINE__)
    #include "mathlib_inline.h"
#else
    #include "mathlib_non_inline.h"
#endif

/*
 * Multiplication
 */

int32  mulI32_I16byU16(int16 a, uint16 b);

int16  mulI16_I16byU16(int16 a, uint16 b);

int32  mulI32_I32byI16(int32 a, int16 b);
int32  mulI32_I32byU16(int32 a, uint16 b);
uint32 mulU32_U32byU16(uint32 a, uint16 b);

int32  mulI32hi_I32byI16(int32 a, int16 b);
int32  mulI32hi_I32byU16(int32 a, uint16 b);
uint32 mulU32hi_U32byU16(uint32 a, uint16 b);

/*
 * Division
 */

uint32 divU32_U32byU16(uint32 n, uint16 d);
int32  divI32_I32byI16(int32 n, int16 d);
int32  divI32_I32byU16(int32 n, uint16 d);

int16  divI16_I32byI16(int32 n, int16 d);
int16  divI16_I32byU16(int32 n, uint16 d);

uint16 divU16_U16byU16(uint16 n, uint16 d);
int16  divI16_I16byI16(int16 n, int16 d);
int16  divI16_I16byU16(int16 n, uint16 d);

uint8  divU8_U8byU8(uint8 n, uint8 d);
int8   divI8_I8byI8(int8 n, int8 d);
int8   divI8_I8byU8(int8 n, uint8 d);

uint8  divU8hi_U8byU8(uint8 n, uint8 d);

/*
 * Power
 */

uint16 isqrt16 (uint16 a);
uint16 isqrt32 (uint32 a);

/*
 * log/exp
 */

uint16 ilog2_U16 (uint16 v);
uint16 ilog2_U32 (uint32 v);

uint16 iexp2_U16 (uint16 v);
uint32 iexp2_U32 (uint16 v);

/*
 * DSP
 */

uint32 vecsumU32_U16(const uint16 *a, uint16 n);
uint32 vecsumU32_U32(const uint32 *a, uint16 n);
uint32 vecsumU48_U32(const uint32 *a, uint16 n, uint16 *msw);
uint32 norm2U32_U16byU16(uint16 a, uint16 b);
uint32 norm2U48_U16byU16(uint16 a, uint16 b, uint16 *msw);
uint32 norm2vectorU32_U16byU16(const uint16 *a, uint16 n);
uint32 norm2vectorU48_U16byU16(const uint16 *a, uint16 n, uint16 *msw);
uint32 dotproductU32_U16byU16(const uint16 *a, const uint16 *b, uint16 n);
uint32 dotproductU48_U16byU16(const uint16 *a, const uint16 *b, uint16 n, uint16 *msw);

/*
 * Trigonometric
 */

int16 sinU16 (uint16 x);
int16 sinI16 (int16 x);

int16 cosU16 (uint16 x);
int16 cosI16 (int16 x);

int32 tanU16 (uint16 x);
int32 tanI16 (int16 x);

int16 atan2U16 (uint16 y, uint16 x);
int16 atan2I16 (int16 y, int16 x);

/*
 * Other
 */

uint32 rand32(uint32 seed);
void   init_lfsr16(uint16 seed);
void   init_lfsr32(uint32 seed);
uint16 lfsr16(void);
uint32 lfsr32(void);

uint16 parity4 (uint8  v);
uint16 parity8 (uint8  v);
uint16 parity16(uint16 v);
uint16 parity32(uint32 v);

uint16 crc16(uint8 c, uint16 crc);
uint8  crc8 (uint8 c, uint8  crc);
uint16 crc_ccitt(uint8 c, uint16 crc);

uint8   bitrev4 (uint8  x);
uint8   bitrev8 (uint8  x);
uint16  bitrev16(uint16 x);

uint8   interleave4 (uint8  x, uint8  y);
uint16  interleave8 (uint8  x, uint8  y);
uint32  interleave16(uint16 x, uint16 y);


/* conversion of q30 to q15 number representation */
/* may result in stack reservation, but not all used :-( */
static __inline__ int16 _q15(int32 a) __attribute__ ((always_inline));
static __inline__ int16 _q15(int32 a)
{
#if defined(__MLX16__) && !defined(HAS_MLX16_COPROCESSOR)

  /* v >> 15 */
  int16 result;

  __asm__ (
       "asl A\n\t"
       "rlc Y"
       : "=y" (result)
       : "b" (a) /* YA register */
       /* : "Y", "A"  */
       );

  return result;
#else
  return (int16) (a >> 15);
#endif
}

/* 16 bit integer average */
static __inline__ uint16 _uavg(uint16 a, uint16 b) __attribute__ ((always_inline));
static __inline__ uint16 _uavg(uint16 a, uint16 b)
{
#ifdef __MLX16__
  uint16 result;
  
  __asm__ ( 
       "add %0, %2\n\t"
       "rrc %0"
       : "=r" (result)
       : "0" (a), "g" (b) );

  return result;
#else
  uint32 sum;
  
  sum = (uint32) a + b;
  sum /= 2;

  return (uint16) sum;
#endif /* __MLX16__ */
}

static __inline__ int16 _iavg(int16 a, int16 b) __attribute__ ((always_inline));
static __inline__ int16 _iavg(int16 a, int16 b)
{
#ifdef __MLX16__
  int16 result;
  
  __asm__ ( 
       "add %0, %2\n\t"
       "jnv NoOverflow_%=\n\t"
       "rrc %0\n\t"
       "jmp Done_%=\n"
       "NoOverflow_%=:\n\t"
       "asr %0, #1\n"
       "Done_%=:"
       : "=r" (result)
       : "0" (a), "g" (b) );

  return result;
#else
  int32 sum;
  
  sum = (int32) a + b;
  sum /= 2;

  return (int16) sum;
#endif /* __MLX16__ */
}

/* supporting functions used by log/exp and sqrt */
static __inline__ int16  _fsb(uint16 v)   __attribute__ ((always_inline));
static __inline__ uint16 _sfb(uint16 v)   __attribute__ ((always_inline));
static __inline__ int16  _fsb32(uint32 v) __attribute__ ((always_inline));
static __inline__ uint32 _sfb32(uint16 v) __attribute__ ((always_inline));

#if defined (HAS_MLX16_FSB_SFB_INSTRUCTIONS)

static __inline__ int16 _fsb(uint16 v) 
{ 
  /* fsb(0) = 0 or -1 */
  asm ("fsb %[res]" : [res] "=b" (v) : "0"(v)); 

  return (int16) v;
}

static __inline__ uint16 _sfb(uint16 v) 
{ 
  asm("sfb %[res]" : [res] "=b" (v) : "0"(v)); 

  return v;
}

#else

/* original MLX16, w/o fsb instruction */
static __inline__ int16 _fsb(uint16 v) 
{ 
  int16 result;
  /* fsb(0) = -1 */ 
  /* note: may be different from MLX16-8/x8 instruction: 0 or -1 */
  if (v == 0) {
    result = -1; /* 0xFFFF */
  } else {
    /* alt: first do a binary tree search */
    uint16 one = 0x8000;
    
    result = 15;
    /* total time, looping over all 16bit values: 249ms */
    while ((v & one) == 0) {
      one >>= 1;
      result--;
    }
  }

  return result;
}

static __inline__ uint16 _sfb(uint16 v) 
{ 
  uint16 one = 1;

  /* first a binary tree search */
  /* 2 level tree */
  /* total time, looping over all 16 values: 81us */
  one = 1;
  if (v > 8) {
    v -= 8;
    one = 1 << 8;
  }
  if (v > 4) {
    v -= 4;
    one <<= 4;
  }
  while (0 != v--) {
    one <<= 1;
  }

  return one;
}
#endif /* HAS_MLX16_FSB_SFB_INSTRUCTIONS */

static __inline__ int16 _fsb32(uint32 v)
{
  int16 result;

  if ((v >> 16) == 0) {
    result = _fsb((uint16) v);
  } else {
    result = 16 + _fsb(((uint16) (v >> 16)));
  }

  return result;
}

static __inline__ uint32 _sfb32(uint16 v) 
{ 
  uint32 result;

  if (v < 16) {
    result = (uint32) _sfb(v);
  } else {
    result = ((uint32) _sfb(v - 16U)) << 16;
  }

  return result;
}

#endif /* ! __ASSEMBLER__ */

#endif /* MATHLIB_H_ */
