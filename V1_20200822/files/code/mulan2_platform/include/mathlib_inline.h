/*
 * Copyright (C) 2014 Melexis N.V.
 *
 * Math Library
 *
 */

/*
 *  Declaration of functions for inline_asm realization
 */

static __inline__ int32 mulI32_I16byI16(int16 a, int16 b) __attribute__ ((always_inline));
static __inline__ uint32 mulU32_U16byU16(uint16 a, uint16 b) __attribute__ ((always_inline));
static __inline__ int16 mulI16_I16byI16(int16 a, int16 b) __attribute__ ((always_inline));
static __inline__ uint16 mulU16_U16byU16(uint16 a, uint16 b) __attribute__ ((always_inline));
static __inline__ int16 mulQ15_Q15byQ15(int16 a, int16 b) __attribute__ ((always_inline));
static __inline__ int32 mulI24_I16byI8(int16 a, int8 b) __attribute__ ((always_inline));
static __inline__ uint32 mulU24_U16byU8(uint16 a, uint8 b) __attribute__ ((always_inline));
static __inline__ int32 mulI24_I16byU8(int16 a, uint8 b) __attribute__ ((always_inline));
static __inline__ int16 mulI16hi_I16byI8(int16 a, int8 b) __attribute__ ((always_inline));
static __inline__ uint16 mulU16hi_U16byU8(uint16 a, uint8 b) __attribute__ ((always_inline));
static __inline__ int16 mulI16hi_I16byU8(int16 a, uint8 b) __attribute__ ((always_inline));
static __inline__ int16 mulI16_I8byI8(int8 a, int8 b) __attribute__ ((always_inline));
static __inline__ int16 mulI16_I8byU8(int8 a, uint8 b) __attribute__ ((always_inline));
static __inline__ uint16 mulU16_U8byU8(uint8 a, uint8 b) __attribute__ ((always_inline));
static __inline__ uint16 divU16_U32byU16(uint32 a, uint16 b) __attribute__ ((always_inline));

/* --------------------
 *    Multiplication
 * --------------------*/

/* ----------------------------------------------------------------------------
 * Signed integer multiplication ( 32 = 16 * 16 )
 *
 * Input :
 *      a           signed 16-bit multiplicand
 *      b           signed 16-bit multiplier
 *
 * Output :
 *      result      signed 32-bit multiplication result
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   int32 result = ((int32) a) * b
 */
static __inline__ int32 mulI32_I16byI16(int16 a, int16 b)
{
    int32 result;

    __asm__ __volatile__ (
         "muls %Q0, A, %w2"
         : "=qb" (result)
         : "%a" (a), "r" (b)
         );

    return result;
}

/* ----------------------------------------------------------------------------
 * Unsigned integer multiplication ( 32 = 16 * 16 )
 *
 * Input :
 *      a           unsigned 16-bit multiplicand
 *      b           unsigned 16-bit multiplier
 *
 * Output :
 *      result      unsigned 32-bit multiplication result
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   uint32 result = ((uint32) a) * b
 */
static __inline__ uint32 mulU32_U16byU16(uint16 a, uint16 b)
{
    uint32 result;

    __asm__ __volatile__ (
         "mulu %Q0, A, %w2"
         : "=qb" (result)
         : "%a" (a), "r" (b)
         );

    return result;
}

/* ----------------------------------------------------------------------------
 * Signed integer multiplication ( 16 = high (16 * 16) )
 *
 * Input :
 *      a           signed 16-bit multiplicand
 *      b           signed 16-bit multiplier
 *
 * Output :
 *      result      signed 16-bit multiplication result
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   int16 result = (int16)( ((int32) a * b) >> 16 );
 */
static __inline__ int16 mulI16_I16byI16(int16 a, int16 b)
{
    int16 result;
    int16 result2;    /* clobbering of the register */

    __asm__ __volatile__ (
         "muls YA, A, %w3"
         : "=y" (result), "=a" (result2)
         : "%a" (a), "r" (b)
         );

    return result;
}

/* ----------------------------------------------------------------------------
 * Unsigned integer multiplication ( 16 = high (16 * 16) )
 *
 * Input :
 *      a           unsigned 16-bit multiplicand
 *      b           unsigned 16-bit multiplier
 *
 * Output :
 *      result      unsigned 16-bit multiplication result
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   uint16 result = (uint16)( ((uint32) a * b) >> 16 );
 */
static __inline__ uint16 mulU16_U16byU16(uint16 a, uint16 b)
{
    uint16 result;
    uint16 result2;    /* clobbering of the register */

    __asm__ __volatile__ (
         "mulu YA, A, %w3"
         : "=y" (result), "=a" (result2)
         : "%a" (a), "r" (b)
         );

    return result;
}

/* ----------------------------------------------------------------------------
 * Signed integer fraction multiplication ( 15 = high (15 * 15) )
 *
 * Input :
 *      a           signed Q15 multiplicand
 *      b           signed Q15 multiplier
 *
 * Output :
 *      result      signed Q15 multiplication result
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   int16 result = (int16)( ((int32) a * b) >> 15 );
 */
static __inline__ int16 mulQ15_Q15byQ15(int16 a, int16 b)
{
    int16 result;
    int16 result2;    /* clobbering of the register */

    __asm__ __volatile__ (
         "muls YA, A, %w3\n\t"
         "asr YA, #15"
         : "=a" (result), "=y" (result2)
         : "%a" (a), "r" (b)
         );

    return result;
}

/* ----------------------------------------------------------------------------
 * Signed integer multiplication ( 24 = 16 * 8 )
 *
 * Input :
 *      a           signed 16-bit multiplicand
 *      b           signed 8-bit multiplier
 *
 * Output :
 *      result      signed 24-bit result (most significant byte 0)
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   int32 result = ((int32) a * (int16) b);
 */
static __inline__ int32 mulI24_I16byI8(int16 a, int8 b)
{
    int32 result;
    int16 b_16 = (int16) b; /* cast to Word */

    __asm__ __volatile__ (
         "muls %Q0, A, %w2"
         : "=qb" (result)
         : "%a" (a), "r" (b_16)
         );

    return result;
}

/* ----------------------------------------------------------------------------
 * Unsigned integer multiplication ( 24 = 16 * 8 )
 *
 * Input :
 *      a           unsigned 16-bit multiplicand
 *      b           unsigned 8-bit multiplier
 *
 * Output :
 *      result      unsigned 24-bit result (most significant byte 0)
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   uint32 result = ((uint32) a * (uint16) b);
 */
static __inline__ uint32 mulU24_U16byU8(uint16 a, uint8 b)
{
    uint32 result;
    uint16 b_16 = (uint16) b; /* cast to Word */

    __asm__ __volatile__ (
         "mulu %Q0, A, %w2"
         : "=qb" (result)
         : "%a" (a), "r" (b_16)
         );

    return result;
}

/* ----------------------------------------------------------------------------
 * Signed integer multiplication ( 24 = 16 * 8 )
 *
 * Input :
 *      a           signed 16-bit multiplicand
 *      b           unsigned 8-bit multiplier
 *
 * Output :
 *      result      signed 24-bit result (most significant byte 0)
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   int32 result = ((int32) a * (int16) b);
 */
static __inline__ int32 mulI24_I16byU8(int16 a, uint8 b)
{
    int32 result;
    int16 b_16 = (int16) b; /* cast to Word */

    __asm__ __volatile__ (
         "muls %Q0, A, %w2"
         : "=qb" (result)
         : "%a" (a), "r" (b_16)
         );

    return result;
}

/* ----------------------------------------------------------------------------
 * Signed integer multiplication ( 16 = high (16 * 8) )
 *
 * Input :
 *      a           signed 16-bit multiplicand
 *      b           signed 8-bit multiplier
 *
 * Output :
 *      result      signed 16-bit result (high part of the 24-bit)
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   int32 result = (int16)( ((int32) a * (int16) b) >> 8);
 */
static __inline__ int16 mulI16hi_I16byI8(int16 a, int8 b)
{
    int16 result;
    int16 result2;            /* clobbering of the register */
    int16 b_16 = (int16) b; /* cast to Word */

    __asm__ __volatile__ (
         "muls YA, A, %w3\n\t"
         "asr YA, #8"
         : "=a" (result), "=y" (result2)
         : "%a" (a), "r" (b_16)
         );

    return result;
}

/* ----------------------------------------------------------------------------
 * Unsigned integer multiplication ( 16 = high (16 * 8) )
 *
 * Input :
 *      a           unsigned 16-bit multiplicand
 *      b           unsigned 8-bit multiplier
 *
 * Output :
 *      result      unsigned 16-bit result (high part of the 24-bit)
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   uint32 result = (uint16)( ((uint32) a * (uint16) b) >> 8);
 */
static __inline__ uint16 mulU16hi_U16byU8(uint16 a, uint8 b)
{
    uint16 result;
    uint16 result2;                /* clobbering of the register */
    uint16 b_16 = (uint16) b;     /* cast to Word */

    __asm__ __volatile__ (
         "mulu YA, A, %w3\n\t"
         "lsr YA, #8"
         : "=a" (result), "=y" (result2)
         : "%a" (a), "r" (b_16)
         );

    return result;
}

/* ----------------------------------------------------------------------------
 * Signed integer multiplication ( 16 = high (16 * 8) )
 *
 * Input :
 *      a           signed 16-bit multiplicand
 *      b           unsigned 8-bit multiplier
 *
 * Output :
 *      result      signed 16-bit result (high part of the 24-bit)
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   int32 result = (int16)( ((int32) a * (int16) b) >> 8);
 */
static __inline__ int16 mulI16hi_I16byU8(int16 a, uint8 b)
{
    int16 result;
    int16 result2;            /* clobbering of the register */
    int16 b_16 = (int16) b; /* cast to Word */

    __asm__ __volatile__ (
         "muls YA, A, %w3\n\t"
         "asr YA, #8"
         : "=a" (result), "=y" (result2)
         : "%a" (a), "r" (b_16)
         );

    return result;
}

/* ----------------------------------------------------------------------------
 * Signed integer multiplication ( 16 = 8 * 8 )
 *
 * Input :
 *      a           signed 8-bit multiplicand
 *      b           signed 8-bit multiplier
 *
 * Output :
 *      result      signed 16-bit multiplication result
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   int16 result = ((int16) a) * b;
 */
static __inline__ int16 mulI16_I8byI8(int8 a, int8 b)
{
    int16 result;

    __asm__ __volatile__ (
         "muls %w0, AL, %b2"
         : "=ay" (result)
         : "%a" (a), "r" (b)
         );

    return result;
}

/* ----------------------------------------------------------------------------
 * Signed integer multiplication ( 16 = 8 * 8 )
 *
 * Input :
 *      a           signed 8-bit multiplicand
 *      b           unsigned 8-bit multiplier
 *
 * Output :
 *      result      signed 16-bit multiplication result
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   int16 result = ((int16) a) * b;
 */
static __inline__ int16 mulI16_I8byU8(int8 a, uint8 b)
{
    int16 result;
    int16 a_16 = (int16) a; /* convert arguments to int16 to mul. mixed types */
    int16 b_16 = (int16) b;

    __asm__ __volatile__ (
         "muls %w0, A, %w2"
         : "=ay" (result)
         : "%a" (a_16), "r" (b_16)
         );

    return result;
}


/* ----------------------------------------------------------------------------
 * Unsigned integer multiplication ( 16 = 8 * 8 )
 *
 * Input :
 *      a           unsigned 8-bit multiplicand
 *      b           unsigned 8-bit multiplier
 *
 * Output :
 *      result      unsigned 16-bit multiplication result
 *
 * \note
 * 1.  Same functionality could be realized more optimally
 *     in some cases by using C operators
 *
 * Example:
 *   uint16    result = ((uint16) a) * b;
 */
static __inline__ uint16 mulU16_U8byU8(uint8 a, uint8 b)
{
    uint16 result;

    __asm__ __volatile__ (
         "mulu %w0, AL, %b2"
         : "=ay" (result)
         : "%a" (a), "r" (b)
         );

    return result;
}

/* --------------------
 *    Division
 * --------------------*/

/* ----------------------------------------------------------------------------
 * Unsigned integer division ( 16 = 32 / 16 )
 *
 * Input :
 *      a           unsigned 32-bit dividend
 *      b           unsigned 16-bit divisor
 *
 * Output :
 *      result      unsigned 16-bit quotient
 */
static __inline__ uint16 divU16_U32byU16(uint32 a, uint16 b)
{
    uint16 result;
    uint16 result2;    /* clobbering of the register */

    __asm__ __volatile__ (
         "divu YA, X\n\t"
         "divu YA, X"
         : "=a" (result), "=y" (result2)
         : "b" (a), "x" (b)
         );

    return result;
}
