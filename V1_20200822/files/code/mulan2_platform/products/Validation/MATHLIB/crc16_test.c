/*
 * Copyright (C) 2009 Melexis N.V.
 *
 * Math Library
 * CRC module
 *
 */

#include <mathlib.h>
#include <crc.h> 

extern uint16 crc16r_A001 (uint8 c, uint16 crc);
extern uint16 crc16r_A001_table256 (uint8 c, uint16 crc);
extern uint16 crc16r_A001_table16 (uint8 c, uint16 crc);
extern uint16 crc16r_A001_tabletwo16 (uint8 c, uint16 crc);

extern uint16 crc16r_8408 (uint8 c, uint16 crc);
extern uint16 crc16r_8408_table256 (uint8 c, uint16 crc);
extern uint16 crc16r_8408_table16 (uint8 c, uint16 crc);
extern uint16 crc16r_8408_tabletwo16 (uint8 c, uint16 crc);

extern uint16 crc16r_A001_table16_data8 (uint8 *data, uint16 length, uint16 init);
extern uint16 crc16r_A001_table16_data16 (uint16 *data, uint16 length, uint16 init);
extern uint16 crc16r_A001_tabletwo16_data8 (uint8 *data, uint16 length, uint16 init);
extern uint16 crc16r_A001_tabletwo16_data16 (uint16 *data, uint16 length, uint16 init);
extern uint16 crc16r_A001_table256_data8 (uint8 *data, uint16 length, uint16 init);
extern uint16 crc16r_A001_table256_data16 (uint16 *data, uint16 length, uint16 init);

extern uint16 crc16r_8408_table16_data8 (uint8 *data, uint16 length, uint16 init);
extern uint16 crc16r_8408_table16_data16 (uint16 *data, uint16 length, uint16 init);
extern uint16 crc16r_8408_tabletwo16_data8 (uint8 *data, uint16 length, uint16 init);
extern uint16 crc16r_8408_tabletwo16_data16 (uint16 *data, uint16 length, uint16 init);
extern uint16 crc16r_8408_table256_data8 (uint8 *data, uint16 length, uint16 init);
extern uint16 crc16r_8408_table256_data16 (uint16 *data, uint16 length, uint16 init);

#define	ERROR()		__asm__("jmp .")

void crc16_test(void);

#ifdef EXTENDED_TESTS
void crc16_id_test     (void);
void crc16r_id_test    (void);
void crc16_16r_id_test  (void);
#endif /* EXTENDED_TESTS */
void crc16_string_test (void);
void crc16_block_test  (void);
void crc16r_block_test (void);
void crc16_16r_block_test (void);

void crc16_test(void)
{
  crc16_string_test ();
  crc16_block_test ();
  crc16r_block_test ();
  crc16_16r_block_test ();

#ifdef EXTENDED_TESTS
  crc16_id_test ();
  crc16r_id_test ();
  crc16_16r_id_test ();
#endif /* EXTENDED_TESTS */
}	/* crc16_test() */


/* crc_ccitt string test */
void crc_ccitt_string_test(void)
{
  int i;
  const char *s = "12345678";
  uint16 crc = 0;

  for (i = 0; i < 8; i++) {
    crc = crc_ccitt (s[i], crc);
  }
  
  if (crc != 0x9015) {
    ERROR ();
  }
}


/* crc_ccitt identity tests */
void crc16_ccitt_id_test(void)
{
  unsigned int i, j;

  for (i = 0; i < 256; i++) {
    for (j = 0; j < 65535; j++) {
      if (crc_ccitt (i, j) != crc16 (i, j)) {
	ERROR ();
      }
    }
  }
}

void crc_ccitt_test(void)
{
  crc_ccitt_string_test ();
#ifdef EXTENDED_TESTS
  crc16_ccitt_id_test ();
#endif /* EXTENDED_TESTS */
}

/*
 *  crc16 identity tests
 */

void crc16_id_test (void)
{
  unsigned int i, j;

  for (i = 0; i < 256; i++) {
    for (j = 0; j < 65535; j++) {
       uint16 crc = crc16 (i, j);
       uint16 crct256 = crc16_table256 (i, j);
       if (crc != crct256) {
	 ERROR ();
       }
       if (crc != crc16_table16 (i, j)) {
	 ERROR ();
       }
       if (crc != crc16_tabletwo16 (i, j)) {
	 ERROR ();
       }
     }
   }
 }	/* crc16_id_test */

 void crc16r_id_test (void)
 {
   unsigned int i, j;

   for (i = 0; i < 256; i++) {
     for (j = 0; j < 65535; j++) {
       uint16 crcr = crc16r (i, j);
       uint16 crcrt256 = crc16r_table256 (i, j);
       if (crcr != crcrt256) {
	 ERROR ();
       }
       if (crcr != crc16r_table16 (i, j)) {
	 ERROR ();
       }
       if (crcr != crc16r_tabletwo16 (i, j)) {
	 ERROR ();
       }
     }
   }
 }	/* crc16r_id_test */

 void crc16_16r_id_test (void)
 {
   unsigned int i, j;

   for (i = 1; i < 256; i++) {
     for (j = 0; j < 65535; j++) {
       uint16 crc = crc16 (i, j);
       uint16 crcr = bitrev16(crc16r_8408 (bitrev8(i), bitrev16(j)));
       if (crc != crcr) {
	 ERROR ();
       }
     }
   }
 }	/* crc16_16r_id_test */

/*
 *  crc16 string tests
 */

void crc16_string_test (void)
{
  int i;
  const char *s = "12345678";
  uint16 crc = 0;

  for (i = 0; i < 8; i++) {
    crc = crc16 (s[i], crc);
  }
  
  if (crc != 0x9015) {
    ERROR ();
  }
}

 /*
  *  crc16 block tests
  */

 void crc16_block_test (void)
 {
   int i;
   const uint16 init = 0xFFFF;
   uint16 crc = init;
   uint8 *data;

   extern uint16 stext; /* linker symbol pointing to the start of .text section */
   uint16 *block = &stext;
   uint16  length = 512; /* in bytes */

   data = (uint8 *) block;
   for (i = 0; i < length; i++) {
     crc = crc16 (*data++, crc);
   }

   if (init != crc16_table256_data8 ((uint8 *)block, 0, init)) {
     ERROR ();
   }
   if (crc != crc16_table256_data8 ((uint8 *)block, length, init)) {
     ERROR ();
   }
   if (init != crc16_table256_data16 (block, 0, init)) {
     ERROR ();
   }
   if (crc != crc16_table256_data16 (block, length / 2, init)) {
     ERROR ();
   }

   if (init != crc16_table16_data8 ((uint8 *) block, 0, init)) {
     ERROR ();
   }
   if (crc != crc16_table16_data8 ((uint8 *) block, length, init)) {
     ERROR ();
   }
   if (init != crc16_table16_data16 (block, 0, init)) {
     ERROR ();
   }
   if (crc != crc16_table16_data16 (block, length / 2, init)) {
     ERROR ();
   }

   if (init != crc16_tabletwo16_data8 ((uint8 *)block, 0, init)) {
     ERROR ();
   }
   if (crc != crc16_tabletwo16_data8 ((uint8 *)block, length, init)) {
     ERROR ();
   }
   if (init != crc16_tabletwo16_data16 (block, 0, init)) {
     ERROR ();
   }
   if (crc != crc16_tabletwo16_data16 (block, length / 2, init)) {
     ERROR ();
   }

 }	/* crc16_block_test */

 void crc16r_block_test (void)
 {
   int i;
   const uint16 init = 0xFFFF;
   uint16 crc = init;
   uint8 *data;

   extern uint16 stext; /* linker symbol pointing to the start of .text section */
   uint16 *block = &stext;
   uint16  length = 512; /* in bytes */

   data = (uint8 *) block;
   for (i = 0; i < length; i++) {
     crc = crc16r_A001 (*data++, crc);
   }

   if (init != crc16r_A001_table256_data8 ((uint8 *)block, 0, init)) {
     ERROR ();
   }
   if (crc != crc16r_A001_table256_data8 ((uint8 *)block, length, init)) {
     ERROR ();
   }
   if (init != crc16r_A001_table256_data16 (block, 0, init)) {
     ERROR ();
   }
   if (crc != crc16r_A001_table256_data16 (block, length / 2, init)) {
     ERROR ();
   }

   if (init != crc16r_A001_table16_data8 ((uint8 *) block, 0, init)) {
     ERROR ();
   }
   if (crc != crc16r_A001_table16_data8 ((uint8 *) block, length, init)) {
     ERROR ();
   }
   if (init != crc16r_A001_table16_data16 (block, 0, init)) {
     ERROR ();
   }
   if (crc != crc16r_A001_table16_data16 (block, length / 2, init)) {
     ERROR ();
   }

   if (init != crc16r_A001_tabletwo16_data8 ((uint8 *)block, 0, init)) {
     ERROR ();
   }
   if (crc != crc16r_A001_tabletwo16_data8 ((uint8 *)block, length, init)) {
     ERROR ();
   }
   if (init != crc16r_A001_tabletwo16_data16 (block, 0, init)) {
     ERROR ();
   }
   if (crc != crc16r_A001_tabletwo16_data16 (block, length / 2, init)) {
     ERROR ();
   }

}	/* crc16r_block_test */

void crc16_16r_block_test (void)
{
  int i;
  const uint16 init = 0xFFFF;
  uint16 crc = init;
  uint16 crcr = bitrev16 (init);
  uint8 *data;

  extern uint16 stext; /* linker symbol pointing to the start of .text section */
  uint16 *block = &stext;
  uint16  length = 512; /* in bytes */

  data = (uint8 *) block;
  for (i = 0; i < length; i++) {
    crc = crc16 (*data++, crc);
  }


#if 1
  {
    uint8 rblock[512];
    uint8 *block8 = (uint8 *) block;

    for (i = 0; i < 512; i++) {
      rblock[i] = bitrev8 (block8[i]);
    }

    crcr = crc16r_8408_table256_data8 ((uint8 *)rblock, length, crcr);
    if (crc != bitrev16 (crcr)) {
      ERROR ();
    }
  }
#else
  data = (uint8 *) block;
  for (i = 0; i < length; i++) {
    crcr = crc16r_8408 (bitrev8(*data++), crcr);
  }
  if (crc != bitrev16 (crcr)) {
    ERROR ();
  }
#endif

}	/* crc16_16r_block_test */
