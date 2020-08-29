/**********************************************
* Copyright (C) 2014 Melexis N.V.
*
* LDF_Nodegenerator_Template_LIN2_x
*
**********************************************/
/* History:
     Revision 1.0
       - Initial release
 **********************************************/

/* ==========================================================================
 * Includes
 * ========================================================================== */
#include <alib.h>
#include <plib.h>

#include "adc.h"

volatile uint16 adcValue[3] = {0x0000,0x0000,0x0000};
/* ==========================================================================
 * Declaration private (static) functions
 * ========================================================================== */
void triggerADC(void);

/****************************************************//**
* Function name: initADC
********************************************************/
void initADC(void){
    ADC_SBASE = (uint16) & adcValue[0]; /* ADC_SBASE: base address for SIN and SREF storage */
    ADC_DBASE = (uint16) & adcValue[2]; /* ADC_DBASE: base address for conversion results */

    adcValue[1] = 0xFFFF;               /* set delimiter 0xFFFF (end of channel list)*/

    /* ADC should do nothing at the moment */
    ADC_CTRL = 0;

} /* initADC */

/****************************************************//**
* Function name: triggerADC
********************************************************/
void triggerADC(void){
    ADC_START_SEQUENCE();            /* start the ADC measurement sequence */
    DELAY_US(10);                    /* ADC settling time 10us */
    ADC_TRIGGER_NEXT_CONVERSION();   /* send soft trigger to start the conversion */
} /* triggerADC */

/****************************************************//**
* Function name: getADCVoltage
********************************************************/
uint16 getADCVoltage(uint16 ADCChannelConfig){

    /* set configuration for ADC measurement */
    adcValue[0] = ADCChannelConfig & ADC_SREF_CLEARTRIGGER;

    /* start a ADC conversion sequence */
    triggerADC();

    /* wait until the ADC conversion is done */
    while ((ADC_CTRL & ADC_START) != 0) {
        WDG_Manager();
    }

    /* return the measurement result */
    return (adcValue[2] & 0x03FF);

} /* getADCVoltage */
