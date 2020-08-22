/*
 * Copyright (C) 2008-2015 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 *  \note:
 *      1.  All functions in this module shall be defined with __MLX_TEXT__
 *          attribute. Thus they will be linked in first half of the Flash.
 *
 *      2.  This module shall NOT use _initialized_ global and static variables!
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
#include <nvram.h>
#include <plib.h>

#if HAS_MLX4_CODE
/*-----------------------------------------------------------------------------
 * MLX16/MLX4 shared RAM configuration defines
 *
 *  \note
 *  1. Sizes of MLX4 private RAM and Shared RAM are imposed by MLX4 FW and
 *     shall be the same in MLX4, linker script (regions shared_ram and
 *     mlx4_ram) and SHRAMH / SHRAML constants.
 *
 * SHRAMH = Round Down( (RAM_size - RAM_private_mlx4) / 16) )
 * SHRAML = (RAM_size - (16*SHRAMH)) + RAM_shared
 */

#if defined(MLX4_FW_STANDALONE_LOADER)

/*
 * Shared RAM configuration defined for Stand-Alone Loader
 *
 * RAM_size : 2048 bytes
 * RAM_shared: 16 bytes (0x10)
 * RAM_private_mlx4: 16 bytes (0x10)
 */

#define DEF_SHRAMH  127  /* (2048 - 16) / 16 =  127 = 0x7F   */
#define DEF_SHRAML   32  /* (2048 - 16*127) + 16 = 32 = 0x20 */

#elif defined (MLX4_FW_LIN2X) || defined(MLX4_FW_LIN13_9600) || defined(MLX4_FW_LIN13_19200)
/*
 * Shared RAM configuration defined LIN application
 *
 * RAM_size : 2048 bytes
 * RAM_shared: 16 bytes (0x10)
 * RAM_private_mlx4: 64 bytes (0x40)
 */
#define DEF_SHRAMH  124  /* (2048 - 64) / 16 =  124 = 0x7C   */
#define DEF_SHRAML   80  /* (2048 - 16*124) + 16 = 80 = 0x50 */


#elif defined (MLX4_FW_LOOP)

/*
 * Shared RAM and MLX4 private memory are not needed for Endless Loop application
 */

#else

#error "Specified MLX4 FW image is not supported"

#endif

#endif /* HAS_MLX4_CODE */


#ifdef HAS_NVRAM_CRC_FAIL_HANG
/* External declarations */
extern void _fatal(void);
#endif /* HAS_NVRAM_CRC_FAIL_HANG */


#ifndef HAS_NVRAM_CRC
#warning "NVRAM CRC checks are disabled"
#endif


/*-----------------------------------------------------------------------------
 * Function low_level_init() is invoked by start-up code before C runtime
 * initialization. Thus function cannot rely on uninitialized data being
 * cleared and cannot use any initialized data, because the .bss and .data
 * sections have not been initialized yet.
 *
 * NOTE: Function with the same name (i.e. low_level_init) linked from
 * application directory overrides this function
 */
__MLX_TEXT__ void _low_level_init (void)
{
    /* The MLX16 is running at full-speed as PLL is started in prestart.c, as specified by MCU_PLL_MULT */

    if ( bistHeader != C_CHIP_HEADER )
    {
        /* update the registers because of RAM test and NOLOAD */
        bistHeader = C_CHIP_HEADER;
        bistResetInfo = C_CHIP_STATE_COLD_START;
    }

#ifdef HAS_NVRAM_CRC

    if (   (bistResetInfo != C_CHIP_STATE_LIN_CMD_RESET)
#ifdef LDR_RESET_ON_ENTER_PROG_MODE
        && (bistResetInfo != C_CHIP_STATE_LOADER_PROG_RESET)
#endif /* LDR_RESET_ON_ENTER_PROG_MODE */
#ifdef HAS_WD_RST_FAST_RECOVERY
        && (bistResetInfo != C_CHIP_STATE_WATCHDOG_RESET)
#endif /* HAS_WD_RST_FAST_RECOVERY */
       )
    {
        uint16 u16CRC;

        /* MLX Chip calibration data - CRC2 */
        u16CRC = nvram_CalcCRC( (uint16*) BGN_MLX_CALIB_ADDRESS_AREA2,
                                ((END_MLX_CALIB_ADDRESS_AREA2 + 1) - BGN_MLX_CALIB_ADDRESS_AREA2)/2 );
        if ( u16CRC != 0xFF )
        {
    #ifdef HAS_NVRAM_CRC_FAIL_HANG
            /* Area #2 is corrupt */
            asm( "mov yl, #0xC9");                  /* C_ERR_INV_MLXPAGE_CRC2 */    /* MLX NVRAM CRC #2 failure */
            asm( "jmpf __fatal");
    #endif /* HAS_NVRAM_CRC_FAIL_HANG */
        }

        /* Other chip values - CRC3 */
        u16CRC = nvram_CalcCRC( (uint16*) BGN_MLX_CALIB_ADDRESS_AREA3,
                                ((END_MLX_CALIB_ADDRESS_AREA3 + 1) - BGN_MLX_CALIB_ADDRESS_AREA3)/2 );
        if ( u16CRC != 0xFF )
        {
    #ifdef HAS_NVRAM_CRC_FAIL_HANG
            /* Area #3 is corrupt */
            asm( "mov yl, #0xCA");                  /* C_ERR_INV_MLXPAGE_CRC3 */    /* MLX NVRAM CRC #3 failure */
            asm( "jmpf __fatal");
    #endif /* HAS_NVRAM_CRC_FAIL_HANG */
        }

        /* Flash/NVRAM#2 Trimming - CRC4 */
        u16CRC = nvram_CalcCRC( (uint16*) BGN_MLX_CALIB_ADDRESS_AREA4,
                                ((END_MLX_CALIB_ADDRESS_AREA4 + 1) - BGN_MLX_CALIB_ADDRESS_AREA4)/2 );
        if ( u16CRC != 0xFF )
        {
    #ifdef HAS_NVRAM_CRC_FAIL_HANG
            /* Area #4 is corrupt */
            asm( "mov yl, #0xCB");                  /* C_ERR_INV_MLXPAGE_CRC4 */    /* MLX NVRAM CRC #4 failure */
            asm( "jmpf __fatal");
    #endif /* HAS_NVRAM_CRC_FAIL_HANG */
        }

        /* Flash/NVRAM#1 Trimming - CRC5 */
        u16CRC = nvram_CalcCRC( (uint16*) BGN_MLX_CALIB_ADDRESS_AREA5,
                                ((END_MLX_CALIB_ADDRESS_AREA5 + 1) - BGN_MLX_CALIB_ADDRESS_AREA5)/2 );
        if (( u16CRC != 0xFF ) ||                                                   /* Check if checksum is corrupted */
            (*((uint16*) (BGN_MLX_CALIB_ADDRESS_AREA5 + 2)) != *((uint16*) (BGN_MLX_CALIB_ADDRESS_AREA4 + 2))) )   /* Trim NVRAM 1 check */
        {
            /* Area #5 is corrupt; Copy from Melexis area and calc CRC */
            *((uint16*)  BGN_MLX_CALIB_ADDRESS_AREA5) = 0;
            *((uint16*) (BGN_MLX_CALIB_ADDRESS_AREA5 + 2)) = *((uint16*) (BGN_MLX_CALIB_ADDRESS_AREA4+2));
            u16CRC = nvram_CalcCRC( (uint16*) BGN_MLX_CALIB_ADDRESS_AREA5,
                                    ((END_MLX_CALIB_ADDRESS_AREA5 + 1) - BGN_MLX_CALIB_ADDRESS_AREA5)/2 );
            *((uint16*) BGN_MLX_CALIB_ADDRESS_AREA5) = (0xFF - u16CRC);
            NVRAM_SavePage(NVRAM1_PAGE2);
            asm( "mov yl, #0xCC");                  /* C_ERR_INV_MLXPAGE_CRC5 */    /* MLX NVRAM CRC #5 failure */
            asm( "jmpf __fatal");
        }
    }
#endif /* HAS_NVRAM_CRC */

    CONTROL |= OUTA_WE;                                                         /* Grant access to ANA_OUTx registers */
    ANA_OUTA = EEP_ANA_OUTA & ~TEST_MODE_DIS;                                   /* ANA_OUTA is not set */
    CONTROL &= ~OUTA_WE;
    ANA_OUTG = 0x0004U;                                                         /* Motor-driver FET switching slew-rate at 100% */

#if HAS_MLX4_CODE

#if defined(DEF_SHRAMH) && defined(DEF_SHRAML)
    /* ------------------------------------------------------------------------
     * Initialize shared memory and start MLX4 processor
     * NOTE: Port SHRAM can only be changed when Mlx4 is in reset
     */
    SHRAM = ((uint16)DEF_SHRAMH << 8) | DEF_SHRAML;
#endif

    MLX4_RESET();
    NOP();
    NOP();
    NOP();
    MLX4_START();
#endif /* HAS_MLX4_CODE */
}

/* EOF */
