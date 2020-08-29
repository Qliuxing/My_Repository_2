/*
 * Copyright (C) 2009-2010 Melexis N.V.
 *
 * Math Library
 * CRC module
 *
 */

#include <mathlib.h>
#include <crc.h>

extern uint8 crc8r_AB (uint8 c, uint8 crc);
extern uint8 crc8r_AB_table256 (uint8 c, uint8 crc);
extern uint8 crc8r_AB_table16 (uint8 c, uint8 crc);
extern uint8 crc8r_AB_tabletwo16 (uint8 c, uint8 crc);

extern uint8 crc8r_AB_table16_data8 (uint8 *data, uint16 length, uint8 init);
extern uint8 crc8r_AB_table16_data16 (uint16 *data, uint16 length, uint8 init);
extern uint8 crc8r_AB_tabletwo16_data8 (uint8 *data, uint16 length, uint8 init);
extern uint8 crc8r_AB_tabletwo16_data16 (uint16 *data, uint16 length, uint8 init);
extern uint8 crc8r_AB_table256_data8 (uint8 *data, uint16 length, uint8 init);
extern uint8 crc8r_AB_table256_data16 (uint16 *data, uint16 length, uint8 init);

#define	ERROR()		__asm__("jmp .")

void crc8_test(void);

#ifdef EXTENDED_TESTS
void crc8_id_test     (void);
void crc8r_id_test    (void);
void crc8_8r_id_test  (void);
#endif /* EXTENDED_TESTS */
void crc8_string_test  (void);
void crc8_block_test  (void);
void crc8r_block_test (void);
void crc8_8r_block_test (void);

void crc8_test(void)
{
  crc8_string_test ();
  crc8_block_test ();
  crc8r_block_test ();
  crc8_8r_block_test ();

#ifdef EXTENDED_TESTS
  crc8_id_test ();
  crc8r_id_test ();
  crc8_8r_id_test ();
#endif /* EXTENDED_TESTS */
}	/* crc8_test() */


/*
 *  crc8 identity tests
 */

void crc8_id_test (void)
{
  int i, j;

  for (i = 0; i < 256; i++) {
    for (j = 0; j < 256; j++) {
      uint8 crc = crc8 (i, j);
      uint8 crct256 = crc8_table256 (i, j);
      if (crc != crct256) {
	ERROR ();
      }
      if (crc != crc8_table16 (i, j)) {
	ERROR ();
      }
      if (crc != crc8_tabletwo16 (i, j)) {
	ERROR ();
      }
    }
  }
}	/* crc8_id_test */

void crc8r_id_test (void)
{
  int i, j;

  for (i = 0; i < 256; i++) {
    for (j = 0; j < 256; j++) {
      uint8 crcr = crc8r (i, j);
      uint8 crcrt256 = crc8r_table256 (i, j);
      if (crcr != crcrt256) {
	ERROR ();
      }
      if (crcr != crc8r_table16 (i, j)) {
	ERROR ();
      }
      if (crcr != crc8r_tabletwo16 (i, j)) {
	ERROR ();
      }
    }
  }
}	/* crc8r_id_test */

void crc8_8r_id_test (void)
{
  int i, j;

  for (i = 0; i < 256; i++) {
    for (j = 0; j < 256; j++) {
      uint8 crc = crc8 (i, j);
      uint8 crcr = bitrev8(crc8r_AB (bitrev8(i), bitrev8(j)));
      if (crc != crcr) {
	ERROR ();
      }
    }
  }
}	/* crc8_8r_id_test */

/*
 *  crc8 string tests
 */

void crc8_string_test (void)
{
  int i;
  const char *s = "12345678";
  uint8 crc = 0;

  for (i = 0; i < 8; i++) {
    crc = crc8 (s[i], crc);
  }
  
  if (crc != 0x45) {
    ERROR ();
  }
}

/*
 *  crc8 block tests
 */

void crc8_block_test (void)
{
  int i;
  const uint8 init = 0xFF;
  uint8 crc = init;
  uint8 *data;

  extern uint16 stext; /* linker symbol pointing to the start of .text section */
  uint16 *block = &stext;
  uint16  length = 512; /* in bytes */

  data = (uint8 *) block;
  for (i = 0; i < length; i++) {
    crc = crc8 (*data++, crc);
  }

  if (init != crc8_table256_data8 ((uint8 *)block, 0, init)) {
    ERROR ();
  }
  if (crc != crc8_table256_data8 ((uint8 *)block, length, init)) {
    ERROR ();
  }
  if (init != crc8_table256_data16 (block, 0, init)) {
    ERROR ();
  }
  if (crc != crc8_table256_data16 (block, length / 2, init)) {
    ERROR ();
  }

  if (init != crc8_table16_data8 ((uint8 *) block, 0, init)) {
    ERROR ();
  }
  if (crc != crc8_table16_data8 ((uint8 *) block, length, init)) {
    ERROR ();
  }
  if (init != crc8_table16_data16 (block, 0, init)) {
    ERROR ();
  }
  if (crc != crc8_table16_data16 (block, length / 2, init)) {
    ERROR ();
  }

  if (init != crc8_tabletwo16_data8 ((uint8 *)block, 0, init)) {
    ERROR ();
  }
  if (crc != crc8_tabletwo16_data8 ((uint8 *)block, length, init)) {
    ERROR ();
  }
  if (init != crc8_tabletwo16_data16 (block, 0, init)) {
    ERROR ();
  }
  if (crc != crc8_tabletwo16_data16 (block, length / 2, init)) {
    ERROR ();
  }

}	/* crc8_block_test */

void crc8r_block_test (void)
{
  int i;
  const uint8 init = 0xFF;
  uint8 crc = init;
  uint8 *data;

  extern uint16 stext; /* linker symbol pointing to the start of .text section */
  uint16 *block = &stext;
  uint16  length = 512; /* in bytes */

  data = (uint8 *) block;
  for (i = 0; i < length; i++) {
    crc = crc8r_AB (*data++, crc);
  }

  if (init != crc8r_AB_table256_data8 ((uint8 *)block, 0, init)) {
    ERROR ();
  }
  if (crc != crc8r_AB_table256_data8 ((uint8 *)block, length, init)) {
    ERROR ();
  }
  if (init != crc8r_AB_table256_data16 (block, 0, init)) {
    ERROR ();
  }
  if (crc != crc8r_AB_table256_data16 (block, length / 2, init)) {
    ERROR ();
  }

  if (init != crc8r_AB_table16_data8 ((uint8 *) block, 0, init)) {
    ERROR ();
  }
  if (crc != crc8r_AB_table16_data8 ((uint8 *) block, length, init)) {
    ERROR ();
  }
  if (init != crc8r_AB_table16_data16 (block, 0, init)) {
    ERROR ();
  }
  if (crc != crc8r_AB_table16_data16 (block, length / 2, init)) {
    ERROR ();
  }

  if (init != crc8r_AB_tabletwo16_data8 ((uint8 *)block, 0, init)) {
    ERROR ();
  }
  if (crc != crc8r_AB_tabletwo16_data8 ((uint8 *)block, length, init)) {
    ERROR ();
  }
  if (init != crc8r_AB_tabletwo16_data16 (block, 0, init)) {
    ERROR ();
  }
  if (crc != crc8r_AB_tabletwo16_data16 (block, length / 2, init)) {
    ERROR ();
  }

}	/* crc8r_block_test */

void crc8_8r_block_test (void)
{
  int i;
  const uint8 init = 0xFF;
  uint8 crc = init;
  uint8 crcr = bitrev8 (init);
  uint8 *data;

  extern uint16 stext; /* linker symbol pointing to the start of .text section */
  uint16 *block = &stext;
  uint16  length = 512; /* in bytes */

  data = (uint8 *) block;
  for (i = 0; i < length; i++) {
    crc = crc8 (*data++, crc);
  }


#if 1
  {
    uint8 rblock[512];
    uint8 *block8 = (uint8 *) block;

    for (i = 0; i < 512; i++) {
      rblock[i] = bitrev8 (block8[i]);
    }

    crcr = crc8r_AB_table256_data8 ((uint8 *)rblock, length, crcr);
    if (crc != bitrev8 (crcr)) {
      ERROR ();
    }
  }
#else
  data = (uint8 *) block;
  for (i = 0; i < length; i++) {
    crcr = crc8r_AB (bitrev8(*data++), crcr);
  }
  if (crc != bitrev8 (crcr)) {
    ERROR ();
  }
#endif

}	/* crc8_8r_block_test */

