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

#include <ioports.h>
#include <syslib.h>
#include <nvram.h>
#include <mlx_eep_map.h>
#include "lib_mlx8110x.h"


#define DEF_FBDIV   (MCU_PLL_MULT - 1)

#if HAS_MLX4_CODE
/*-----------------------------------------------------------------------------
 * MLX16/MLX4 shared RAM configuration defines
 *
 *  \note
 *  1. Sizes of MLX4 private RAM and Shared RAM are imposed by MLX4 FW and
 *     shall be the same in MLX4, linker sctipt (regions shared_ram and 
 *     mlx4_ram) and SHRAMH / SHRAML constants.
 *
 * SHRAMH = Round Down( (RAM_size - RAM_private_mlx4) / 16) )
 * SHRAML = (RAM_size - (16*SHRAMH)) + RAM_shared
 */

#if defined(MLX4_FW_STANDALONE_LOADER)

/*
 * Shared RAM configuration defined for Standalone Loader
 *
 * RAM_size : 1024 bytes
 * RAM_shared:  16 bytes (0x10)
 * RAM_private_mlx4: 16 bytes (0x10)
 */
   #define DEF_SHRAMH   63  /* (1024 - 16) / 16 =  63 = 0x3F   */
   #define DEF_SHRAML   32  /* (1024 - 16*63) + 16 = 32 = 0x20 */

#elif defined (MLX4_FW_LIN2X) || defined(MLX4_FW_LIN13_9600) || defined(MLX4_FW_LIN13_19200)

/*
 * Shared RAM configuration defined LIN application
 *
 * RAM_size : 1024 bytes
 * RAM_shared:  16 bytes (0x10)
 * RAM_private_mlx4: 64 bytes (0x40)
 */
   #define DEF_SHRAMH   60  /* (1024 - 64) / 16 =  60 = 0x3C   */
   #define DEF_SHRAML   80  /* (1024 - 16*60) + 16 = 80 = 0x50 */

#else

#error "Specified MLX4 FW image is not supported"

#endif

#endif /* HAS_MLX4_CODE */

/*
 * Calibration of 1MHz internal clock (depends on CPU clock)
 *
 *  CK_TRIM[5:0] => 0..63
 *  CK_TRIM = 64 - FPLL_MHz
 */
#define DEF_CK_TRIM  ((64000UL - FPLL + 500) / 1000)

/* Range validation */
#if (DEF_CK_TRIM > 63)
#error "Incorrect DEF_CK_TRIM value"
#endif

extern volatile uint16 EEP_ANA_OUTI_PT50    __attribute__((nodp, addr(0x11CE)));  /* LIN pull-up, slew rate and VCM*/


/* Forward declaration */
static void init_RC (void);
static void install_ram_functions (void);
void init_TM_TR_register (void);


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
    NVRAM_LoadAll();   /* Load NVRAM before trimming (NVRAM will be used by trimming) */

    /*
     * Trimming
     *
     * Startup steps:
     *      1. Trimming of the bandgap
     *      2. Trimming of the VDDA supply voltage
     *      3. Trimming of the VDDD supply voltage
     *      4. Trimming of the Biasing
     *      5. Trimming of the RC Oscillator
     *      6. Trimming of Watchdog RC Oscillator
     *      7. Trimming of the ADC Reference voltages VRH1..3
     *      8. Configure PLL
     */

    CONTROL = OUTA_WE | OUTB_WE | OUTC_WE; /* enable access to ANA_OUTx registers */

    /* ANA_OUTA is not used on MLX8110x */
    ANA_OUTB = EEP_ANA_OUTB_PT50;                 /* Vdd, Bias, Bandgap   */

    ANA_OUTC = EEP_ANA_OUTC_PT50;                 /* PLL, Oscillator      */

    ANA_OUTD = EEP_ANA_OUTD_PT50;                 /* ADC references 1 and 2 trimming, RCO */

    ANA_OUTE = EEP_ANA_OUTE_PT50 & 0x00FF;        /* ADC references 3; only low 8 bits are relevant */

    ANA_OUTH = (SH4|SH3|SH1|CDEN|EN_LINAA);       /* Set a default current DAC value to activate 0mA */ 

    ANA_OUTC |= 0x8000;                           /* using internal MULAN 2 1MHz for FLASH operation and for MULAN 2 TIMER */
    ANA_OUTI = EEP_ANA_OUTI_PT50;                 /* setting LIN slope from NVRAM*/

    CONTROL &= ~(OUTA_WE | OUTB_WE | OUTC_WE);

    CONTROL_EXT = (CONTROL_EXT & ~RDY_OPTION) | (RDY_OPT_1|RDY_OPT_2);  /* 1 wait-state + Address is fetched only when a new access is done */

    if ( EEP_TM_TR_LSW | EEP_TM_TR_MSW )          /* Check if both Upper 16-bits and Lower 16-bits of TM_TR are NON-ZERO */
    {
        install_ram_functions();                  /* .. install RAM functions (TM_TR can be updated only from RAM ..  */
        init_TM_TR_register();                    /* .. and init TM_TR register to trim the Flash; can start PLL now  */
    }
    /* else: skip TM_TR initialization */

    init_RC();

    /*
     * Calibrate 1MHz internal clock using CK_TRIM divider
     * This 1MHz clock is used by 15-bit core timer, watchdog and EEPROM
     */
    CK_TRIM = DEF_CK_TRIM;


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


/* -----------------------------------------------------------------------
 * Initialize and start RC high frequency
 *
 */
__MLX_TEXT__ static void init_RC (void)
{
    RC_CTRL = (((uint16)DEF_FBDIV << 8) | RC_EN);

    NOP();
    NOP();
    NOP();
}


/* ----------------------------------------------------------------------------
 * Copy RAM functions from Flash to RAM
 */
__MLX_TEXT__ static void install_ram_functions (void)
{
    extern uint16 _ramfunc_load_start;
    extern uint16 _ramfunc_start;
    extern uint16 _ramfunc_end;

    uint16 *w;
    uint16 *r;

    r = &_ramfunc_load_start;

    for (w = &_ramfunc_start; w < &_ramfunc_end; ) {
        *w++ = *r++;
    }
}


/* ----------------------------------------------------------------------------
 *  Write TM_TR register
 */
__attribute__ ((noinline, section(".ramfunc")))
void init_TM_TR_register (void)
{
    uint16_t volatile *p = (uint16_t volatile *) 0x6000;    /* 0x6000 : any word-aligned address inside the Flash region */
    *p++ = EEP_TM_TR_LSW;                                   /* Write TR_DIN[15:0] at TR_TM */
    uint16_t ctrl = CONTROL_EXT;
    CONTROL_EXT = ctrl | TM_TR;                             /* Set TM_TR */
    *p = EEP_TM_TR_MSW;                                     /* Write TR_DIN[31:16] at (TR_TM + 2) */
    CONTROL_EXT = ctrl;                                     /* Clear TM_TR */

}

/* EOF */
