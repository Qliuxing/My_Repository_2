/*
 * Copyright (C) 2008-2011 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <ioports.h>
#include <syslib.h>

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


/* ---------------------------------------------------------------------------
 * Function low_level_init() is invoked by start-up code before C runtime
 * initialization. Thus function cannot rely on uninitialized data being
 * cleared and cannot use any initialized data, because the .bss and .data
 * sections have not been initialized yet.
 *
 * NOTE: Function with the same name (i.e. low_level_init) linked from
 * application directory overrides this function
 */
void _low_level_init (void)
{
    /*
     * Enable/apply ROM patches if needed
     */

    CONTROL = OUTA_WE | OUTB_WE | OUTC_WE;  /* grant access to ANA_OUTx registers */

    /*
     * Trimming and initialization of analog part
     * NB! Before first EEPROM access delay might be needed (see issue MLX12123-22)
     *
     * ANA_OUTA = ...
     * ANA_OUTB = ...
     * ANA_OUTC = ...
     *  ...
     */

    CONTROL &= ~(OUTA_WE | OUTB_WE | OUTC_WE);

    /*
     * Calibrate 1MHz internal clock using CK_TRIM divider
     * This 1MHz clock is used by 15-bit core timer, watchdog and EEPROM
     */
    CK_TRIM = DEF_CK_TRIM;

} 
/* EOF */
