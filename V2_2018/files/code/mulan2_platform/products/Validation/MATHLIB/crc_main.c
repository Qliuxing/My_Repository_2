/*
 * Copyright (C) 2007-2009 Melexis N.V.
 *
 * Math Library 
 * CRC module
 *
 */

#include <mathlib.h>
#include <stdio.h>

extern void crc8_test  (void);
extern void crc16_test (void);
extern void crc_ccitt_test (void);
extern void crc32_test (void);

int main(void)
{
  	crc8_test();
	crc16_test();
	crc_ccitt_test ();
	crc32_test();

	return 0;
}
