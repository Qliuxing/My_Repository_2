/*
 * Copyright (C) 2007 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>

#define	ERROR()		__asm__("jmp .")

void rand_test(void);

static uint32 t[]=
{
	0x00003039,
	0x33386B1A,
	0x57BA30E3,
	0xF236FBD4,
	0xA8A0D02D,
	0xB6BD4C2E,
	0xB5EF3817,
	0xD31A9628,
	0x482732A1,
	0x2A45B3C2,
};

void rand_test(void)
{
	uint32 x = 0;
	int i;

	for(i = 0; i < 10; ++i)
	{
		x = rand32(x);
		if(t[i] != x)
		{
		 	ERROR();
		}
	}
}
