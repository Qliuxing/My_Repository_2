/*
 * Copyright (C) 2008-2012 Melexis N.V.
 *
 * DSP Library
 *
 */

#ifndef MATHLIB_DSP_H_
#define MATHLIB_DSP_H_

extern uint8  vecsumU8_U8  (const uint8  *a, uint16 n);
extern  int8  vecsumI8_I8  (const  int8  *a, uint16 n);
extern uint16 vecsumU16_U8 (const uint8  *a, uint16 n);
extern  int16 vecsumI16_I8 (const  int8  *a, uint16 n);
extern uint16 vecsumU16_U16(const uint16 *a, uint16 n);
extern  int16 vecsumI16_I16(const  int16 *a, uint16 n);

extern int32 vecsumI32_I16(const int16 *a, uint16 n);
extern int32 vecsumI32_I32(const int32 *a, uint16 n);
extern int32 vecsumI48_I32(const int32 *a, uint16 n, int16 *msw);

extern uint16 vecmaxU16_U16(const uint16 *a, uint16 n);
extern uint32 vecmaxU32_U32(const uint32 *a, uint16 n);

extern uint32 norm2U32_I16byI16(int16 a, int16 b);
extern uint32 norm2U48_I16byI16(int16 a, int16 b, uint16 *msw);
extern uint32 norm2vectorU32_I16byI16(const int16 *a, uint16 n);
extern uint32 norm2vectorU48_I16byI16(const int16 *a, uint16 n, uint16 *msw);
extern int32 dotproductI32_I16byI16(const int16 *a, const int16 *b, uint16 n);
extern /*u*/int32 dotproductI48_I16byI16(const int16 *a, const int16 *b, uint16 n, int16 *msw);

extern uint32 norm1vectorU32_I16 (const int16 *a, uint16 n);
extern uint32 norm1vectorU32_I32(const int32 *a, uint16 n);
extern uint32 norm1vectorU48_I32(const int32 *a, uint16 n, uint16 *msw);

extern uint16 normmaxvectorU16_I16 (int16 *v, uint16 n);
extern uint32 normmaxvectorU32_I32 (int32 *v, uint16 n);


#endif /* MATHLIB_DSP_H_ */
