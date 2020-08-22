/*
 * Copyright (C) 2005-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef TYPELIB_H_
#define TYPELIB_H_

/* SW Platform types */
typedef unsigned char       uint8;
typedef signed char         int8;
typedef unsigned int        uint16;
typedef signed int          int16;
typedef unsigned long       uint32;
typedef signed long         int32;

#if __STDC_VERSION__ == 199901L
/* Compiled with -std=c99 or -std=gnu99 */
typedef unsigned long long  uint64;
typedef signed long long    int64;
#endif

#ifndef __MLX16_STDINT_H__

#if __STDC_VERSION__ == 199901L
/*
 * C99 types
 * Normally should be defined in compiler's stdint.h
 */
/* Exact-width integer types */
typedef signed char             int8_t;
typedef unsigned char           uint8_t;
typedef signed int              int16_t;
typedef unsigned int            uint16_t;
typedef signed long int         int32_t;
typedef unsigned long int       uint32_t;
typedef signed long long int    int64_t;
typedef unsigned long long int  uint64_t;

/* Minimum-width integer types */
typedef int8_t      int_least8_t;
typedef uint8_t     uint_least8_t;
typedef int16_t     int_least16_t;
typedef uint16_t    uint_least16_t;
typedef int32_t     int_least32_t;
typedef uint32_t    uint_least32_t;
typedef int64_t     int_least64_t;
typedef uint64_t    uint_least64_t;

/* Fastest minimum-width integer types */
typedef int16_t     int_fast8_t;
typedef uint16_t    uint_fast8_t;
typedef int16_t     int_fast16_t;
typedef uint16_t    uint_fast16_t;
typedef int32_t     int_fast32_t;
typedef uint32_t    uint_fast32_t;
typedef int64_t     int_fast64_t;
typedef uint64_t    uint_fast64_t;

/* Greatest-width integer types */
typedef int64_t     intmax_t;
typedef uint64_t    uintmax_t;
#endif

#endif /* __MLX16_STDINT_H__ */

#endif /* TYPELIB_H_ */
