/*
 * Copyright (C) 2011-2015 Melexis N.V.
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

#include <syslib.h>
#include <plib.h>               /* product specific libraries */

#include "lin.h"
#include "flashfunctions.h"
#include "flashupload_cfg.h"
#include "flashupload.h"

#define _FAST                            1        /* Faster-code and reduced code-size */

#if defined (SUPPORT_LINNETWORK_LOADER)
#include "lin_nad.h"
#endif /* SUPPORT_LINNETWORK_LOADER */

#if defined (HAS_FLASH_WRITE_BUFFER_IN_NVRAM_SRAM)
#include <nvram.h>              /* NVRAM_BufferFill */
#endif


/*
 * Flash operation timeouts
 */
#define FL_TIME_5MS     (        0 |         0 |         0)
#define FL_TIME_10MS    (        0 |         0 | FL_ERA_T0)
#define FL_TIME_15MS    (        0 | FL_ERA_T1 |         0)
#define FL_TIME_20MS    (        0 | FL_ERA_T1 | FL_ERA_T0)
#define FL_TIME_40MS    (FL_ERA_T2 |         0 |         0)     // 30ms
#define FL_TIME_80MS    (FL_ERA_T2 |         0 | FL_ERA_T0)     // 40ms
#define FL_TIME_120MS   (FL_ERA_T2 | FL_ERA_T1 |         0)     // 80ms
#define FL_TIME_160MS   (FL_ERA_T2 | FL_ERA_T1 | FL_ERA_T0)     // 160ms

/* Commands */
#define FL_ERASE        (           0 | FL_WRERA_EN)
#define FL_WRITE        (FL_WRERA_SEL | FL_WRERA_EN)

/* Configure write and erase time */
#define FL_WRITE_TIME   FL_TIME_5MS
#define FL_ERASE_TIME   FL_TIME_20MS

/* Timeouts for external tool */
#if (FL_WRITE_TIME == FL_TIME_5MS) && (FL_ERASE_TIME == FL_TIME_20MS)
#define ML_FLASH_WRITE_ONLY_TIMEOUT_MS          6
#define ML_FLASH_H12_ERASE_WRITE_TIMEOUT_MS     (2 * 21 + ML_FLASH_WRITE_ONLY_TIMEOUT_MS)   /* 2x21ms since it is erased twice in raw (PLTF-700) */
#define ML_FLASH_H11_ERASE_WRITE_TIMEOUT_MS     (2 * 21 * ML_FLASH_SECTOR_SIZE_IN_PAGES + ML_FLASH_WRITE_ONLY_TIMEOUT_MS)

#else
#error "External tool timeouts shall be adjusted to match FL_WRITE_TIME and FL_ERASE_TIME settings"
#endif


#if defined (LDR_HAS_PAGE_BUFFER_ON_STACK)
/*
 * Pointer to the buffer in RAM aligned on the word boundary.
 * Page buffer on the stack should be used after device reset w/o started application.
 * Otherwise, there is no guarantee that application leave enough space for buffer on stack.
 */
ml_uint8 *page_buffer;

#if !defined (LDR_RESET_ON_ENTER_PROG_MODE)
#error "The LDR_RESET_ON_ENTER_PROG_MODE option shall be used when LDR_HAS_PAGE_BUFFER_ON_STACK is enabled"
#endif /* LDR_RESET_ON_ENTER_PROG_MODE */

#else /* LDR_HAS_PAGE_BUFFER_ON_STACK */

#if defined (HAS_FLASH_WRITE_BUFFER_IN_NVRAM_SRAM)
static uint8_t * const page_buffer = (uint8_t * const)FLASH_WRITE_BUFFER_ADDR;  /* use NVRAM SRAM area as a page buffer  */
#else /* HAS_FLASH_WRITE_BUFFER_IN_NVRAM_SRAM */
static ml_uint8 page_buffer[128] __attribute__((aligned(2)));                   /* buffer in RAM aligned on the word boundary   */
#endif /* HAS_FLASH_WRITE_BUFFER_IN_NVRAM_SRAM */

#endif /* LDR_HAS_PAGE_BUFFER_ON_STACK */


#if (ML_FLASH_NUMBER_OF_SECTORS > 16)
#error "Size of erase_sectors_bitmap is not enough to store more than 16 flags"
#endif

static uint16_t erase_sectors_bitmap;           /* indicates which sectors have been already erased */

/* Module's functions */
#if (LDR_FLASH_WRITE_TEST != FLASH_TEST_NONE)
uint16_t Flash_PageVerifyBuffer (uint16_t addr);

#endif /* LDR_FLASH_WRITE_TEST */

static  uint16_t AddrToSector (uint16_t addr);
static  bool     IsSectorErased (uint16_t sector);
static  void     EraseSector (uint16_t sector);
static  bool     HasSectorEraseByHw (void);

/* ----------------------------------------------------------------------------
 * Initializes Flash Driver
 *
 * \note
 *  1. Skip erasing of the last sector of the Flash in loader state 3, since
 *     this sector was already erased at the end of the state 2. The far_page_0
 *     of the new application (with new Reset Vector) was also already written
 *     at the end of state 2.
 */
__MLX_TEXT__ void Flash_InitDriver(void)
{
    if (LDR_GetState() == 3) {                                          /* in state 3 skip erasing of the last sector (see notes above) .. */
        erase_sectors_bitmap = 1u << (ML_FLASH_NUMBER_OF_SECTORS - 1);  /* .. => mark last sector as erased  */
    }
    else {
        erase_sectors_bitmap = 0;                                       /*  so far, no sectors have been erased yet */
    }
}

/* ----------------------------------------------------------------------------
 * Check (at runtime) if Flash HW can erase sector of N-pages at once
 *
 * \return `true' if Flash HW can erase a sector of N-pages at once
 *
 * \note
 * 1. H12 Flash (and above) supports sector erase (16 pages), while H11 Flash
 *    erases only page-by-page.
 * 2. To identify presence of H12 Flash (and above), EEPROM value EEP_FLASH_ERASE_SIZE
 *    can be used [EEPROM byte @ 0x11B2]:
 *      1 (or 0): single page erase is possible
 *      16: erase unit size is 16 pages
 * 3. Alternatively (to be confirmed), the trimming value EEP_TM_TR_LSW and
 *    EEP_TM_TR_MSW can be used. Non-zero values indicate presence of H12 flash.
 */
__MLX_TEXT__ static INLINE bool HasSectorEraseByHw (void) {
#if defined (DEBUG_FORCE_H12_FLASH_DETECTION)
# warning   "DEBUG_FORCE_H12_FLASH_DETECTION is enabled"
    return true;

#elif defined (DEBUG_FORCE_H11_FLASH_DETECTION)
# warning     "DEBUG_FORCE_H11_FLASH_DETECTION is enabled"
    return false;

#else
    return (EEP_FLASH_ERASE_SIZE > 1U);
#endif
}


/* ----------------------------------------------------------------------------
 * Return time needed to complete the write operation for the specified
 * address `addr`. When address is inside the sector which was not written
 * before, then write time will also include a time needed to erase the sector.
 */
__MLX_TEXT__ uint16_t Flash_GetWriteTime(uint16_t addr)
{
    uint16_t wr_time;

    addr = addr & ~ML_FLASH_BUFFER_MASK;            /* get start address of the page */

    uint16_t sector_number = AddrToSector(addr);
    if ( ! IsSectorErased(sector_number) ) {        /* if Flash sector is not erased yet ..  */
        if (HasSectorEraseByHw()) {                 /* if full sector can be erase by HW ..  */
            wr_time = ML_FLASH_H12_ERASE_WRITE_TIMEOUT_MS;
        }
        else {                                      /* else: more time is needed to simulate sector erase in SW */
            wr_time = ML_FLASH_H11_ERASE_WRITE_TIMEOUT_MS;
        }
    }
    else {                                          /* else: Flash sector was already erased */
        wr_time = ML_FLASH_WRITE_ONLY_TIMEOUT_MS;
    }

    return wr_time;
}


/* ----------------------------------------------------------------------------
 * Reads Flash Page into RAM buffer
 *
 * Notes:
 *  1. Automatically aligns requested address to the nearest page located
 *     below in memory.
 */
__MLX_TEXT__ void Flash_PageRead (uint16_t addr)
{
#if _FAST
    uint16_t *src = (uint16_t *)(addr & ~(ML_FLASH_BUFFER_SIZE_IN_WORDS * 2 - 1));    /* get page start address */
    uint16_t *dst = (uint16_t *)page_buffer;

    do
    {
        *dst++ = *src++;
    } while (dst < (uint16_t *)(page_buffer + ML_FLASH_BUFFER_SIZE_IN_WORDS * sizeof(uint16_t)));
#else  /* _FAST */
    uint16_t size = ML_FLASH_BUFFER_SIZE_IN_WORDS;
    uint16_t *src = (uint16_t *)(addr & ~ML_FLASH_BUFFER_MASK); /* get page start address */
    uint16_t *dst = (uint16_t *)page_buffer;

    do {
        *dst++ = *src++;
    } while (--size != 0);
#endif /* _FAST */
}


/* ----------------------------------------------------------------------------
 * Writes the 'data' byte into NVRAM buffer with byte 'offset'
 */
__MLX_TEXT__  void Flash_PageBufferFill (uint16_t offset, uint8_t data)
{

    offset &= ML_FLASH_BUFFER_MASK;                         /* mask address bits and leave only the offset */

#if defined (HAS_FLASH_WRITE_BUFFER_IN_NVRAM_SRAM)
    NVRAM_BufferFill((uint16_t)&page_buffer[offset], data); /* use a helper function to write bytes into NVRAM buffer */
#else
    page_buffer[offset] = data;                             /* RAM allows byte access ==> can write directly          */
#endif
}


/* ----------------------------------------------------------------------------
 * Write internal RAM buffer to Flash Page specified by address
 *  \param[in]  addr    Start address of the Flash Page address to write to
 *
 *  \return             Operation status
 *      FLASH_ERR_NONE                  : no errors
 *      FLASH_ERR_VERIFICATION_FAILED   : error during page verification
 *
 * \note
 *  1. Writing to FL_CTRL0 port will not affect state of MLX4_RELOC bit since
 *     MLX4 is already started at the time when FlashWrite function is called.
 *
 *  2. Flash page for the writing/erasing is selected by the last flash address
 *     written in latch mode, i.e. when FL_CTRL0 = 0. Two consecutive words
 *     (32 bits) within the target page should be written to properly decode
 *     the page address by HW. Note, that if FL_CTRL0 = FL_ERASE (or FL_WRITE)
 *     writing to the flash address only triggers the erase or write operation,
 *     but not selects the flash page.
 */
__MLX_TEXT__ uint16_t Flash_PageWrite (uint16_t addr)
{
    uint16_t volatile *dst;

    addr = addr & ~ML_FLASH_BUFFER_MASK;    /* get start address of the page */

#if defined (SUPPORT_LINNETWORK_LOADER)
    if ( (LDR_GetState() == 1) && (addr == (((uint16_t)&loader_rst_state) & ~ML_FLASH_BUFFER_MASK)) )
    {
        /* Programming the LoaderB (LoaderState 1); This page is the LoaderState-page of LoaderB; Write LIN NAD in this page too. */
#if defined (HAS_FLASH_WRITE_BUFFER_IN_NVRAM_SRAM)               /* use NVRAM SRAM area as a page buffer */
        NVRAM_BufferFill((uint16_t)&page_buffer[0x76], LIN_nad); /* use a helper function to write bytes into NVRAM buffer */
#else                                                            /* RAM allows byte access ==> can write directly          */
        page_buffer[0x76] = LIN_nad;                             /* Write NAD into STACK_IT segment; This same address is used by LoaderB premain */
#endif /* HAS_FLASH_WRITE_BUFFER_IN_NVRAM_SRAM */

    }
#endif /* SUPPORT_LINNETWORK_LOADER */

    WDG_Manager();

    if ( !mlx_isPowerOk() ) {                               /* if power supply is not Ok .. */
        return FLASH_ERR_VERIFICATION_FAILED;
    }
    /* else: power is Ok */


    if (addr != ML_APP_CONTROL_PAGE_ADDRESS) {              /* if not the Application Control Page ..   */
        uint16_t sector_number = AddrToSector(addr);
        if ( ! IsSectorErased(sector_number) ) {            /* if Flash sector is not erased yet ..     */
            WDG_Manager();
            EraseSector(sector_number);                     /* .. erase it */
        }
        /* else: Flash sector was already erased */
    }
    /* else: erase should not be used for Application Control Page */

    /*
     * If power outage happens here during the re-programming of
     * the Reset Vector (page @ 0xBF68) the chip will be lost for further programming
     */

    WDG_Manager();

    /*
     * Load flash latches with data to be written
     */
    FL_CTRL0 = 0;                           /* set LATCH mode   */

#if _FAST
    uint16_t *src = (uint16_t *)page_buffer;
    dst = (uint16_t *)addr;

    do                                      /* load flash latches .. */
    {
        *dst++ = *src++;
    } while (src < (uint16_t *)(page_buffer + ML_FLASH_BUFFER_SIZE_IN_WORDS * sizeof(uint16_t)));
#else  /* _FAST */
    uint16_t size = ML_FLASH_BUFFER_SIZE_IN_WORDS;
    uint16_t *src = (uint16_t *)page_buffer;
    dst = (uint16_t *)addr;

    do {                                    /* load flash latches .. */
        *dst++ = *src++;
    } while (--size != 0);
#endif /* _FAST */


    /*
     * Write data from latches to flash page (CPU is frozen while flash is being erased)
     */
    FL_CTRL0 = FL_WRITE | FL_WRITE_TIME;    /* set WRITE mode and specify write timing */

    if (ml_driver_mode != kLinLoaderMode) { /* abort write operation (PLTF-732) */
        FL_CTRL0 = 0;
    }

    dst = (uint16_t *)addr;
    *dst = (uint16_t)dst;                   /* trigger write operation by writing to flash */

    FL_CTRL0 = 0;                           /* reset command register */

#if   (LDR_FLASH_WRITE_TEST == FLASH_TEST_NONE)

    return FLASH_ERR_NONE;

#elif (LDR_FLASH_WRITE_TEST == FLASH_TEST_NORMAL)

    WDG_Manager();

    uint16_t  status;
    status = Flash_PageVerifyBuffer(addr);
    return status;

#elif (LDR_FLASH_WRITE_TEST == FLASH_TEST_MARGIN)

    #define MIN_IREF             0
    #define MAX_IREF            63  /* 6-bits */

    #define LO_MARGIN_OFFSET    12  /* defined by PLTF-613 on Jira */
    #define HI_MARGIN_OFFSET    15  /* .. ditto */

    union {
        struct  __attribute__((packed, aligned(2))) {
            uint16_t        : 9;    /* LSBit first */
            uint16_t iref   : 6;
            uint16_t        : 1;
        };
        uint16_t u16;
    } io_flash_trim_a;

    uint16_t saved_io;

    WDG_Manager();

    /*
     * Change IREF in FLASHTRIMA register.
     * Note, that this is read-modify-write access to FLASHTRIMA
     */
    saved_io = FLASHTRIMA;
    io_flash_trim_a.u16 = saved_io;

    if (io_flash_trim_a.iref < (MIN_IREF + LO_MARGIN_OFFSET)) {
        io_flash_trim_a.iref = MIN_IREF;
    }
    else {
        io_flash_trim_a.iref -= LO_MARGIN_OFFSET;
    }

    FLASHTRIMA = io_flash_trim_a.u16;

    if (Flash_PageVerifyBuffer(addr) != FLASH_ERR_NONE) {   /* if verification failed .. */
        FLASHTRIMA = saved_io;                              /* restore initial IREF */
        return FLASH_ERR_VERIFICATION_FAILED; 
    }
    /* else: next test */


    
    io_flash_trim_a.u16 = saved_io;

    if (io_flash_trim_a.iref > (MAX_IREF - HI_MARGIN_OFFSET)) {
        io_flash_trim_a.iref = MAX_IREF;
    }
    else {
        io_flash_trim_a.iref += HI_MARGIN_OFFSET;
    }

    FLASHTRIMA = io_flash_trim_a.u16;

    
    if (Flash_PageVerifyBuffer(addr) != FLASH_ERR_NONE) {   /* if verification failed .. */
        FLASHTRIMA = saved_io;                              /* restore initial IREF */
        return FLASH_ERR_VERIFICATION_FAILED; 
    }
    /* else: next test */


    FLASHTRIMA = saved_io;                                 /* restore initial IREF */
    return FLASH_ERR_NONE;

#else
    #error "Incorrect setting for LDR_FLASH_WRITE_TEST define"

#endif /* LDR_FLASH_WRITE_TEST */
}


/* ----------------------------------------------------------------------------
 * Change Flash threshold for MarginRead verification procedure
 *  \param[in]    int16  offset_iref                         Offset to IREF
 *
 *  \return       uint16 (MSB-new_iref, LSB-old_iref)        IREF thresholds
 */
__MLX_TEXT__ uint16 Flash_IREF_Offset (int16_t offset_iref)
{
#define MIN_IREF             0
#define MAX_IREF            63  /* 6-bits */

    union {
        struct __attribute__((packed, aligned(2))) {
            uint16         :    9; /* LSBit first */
            uint16 iref    :    6;
            uint16         :    1;
        };
        uint16 u16;
    } io_flash_trim_a;

    /*
     * Change IREF in FLASHTRIMA register.
     * Note, that this is read-modify-write access to FLASHTRIMA
     */
    io_flash_trim_a.u16 = FLASHTRIMA;                /* save old io value */
    uint16_t saved_iref = io_flash_trim_a.iref;      /* save old iref value */

    /* IREF value saturation when overflow */
    int16 set_iref = saved_iref + offset_iref;
    if (set_iref > 0x3F) {
        set_iref = 0x3F;
    }
    else if (set_iref < 0) {
        set_iref = 0;
    }

    /* Initialise threshold by absolute value */
    io_flash_trim_a.iref = (set_iref);
    FLASHTRIMA = io_flash_trim_a.u16;            /* set new io_port value */

    /* return MSB-new_IREF; LSB-old_IREF */
    saved_iref |= (io_flash_trim_a.iref << 8);

    /* else - return only saved_iref value, no any changes with IREF */

    return saved_iref;
}


/* ----------------------------------------------------------------------------
 * Wrapper for Flash_PageWrite which checks the input addresses to eliminate
 * writing requests to certain Flash pages (depending on the loader state)
 */
__MLX_TEXT__ uint16_t Flash_PageWriteFiltered (uint16_t addr)
{
    addr = addr & ~ML_FLASH_BUFFER_MASK;        /* get start address of the page */

    if (addr == ML_APP_CONTROL_PAGE_ADDRESS) {  /* if writing of Application Control Page is requested .. */
        return FLASH_ERR_NONE;                  /* .. skip it */
    }
    /* else: continue with writing procedure  */

#if !defined (HAS_H12_LOADER_PROTOCOL)
    if ((addr == ML_MCU_FAR_PAGE_0_ADDRESS)     /* if writing of Far Page 0 is requested .. */
        && (LDR_GetState() == 3)) {             /* .. in State 3 of the loader ..           */
        return FLASH_ERR_NONE;                  /* .. then skip writing (this page was already written in State 2) */
    }
    /* else: continue with writing procedure */
#endif


    return Flash_PageWrite(addr);
}


#if (LDR_FLASH_WRITE_TEST != FLASH_TEST_NONE)
/* ----------------------------------------------------------------------------
 * Verifies Flash Page at 'addr' against RAM buffer
 */
__MLX_TEXT__  uint16_t Flash_PageVerifyBuffer (uint16_t addr)
{
#if _FAST
    uint16_t *dst = (uint16_t *)(addr & ~ML_FLASH_BUFFER_MASK);    /* get page start address */
    uint16_t *src = (uint16_t *)page_buffer;

    do
    {
        if ( *src++ != *dst++ )
        {
            return FLASH_ERR_VERIFICATION_FAILED;
        }
    } while (src < (uint16_t *)(page_buffer + ML_FLASH_BUFFER_SIZE_IN_WORDS * sizeof(uint16_t)));
#else  /* _FAST */
    uint16_t size = ML_FLASH_BUFFER_SIZE_IN_WORDS;
    uint16_t *src = (uint16_t *)page_buffer;
    uint16_t *dst = (uint16_t *)(addr & ~ML_FLASH_BUFFER_MASK); /* get page start address */


    do {
        if (*dst != *src) {
            return FLASH_ERR_VERIFICATION_FAILED;
        }
        /* else: compare Ok */

        dst++;
        src++;

    } while (--size != 0);
#endif /* _FAST */

    return FLASH_ERR_NONE;
}
#endif


/* ----------------------------------------------------------------------------
 * Returns sector number which corresponds to the input address
 *
 *  \note
 *  Translation formula:
 *      sector = (addr - flash_start_address) / flash_sector_size
 *      E.g. for MULAN2_H12_Flash: sector = (addr - 0x4000) / 2Kbytes
 */
__MLX_TEXT__ static uint16_t AddrToSector (uint16_t addr)
{
    //TODO: assert ((addr >= ML_FLASH_START_ADDRESS) && (addr < (ML_FLASH_START_ADDRESS + ML_FLASH_SIZE_IN_BYTES))

    return (addr - ML_FLASH_START_ADDRESS) / ML_FLASH_SECTOR_SIZE_IN_BYTES;
}


/* ----------------------------------------------------------------------------
 * Returns true if specified `sector` was already erased
 *
 * \param   sector  sector number to be checked (0-15)
 * \return          true if specified sector was already erased
 */
__MLX_TEXT__ static bool IsSectorErased (uint16_t sector)
{
    return erase_sectors_bitmap & (1u << sector);
}


/* ----------------------------------------------------------------------------
 * Erases specified sector
 */
__MLX_TEXT__ static void EraseSector(uint16_t sector)
{
    uint16_t volatile *dst;
    uint16_t addr = ML_FLASH_START_ADDRESS + (sector * ML_FLASH_SECTOR_SIZE_IN_BYTES);


    uint_fast8_t i;
    if (HasSectorEraseByHw()) {
        i = 1;  /* single iteration to erase sector */
    }
    else {
        i = ML_FLASH_SECTOR_SIZE_IN_PAGES;  /* emulate sector erase by erasing page-by-page */
    }

    do {
        /*
         * Select page for erasing:
         *  - set latch mode
         *  - write (any value) to the two consecutive words within the page
         */
        FL_CTRL0 = 0;                           /* set LATCH mode                           */
        dst = (uint16_t *)addr;                 /* select page by writing to the latches .. */
        *dst++ = 0;                             /* .. at least two words                    */
        *dst++ = 0;

#if defined (DEBUG_FLASH_ERASE_TIMING)
        IO_EXTIO = IO5_ENABLE;
        IO_EXTIO |= IO5_OUT;
#endif
        /*
         * Erase selected flash page (CPU is frozen while flash is being erased)
         */
        for (uint8_t ers = 2; ers > 0; ers--) {     /* erase selected page twice (PLTF-700)     */
            FL_CTRL0 = FL_ERASE | FL_ERASE_TIME;    /* set ERASE mode and specify erase timing  */
            if (ml_driver_mode != kLinLoaderMode) { /* abort illegal erase operation (PLTF-732) */
                FL_CTRL0 = 0;
            }
            *dst = 0;                               /* trigger ERASE operation by writing to flash  */
            /* .. here software is frozen until erase operation will be finished  */
        }

#if defined (DEBUG_FLASH_ERASE_TIMING)
        IO_EXTIO &= ~IO5_OUT;
#endif

        addr += ML_FLASH_PAGE_SIZE_IN_BYTES;    /* next page */
        i--;                                    /* next erase iteration */
    } while (i != 0);


    erase_sectors_bitmap |= (1u << sector);     /* flag the sector as ERASED */
}

/* EOF */
