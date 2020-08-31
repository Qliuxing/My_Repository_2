/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef NVRAM_H_
#define NVRAM_H_

/*
 * NVRAM pages, each 128 bytes
 */
#define NVRAM1_PAGE1    0   /* 0x1000: Application customer     */
#define NVRAM1_PAGE2    1   /* 0x1080: Customer EOL/Production  */
#define NVRAM2_PAGE1    2   /* 0x1100: Application customer     */
#define NVRAM2_PAGE2    3   /* 0x1180: Melexis private          */

#define NVRAM_PAGE_WR_SKIP_WAIT	0x8000

/* ----------------------------------------------------------------------------
 * Saves NVRAM page buffer back to NVRAM
 *
 * Parameters:
 *          page    page number (NVRAM1_PAGE1, NVRAM1_PAGE2 or NVRAM2_PAGE1)
 *
 * Return:
 *          none
 *
 * Notes:
 *  1. If needed this function can be called from UnderVoltage interrupt
 */
extern  void NVRAM_SavePage (uint16_t page);

/* ----------------------------------------------------------------------------
 * Writes the 'data' byte into NVRAM buffer with byte 'address'
 *
 * Notes:
 *  1. NVRAM buffer has only word-size access for writing
 */
extern  void NVRAM_BufferFill (uint16_t address, uint8_t data);

/* ----------------------------------------------------------------------------
 * Loads NVRAM area from flash.
 *
 * Notes:
 *  1. This function is automatically called at startup.
 *
 */
extern void NVRAM_LoadAll (void);

/* ----------------------------------------------------------------------------
 * Saves all writable areas of the NVRAM:
 *      NVRAM1/PAGE1
 *      NVRAM1/PAGE2
 *      NVRAM2/PAGE1
 *
 * Notes:
 *  1. Should NOT be used by application. Use NVRAM_SavePage instead.
 */
extern  void NVRAM_SaveAll (void);

#endif /* NVRAM_H_ */
