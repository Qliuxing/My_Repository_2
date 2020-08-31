/*
 * Copyright (C) 2014 Melexis N.V.
 *
 * Math Library
 *
 */

/*
 * Declaration of functions for asm realization
 */

/* --------------------
 *    Multiplication
 * --------------------*/
uint32 mulU32_U16byU16(uint16 a, uint16 b);
int32  mulI32_I16byI16(int16 a, int16 b);
int16  mulI16_I16byI16(int16 a, int16 b);
uint16 mulU16_U16byU16(uint16 a, uint16 b);
int16  mulQ15_Q15byQ15(int16 a, int16 b);
int32  mulI24_I16byI8(int16 a, int8 b);
uint32 mulU24_U16byU8(uint16 a, uint8 b);
int32  mulI24_I16byU8(int16 a, uint8 b);
int16  mulI16hi_I16byI8(int16 a, int8 b);
uint16 mulU16hi_U16byU8(uint16 a, uint8 b);
int16  mulI16hi_I16byU8(int16 a, uint8 b);
int16  mulI16_I8byI8(int8 a, int8 b);
int16  mulI16_I8byU8(int8 a, uint8 b);
uint16 mulU16_U8byU8(uint8 a, uint8 b);

/* --------------------
 *    Division
 * --------------------*/
uint16 divU16_U32byU16(uint32 n, uint16 d);
