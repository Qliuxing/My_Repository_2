/*
 * Copyright (C) 2007-2010 Melexis N.V.
 *
 * Math library
 *
 */

#include <typelib.h>
#include <mathlib.h>

extern const uint8 bitrev4_16[16];

/* 6 insn (24 cycles, 14+16(table)=30bytes) */
uint8 bitrev4(uint8 x)
{
	return bitrev4_16[x & 0xF];
}

/* EOF */
