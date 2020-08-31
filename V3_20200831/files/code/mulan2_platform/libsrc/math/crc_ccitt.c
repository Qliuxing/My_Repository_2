/*
 * Copyright (C) 2007-2010 Melexis N.V.
 *
 * Math library
 *
 */

#include <typelib.h>
#include <mathlib.h>

/* 
   Optimized implementation of CRC16 using the CCITT polynomial 1021
   (X^16 + X^12 + X^5 + 1)

   See e.g. http://www.ccsinfo.com/forum/viewtopic.php?t=24977 and others
*/

/* 22 insn , no tables */
uint16 crc_ccitt (uint8 c, uint16 crc)
{
    crc  = (uint8)(crc >> 8) | (crc << 8);
    crc ^= c;
    crc ^= (uint8)(crc & 0xff) >> 4;
    crc ^= (crc << 8) << 4;
    crc ^= ((crc & 0xff) << 4) << 1;

    return crc;
}

/* EOF */
