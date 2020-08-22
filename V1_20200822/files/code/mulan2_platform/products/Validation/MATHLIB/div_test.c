/*
 * Copyright (C) 2007-2012 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>

#define	ERROR()		__asm__("jmp .")

void div_test(void);

void divU16_U16byU16_test(void);
void divI16_I16byI16_test(void);
void divI16_I16byU16_test(void);

void divU16_U32byU16_test(void);
void divI16_I32byI16_test(void);
void divI16_I32byU16_test(void);

void divU32_U32byU16_test(void);
void divI32_I32byI16_test(void);
void divI32_I32byU16_test(void);

void divU8_U8byU8_test(void);
void divI8_I8byI8_test(void);
void divI8_I8byU8_test(void);

void divU8hi_U8byU8_test(void);

void div_test(void)
{
	divU32_U32byU16_test();
	divI32_I32byI16_test();
	divI32_I32byU16_test();

	divU16_U16byU16_test();
	divI16_I16byI16_test();
	divI16_I16byU16_test();

	divU16_U32byU16_test();
	divI16_I32byI16_test();
	divI16_I32byU16_test();

	divU8_U8byU8_test();
	divI8_I8byI8_test();
	divI8_I8byU8_test();

	divU8hi_U8byU8_test();

}

/* 
 * 16 = 16 / 16
 */

void divU16_U16byU16_test(void)
{
	uint16 u;

    u = divU16_U16byU16(0xe100, 0xf000);
	if(0xf000 != u)
	{
		ERROR();
	}

    u = divU16_U16byU16(0x1000, 0x2000);
	if(0x8000 != u)
	{
		ERROR();
	}
}

void divI16_I16byI16_test(void)
{
	int16 i;

    i = divI16_I16byI16(0x1000, 0x4000);
	if(0x4000 != i)
	{
		ERROR();
	}

    i = divI16_I16byI16(0x1000, 0xC000);
	if(0xC000 != (uint16)i)
	{
		ERROR();
	}

    i = divI16_I16byI16(0xF000, 0x4000);
	if(0xC000 != (uint16)i)
	{
		ERROR();
	}

    i = divI16_I16byI16(0xF000, 0xC000);
	if(0x4000 != i)
	{
		ERROR();
	}
}

void divI16_I16byU16_test(void)
{
	int16 i;

    i = divI16_I16byU16(0x1000, 0x8000);
	if(0x2000 != i)
	{
		ERROR();
	}

    i = divI16_I16byU16(0xF000, 0x8000);
	if(0xE000 != (uint16)i)
	{
		ERROR();
	}
}

/* 
 * 16 = 32 / 16
 */

void divU16_U32byU16_test(void)
{
	uint16 u;

    u = divU16_U32byU16(0x0004, 2);
	if(0x0002 != u)
	{
		ERROR();
	}

    u = divU16_U32byU16(0xfffe0001, 0xffff);
	if(0xffff != u)
	{
		ERROR();
	}

}	/* divU16_U32byU16_test */

void divI16_I32byU16_test(void)
{
	int16 i;

    i = divI16_I32byU16(0x7ffe8001, 0xffff);
	if(0x7fff != i)
	{
		ERROR();
	}

    i = divI16_I32byU16(0x80017fff, 0xffff);
	if(0x8001 != (uint16)i)
	{
		ERROR();
	}

    i = divI16_I32byU16(0x80008000, 0xffff);
	if(0x8000 != (uint16)i)
	{
		ERROR();
	}

}	/* divI16_I32byU16_test */


void divI16_I32byI16_test(void)
{
	int16 i;

    i = divI16_I32byI16(0x3fff0001, 0x7fff);
	if(0x7fff != i)
	{
		ERROR();
	}

    i = divI16_I32byI16(0x3fff0001, 0x8001);
	if(0x8001 != (uint16)i)
	{
		ERROR();
	}

    i = divI16_I32byI16(0xc000ffff, 0x8001);
	if(0x7fff != i)
	{
		ERROR();
	}

    i = divI16_I32byI16(0xc000ffff, 0x7fff);
	if(0x8001 != (uint16)i)
	{
		ERROR();
	}


    i = divI16_I32byI16(0xc0008000, 0x8000);
	if(0x7fff != i)
	{
		ERROR();
	}

    i = divI16_I32byI16(0xc0008000, 0x7fff);
	if(0x8000 != (uint16)i)
	{
		ERROR();
	}

    i = divI16_I32byI16(0x3fff8000, 0x8000);
	if(0x8001 != (uint16)i)
	{
		ERROR();
	}

}

/*
 * 32 = 32 / 16
 */

void divI32_I32byI16_test(void)
{
	int32 i;

    /* --- Small vs Small (with reminder) --------------------------- */
    /* positive / positive */
    i = divI32_I32byI16((int32)13, (int16)3);
	if(4 != i)
	{
        ERROR();
	}

    /* positive / negative */
    i = divI32_I32byI16((int32)17, (int16)(-5));
    if(-3 != i)
	{
        ERROR();
	}

    /* negative / positive */
    i = divI32_I32byI16((int32)(-19), (int16)7);
    if(-2 != i)
	{
        ERROR();
	}

    /* negative / negative */
    i = divI32_I32byI16((int32)(-23), (int16)(-11));
    if(2 != i)
	{
        ERROR();
	}


    /* --- Small vs Large (with reminder) --------------------------------- */
    /* positive / positive */
    i = divI32_I32byI16((int32)1, (int16)( (1UL<<15) - 1) );
	if(0 != i)
	{
        ERROR();
	}

    /* positive / negative */
    i = divI32_I32byI16((int32)3, (int16)(1UL<<15));
    if(0 != i)
	{
        ERROR();
	}

    /* negative / positive */
    i = divI32_I32byI16((int32)(-5), (int16)( (1UL<<15) - 1) );
    if(0 != i)
	{
        ERROR();
	}

    /* negative / negative */
    i = divI32_I32byI16((int32)(-7), (int16)(1UL<<15));
    if(0 != i)
	{
        ERROR();
	}

    /* --- Large vs Small (with reminder) --------------------------------- */
    /* positive / positive */
    i = divI32_I32byI16((int32)( (1UL<<31) - 1), (int16)3);
	if(715827882L != i)
	{
        ERROR();
	}

    /* positive / negative */
    i = divI32_I32byI16((int32)( (1UL<<31) - 1), (int16)(-5));
    if(-429496729L != i)
	{
        ERROR();
	}

    /* negative / positive */
    i = divI32_I32byI16((int32)(1UL<<31), (int16)7);
    if(-306783378L != i)
	{
        ERROR();
	}

    /* negative / negative */
    i = divI32_I32byI16((int32)(1UL<<31), (int16)(-11));
    if(195225786L != i)
	{
        ERROR();
	}

    /* --- Large vs Large (with reminder) -------------------------------- */
    /* positive / positive */
    i = divI32_I32byI16((int32)( (1UL<<31) - 1), (int16)((1UL<<15) - 1));
	if(65538L != i)
	{
        ERROR();
	}

    /* positive / negative */
    i = divI32_I32byI16((int32)( (1UL<<31) - 1), (int16)(1UL<<15));
    if(-65535L != i)
	{
        ERROR();
	}

    /* negative / positive */
    i = divI32_I32byI16((int32)(1UL<<31), (int16)((1UL<<15) - 1));
    if(-65538L != i)
	{
        ERROR();
	}

    /* negative / negative */
    i = divI32_I32byI16((int32)(1UL<<31), (int16)(-32767));
    if(65538L != i)
	{
        ERROR();
	}


    /* --- Small vs Small (w/o reminder) --------------------------- */
    /* positive / positive */
    i = divI32_I32byI16((int32)14, (int16)2);
	if(7 != i)
	{
        ERROR();
	}

    /* positive / negative */
    i = divI32_I32byI16((int32)18, (int16)(-3));
    if(-6 != i)
	{
        ERROR();
	}

    /* negative / positive */
    i = divI32_I32byI16((int32)(-20), (int16)4);
    if(-5 != i)
	{
        ERROR();
	}

    /* negative / negative */
    i = divI32_I32byI16((int32)(-24), (int16)(-6));
    if(4 != i)
	{
        ERROR();
	}


    /* --- Large vs Small (w/o reminder) --------------------------------- */
    /* positive / positive */
    i = divI32_I32byI16((int32)( (1UL<<31) - 2), (int16)2);
	if(1073741823L != i)
	{
        ERROR();
	}

    /* positive / negative */
    i = divI32_I32byI16((int32)( (1UL<<31) - 4), (int16)(-4));
    if(-536870911L != i)
	{
        ERROR();
	}

    /* negative / positive */
    i = divI32_I32byI16((int32)(1UL<<31), (int16)8);
    if(-268435456L != i)
	{
        ERROR();
	}

    /* negative / negative */
    i = divI32_I32byI16((int32)(1UL<<31), (int16)(-16));
    if(134217728L != i)
	{
        ERROR();
	}

    /* --- Large vs Large (w/o reminder) -------------------------------- */
    /* positive / positive */
    i = divI32_I32byI16((int32)((1UL<<31) - 2), (int16)((1UL<<15) - 1));
	if(65538L != i)
	{
        ERROR();
	}

    /* positive / negative */
    i = divI32_I32byI16((int32)((1UL<<31) - (1UL<<15)), (int16)(1UL<<15));
    if(-65535L != i)
	{
        ERROR();
	}

    /* negative / positive */
    i = divI32_I32byI16((int32)((1UL<<31)), (int16)(1UL<<14));
    if(-131072L != i)
	{
        ERROR();
	}

    /* negative / negative */
    i = divI32_I32byI16((int32)(1UL<<31), (int16)(1UL<<15));
    if(65536L != i)
	{
        ERROR();
	}

    /* --- Some additional tests with carry   -------------------------------- */
    i = divI32_I32byI16((int32)0xFFFE0001UL, (int16)0xFFFFUL);  /* -131071 / -1 */
    if(0x0001FFFFUL != i)
	{
        ERROR();
	}


    i = divI32_I32byI16((int32)(0xFFFE0001UL - 1), (int16)0xFFFFUL);  /* -131072 / -1 */
    if(0x00020000UL != i)
	{
        ERROR();
	}

    i = divI32_I32byI16((int32)(0xFFFE0001UL + 1), (int16)0xFFFFUL);  /* -131070 / -1 */
    if(0x0001FFFEUL != i)
	{
        ERROR();
	}

    i = divI32_I32byI16((int32)(0xFFFE0001UL - 0xFFFFUL), (int16)0xFFFFUL);  /* -196606 / -1 */
    if(0x0002FFFEUL != i)
	{
        ERROR();
	}

    i = divI32_I32byI16((int32)(0xFFFE0001UL + 0xFFFFUL), (int16)0xFFFFUL);  /* -65536 / -1 */
    if(0x00010000UL != i)
	{
        ERROR();
	}

    i = divI32_I32byI16((int32)(0xFFFE0001UL - 0xFFFFUL - 1), (int16)0xFFFFUL);  /* -196607 / -1 */
    if(0x0002FFFFUL != i)
	{
        ERROR();
	}

    i = divI32_I32byI16((int32)(0xFFFE0001UL - 0xFFFFUL + 1), (int16)0xFFFFUL);  /* -196605 / -1 */
    if(0x0002FFFDUL != i)
	{
        ERROR();
	}

    i = divI32_I32byI16((int32)(0xFFFE0001UL + 0xFFFFUL - 1), (int16)0xFFFFUL);  /* -65537 / -1 */
    if(0x00010001UL != i)
	{
        ERROR();
	}

    i = divI32_I32byI16((int32)(0xFFFE0001UL + 0xFFFFUL + 1), (int16)0xFFFFUL);  /* -65535 / -1 */
    if(0x0000FFFFUL != i)
	{
        ERROR();
	}

}

void divI32_I32byU16_test(void)
{
	int32 i;

    i = divI32_I32byU16(0x7ffffffe, 0x0002);
	if(0x3fffffff != i)
	{
		ERROR();
	}

    i = divI32_I32byU16(0x7fffc000, 0x4000);
	if(0x0001ffff != i)
	{
		ERROR();
	}

    i = divI32_I32byU16(0x7fff8000, 0xffff);
	if(0x00008000 != i)
	{
		ERROR();
	}

    i = divI32_I32byU16(0x80000000, 0x0002);
	if(0xC0000000 != (uint32)i)
	{
		ERROR();
	}

    i = divI32_I32byU16(0x80000000, 0x4000);
	if(0xfffe0000 != (uint32)i)
	{
		ERROR();
	}

    i = divI32_I32byU16(0x80008000, 0xffff);
	if(0xffff8000 != (uint32)i)
	{
		ERROR();
	}

    /* --- Some additional tests with carry   -------------------------------- */
    i = divI32_I32byU16((int32)0xFFFE0001UL, 0xFFFFUL);  /* -131071 / 65535 = -2 */
    if ( -2 != i )
	{
        ERROR();
	}


    i = divI32_I32byU16((int32)(0xFFFE0001UL - 1), 0xFFFFUL);  /* -131072 / 65535 = -2 */
    if ( -2 != i )
	{
        ERROR();
	}

    i = divI32_I32byU16((int32)(0xFFFE0001UL + 1), 0xFFFFUL);  /* -131070 / 65535 = -2 */
    if ( -2 != i )
	{
        ERROR();
	}

    i = divI32_I32byU16((int32)(0xFFFE0001UL - 0xFFFFUL), 0xFFFFUL);  /* -196606 / 65535 = -3 */
    if ( -3 != i )
	{
        ERROR();
	}

    i = divI32_I32byU16((int32)(0xFFFE0001UL + 0xFFFFUL), 0xFFFFUL);  /* -65536 / 65535 = -1 */
    if ( -1 != i )
	{
        ERROR();
	}

    i = divI32_I32byU16((int32)(0xFFFE0001UL - 0xFFFFUL - 1), 0xFFFFUL);  /* -196607 / 65535 = -3 */
    if ( -3 != i )
	{
        ERROR();
	}

    i = divI32_I32byU16((int32)(0xFFFE0001UL - 0xFFFFUL + 1), 0xFFFFUL);  /* -196605 / 65535 = -3 */
    if ( -3 != i )
	{
        ERROR();
	}

    i = divI32_I32byU16((int32)(0xFFFE0001UL + 0xFFFFUL - 1), 0xFFFFUL);  /* -65537 / 65535 = -1 */
    if ( -1 != i )
	{
        ERROR();
	}

    i = divI32_I32byU16((int32)(0xFFFE0001UL + 0xFFFFUL + 1), 0xFFFFUL);  /* -65535 / 65535 */
    if ( -1 != i )
	{
        ERROR();
	}
}	/* divI32_I32byU16_test */


void divU32_U32byU16_test(void)
{
	uint32 u;

    u = divU32_U32byU16(0xffffffff, 0xffff);
	if(0x00010001 != u)
	{
		ERROR();
	}

    u = divU32_U32byU16(0xffffffff, 1);
	if(0xffffffff != u)
	{
		ERROR();
	}

    /* --- Some additional tests with carry   -------------------------------- */
    u = divU32_U32byU16(0xFFFE0001UL, 0xFFFFUL);
    if ( 0xFFFF != u )
	{
        ERROR();
	}


    u = divU32_U32byU16((0xFFFE0001UL - 1), 0xFFFFUL);
    if ( 0xFFFE != u )
	{
        ERROR();
	}

    u = divU32_U32byU16((0xFFFE0001UL + 1), 0xFFFFUL);
    if ( 0xFFFF != u )
	{
        ERROR();
	}

    u = divU32_U32byU16((0xFFFE0001UL - 0xFFFFUL), 0xFFFFUL);
    if ( 0xFFFE != u )
	{
        ERROR();
	}

    u = divU32_U32byU16((0xFFFE0001UL + 0xFFFFUL), 0xFFFFUL);
    if ( 0x10000 != u )
	{
        ERROR();
	}

    u = divU32_U32byU16((0xFFFE0001UL - 0xFFFFUL - 1), 0xFFFFUL);
    if ( 0xFFFD != u )
	{
        ERROR();
	}

    u = divU32_U32byU16((0xFFFE0001UL - 0xFFFFUL + 1), 0xFFFFUL);
    if ( 0xFFFE != u )
	{
        ERROR();
	}

    u = divU32_U32byU16((0xFFFE0001UL + 0xFFFFUL - 1), 0xFFFFUL);
    if ( 0xFFFF != u )
	{
        ERROR();
	}

    u = divU32_U32byU16((0xFFFE0001UL + 0xFFFFUL + 1), 0xFFFFUL);
    if ( 0x10000 != u )
	{
        ERROR();
	}


}	/* divU32_U32byU16_test */


/*
 * 8 = 8 / 8
 */

void divI8_I8byI8_test(void)
{
  int16  i;
  int16  j;

  for (i = -128; i < 128; i++) {
    for (j = -128; j < 128; j++) {
      if (j !=0 ) {
	int16 div1 = i/j; /* 0 / 0 -> 0 */
	int8  div2 = divI8_I8byI8 (i, j); /* 0 / 0 -> FF */

	if (div1 != (int16) div2) {
	  /* -128 / -1 = 128 does not fit in int8 */
	  if (! ((j == -1) && (i == -128) && (div1 == 128) &&(((int8) div2) == -128))) {
	    ERROR ();
	  }
	}
      }
    }
  }

}

void divI8_I8byU8_test(void)
{
  int16  i;
  /* note that C promotes int16 / uint16j to unsigned div
     hence use of uint8 for j instead of uint16
     See e.g.: https://bugzilla.redhat.com/show_bug.cgi?id=210529
  */
  uint8 j;

  for (i = -128; i < 128; i++) {
    for (j = 1; j < 255; j++) {
      int16 div1 = i/j; /* 0 / 0 -> 0 */
      int8  div2 = divI8_I8byU8 (i, j); /* 0 / 0 -> FF */
      
      if (div1 != (int16) div2) {
	ERROR ();
      }
    }
    /* last value */
    {
      int16 div1;
      int8  div2;

      j = 255;
      div1 = i/j; /* 0 / 0 -> 0 */
      div2 = divI8_I8byU8 (i, j); /* 0 / 0 -> FF */
      
      if (div1 != (int16) div2) {
	ERROR ();
      }
    }
  }

}

void divU8_U8byU8_test(void)
{
  uint16 i;
  uint16 j;

  for (i = 0; i < 256; i++) {
    for (j = 1; j < 256; j++) {
      uint16 div1 = i / j; /* 0 / 0 -> 0 */
      uint8 div2 = divU8_U8byU8 (i, j); /* 0 / 0 -> FF */
      
      if (div1 != (uint16) div2) {
	ERROR ();
      }
    }
  }

}

void divU8hi_U8byU8_test(void)
{
    uint16 i;
    uint16 j;

    for (i = 0; i < 256; i++) {
        for (j = i + 1; j < 256; j++) {
#if 0
            uint16 div1 = ((uint16) i * 256) / ((uint16) j); /* 0 / 0 -> 0 */
            uint8 div2 = divU8hi_U8byU8(i, j); /* 0 / 0 -> FF */

            if (div1 != (uint16) div2) {
                ERROR ();
            }
#else
            uint16 div1 = (uint8)(((uint16) i * 256) / ((uint16) j));   /* 0 / 0 -> 0 */
            uint16 div2 = divU8hi_U8byU8(i, j);                         /* 0 / 0 -> FF */

            if (div1 != div2) {
                ERROR ();
            }
#endif
        }
    }

}

/* EOF */
