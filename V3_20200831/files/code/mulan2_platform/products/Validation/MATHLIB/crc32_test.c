/*
 * Copyright (C) 2009 Melexis N.V.
 *
 * Math Library
 * CRC module
 *
 */

#include <mathlib.h>
#include <crc.h>

extern uint32 crc32r_EDB88320_table256_data8 (uint8 *data, uint16 length, uint32 init);

#define	ERROR()		__asm__("jmp .")

void crc32_test(void);

void crc32_string_test  (void);
void crc32_block_test  (void);
void crc32r_block_test (void);
void crc32_32r_block_test (void);

void crc32_test(void)
{
  crc32_string_test ();
  crc32_block_test ();
  crc32r_block_test ();
  crc32_32r_block_test ();

}	/* crc32_test() */


/*
 *  crc32 string tests
 */

/* See e.g. 
   http://www.lammertbies.nl/comm/info/crc-calculation.html?crc=12345678&method=ascii
   
  uint32 crc = 0xFFFFFFFF;

  for (i = 0; i < 8; i++) {
    crc = crc32r_EDB88320 (s[i], crc);
  }
  crc ^= 0xFFFFFFFF;

  if (crc != 0x9AE0DAAF) {
    ERROR ();
  }
*/

void crc32_string_test (void)
{
  int i;
  const char *s = "12345678";
  uint32 crc = 0;

  for (i = 0; i < 8; i++) {
    crc = crc32r (s[i], crc);
  }

  if (crc != 0x016D8E03) {
    ERROR ();
  }
}

 /*
  *  crc32 block tests
  */

 void crc32_block_test (void)
 {
   int i;
   const uint32 init = 0xFFFFFFFF;
   uint32 crc = init;
   uint8 *data;

   extern uint16 stext; /* linker symbol pointing to the start of .text section */
   uint16 *block = &stext;
   uint16  length = 512; /* in bytes */

   data = (uint8 *) block;
   for (i = 0; i < length; i++) {
     crc = crc32 (*data++, crc);
   }

   if (init != crc32_table256_data8 ((uint8 *)block, 0, init)) {
     ERROR ();
   }
   if (crc != crc32_table256_data8 ((uint8 *)block, length, init)) {
     ERROR ();
   }
   if (init != crc32_table256_data16 (block, 0, init)) {
     ERROR ();
   }
   if (crc != crc32_table256_data16 (block, length / 2, init)) {
     ERROR ();
   }

   if (init != crc32_table16_data8 ((uint8 *) block, 0, init)) {
     ERROR ();
   }
   if (crc != crc32_table16_data8 ((uint8 *) block, length, init)) {
     ERROR ();
   }
   if (init != crc32_table16_data16 (block, 0, init)) {
     ERROR ();
   }
   if (crc != crc32_table16_data16 (block, length / 2, init)) {
     ERROR ();
   }

   if (init != crc32_tabletwo16_data8 ((uint8 *)block, 0, init)) {
     ERROR ();
   }
   if (crc != crc32_tabletwo16_data8 ((uint8 *)block, length, init)) {
     ERROR ();
   }
   if (init != crc32_tabletwo16_data16 (block, 0, init)) {
     ERROR ();
   }
   if (crc != crc32_tabletwo16_data16 (block, length / 2, init)) {
     ERROR ();
   }

 }	/* crc32_block_test */

 void crc32r_block_test (void)
 {
   int i;
   const uint32 init = 0xFFFFFFFF;
   uint32 crc = init;
   uint8 *data;

   extern uint16 stext; /* linker symbol pointing to the start of .text section */
   uint16 *block = &stext;
   uint16  length = 512; /* in bytes */

   data = (uint8 *) block;
   for (i = 0; i < length; i++) {
     crc = crc32r (*data++, crc);
   }

#if 1
   /* not supported with GCC pre 1.8 due to internal compiler error */
   if (init != crc32r_table256_data8 ((uint8 *)block, 0, init)) {
     ERROR ();
   }
   if (crc != crc32r_table256_data8 ((uint8 *)block, length, init)) {
     ERROR ();
   }
   if (init != crc32r_table256_data16 (block, 0, init)) {
     ERROR ();
   }
   if (crc != crc32r_table256_data16 (block, length / 2, init)) {
     ERROR ();
   }
#endif

   if (init != crc32r_table16_data8 ((uint8 *) block, 0, init)) {
     ERROR ();
   }
   if (crc != crc32r_table16_data8 ((uint8 *) block, length, init)) {
     ERROR ();
   }
   if (init != crc32r_table16_data16 (block, 0, init)) {
     ERROR ();
   }
   if (crc != crc32r_table16_data16 (block, length / 2, init)) {
     ERROR ();
   }

   if (init != crc32r_tabletwo16_data8 ((uint8 *)block, 0, init)) {
     ERROR ();
   }
   if (crc != crc32r_tabletwo16_data8 ((uint8 *)block, length, init)) {
     ERROR ();
   }
   if (init != crc32r_tabletwo16_data16 (block, 0, init)) {
     ERROR ();
   }
   if (crc != crc32r_tabletwo16_data16 (block, length / 2, init)) {
     ERROR ();
   }

}	/* crc32r_block_test */


static uint32 bitrev32 (uint32 v)
{
  uint16 r1, r2;

  r1 = bitrev16 (v >> 16);
  r2 = bitrev16 ((uint16) v);

  return (((uint32) r2) << 16) | r1;
}


void crc32_32r_block_test (void)
{
  int i;
  const uint32 init = 0xFFFFFFFF;
  uint32 crc = init;
  uint32 crcr = bitrev32 (init);
  uint8 *data;

  extern uint16 stext; /* linker symbol pointing to the start of .text section */
  uint16 *block = &stext;
  uint16  length = 512; /* in bytes */

  data = (uint8 *) block;
  for (i = 0; i < length; i++) {
    crc = crc32 (*data++, crc);
  }


#if 1
  {
    uint8 rblock[512];
    uint8 *block8 = (uint8 *) block;

    for (i = 0; i < 512; i++) {
      rblock[i] = bitrev8 (block8[i]);
    }

    crcr = crc32r_EDB88320_table256_data8 ((uint8 *)rblock, length, crcr);
    if (crc != bitrev32 (crcr)) {
      ERROR ();
    }
  }
#else
  data = (uint8 *) block;
  for (i = 0; i < length; i++) {
    crcr = crc32r_EDB88320 (bitrev8(*data++), crcr);
  }
  if (crc != bitrev32 (crcr)) {
    ERROR ();
  }
#endif

}	/* crc32_32r_block_test */
