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

#include <syslib.h>
#include <plib.h>
#include <ioports.h>
#include <adclib.h>

/*
 * Low threshold of power supply = 12V - 2% = 11.76V.
 *
 * ADC input parameters:
 *      - channel index:              0
 *      - voltage reference (V):     2.5
 *      - voltage divisor           1 : 14
 *      - data capacity (bit)        10
 */
#define POWER_LOW    344    /* = ( 11.76V /14 /2.5 ) * 1024 */

/* ----------------------------------------------------------------------------
 * To check power supply.
 */
__MLX_TEXT__  bool mlx_isPowerOk (void)
{
    /* ADC setting and result place in stack */
    volatile uint16 adc_setting[] = { (ADC_CH0 | ADC_REF_2_50_V), ADC_CFG_END_MARKER };
    volatile uint16 adc_result[]  = { 0 };

    ADC_INIT( adc_setting,
              adc_result,
              ADC_SOFTWARE_TRIGGER,
              ADC_SINGLE_SEQUNCE_MODE );

    ADC_START_SEQUENCE();
    DELAY(500);                     /* some delay for sample and hold */
    ADC_TRIGGER_NEXT_CONVERSION();

    while (ADC_IS_BUSY()) {         /* wait until current channel conversion completed */
        WDG_Manager();
    }

    if ( adc_result[0] <= POWER_LOW ) {
        return false;
    }
    else {
        return true;
    }
}

/* EOF */
