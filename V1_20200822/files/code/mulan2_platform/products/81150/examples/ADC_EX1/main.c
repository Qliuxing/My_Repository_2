/*
 * Copyright (C) 2009 Melexis N.V.
 *
 * Software Platform
 *
 */


/*
 * ADC_EX1: Software triggered single cycle conversion without notification
 * \note:
 *  1. TODO: example 81150/ADC_EX1 is NOT tested on HW yet
 *  2. Conversion of single channel 
 *  3. Conversion of the group of 4 channels
 */
#include <ioports.h>
#include <syslib.h>
#include <adclib.h>
#include <plib.h>       /* product libs */

/*
 * Configuration 1: one sequence with one channel conversion
 */
#define N_CONVERSIONS_CFG1       1                          /* number of conversions inside the sequence */
const uint16 adc_Settings_cfg1[N_CONVERSIONS_CFG1 + 1] = {  /* +1 to accommodate ADC_CFG_END_MARKER */
    (ADC_CH0 | ADC_REF_2_50_V),
    ADC_CFG_END_MARKER
};

uint16 adc_Results_cfg1[N_CONVERSIONS_CFG1];                /* results buffer to be filled by DMA */


/*
 * Configuration 2: multiple channel conversion
 */
#define N_CONVERSIONS_CFG2       4                          /* number of conversions inside the sequence */
const uint16 adc_Settings_cfg2[N_CONVERSIONS_CFG2 + 1] = {  /* +1 to accommodate ADC_CFG_END_MARKER */
    (ADC_CH0 | ADC_REF_2_50_V),
    (ADC_CH1 | ADC_REF_2_50_V),
    (ADC_CH2 | ADC_REF_2_50_V),
    (ADC_CH3 | ADC_REF_2_50_V),
    ADC_CFG_END_MARKER
};

uint16 adc_Results_cfg2[N_CONVERSIONS_CFG2];                /* results buffer to be filled by DMA */


/*
 * Main
 */
int main (void)
{
    /*
     * Init ADC for one sequence containing one conversion
     */
    ADC_INIT(adc_Settings_cfg1,
            adc_Results_cfg1,
            ADC_SOFTWARE_TRIGGER,
            ADC_SINGLE_SEQUNCE_MODE);

    ADC_START_SEQUENCE();
    DELAY(500);                     /* some delay for sample and hold */

    ADC_TRIGGER_NEXT_CONVERSION();

    while (ADC_IS_BUSY()) {         /* wait until current channel conversion completed */
        WDG_Manager();
    }

    /*
     * Result is in adc_Results_cfg1[0];
     */


    /* --- Multiple channel conversion ------------------------------------- */
    ADC_INIT(adc_Settings_cfg2,
            adc_Results_cfg2,
            ADC_SOFTWARE_TRIGGER,
            ADC_SINGLE_SEQUNCE_MODE);

    ADC_START_SEQUENCE();
    DELAY(500);                                 /* some delay for sample and hold */
    
    while (ADC_IS_SEQUENCE_COMPLETED() == 0) { /* run one cycle for all specified
                                                * channels
                                                */

        ADC_TRIGGER_NEXT_CONVERSION();          /* trigger conversion of one channel */

        while (ADC_IS_BUSY()) {                 /* wait until current channel conversion completed */
            WDG_Manager();
        }
    }

    /*
     * Result is in adc_Results_cfg2[0] .. adc_Results_cfg2[3];
     */

    for (;;) {
        WDG_Manager();
    }

    return 0;
}  


/* EOF */
