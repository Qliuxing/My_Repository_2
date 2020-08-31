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
#include <plib.h>           /* Product Specific Libraries */

#if (LIN_PIN_LOADER != 0)
#include <flashupload.h>  /* LDR_GetState() */
#endif


/*-----------------------------------------------------------------------------
 * PLL configuration defines
 *
 *
 * Fck = Fosc * (FBDIV + 1)
 *
 * Fck : system clock
 * Fosc : system oscillator (typically 250 kHz)
 * The PLL will operate in a output frequency range of nominal ~12-32MHz
 */
#define DEF_FBDIV       (MCU_PLL_MULT - 1)


/*
 * Calibration of 1MHz internal clock (depends on CPU clock)
 *
 *  CK_TRIM[5:0] => 0..63
 *  CK_TRIM = 64 - FPLL_MHz
 */
#define DEF_CK_TRIM     (64 - ((MCU_PLL_MULT+2)/4))

/* Range validation */
#if (DEF_CK_TRIM > 63)
#error "Incorrect DEF_CK_TRIM value"
#endif


/* Forward declaration */
static void init_PLL(void);
static void install_ram_functions(void);
void WriteTMTR(void);

/* External declarations */
extern uint16 stack;    /* Linker symbols (these objects are not created in the memory) */

extern void RAM_Test(void);
extern void _fatal(void);

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
__MLX_TEXT__ void _prestart(void)
{
    SYS_clearCpuStatus();   /* Initialize M register.
                             * Note that UPR register (== M[11:8]) was already
                             * initialized during execution of the reset vector
                             * (see JMPVECTOR macro)
                             */

    PRIO = 0xFFFF;          /* set lowest priorties (undefined after reset) */

    SET_STACK(&stack);      /* Init stack */

    NVRAM_LoadAll();        /* Load NVRAM before trimming (NVRAM will be used by trimming) (MMP110921-1) */

#ifdef HAS_PATCH_SUPPORT
    patch_Load();
#endif /* HAS_PATCH_SUPPORT */

    bist_CheckReset();      /* Check the reason of the reset and store in non initialized memory */

#ifdef HAS_NVRAM_CRC
    /* Incase of Watchdog-reset or LIN-command reset, skip CRC check as it's already performed at cold-start */
    if (
          ( bistResetInfo != C_CHIP_STATE_LIN_CMD_RESET )
#ifdef LDR_RESET_ON_ENTER_PROG_MODE
        && (bistResetInfo != C_CHIP_STATE_LOADER_PROG_RESET)
#endif /* LDR_RESET_ON_ENTER_PROG_MODE */
#ifdef HAS_WD_RST_FAST_RECOVERY
        && (bistResetInfo != C_CHIP_STATE_WATCHDOG_RESET)
#endif /* HAS_WD_RST_FAST_RECOVERY */
       )
    {
        /* First check the CRC on the Melexis NVRAM block which has the critical power-up parameters
         * Check Melexis Trimming and Calibration content (Melexis Production area - CRC1)
         * This check takes approx 10.8 ms (MLX16 runs at 250kHz RC-clock) */
        uint16 u16CRC;

        /* Melexis Production area - CRC1 */
        u16CRC = nvram_CalcCRC( (uint16*) BGN_MLX_CALIB_ADDRESS_AREA1,
                                ((END_MLX_CALIB_ADDRESS_AREA1 + 1) - BGN_MLX_CALIB_ADDRESS_AREA1)/2 );
        if ( u16CRC != 0xFF )
        {
#ifdef HAS_NVRAM_CRC_FAIL_HANG
            /* Area #1 is corrupt */
            asm( "mov yl, #0xC8");              /* C_ERR_INV_MLXPAGE_CRC1 */    /* MLX NVRAM CRC #1 failure */
            asm( "jmpf __fatal");
#endif /* HAS_NVRAM_CRC_FAIL_HANG */
        }
    }
#endif /* HAS_NVRAM_CRC */

    /* Trimming
     * Checksum of trimming value's is correct. Configure the chip to allow PLL to be active ASAP.
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
    CONTROL = OUTB_WE | OUTC_WE;                                            /* Grant access to ANA_OUTx registers */
    ANA_OUTB = EEP_ANA_OUTB_PT35;                                           /* Vdd, Bias, Bandgap */
    ANA_OUTC = EEP_ANA_OUTC_PT35 | 0x8000;                                  /* PLL, Oscillator, Ena CK_TRIM */
    ANA_OUTD = EEP_ANA_OUTD_PT35;                                           /* ADC references 1 and 2 trimming */
    ANA_OUTE = (EEP_ANA_OUTE_PT35 & 0x00FF) | 0xC000;                       /* ADC references 3; only low 8 bits are relevant; ADC @ 4MHz */
    CONTROL &= ~(OUTB_WE | OUTC_WE);

#if (MCU_PLL_MULT <= 80)    /* if CPU clock <= 20 MHz */
    CONTROL_EXT = (CONTROL_EXT & ~RDY_OPTION) | RDY_OPT_FL;                 /* use flash-ready option */
#else
    CONTROL_EXT = (CONTROL_EXT & ~RDY_OPTION) | RDY_OPT_1;                  /* use 1 wait-state */
#endif

    if ( (FL_CTRL0 & FL_DETECT) != 0 )
    {
        /* This is a flash chip */
        if ((EEP_TM_TR_LSW | EEP_TM_TR_MSW) != 0)                           /* Check if both Upper 16-bits and Lower 16-bits of TM_TR are NON-ZERO */
        {
            install_ram_functions();                                        /* .. install RAM functions (TM_TR can be updated only from RAM ..  */
            WriteTMTR();                                                    /* .. and init TM_TR register to trim the Flash; can start PLL now  */
        }
        /* else: skip TM_TR initialization */
    }
    else
    {
        /* No TMTR for ROM chips */
    }

    init_PLL();

    /*
     * Calibrate 1MHz internal clock using CK_TRIM divider
     * This 1MHz clock is used by 15-bit core timer, watchdog and EEPROM
     */
    CK_TRIM = DEF_CK_TRIM;

#ifdef HAS_RAM_TEST
    if (       (bistResetInfo == C_CHIP_STATE_LIN_CMD_RESET)
    #if (LIN_PIN_LOADER != 0)
            || ( LDR_GetState() != 0 )
    #endif /* (LIN_PIN_LOADER != 0) */
    #ifdef LDR_RESET_ON_ENTER_PROG_MODE
            || (bistResetInfo == C_CHIP_STATE_LOADER_PROG_RESET)
    #endif /* LDR_RESET_ON_ENTER_PROG_MODE */
    #ifdef HAS_WD_RST_FAST_RECOVERY
            || (bistResetInfo == C_CHIP_STATE_WATCHDOG_RESET)
    #endif /* HAS_WD_RST_FAST_RECOVERY */
       )
    {
        /* Else: skip RAM test during Flash reprogramming (loader state != 0)
         * otherwise stFixedRamNAD.nad stored in no-init RAM will be corrupted
         */
        __asm__ __volatile__ ("jmp _start");
    }
    else
    {
        /* We're about to start the application .. */
        /* .. execute the RAM test first */
        __asm__ __volatile__ ("jmp _RAM_Test");
    }
#else /* HAS_RAM_TEST */
    __asm__ __volatile__ ("jmp _start");
#endif /* HAS_RAM_TEST */
}

/* -----------------------------------------------------------------------
 * Initialize and start PLL
 *
 * 82050 spec, Chapter 9.3.4 Oscillator concept
 * The RC oscillator clock is referred further as SLOWCLK, the PLL clock as FASTCLK
 * After power on or watchdog reset the chip always starts at the SLOWCLK as system clock
 *
 * The following sequence is recommended to run the system from PLL clock:
 * - transfer RC trimming
 * - transfer PLL trimming
 * - define the PLL output frequency by writing FBDIV and start the PLL (set bit PLLEN)
 * - hardware will switch system to PLL clock after PLL has locked, clock monitor will be started
 * - the system now runs at FASTCLK
 */
#define NSAMPLES  100   /* number of sampled of LOCKED status */

__MLX_TEXT__ static void init_PLL (void)
{
    uint_fast8_t i;

    PLL_CTRL = ((uint16)DEF_FBDIV << 8)
                | (0x0D << 4)           /* most relaxed settings */
                | PLL_EN;

    /* The hardware itself takes care that the PLL has locked and therefore
     * reached its final stable frequency before enabling its use.
     */
    for ( i = 0; i < NSAMPLES; i++ )
    {
        if ( PLL_STAT & PLL_LOCKED )
        {
            break;  /* PLL has successfully locked */
        }
        /* else : try locking again */
    }

    if ( NSAMPLES == i )
    {
        /* Locking failed */
        /* TBD */
        for (;;); /* loop until WD reset for the time being */
    }
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
        *w++ = *r++;
        *w++ = *r++;
        *w++ = *r++;
    }
} /* End of install_ram_functions() */

/* ----------------------------------------------------------------------------
 *  Write TM_TR register
 */
__attribute__ ((noinline, section(".ramfunc")))
void WriteTMTR (void)
{
    __asm__ __volatile__
    (
        "lod   X, #0x6000   \n\t"           /* X = pu16TM_TR */
        "lod   Y, #0x11B4   \n\t"           /* Y = EEP_TM_TR_LSW */
        "movsw [X++], [Y++] \n\t"           /* TR_DIN[15:0] = EEP_TM_TR_LSW */
        "lod   A, 0x2054    \n\t"           /* A = CONTROL_EXT */
        "or    A, #0x0010   \n\t"
        "mov   0x2054, A    \n\t"           /* CONTROL_EXT = A | TM_TR */
        "movsw [X++], [Y++] \n\t"           /* TR_DIN[31:16] = EEP_TM_TR_MSW */
        "and   A, #0xFFEF   \n\t"
        "mov   0x2054, A    \n\t"           /* CONTROL_EXT = A */
    );
} /* End of WriteTMTR() */

/* EOF */
