/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 *  NOTES:
 *      1.  All functions in this module should be defined with __MLX_TEXT__
 *          attribute. Thus they will be linked in first half of the Flash.
 *
 *      2.  This module should NOT use _initialized_ global and static variables!
 *          Such variables are linked into .data or .dp.data sections and their
 *          initialization lists are stored at load address (LMA) in the Flash.
 *          Since there is no control on the position of the load address, the
 *          linker might link it to second half of the Flash and thus
 *          initialization values will be overwritten by the loader during
 *          programming of the a new application. As a result, variables in .data
 *          sections will not be correctly initialized.
 *          Use uninitialized variables instead (will be linked to .bss section).
 */

#include <ioports.h>
#include <syslib.h>
#include <plib.h>
#include <nvram.h>


/* ----------------------------------------------------------------------------
 * Loads NVRAM into SRAM buffer to be accessible by application
 */
__MLX_TEXT__  void NVRAM_LoadAll (void)
{
    while ((NV_CTRL & NV_BUSY) != 0) {  /* wait if previous access is ongoing */
        WDG_Manager(); /* polling */
    }

    /* yso: EEPROM Select and Mode select should in two different commands according to spr */
    NV_CTRL = NV_SEL;                   /* select NVRAM2 (both pages) */
    NV_CTRL = NV_CONF_RECALL | NV_SEL;  /* Execute Recall command (Flash -> RAM) for NVRAM2 */
    while ((NV_CTRL & NV_BUSY) != 0) {  /* wait until command will be finished */
        WDG_Manager(); /* polling */
    }

    NV_CTRL = 0;                        /* select NVRAM1 (both pages) */
    NV_CTRL = NV_CONF_RECALL;           /* Execute Recall (Flash -> RAM) for NVRAM1 */
    while ((NV_CTRL & NV_BUSY) != 0) {  /* wait command will be finished */
        WDG_Manager(); /* polling */
    }

#if 0 /* spr: switching back to SRAM_MODE is not needed; SRAM can be read in any mode */
    NV_CTRL = NV_CONF_SRAM_MODE;        /* Set SRAM mode. This is normal more of operation */
    while ((NV_CTRL & NV_BUSY) != 0) {  /* wait command will be finished */
        WDG_Manager(); /* polling */
    }
#endif
}


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
__MLX_TEXT__  void NVRAM_SavePage (uint16_t page)
{
    uint16_t mode;


    switch (page & ~NVRAM_PAGE_WR_SKIP_WAIT) {
        case NVRAM1_PAGE1:
            mode = 0 | NV1_MEM_ALLC | 0;
            break;

        case NVRAM1_PAGE2:
            mode = 0 | NV1_MEM_ALLC | NV1_MEM_SEL;
            break;

        case NVRAM2_PAGE1:
            mode = NV_SEL | NV2_MEM_ALLC | 0;
            break;

        /*
         * NVRAM2_PAGE2 should never be saved from any SW
         */

        default:
             /* Wrong page requested */
            return;
            break;
    }

    /* Bit NV_SRAMWR is set when any SRAM page (out of 4 available) are updated to indicated
     * that the save procedure should be executed. Also this NV_SRAMWR bit is cleared immediately
     * when any of SRAM pages are saved back to the NVRAM.
     * This means that already after saving one page NV_SRAMWR bit will be cleared and we can not
     * use it anymore to find out if saving of other pages is needed.
     * Thus we can not use this bit in case a separate page saving is needed.
     */
#if 0
    if ((NV_CTRL & NV_SRAMWR) != 0) {       /* if RAM area contains unsaved data */
#endif
        while ((NV_CTRL & NV_BUSY) != 0) {  /* wait if previous access is ongoing */
            WDG_Manager(); /* polling */
        }

        /* yso: EEPROM Select and Mode select should in two different commands according to spr */
        NV_CTRL = mode;                     /* select NVRAM chip and page           */
        NV_CTRL = mode | NV_CONF_STORE;     /* Execute Store command (RAM -> Flash) */

		if ( (page & NVRAM_PAGE_WR_SKIP_WAIT) == 0 )
		{
			while ((NV_CTRL & NV_BUSY) != 0) {  /* wait until command will be finished */
				WDG_Manager(); /* polling */
			}
		}
#if 0
    }
#endif
    /* else: there is no unsaved data in RAM */
}


/* ----------------------------------------------------------------------------
 * Writes the 'data' byte into NVRAM buffer with byte 'address'
 *
 * Notes:
 *  1. NVRAM buffer has only word-size access for writing
 */
__MLX_TEXT__  void NVRAM_BufferFill (uint16_t address, uint8_t data)
{
    uint16_t *dst;
    uint16_t w;


    if ((address & 1) != 0) {                                   /* if address is odd ..                 */
        address &= ~1;                                          /* .. make it even ..                   */
        dst = (uint16_t *)(address);
        w = *dst;                                               /* .. and read the word                 */
        w = (w & 0x00FF) | ((uint16_t)data << 8);               /* replace high-byte with a data byte   */
    }
    else {                                                      /* else: address is even                */
        dst = (uint16_t *)(address);
        w = *dst;                                               /* .. and read the word                 */
        w = (w & 0xFF00) | data;                                /* replace low-byte with a data byte    */
    }

    *dst = w;                                                   /* write the word back to memory        */
}


/* ----------------------------------------------------------------------------
 * Saves all writable areas of the NVRAM:
 *      NVRAM1/PAGE1
 *      NVRAM1/PAGE2
 *      NVRAM2/PAGE1
 *
 * Notes:
 *  1. Should NOT be used by application. Use NVRAM_Save instead.
 */
__MLX_TEXT__  void NVRAM_SaveAll (void)
{
    NVRAM_SavePage(NVRAM1_PAGE1);
    NVRAM_SavePage(NVRAM1_PAGE2);
    NVRAM_SavePage(NVRAM2_PAGE1);
}

/* EOF */
