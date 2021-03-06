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
 *  CK_TRIM[7:0] => 0..255 (see issue MLX12126-19 on Jira)
 *  CK_TRIM = 256 - FPLL_MHz
 */
#define DEF_CK_TRIM  ((256000UL - FPLL + 500) / 1000)

/* Range validation */
#if (DEF_CK_TRIM > 255)
#error "Incorrect DEF_CK_TRIM value"
#endif


/*
 * Function low_level_init() is invoked by start-up code before C runtime
 * initialization. Thus function cannot rely on uninitialized data being
 * cleared and cannot use any initialized data, because the .bss and .data
 * sections have not been initialized yet.
 *
 * NOTE:
 *  1. Function with the same name (i.e. low_level_init) linked from
 *     application directory overrides this function
 *  2. Before first EEPROM access delay might be needed (see issue MLX12123-22)
 *  3. Instruction cycle is equal to 5 clocks by default for this product.
 *     Use WAIT_CYCLE_DISABLE to switch to 4 clocks cycle
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
    /*-------------------------------------------------------------------------
     * Trimming
     */

    CONTROL = OUTA_WE | OUTB_WE | OUTC_WE;  /* grant access to ANA_OUTx registers */


    /* Application specific configuration:
     * - trimming
     * - patching
     * 
     * ANA_OUTA = ...
     *  ...
     */

    CONTROL &= ~(OUTA_WE | OUTB_WE | OUTC_WE);
#endif /* 0 */

    /*
     * Calibrate 1MHz internal clock using CK_TRIM divider
     * This 1MHz clock is used by 15-bit core timer, watchdog and EEPROM
     */
    CK_TRIM = DEF_CK_TRIM;


    /*
     * The 2nd level interrupt controller pending flags and masks are undefined
     * after reset. Mask (disable) all 2nd level interrupts and reset any
     * pending flags (by writing 1s).
     */
    PWM_IO_IRQ_MASK     = 0;
    SPI_TIMERS_IRQ_MASK = 0;
    DIAGNOSTIC_MASK     = 0;
    
    PWM_IO_IRQ_STAT     = 0xFF;
    SPI_TIMERS_IRQ_STAT = 0x1FFF; /* bits [12:0] */
    DIAG_IRQ_STAT       = 0x01FF; /* bits [8:0]  */


} 
/* EOF */
