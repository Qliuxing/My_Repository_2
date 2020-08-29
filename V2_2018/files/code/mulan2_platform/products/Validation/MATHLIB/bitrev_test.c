/*
 * Copyright (C) 2009-2010 Melexis N.V.
 *
 * Math Library
 *
 */

#include <mathlib.h>

#define	ERROR()		__asm__("jmp .")

/* nibble reverse lookup table */
const uint8 revt[16]=
{
	0x0, /* 0000 -> 0000 */
	0x8, /* 0001 -> 1000 */
	0x4, /* 0010 -> 0100 */
	0xc, /* 0011 -> 1100 */
	0x2, /* 0100 -> 0010 */
	0xa, /* 0101 -> 1010 */
	0x6, /* 0110 -> 0110 */
	0xe, /* 0111 -> 1110 */
	0x1, /* 1000 -> 0001 */
	0x9, /* 1001 -> 1001 */
	0x5, /* 1010 -> 0101 */
	0xd, /* 1011 -> 1101 */
	0x3, /* 1100 -> 0011 */
	0xb, /* 1101 -> 1011 */
	0x7, /* 1110 -> 0111 */
	0xf  /* 1111 -> 1111 */
};

void bitrev_test(void);

void bitrev4_test(void);
void bitrev8_test(void);
void bitrev16_test(void);

void bitrev4_test(void)
{
	int i;

	for(i = 0; i < 16; i++)
	{
	  	uint8 x = bitrev4(i);

		if (x != revt[(i & 0xF)]) {
		  ERROR ();
		}
	}
}

void bitrev8_test(void)
{
	int i;

	for(i = 0; i < 256; i++)
	{
	  	uint8 x = bitrev8(i);

		if ((x >> 4) != revt[(i & 0xF)]) {
		  ERROR ();
		}
		if ((x & 0xF) != revt[(i >> 4)]) {
		  ERROR ();
		}
	}
}

void bitrev16_test(void)
{
	unsigned int i;

	for(i = 0; i < 65535; i++)
	{
	  	uint16 x = bitrev16(i);

		if ((x >> 12) != revt[(i & 0xF)]) {
		  ERROR ();
		}
		if (((x & 0x0F00) >> 8) != revt[(i >> 4) & 0xF]) {
		  ERROR ();
		}
		if (((x & 0x00F0) >> 4) != revt[(i >> 8) & 0xF]) {
		  ERROR ();
		}
		if ((x & 0x000F) != revt[(i >> 12)]) {
		  ERROR ();
		}
	}
	/* last entry */
	if (bitrev16(65535U) != 0xFFFF) {
	  ERROR ();
	}
}

void bitrev_test(void)
{
	bitrev4_test();
	bitrev8_test();
	bitrev16_test();
}
