/*
 * Copyright (C) 2007-2009 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>

#define	ERROR()		__asm__("jmp .")

uint32 mdiff;
uint32 pdiff;

void mul_test(void);

void mulI16_I16byI16_test(void);
void mulI16_I16byU16_test(void);
void mulU16_U16byU16_test(void);

void mulQ15_Q15byQ15_test(void);

void mulI32_I16byI16_test(void);
void mulI32_I16byU16_test(void);
void mulU32_U16byU16_test(void);

void mulI32_I32byI16_test(void);
void mulI32_I32byU16_test(void);
void mulU32_U32byU16_test(void);

void mulI32hi_I32byI16_test(void);
void mulI32hi_I32byU16_test(void);
void mulU32hi_U32byU16_test(void);

void mulI24_I16byI8_test(void);
void mulI24_I16byU8_test(void);
void mulU24_U16byU8_test(void);

void mulI16hi_I16byI8_test(void);
void mulI16hi_I16byU8_test(void);
void mulU16hi_U16byU8_test(void);

void mulI16_I8byI8_test(void);
void mulI16_I8byU8_test(void);
void mulU16_U8byU8_test(void);

void mul_test(void)
{
	mulI16_I16byI16_test();
	mulI16_I16byU16_test();
	mulU16_U16byU16_test();

	mulQ15_Q15byQ15_test();

	mulI32_I16byI16_test();
	mulI32_I16byU16_test();
	mulU32_U16byU16_test();

	mulI32_I32byI16_test();
	mulI32_I32byU16_test();
	mulU32_U32byU16_test();

	mulI32hi_I32byI16_test();
	mulI32hi_I32byU16_test();
	mulU32hi_U32byU16_test();

	mulI24_I16byI8_test();
	mulI24_I16byU8_test();
	mulU24_U16byU8_test();

	mulI16hi_I16byI8_test();
	mulI16hi_I16byU8_test();
	mulU16hi_U16byU8_test();

	mulI16_I8byI8_test();
	mulI16_I8byU8_test();
	mulU16_U8byU8_test();

}	/* mul_test() */

/*
 *  32=32x16
 */

void mulI32_I32byI16_test(void)
{
	int32 i;

	/* 1 - same as 16x16 */

	i = mulI32_I32byI16(0x7fff, 0x7fff);
	if((int32)0x3fff0001 != i)
	{
		ERROR();
	} 

	i = mulI32_I32byI16(0x7fff, 0x8000);
	if((int32)0xC0008000 != i)
	{
		ERROR();
	} 

	i = mulI32_I32byI16(0xffff8000, 0x7fff);
	if((int32)0xC0008000 != i)
	{
		ERROR();
	} 

	i = mulI32_I32byI16(0xffff8000, 0x8000);
	if((int32)0x40000000 != i)
	{
		ERROR();
	} 


	/* 2 */

	i = mulI32_I32byI16(32768, 16384);
	if((int32)0x20000000 != i)
	{
		ERROR();
	} 

	i = mulI32_I32byI16(32768, -16384);
	if((int32)0xE0000000 != i)
	{
		ERROR();
	} 

	i = mulI32_I32byI16(-32768, 16384);
	if((int32)0xE0000000 != i)
	{
		ERROR();
	} 

	i = mulI32_I32byI16(-32768, -16384);
	if((int32)0x20000000 != i)
	{
		ERROR();
	} 

	/* 3 */

	i = mulI32_I32byI16(0x3fffffff, 2); /* (1 << 30) - 1 */
	if((int32)0x7ffffffe != i)
	{
		ERROR();
	} 

	i = mulI32_I32byI16(0x40000000, -2); /* 1 << 30 */
	if((int32)0x80000000 != i)
	{
		ERROR();
	} 

	i = mulI32_I32byI16(0xC0000000, 2); /* -(1 << 30) */
	if((int32)0x80000000 != i)
	{
		ERROR();
	} 

	i = mulI32_I32byI16(0xC0000001, -2); /* -((1 << 30)-1) */
	if((int32)0x7ffffffe != i)
	{
		ERROR();
	} 

	i = mulI32_I32byI16(0x80000000, 1); /* 1<<31 */
	if((int32)0x80000000 != i)
	{
		ERROR();
	} 

	/* 4 - with overflow */

	i = mulI32_I32byI16(0x7fffffff, 0x7fff);
	if((int32)0x7fff8001 != i)
	{
		ERROR();
	} 

	i = mulI32_I32byI16(0x7fffffff, 0x8000);
	if((int32)0x00008000 != i)
	{
		ERROR();
	} 

	i = mulI32_I32byI16(0x80000000, 0x7fff);
	if((int32)0x80000000 != i)
	{
		ERROR();
	} 

	i = mulI32_I32byI16(0x80000000, 0x8000);
	if((int32)0x00000000 != i)
	{
		ERROR();
	} 
}	/* mulI32_I32byI16_test */

void mulI32_I32byU16_test(void)
{
	int32 i;

	/* 1 */

	i = mulI32_I32byU16(0x7fff, 0xffff);
	if((int32)0x7ffe8001 != i)
	{
		ERROR();
	} 

	i = mulI32_I32byU16(0xffff8000, 0xffff);
	if((int32)0x80008000 != i)
	{
		ERROR();
	} 

	/* 2 */

	i = mulI32_I32byU16(0x3fffffff, 2); /* (1 << 30) - 1 */
	if((int32)0x7ffffffe != i)
	{
		ERROR();
	} 

	i = mulI32_I32byU16(0xC0000000, 2); /* -(1 << 30) */
	if((int32)0x80000000 != i)
	{
		ERROR();
	} 

	/* 3 */

	i = mulI32_I32byU16(0x7fffffff, 0xffff);
	if((int32)0x7fff0001 != i)
	{
		ERROR();
	} 

	i = mulI32_I32byU16(0x80000000, 0xffff);
	if((int32)0x80000000 != i)
	{
		ERROR();
	} 

}	/* mulI32_I32byU16_test */

void mulU32_U32byU16_test(void)
{
	uint32 u;

	u = mulU32_U32byU16(0xffff, 0xffff);
	if(0xfffe0001 != u)
	{
		ERROR();
	} 

	u = mulU32_U32byU16(0xffffffff, 0xffff);
	if(0xffff0001 != u)
	{
		ERROR();
	} 
}

/*
 *  16=16x16
 */

void mulI16_I16byI16_test(void)
{
	int16 i;

	i = mulI16_I16byI16(0x7fff, 0x7fff);
	if((int16)0x3fff != i)
	{
		ERROR();
	} 

	i = mulI16_I16byI16(0x7fff, 0x8000);
	if((int16)0xC000 != i)
	{
		ERROR();
	} 

	i = mulI16_I16byI16(0x8000, 0x7fff);
	if((int16)0xC000 != i)
	{
		ERROR();
	} 

	i = mulI16_I16byI16(0x8000, 0x8000);
	if((int16)0x4000 != i)
	{
		ERROR();
	} 
}

void mulI16_I16byU16_test(void)
{
	int16 i;

	i = mulI16_I16byU16(0x7fff, 0xffff);
	if((int16)0x7ffe != i)
	{
		ERROR();
	} 

	i = mulI16_I16byU16(0x8000, 0xffff);
	if((int16)0x8000 != i)
	{
		ERROR();
	} 
}

void mulU16_U16byU16_test(void)
{
	uint16 u;

	u = mulU16_U16byU16(0xffff, 0xffff);
	if(0xfffe != u)
	{
		ERROR();
	} 
}

/*
 *  Q15=Q15xQ15
 */

void mulQ15_Q15byQ15_test(void)
{
	int16 i;

	i = mulQ15_Q15byQ15(0xffff, 0xffff);
	if(0x0000 != i)
	{
		ERROR();
	} 

	i = mulQ15_Q15byQ15(0x7fff, 0x7fff);
	if((int16)0x7ffe != i)
	{
		ERROR();
	} 

	i = mulQ15_Q15byQ15(0x7fff, 0x8000);
	if((int16)0x8001 != i)
	{
		ERROR();
	} 

	i = mulQ15_Q15byQ15(0x8000, 0x7fff);
	if((int16)0x8001 != i)
	{
		ERROR();
	} 

	i = mulQ15_Q15byQ15(0x8000, 0x8000);
	if((int16)0x8000 != i) /* ! */
	{
		ERROR();
	} 

	/* 0.1422212 * 0.6577759 = .0935436 */
	i = mulQ15_Q15byQ15(0x1234, 0x5432);
	if((int16)0x0bf9 != i) /* ! */
	{
		ERROR();
	} 
}

/*
 *  32=16x16
 */

void mulI32_I16byI16_test(void)
{
	int32 i;

	i = mulI32_I16byI16(0x7fff, 0x7fff);
	if((int32)0x3fff0001 != i)
	{
		ERROR();
	} 

	i = mulI32_I16byI16(0x7fff, 0x8000);
	if((int32)0xC0008000 != i)
	{
		ERROR();
	} 

	i = mulI32_I16byI16(0x8000, 0x7fff);
	if((int32)0xC0008000 != i)
	{
		ERROR();
	} 

	i = mulI32_I16byI16(0x8000, 0x8000);
	if((int32)0x40000000 != i)
	{
		ERROR();
	} 
}

void mulI32_I16byU16_test(void)
{
	int32 i;

	i = mulI32_I16byU16(0x7fff, 0xffff);
	if((int32)0x7ffe8001 != i)
	{
		ERROR();
	} 

	i = mulI32_I16byU16(0x8000, 0xffff);
	if((int32)0x80008000 != i)
	{
		ERROR();
	} 
}

void mulU32_U16byU16_test(void)
{
	uint32 u;

	u = mulU32_U16byU16(0xffff, 0xffff);
	if(0xfffe0001 != u)
	{
		ERROR();
	} 
}

/*
 *  32hi=32x16
 */

void mulI32hi_I32byI16_test(void)
{
	int32 i;

	/* 1 - same as 16x16 */

	i = mulI32hi_I32byI16(0x7fff, 0x7fff);
	if((int32)0x00003fff != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0x7fff, 0x8000);
	if((int32)0xFFFFC000 != i) /* ! ffffc001*/
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0xffff8000, 0x7fff);
	if((int32)0xFFFFC000 != i) /* ! ffffc001*/
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0xffff8000, 0x8000);
	if((int32)0x00004000 != i)
	{
		ERROR();
	} 


	i = mulI32hi_I32byI16(0xffff8000, 0x7ffe);
	if((int32)0xFFFFC001 != i) 
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0xffff8000, 0x7ffd);
	if((int32)0xFFFFC001 != i) /* ! ffffc002*/
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0xffff8000, 0x7ffc);
	if((int32)0xFFFFC002 != i)
	{
		ERROR();
	} 

	/* 2 */

	i = mulI32hi_I32byI16(32768, 16384);
	if((int32)0x00002000 != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(32768, -16384);
	if((int32)0xFFFFE000 != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(-32768, 16384);
	if((int32)0xFFFFE000 != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(-32768, -16384);
	if((int32)0x00002000 != i)
	{
		ERROR();
	} 

	/* 3 */

	i = mulI32hi_I32byI16(0x3fffffff, 2); /* (1 << 30) - 1 */
	if((int32)0x00007fff != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0x40000000, -2); /* 1 << 30 */
	if((int32)0xFFFF8000 != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0xC0000000, 2); /* -(1 << 30) */
	if((int32)0xFFFF8000 != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0xC0000001, -2); /* -((1 << 30)-1) */
	if((int32)0x00007fff != i) /* ! 00008000 */
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0x80000000, 1); /* 1<<31 */
	if((int32)0xFFFF8000 != i)
	{
		ERROR();
	} 

	/* 4 - with overflow */

	i = mulI32hi_I32byI16(0x7fffffff, 0x7fff);
	if((int32)0x3fff7fff != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0x7fffffff, 0x8000);
	if((int32)0xC0000000 != i) 
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0x80000000, 0x7fff);
	if((int32)0xC0008000 != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0x80000000, 0x8000);
	if((int32)0x40000000 != i) 
	{
		ERROR();
	} 

	/* 5 */
	i = mulI32hi_I32byI16(0x12345678, 0x1234);
	if((int32)0x014b60b6 != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0x12345678, 0x1233);
	if((int32)0x014b4e81 != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0x87654321, 0x1234);
	if((int32)0xf76c9f49 != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byI16(0x87654321, 0x1233);
	if((int32)0xf76d17e4 != i)
	{
		ERROR();
	} 
}	/* mulI32hi_I32byI16_test */


void mulI32hi_I32byU16_test(void)
{
	int32 i;

	/* 1 */

	i = mulI32hi_I32byU16(0x7fff, 0xffff);
	if((int32)0x00007ffe != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byU16(0x7fff, 0x8000);
	if((int32)0x00003fff != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byU16(0xffff8000, 0xffff);
	if((int32)0x0ffff8000 != i) /* ffff8001 */
	{
		ERROR();
	} 

	i = mulI32hi_I32byU16(0xffff8000, 0x7fff);
	if((int32)0x0ffffc000 != i) /* ffffc001 */
	{
		ERROR();
	} 

	/* 2 */

	i = mulI32hi_I32byU16(0x3fffffff, 2); /* (1 << 30) - 1 */
	if((int32)0x00007fff != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byU16(0xC0000000, 2); /* -(1 << 30) */
	if((int32)0xffff8000 != i)
	{
		ERROR();
	} 

	/* 3 */

	i = mulI32hi_I32byU16(0x7fffffff, 0xffff);
	if((int32)0x7fff7fff != i)
	{
		ERROR();
	} 

	i = mulI32hi_I32byU16(0x80000000, 0xffff);
	if((int32)0x80008000 != i)
	{
		ERROR();
	} 

}	/* mulI32hi_I32byU16_test */

void mulU32hi_U32byU16_test(void)
{
	uint32 u;
	uint32 a;
	uint16 b;

	u = mulU32hi_U32byU16(0xffff, 0xffff);
	if(0x0000fffe != u)
	{
		ERROR();
	} 

	u = mulU32hi_U32byU16(0xffffffff, 0xffff);
	if(0xfffeffff != u)
	{
		ERROR();
	} 

	u = mulU32hi_U32byU16(0xffffffff, 0xfffe);
	if(0xfffdffff != u)
	{
		ERROR();
	} 

	u = mulU32hi_U32byU16(0xffffffff, 0xfffd);
	if(0xfffcffff != u)
	{
		ERROR();
	} 

	/* 1 - same as 16x16 */

	a = 0x7fff;
	b = 0x7fff;
	u = mulU32hi_U32byU16(a, b);
	if(0x0003fff != u)
	{
	  	ERROR();
	}

	a = 0x7fff;
	b = 0x8000;
	u = mulU32hi_U32byU16(a, b);
	if(0x00003fff != u)
	{
		ERROR();
	} 

	a = 0xffff8000;
	b = 0x7fff;
	u = mulU32hi_U32byU16(a, b);
	if(0x7ffec000 != u)
	{
		ERROR();
	} 

	a = 0xffff8000;
	b = 0x7ffe;
	u = mulU32hi_U32byU16(a, b);
	if(0x7ffdc001 != u)
	{
		ERROR();
	} 

	a = 0xffff8000;
	b = 0x8000;
	u = mulU32hi_U32byU16(a, b);
	if(0x7fffc000 != u)
	{
		ERROR();
	} 

	/* 2 */

	a = 32768;
	b = 16384;
	u = mulU32hi_U32byU16(a, b);
	if(0x00002000 != u)
	{
		ERROR();
	} 

	a = 32768;
	b = -16384;
	u = mulU32hi_U32byU16(a, b);
	if(0x00006000 != u)
	{
		ERROR();
	} 

	a = -32768;
	b = 16384;
	u = mulU32hi_U32byU16(a, b);
	if(0x3fffe000 != u)
	{
		ERROR();
	} 

	a = -32768;
	b = -16384;
	u = mulU32hi_U32byU16(a, b);
	if(0xbfffa000 != u)
	{
		ERROR();
	} 

	/* 3 */

	a = 0x3fffffff;
	b = 2;
	u = mulU32hi_U32byU16(a, b);
	if(0x00007fff != u)
	{
		ERROR();
	} 

	a = 0x40000000;
	b = -2; 
	u = mulU32hi_U32byU16(a, b);
	if(0x3fff8000 != u)
	{
		ERROR();
	} 

	a = 0xC0000000;
	b = 2;
	u = mulU32hi_U32byU16(a, b);
	if(0x00018000 != u)
	{
		ERROR();
	} 

	a = 0xC0000001;
	b = -2;
	u = mulU32hi_U32byU16(a, b);
	if(0xbffe8000 != u)
	{
		ERROR();
	} 

	a = 0x80000000;
	b = 1;
	u = mulU32hi_U32byU16(a, b);
	if(0x00008000 != u)
	{
		ERROR();
	} 

	/* 4 - with overflow */
	
	a = 0x7fffffff;
	b = 0x7fff;
	u = mulU32hi_U32byU16(a, b);
	if(0x3fff7fff != u)
	{
		ERROR();
	} 

	a = 0x7fffffff;
	b = 0x8000;
	u = mulU32hi_U32byU16(a, b);
	if(0x3fffffff != u)
	{
		ERROR();
	} 

	a = 0x80000000;
	b = 0x7fff;
	u = mulU32hi_U32byU16(a, b);
	if(0x3fff8000 != u)
	{
		ERROR();
	} 

	a = 0x80000000;
	b = 0x8000;
	u = mulU32hi_U32byU16(a, b);
	if(0x40000000 != u)
	{
		ERROR();
	} 
}	/* mulU32hi_U32byU16_test */

/*
 *  24=16x8
 */

void mulI24_I16byI8_test(void)
{
	int32 i;

	i = mulI24_I16byI8(0x7fff, 0x7f);
	if((int32)0x003f7f81 != i)
	{
		ERROR();
	} 

	i = mulI24_I16byI8(0x7fff, 0xff);
	if((int32)0xffff8001 != i)
	{
		ERROR();
	} 

	i = mulI24_I16byI8(0x7fff, 0x80);
	if((int32)0xffc00080 != i)
	{
		ERROR();
	} 

	i = mulI24_I16byI8(0xffff, 0x7f);
	if((int32)0xffffff81 != i)
	{
		ERROR();
	} 

	i = mulI24_I16byI8(0x8000, 0x7f);
	if((int32)0xffc08000 != i)
	{
		ERROR();
	} 

	i = mulI24_I16byI8(0xffff, 0xff);
	if((int32)0x00000001 != i)
	{
		ERROR();
	} 

	i = mulI24_I16byI8(0x8000, 0x80);
	if((int32)0x00400000 != i)
	{
		ERROR();
	} 

	/* 4660 * 18 */
	i = mulI24_I16byI8(0x1234, 0x12);
	if((int32)0x000147a8 != i)
	{
		ERROR();
	} 

	/* 4660 * -121 */
	i = mulI24_I16byI8(0x1234, 0x87);
	if((int32)0xfff7656c != i)
	{
		ERROR();
	} 

	/* -30875 * -121 */
	i = mulI24_I16byI8(0x8765, 0x87);
	if((int32)0x00390143 != i)
	{
		ERROR();
	} 

	/* -30875 * 18 */
	i = mulI24_I16byI8(0x8765, 0x12);
	if((int32)0xfff7851a != i)
	{
		ERROR();
	} 

}	/* mulI24_I16byI8_test */

void mulI24_I16byU8_test(void)
{
	int32 i;

	i = mulI24_I16byU8(0x7fff, 0x7f);
	if((int32)0x003f7f81 != i)
	{
		ERROR();
	} 

	i = mulI24_I16byU8(0x7fff, 0xff);
	if((int32)0x007f7f01 != i)
	{
		ERROR();
	} 

	i = mulI24_I16byU8(0x7fff, 0x80);
	if((int32)0x003fff80 != i)
	{
		ERROR();
	} 

	i = mulI24_I16byU8(0xffff, 0x7f);
	if((int32)0xffffff81 != i)
	{
		ERROR();
	} 

	i = mulI24_I16byU8(0x8000, 0x7f);
	if((int32)0xffc08000 != i)
	{
		ERROR();
	} 

	i = mulI24_I16byU8(0xffff, 0xff);
	if((int32)0xffffff01 != i)
	{
		ERROR();
	} 

	i = mulI24_I16byU8(0x8000, 0x80);
	if((int32)0xffc00000 != i)
	{
		ERROR();
	} 

	/* 4660 * 18 */
	i = mulI24_I16byU8(0x1234, 0x12);
	if((int32)0x000147a8 != i)
	{
		ERROR();
	} 

	/* 4660 * 135 */
	i = mulI24_I16byU8(0x1234, 0x87);
	if((int32)0x0009996c != i)
	{
		ERROR();
	} 

	/* -30875 * 135 */
	i = mulI24_I16byU8(0x8765, 0x87);
	if((int32)0xffc06643 != i)
	{
		ERROR();
	} 

	/* -30875 * 18 */
	i = mulI24_I16byU8(0x8765, 0x12);
	if((int32)0xfff7851a != i)
	{
		ERROR();
	} 

}	/* mulI24_I16byU8_test */

void mulU24_U16byU8_test(void)
{
	uint32 u;

	u = mulU24_U16byU8(0x00ff, 0xff);
	if(0x0000fe01 != u)
	{
		ERROR();
	} 

	u = mulU24_U16byU8(0xffff, 0xff);
	if(0x00feff01 != u)
	{
		ERROR();
	} 

	/* 4660 * 18 */
	u = mulU24_U16byU8(0x1234, 0x12);
	if(0x000147a8 != u)
	{
		ERROR();
	} 

	/* 4660 * 135 */
	u = mulU24_U16byU8(0x1234, 0x87);
	if(0x0009996c != u)
	{
		ERROR();
	} 

	/* 34661 * 135 */
	u = mulU24_U16byU8(0x8765, 0x87);
	if(0x00476643 != u)
	{
		ERROR();
	} 

	/* 34661 * 18 */
	u = mulU24_U16byU8(0x8765, 0x12);
	if(0x0009851a != u)
	{
		ERROR();
	} 
#if 0
  uint16 i;
  uint16 j;

  /* note: compiler performs strength reduction of i*j in loop -> addition */
  for (i = 0; i < 65535; i++) {
    for (j = 0; j < 256; j++) {
      uint32 acc = ((uint32) i) * j;
      uint32 ret = mulU24_U16byU8 ((uint16) i, (uint8) j);

      if (ret != acc) {
	ERROR();
      }
    }
  }
  /* last entry, as otherwise infinite loop */
  {
    i = 65535U; /* not to rely on for-loop */
    for (j = 0; j < 256; j++) {
      uint32 acc = ((uint32) i) * j;
      uint32 ret = mulU24_U16byU8 ((uint16) i, (uint8) j);

      if (ret != acc) {
	ERROR();
      }
    }
  }
#endif /* 0 */
}	/* mulU24_U16byU8_test */


/*
 *  24=16x8
 */

void mulI16hi_I16byI8_test(void)
{
	int16 i;

	i = mulI16hi_I16byI8(0x7fff, 0x7f);
	if((int16)0x3f7f != i)
	{
		ERROR();
	} 

	i = mulI16hi_I16byI8(0x7fff, 0xff);
	if((int16)0xff80 != i)
	{
		ERROR();
	} 

	i = mulI16hi_I16byI8(0x7fff, 0x80);
	if((int16)0xc000 != i)
	{
		ERROR();
	} 

	i = mulI16hi_I16byI8(0xffff, 0x7f);
	if((int16)0xffff != i)
	{
		ERROR();
	} 

	i = mulI16hi_I16byI8(0x8000, 0x7f);
	if((int16)0xc080 != i)
	{
		ERROR();
	} 

	i = mulI16hi_I16byI8(0xffff, 0xff);
	if((int16)0x0000 != i)
	{
		ERROR();
	} 

	i = mulI16hi_I16byI8(0x8000, 0x80);
	if((int16)0x4000 != i)
	{
		ERROR();
	} 

	/* 4660 * 18 */
	i = mulI16hi_I16byI8(0x1234, 0x12);
	if((int16)0x0147 != i)
	{
		ERROR();
	} 

	/* 4660 * -121 */
	i = mulI16hi_I16byI8(0x1234, 0x87);
	if((int16)0xf765 != i)
	{
		ERROR();
	} 

	/* -30875 * -121 */
	i = mulI16hi_I16byI8(0x8765, 0x87);
	if((int16)0x3901 != i)
	{
		ERROR();
	} 

	/* -30875 * 18 */
	i = mulI16hi_I16byI8(0x8765, 0x12);
	if((int16)0xf785 != i)
	{
		ERROR();
	} 

#if 0
  int16 i;
  int16 j;

  /* note: compiler performs strength reduction of i*j in loop -> addition */
  for (i = -32768; i < 32767; i++) {
    for (j = -128; j < 128; j++) {
      int32 acc = ((int32) i) * j;
      int16 acc16 = (int16) (acc >> 8); /* looks like compiler does rounding */
      int16 ret = mulI16hi_I16byI8 ((int16) i, (int8) j);

      switch (acc16 - ret) {
      case -1:
	mdiff++;
	break;
      case 1:
	pdiff++;
	break;
      case 0:
	break;
      default:
	ERROR ();
      }
    }
  }
  /* last entry, as otherwise infinite loop */
  {
    i = 32767; /* not to rely on for-loop */
    for (j = -128; j < 128; j++) {
      int32 acc = ((int32) i) * j;
      int16 acc16 = (int16) (acc >> 8);
      int16 ret = mulI16hi_I16byI8 ((int16) i, (int8) j);

      switch (acc16 - ret) {
      case -1:
	mdiff++;
	break;
      case 1:
	pdiff++;
	break;
      case 0:
	break;
      default:
	ERROR ();
      }
    }
  }
#endif /* 0 */
}	/* mulI16hi_I16byI8_test */

void mulI16hi_I16byU8_test(void)
{
	int16 i;

	i = mulI16hi_I16byU8(0x7fff, 0x7f);
	if((int16)0x3f7f != i)
	{
		ERROR();
	} 

	i = mulI16hi_I16byU8(0x7fff, 0xff);
	if((int16)0x7f7f != i)
	{
		ERROR();
	} 

	i = mulI16hi_I16byU8(0x7fff, 0x80);
	if((int16)0x3fff != i)
	{
		ERROR();
	} 

	i = mulI16hi_I16byU8(0xffff, 0x7f);
	if((int16)0xffff != i)
	{
		ERROR();
	} 

	i = mulI16hi_I16byU8(0x8000, 0x7f);
	if((int16)0xc080 != i)
	{
		ERROR();
	} 

	i = mulI16hi_I16byU8(0xffff, 0xff);
	if((int16)0xffff != i)
	{
		ERROR();
	} 

	i = mulI16hi_I16byU8(0x8000, 0x80);
	if((int16)0xc000 != i)
	{
		ERROR();
	} 

	/* 4660 * 18 */
	i = mulI16hi_I16byU8(0x1234, 0x12);
	if((int16)0x0147 != i)
	{
		ERROR();
	} 

	/* 4660 * 135 */
	i = mulI16hi_I16byU8(0x1234, 0x87);
	if((int16)0x0999 != i)
	{
		ERROR();
	} 

	/* -30875 * 135 */
	i = mulI16hi_I16byU8(0x8765, 0x87);
	if((int16)0xc066 != i)
	{
		ERROR();
	} 

	/* -30875 * 18 */
	i = mulI16hi_I16byU8(0x8765, 0x12);
	if((int16)0xf785 != i)
	{
		ERROR();
	} 

#if 0
  int16 i;
  uint16 j;

  /* note: compiler performs strength reduction of i*j in loop -> addition */
  for (i = -32768; i < 32767; i++) {
    for (j = 0; j < 256; j++) {
      int32 acc = ((int32) i) * j;
      int16 acc16 = (int16) (acc >> 8); /* looks like compiler does rounding */
      int16 ret = mulI16hi_I16byU8 ((int16) i, (uint8) j);

      switch (acc16 - ret) {
      case -1:
	mdiff++;
	break;
      case 1:
	pdiff++;
	break;
      case 0:
	break;
      default:
	ERROR ();
      }
    }
  }
  /* last entry, as otherwise infinite loop */
  {
    i = 32767; /* not to rely on for-loop */
    for (j = 0; j < 256; j++) {
      int32 acc = ((int32) i) * j;
      int16 acc16 = (int16) (acc >> 8);
      int16 ret = mulI16hi_I16byU8 ((int16) i, (uint8) j);

      switch (acc16 - ret) {
      case -1:
	mdiff++;
	break;
      case 1:
	pdiff++;
	break;
      case 0:
	break;
      default:
	ERROR ();
      }
    }
  }
#endif /* 0 */
}	/* mulI16hi_I16byU8_test */

void mulU16hi_U16byU8_test(void)
{
	uint16 u;
  
	u = mulU16hi_U16byU8(0xff, 0xff);
	if(0x00fe != u)
	{
		ERROR();
	} 

	u = mulU16hi_U16byU8(0xffff, 0xff);
	if(0xfeff != u)
	{
		ERROR();
	} 

	/* 4660 * 18 */
	u = mulU16hi_U16byU8(0x1234, 0x12);
	if(0x0147 != u)
	{
		ERROR();
	} 

	/* 4660 * 135 */
	u = mulU16hi_U16byU8(0x1234, 0x87);
	if(0x0999 != u)
	{
		ERROR();
	} 

	/* 34661 * 135 */
	u = mulU16hi_U16byU8(0x8765, 0x87);
	if(0x4766 != u)
	{
		ERROR();
	} 

	/* 34661 * 18 */
	u = mulU16hi_U16byU8(0x8765, 0x12);
	if(0x0985 != u)
	{
		ERROR();
	} 

#if 0
  uint16 i;
  uint16 j;

  /* note: compiler performs strength reduction of i*j in loop -> addition */
  for (i = 0; i < 65535; i++) {
    for (j = 0; j < 256; j++) {
      uint32 acc = ((uint32) i) * j;
      uint16 acc16 = (uint16) (acc >> 8);
      uint16 ret = mulU16hi_U16byU8 ((uint16) i, (uint8) j);

      if (ret != acc16) {
	ERROR();
      }
      if (0 != (acc & 0xFF000000)) {
	ERROR();
      }
    }
  }
  /* last entry, as otherwise infinite loop */
  {
    i = 65535U; /* not to rely on for-loop */
    for (j = 0; j < 256; j++) {
      uint32 acc = ((uint32) i) * j;
      uint16 acc16 = (uint16) (acc >> 8);
      uint16 ret = mulU16hi_U16byU8 ((uint16) i, (uint8) j);

      if (ret != acc16) {
	ERROR();
      }
      if (0 != (acc & 0xFF000000)) {
	ERROR();
      }
    }
  }
#endif /* 0 */
}	/* mulU16hi_U16byU8_test */


/*
 *  16=8x8
 */

void mulI16_I8byI8_test(void)
{
	int16 i, j;
	
	for (i = 0; i < 128; i++) {
	  int16 sum = 0;
	  
	  if (0 != mulI16_I8byI8 ((int8) i, (uint8) 0)) {
	    ERROR ();
	  }
	  for (j = 1; j < 128; j++) {
	    int16 m = mulI16_I8byI8 ((int8) i, (uint8) j);

	    /* i * j = i * (j -1 + 1) = i * (j-1) + i */
	    sum += i;
	    if (m != sum) {
	      ERROR ();
	    }
	  }
	  sum = 0;
	  for (j = -1; j >= -128; j--) {
	    int16 m = mulI16_I8byI8 ((int8) i, (uint8) j);

	    /* i * j = i * (j -1 + 1) = i * (j-1) + i */
	    sum -= i;
	    if (m != sum) {
	      ERROR ();
	    }
	  }
	}

	for (i = 0; i >= -128; i--) {
	  int16 sum = 0;
	  
	  if (0 != mulI16_I8byI8 ((int8) i, (uint8) 0)) {
	    ERROR ();
	  }
	  for (j = 1; j < 128; j++) {
	    int16 m = mulI16_I8byI8 ((int8) i, (uint8) j);

	    /* i * j = i * (j -1 + 1) = i * (j-1) + i */
	    sum += i;
	    if (m != sum) {
	      ERROR ();
	    }
	  }
	  sum = 0;
	  for (j = -1; j >= -128; j--) {
	    int16 m = mulI16_I8byI8 ((int8) i, (uint8) j);

	    /* i * j = i * (j -1 + 1) = i * (j-1) + i */
	    sum -= i;
	    if (m != sum) {
	      ERROR ();
	    }
	  }
	}

}	/* mulI16_I8byI8_test */

void mulI16_I8byU8_test(void)
{
	int16 i, j;
	
	for (i = 0; i < 128; i++) {
	  int16 sum = 0;
	  
	  if (0 != mulI16_I8byU8 ((int8) i, (uint8) 0)) {
	    ERROR ();
	  }
	  for (j = 1; j < 256; j++) {
	    int16 m = mulI16_I8byU8 ((int8) i, (uint8) j);

	    /* i * j = i * (j -1 + 1) = i * (j-1) + i */
	    sum += i;
	    if (m != sum) {
	      ERROR ();
	    }
	  }
	}

	for (i = 0; i >= -128; i--) {
	  int16 sum = 0;
	  
	  if (0 != mulI16_I8byU8 ((int8) i, (uint8) 0)) {
	    ERROR ();
	  }
	  for (j = 1; j < 256; j++) {
	    int16 m = mulI16_I8byU8 ((int8) i, (uint8) j);

	    /* i * j = i * (j -1 + 1) = i * (j-1) + i */
	    sum += i;
	    if (m != sum) {
	      ERROR ();
	    }
	  }
	}

}	/* mulI16_I8byU8_test */

void mulU16_U8byU8_test(void)
{
	int16 i, j;
	
	for (i = 0; i < 256; i++) {
	  uint16 sum = 0;
	  
	  if (0 != mulU16_U8byU8 ((uint8) i, (uint8) 0)) {
	    ERROR ();
	  }
	  for (j = 1; j < 256; j++) {
	    uint16 m = mulU16_U8byU8 ((uint8) i, (uint8) j);

	    /* i * j = i * (j -1 + 1) = i * (j-1) + i */
	    sum += i;
	    if (m != sum) {
	      ERROR ();
	    }
	  }
	}

}	/* mulU16_U8byU8_test */


