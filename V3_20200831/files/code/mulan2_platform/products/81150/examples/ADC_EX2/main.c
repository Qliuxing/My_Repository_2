/*
 * Copyright (C) 2009 Melexis N.V.
 *
 * Software Platform
 *
 */


/*
 * ADC_EX2: Hardware triggered continuous cycle conversion with notification
 * \note:
 *  1. Example 81150/ADC_EX2 is NOT tested on HW yet
 *  2. Within cycle next conversion is triggered by the timer
 *  3. Group of 4 channels is coverted during each cycle
 */
#include <ioports.h>
#include <syslib.h>
#include <adclib.h>
#include <timerlib.h>
#include <plib.h>       /* product libs */


/*
 * Configuration: multiple channel conversion
 */
#define N_CONVERSIONS    4
const uint16 adc_Settings[N_CONVERSIONS + 1] = { /* +1 for ADC_CFG_END_MARKER */

    (ADC_CH0 | ADC_HW_TRIGGER_TMR1_CMPB | ADC_REF_2_50_V),
    (ADC_CH1 | ADC_HW_TRIGGER_TMR1_CMPB | ADC_REF_2_50_V),
    (ADC_CH2 | ADC_HW_TRIGGER_TMR1_CMPB | ADC_REF_2_50_V),
    (ADC_CH3 | ADC_HW_TRIGGER_TMR1_CMPB | ADC_REF_2_50_V),

    ADC_CFG_END_MARKER
};

uint16 adc_Results[N_CONVERSIONS];


/*
 * Main
 */
int main (void)
{
    /* --- ADC init/start ------------------------------------------- */
    ADC_INIT(adc_Settings,
            adc_Results,
            ADC_HARDWARE_TRIGGER,
            ADC_CONTINUOUS_MODE);

    ADC_INT_ENABLE(3);
    ADC_START_SEQUENCE();

    /* --- Timer init/start ----------------------------------------- */
    TIMER1_AUTOLOAD_INIT(TIMER_DIV_256, 0.001 /* sec */ * FPLL * 1000 / 256 - 0.5);

    for (;;) {
        WDG_Manager();
    }

    return 0;
}  

/*
 * End of ADC cycle notification
 */
__interrupt__ void ADC_IT (void)
{
    /*
     * Data from adc_Results[] should be processed before
     * first conversion of a new cycle will store its result
     */
}

/* EOF */
