/*
 * Copyright (C) 2007-2013 Melexis N.V.
 *
 * Math Library
 *
 * Revision $Name: PLTF_MULAN2_MMC16_RELEASE_3_3_0 $
 *
 * File $RCSfile: mathlib.h,v $
 *
 */

#ifndef PRIVATE_MATHLIB_H_
#define PRIVATE_MATHLIB_H_

#include <mlx16_cfg.h>

/* Validate MLX16-GCC version */
#if ((__MLX16_GCC_MAJOR__ == 1) && (__MLX16_GCC_MINOR__ >= 8)) || (__MLX16_GCC_MAJOR__ > 1)
    /* ok */
#else
#warning "Math library requires MLX16-GCC release 1.8 or later"
#endif


#include "typelib.h"

/*
 *	Multiplication
 */
static __inline__ int16 muldivI16_I16byI16byI16(int16 a, int16 b, int16 c) __attribute__ ((always_inline));
static __inline__ int16 muldivI16_I16byI16byI16(int16 a, int16 b, int16 c)
{
    int16 result;  /*lint -e530 */
    int16 result2; /*lint -e529 */												/* clobbering of the register */

    __asm__ __volatile__ (
         "muls YA, A, Y\n\t"
         "divs	YA, X\n\t"
         "divs	YA, X\n\t"
         "dadjs	YA, X"
         : "=a" (result), "=y" (result2)
         : "%a" (a), "y" (b), "x" (c)
         );

    return result;
} /* End of muldivI16_I16byI16byI16() */

static __inline__ uint16 muldivU16_U16byU16byU16(uint16 a, uint16 b, uint16 c) __attribute__ ((always_inline));
static __inline__ uint16 muldivU16_U16byU16byU16(uint16 a, uint16 b, uint16 c)
{
    int16 result;  /*lint -e530 */
    int16 result2; /*lint -e529 */												/* clobbering of the register */

    __asm__ __volatile__ (
         "mulu YA, A, Y\n\t"
         "divu	YA, X\n\t"
         "divu	YA, X\n\t"
         : "=a" (result), "=y" (result2)
         : "%a" (a), "y" (b), "x" (c)
         );

    return result;
} /* End of muldivU16_U16byU16byU16() */

static __inline__ int16 mulI16_I16byI16Shft4(int16 a, int16 b) __attribute__ ((always_inline));
static __inline__ int16 mulI16_I16byI16Shft4(int16 a, int16 b)
{
    int16 result;  /*lint -e530 */
    int16 result2; /*lint -e529 */												/* clobbering of the register */

    __asm__ __volatile__ (
         "muls YA, A, %w3\n\t"
         "asr YA, #4"
         : "=y" (result), "=a" (result2)
         : "%a" (a), "r" (b)
         );

    return result;
} /* End of mulI16_I16byI16Shft4() */

/* ----------------------------------------------------------------------------
 * Signed integer division ( 16 = 32 / 16 )
 *
 * Input :
 *      a           signed 32-bit dividend
 *      b           signed 16-bit divisor
 *
 * Output :
 *      result      unsigned 16-bit quotient
 */
static __inline__ int16 divI16_I32byI16(int32 a, int16 b) __attribute__ ((always_inline));
static __inline__ int16 divI16_I32byI16(int32 a, int16 b)
{
    int16 result;
    int16 result2;    /* clobbering of the register */

    __asm__ __volatile__ (
         "divs YA, X\n\t"
         "divs YA, X\n\t"
         "dadjs YA,X"
         : "=a" (result), "=y" (result2)
         : "b" (a), "x" (b)
         );

    return result;
} /* End of divI16_I32byI16() */

static __inline__ int16 mulI16_I16byI16RndDiv64(int16 a, int16 b) __attribute__ ((always_inline));
static __inline__ int16 mulI16_I16byI16RndDiv64(int16 a, int16 b)
{
    int16 result;
    int16 result2;    /* clobbering of the register */

    __asm__ __volatile__ (
         "muls YA, A, %w3\n\t"
         "asr YA, #6\n\t"
         "adc A, #0"
         : "=a" (result), "=y" (result2)
         : "%a" (a), "r" (b)
         );

    return result;
} /* End of mulI16_I16byI16RndDiv64() */

#endif /* PRIVATE_MATHLIB_H_ */
