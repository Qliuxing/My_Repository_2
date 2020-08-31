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

#else

#error "Specified MLX4 FW image is not supported"

#endif

#endif /* HAS_MLX4_CODE */


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
#define DEF_FBDIV   (MCU_PLL_MULT - 1)


/*
 * Calibration of 1MHz internal clock (depends on CPU clock)
 *
 *  CK_TRIM[7:0] => 0..255 (see issue MLX12126-19 on Jira)
 *  CK_TRIM = 256 - FPLL_MHz
 */
#define DEF_CK_TRIM  ((256000UL - FPLL + 500) / 1000)

/* Range validation */
#if (DEF_CK_TRIM > 255)
#error "Incorrect DEF_CK_TRIM value"
#endif


/* Forward declaration */
static void init_PLL (void);
static void install_ram_functions (void);

void WriteTMTR (void);
void init2 (void);


/* External declarations */
extern uint16 stack;    /* Linker symbols (these objects are not created in the memory) */

extern void RAM_Test (void);
extern void _ram_section_init (void);
extern void _premain(void);
extern int  main(void);
extern void _fatal(void);


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
    NVRAM_LoadAll();   /* Load NVRAM before trimming (NVRAM will be used by trimming) (MMP110921-1) */

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

    CONTROL = OUTB_WE | OUTC_WE;                                                /* Grant access to ANA_OUTx registers */

    /* Set trim bits for ... */
    ANA_OUTB = EEP_ANA_OUTB_PT35;                                               /* Vdd, Bias, Bandgap */

#if 1
    ANA_OUTC = EEP_ANA_OUTC_PT35 | 0x8000;                                      /* PLL, Oscillator, Ena CK_TRIM */
#else
    ANA_OUTC = EEP_ANA_OUTC_PT35;                                               /* PLL, Oscillator */
#endif

    ANA_OUTD = EEP_ANA_OUTD_PT35;                                               /* ADC references 1 and 2 trimming */
    ANA_OUTE = (EEP_ANA_OUTE_PT35 & 0x00FF)                                     /* ADC references 3; only l ow 8 bits are relevant; ADC @ 2MHz */
                | 0xC000;                                                       /* ADC @ 4MHz */

    ANA_OUTF = EEP_ANA_OUTF_PT35;

    CONTROL &= ~(OUTB_WE | OUTC_WE);


#if (MCU_PLL_MULT <= 80)    /* if CPU clock <= 20 MHz */
    CONTROL_EXT = (CONTROL_EXT & ~RDY_OPTION) | RDY_OPT_FL;                 /* use flash-ready option */
#else
    CONTROL_EXT = (CONTROL_EXT & ~RDY_OPTION) | RDY_OPT_1;                  /* use 1 wait-state */
#endif

    if ( EEP_TM_TR_LSW | EEP_TM_TR_MSW )                                    /* Check if both Upper 16-bits and Lower 16-bits of TM_TR are NON-ZERO */
    {
        install_ram_functions();                                              /* .. install RAM functions (TM_TR can be updated only from RAM ..  */
        WriteTMTR();                                                          /* .. and init TM_TR register to trim the Flash; can start PLL now  */
    }
    /* else: skip TM_TR initialization */

    init_PLL();

    CONTROL = OUTA_WE;                                                          /* Grant access to ANA_OUTx registers */
    ANA_OUTA = EEP_ANA_OUTA;                                                    /* ANA_OUTA is not set */
    CONTROL &= ~OUTA_WE;
    ANA_OUTG = 0x0004;          /* EEP_ANA_OUTG; */
    
    /*
     * Calibrate 1MHz internal clock using CK_TRIM divider
     * This 1MHz clock is used by 15-bit core timer, watchdog and EEPROM
     */
    CK_TRIM = (64 - ((MCU_PLL_MULT+2)/4));
    ANA_TESTY |= 0x0400;                                                        /* CK_TRIM, depended on MCU_PLL_MULT */

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
    for (i = 0; i < NSAMPLES; i++) {
        if (PLL_STAT & PLL_LOCKED) {
            break;  /* PLL has successfully locked */
        }
        /* else : try locking again */
    }

    if (NSAMPLES == i) { /* if locking failed */
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
    }
}


/* ----------------------------------------------------------------------------
 *  Write TM_TR register
 */
__attribute__ ((noinline, section(".ramfunc")))
void WriteTMTR (void)
{
    uint16_t *pu16TM_TR = (uint16_t *) 0x6000;          /* 0x6000 : any word-aligned address inside the Flash region */
    *pu16TM_TR++ = EEP_TM_TR_LSW;                       /* Write TR_DIN[15:0] at TR_TM */
    uint16_t u16Ctrl = CONTROL_EXT;
    CONTROL_EXT = u16Ctrl | TM_TR;                      /* Set TM_TR */
    *pu16TM_TR = EEP_TM_TR_MSW;                         /* Write TR_DIN[31:16] at (TR_TM + 2) */
    CONTROL_EXT = u16Ctrl;                              /* Clear TM_TR */

}


/* EOF */
