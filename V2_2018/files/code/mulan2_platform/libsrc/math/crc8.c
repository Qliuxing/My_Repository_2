/*
 * Copyright (C) 2007-2009 Melexis N.V.
 *
 * Math library
 *
 */

#include <typelib.h>
#include <mathlib.h>

/* Standard polynomial	: D5
   http://www.ece.cmu.edu/~koopman/roses/dsn04/koopman04_crc_poly_embedded.pdf 

   Other polynomials:
   CRC-8 SAE J1850	: 1D
   CRC-8 CCITT		: 8D
   CRC-8 ATM		: 07
*/

#ifndef CRC_POLYNOME
#define CRC_POLYNOME 0xD5
#endif


uint8 crc8 (uint8 c, uint8 crc)
{
  uint16 i;

  crc = crc ^ c;

  for (i = 0; i < 8; ++i) {
    /* 6 insn 
       (of which usex redundant inside the loop, only needed once at end)  
    */
    if ((crc & 0x80) != 0) {
      crc = (crc << 1) ^ CRC_POLYNOME;
    }
    else {
      crc = crc << 1;
    }
  }

  return crc;
}

/* EOF */
