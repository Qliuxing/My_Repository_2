/*
 * Copyright (C) 2008-2009 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <ioports.h>
#include <syslib.h>

/*
 * Calibration of 1MHz internal clock (depends on CPU clock)
 *
 *  CK_TRIM[5:0] => 0..63
 *  CK_TRIM = 64 - FPLL_MHz
 */
#define DEF_CK_TRIM  ((64000UL - FPLL + 500) / 1000)

#if (DEF_CK_TRIM > 63)
#error "Incorrect DEF_CK_TRIM value"
#endif


/*
 * Trimming map from file:
 * http://cvs.tess.elex.be/cgi-bin/cvsweb/~checkout~/design/80252/product/design_input/80250B_80150/calibration/calibration_process_80252_80152.doc?rev=1.4
 */
extern volatile uint16 EEP_ANA_OUTA __attribute__((nodp, addr(0x10BC)));  /* SREF_TRIM, RCO     */
extern volatile uint16 EEP_ANA_OUTB __attribute__((nodp, addr(0x10BA)));  /* V3V_SET, BGT, IREF */


/* ----------------------------------------------------------------------------
 * Function low_level_init() is invoked by start-up code before C runtime
 * initialization. Thus function cannot rely on uninitialized data being
 * cleared and cannot use any initialized data, because the .bss and .data
 * sections have not been initialized yet.
 *
 * NOTE:
 *  1. Function with the same name (i.e. low_level_init) linked from
 *     application directory overrides this function
 *  2. Before first EEPROM access delay might be needed (see issue MLX12123-22)
 */
void _low_level_init (void)
{
    NOP();      /* group of NOPs as a placeholder to patch the final application */
    NOP();
    NOP();
    NOP();

    NOP();
    NOP();
    NOP();
    NOP();

    NOP();
    NOP();
    NOP();
    NOP();

    NOP();
    NOP();
    NOP();
    NOP();

#if 0
    DELAY_US(250);      /* EEPROM can not be accessed during first 150-200us */

    /*
     * Trimming
     */
    CONTROL = OUTA_WE | OUTB_WE | OUTC_WE;  /* grant access to ANA_OUTx registers */
    ANA_OUTA = EEP_ANA_OUTA;
    ANA_OUTB = EEP_ANA_OUTB;
    CONTROL &= ~(OUTA_WE | OUTB_WE | OUTC_WE);
#endif

    /*
     * Calibrate 1MHz internal clock using CK_TRIM divider
     * This 1MHz clock is used by 15-bit core timer, watchdog and EEPROM
     */
    CK_TRIM = DEF_CK_TRIM;

} 
/* EOF */
