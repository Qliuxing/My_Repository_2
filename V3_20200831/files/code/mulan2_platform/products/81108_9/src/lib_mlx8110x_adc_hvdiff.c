/*
 * Copyright (C) 2014-2015 Melexis GmbH
 *
 * This file is part of the Mlx8110x module library.
 *
 *
 * Module prefix: mdl_ADC_HVDIFF
 * All module interface functions start with this letters.
 *
 * This library provides functions for differential ADC measurements between
 * VS and HV pins.
 *
 *
 * ==========================================================================
 * History:
 *   Revision 1.0
 *     - Initial release
 *
 * ========================================================================== */

/* ==========================================================================
 * Includes
 * ========================================================================== */
#include <alib.h>                                    
#include <plib.h>

enum {
    VS_ADC,
    TEMPERATUR_ADC,
    HVDIFFOFFSET_ADC,
    HVDIFF_ADC
};

extern volatile uint16 adcResults[4];

/* ==========================================================================
 * Private type definitions, macros, defines
 * ========================================================================== */
#define DAC_10B         (0x03FFu) /* mask for DAC value */
#define RESET_HV_INEN   (0x00FFu) /* Reset the HVDIFFSELx and HVENFCMx bits of the HV_INEN register */

/** switch off the constant current source
 * @param void
 * @return void
 */
#define ADC_HVDIFF_RESET_CURRENT_OUTPUT()                              \
    do  {                                                              \
        ANA_OUTK = 0;                                                  \
        ANA_OUTH = (SH4 | SH3 | SH1 | CDEN | EN_LINAA);                \
    } while (0);

/** switch on the high precision constant current source
 * @param
 * @return void
 *
 */
#define ADC_HVDIFF_SET_CURRENT_OUTPUT(DAC_VALUE)                       \
    do  {                                                              \
        ANA_OUTH = (EN_LIN_AA_DAC | SH3 | SH2 | SH1 | EN_LINAA);       \
        ANA_OUTI |= SWI_DAC_OUT;                                       \
        ANA_OUTK = (DAC_VALUE & DAC_10B);                              \
        MLX8110x_USEC_DELAY(9); /*9us*/                                \
        ANA_OUTH = (EN_LIN_AA_DAC | SH2 | SH1 | EN_LINAA);             \
        MLX8110x_USEC_DELAY(1); /*1us*/                                \
        ANA_OUTH = (EN_LIN_AA_DAC | SH2 | EN_LINAA);                   \
        MLX8110x_USEC_DELAY(1); /*1us*/                                \
        ANA_OUTH = (EN_LIN_AA_DAC | EN_LINAA);                         \
        MLX8110x_USEC_DELAY(1); /*1us*/                                \
        ANA_OUTH = (EN_LIN_AA_DAC | SH3 | EN_LINAA);                   \
        MLX8110x_USEC_DELAY(1); /*1us*/                                \
        ANA_OUTH = (SH4 | SH3 | CDEN | EN_LINAA);                      \
        MLX8110x_USEC_DELAY(1); /*1us*/                                \
        ANA_OUTI &= ~SWI_DAC_OUT;                                      \
        ANA_OUTK = 0;                                                  \
    } while (0);

/* ADC measurement table for HVx differential measurement*/
uint16 const SBASE_ADC_HVDIFF[3] = {
    (ADC_SIN_HVIODIFF | ADC_HVDIFF_ADC_REF),                  /* Channel 20 : Difference (VS-HV[x])/5 */
    (ADC_SIN_HVIODIFF | ADC_HVDIFF_ADC_REF),                  /* Channel 20 : Difference (VS-HV[x])/5 */
    0xFFFF
};                                                            /* End-of-table */

/* ==========================================================================
 * Declaration public variables
 * ========================================================================== */

/* ==========================================================================
 * Declaration private (static) functions
 * ========================================================================== */

/* ==========================================================================
 * Implementation public functions
 * ========================================================================== */

/* ----------------------------------------------------------------------------
 * initialize and run a differential ADC measurements between VS and HV pins
 * depending on the fact if the precision current source should be used or
 * not the correct configuration value for the parameter HV_pin_config has to be used
 *
 * Parameters:
 *     HV_pin_config         configuration for the HV pin used for differential measurement
 *
 *                           precision current source (PCS) will be used:
 *                           ADC_HVDIFF_HV0_PCS => select HVDIFFSEL0 and HVENFCM0
 *                           ADC_HVDIFF_HV1_PCS => select HVDIFFSEL1 and HVENFCM1
 *                           ADC_HVDIFF_HV2_PCS => select HVDIFFSEL2 and HVENFCM2
 *                           ADC_HVDIFF_HV3_PCS => select HVDIFFSEL3 and HVENFCM3
 *
 *                           precision current source (PCS) will not be used:
 *                           ADC_HVDIFF_HV0 => select HVDIFFSEL0
 *                           ADC_HVDIFF_HV1 => select HVDIFFSEL1
 *                           ADC_HVDIFF_HV2 => select HVDIFFSEL2
 *                           ADC_HVDIFF_HV3 => select HVDIFFSEL3
 *
 *     HVDIFF_PCS_current    output current for precision current source
 *                           ADC_HVDIFF_CURRENT_100uA  => 100uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_200uA  => 200uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_300uA  => 300uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_400uA  => 400uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_500uA  => 500uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_600uA  => 600uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_700uA  => 70uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_800uA  => 800uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_900uA  => 900uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_1000uA  => 1000uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_1100uA  => 1100uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_1200uA  => 1200uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_1300uA  => 1300uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_1400uA  => 1400uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_1500uA  => 1500uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_1600uA  => 1600uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_1700uA  => 17000uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_1800uA  => 1800uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_1900uA  => 1900uA output for precision current source
 *                           ADC_HVDIFF_CURRENT_2000uA  => 2000uA output for precision current source
 *
 * Return:  (int16) 10bit differential voltage (VS - V_HVx) in digits
 *
 *               The resolution depends on the choosen reference voltage ADC_HVDIFF_ADC_REF
 *                  ADC_HVDIFF_ADC_REF = ADC_SREF_2V5  =>  2.5V * 5 /1023 = 12.21mV  (input range -6.25 ... 6.25V)
 *                  ADC_HVDIFF_ADC_REF = ADC_SREF_1V5  =>  1.5V * 5 /1023 =  7.33mV  (input range -1.25 ... 6.25V)
 *
 * Notes:
 *  1. This function is changing the ADC configuration. Because of this fact
 *     the ADC configuration of the application must be restored after leaving this function.
 */

int16 mdl_ADC_HVDIFF_Measurement(uint16 HV_pin_config, uint16 HVDIFF_PCS_current){

//    volatile int16 ADC_HVDIFF_adcValue[2] = {0,0};
//
//    /* ------------------ stop the current ADC action ------------------------- */
//    ADC_INT_DISABLE();                                     /* disable ADC interrupt */
//    ADC_CTRL &= ~(ADC_LOOP | ADC_TRIG_SRC | ADC_SYNC_SOC); /* set single cycle conversion and set trigger source to software trigger and  */
//
//    while ( (ADC_CTRL & ADC_START) == ADC_START ) {        /* In case ADC is active, wait to finish it */
//        ADC_CTRL |= ADC_SOFT_TRIG;                         /* start next conversion using the software trigger  */
//    }
//
//    /* ------------------ initialize the ADC for the HVDIFF sequence----------- */
//    ADC_SBASE = (uint16) & SBASE_ADC_HVDIFF;               /* set ADC input sources to ADC measurement table for HVDIFF */
//    ADC_DBASE = (uint16) & ADC_HVDIFF_adcValue;            /* set RAM memory array for ADC results DMA access*/
//    ADC_SELECT_ADCFREQ(ADC_SET_FREQ_2MHZ);                 /* <7us expected conversion time */
//    ADC_START_SEQUENCE();                                  /* start ADC sampling */
//
//    ANA_OUTI |= EN1V8V;                                    /* enable differential amplifier supply 1.8V for HVDIFF measurement */
//    MLX8110x_USEC_DELAY(5);                                /* 5us settling time */
//
//    HV_INEN &= RESET_HV_INEN;                              /* Reset the HVDIFFSELx and HVENFCMx bits of the HV_INEN register */

    /* ------ start differential amplifier reference (1.25V) measurement ------ */
    ANA_OUTI |= CP1;                                       /* set CP1 */
    MLX8110x_USEC_DELAY(10);                               /* 10us settling time */

    ADC_TRIGGER_NEXT_CONVERSION();                         /* Trigger first ADC conversion */
    MLX8110x_USEC_DELAY(10);                               /* Wait ADC conversion time 10us */

    ANA_OUTI &= ~CP1;                                      /* reset CP1 */

    MLX8110x_USEC_DELAY(1);                                /* 1us settling time */
    /* ------- end differential amplifier reference (1.25V) measurement ------- */

    /* ------------------ start differential ADC measurement ------------------ */
    ANA_OUTI |= CP2;                                       /* set CP2 */
    MLX8110x_USEC_DELAY(4);                                /* 4us settling time */

    HV_INEN |= HV_pin_config;                              /* select HVDIFFSELx and HVENFCMx according to the selected HV pin */
                                                           /* HVDIFFSELx => select ADC differential amplifier input pin */
                                                           /* HVENFCMx => select precision constant current source output pin */

//    if (HV_pin_config & 0x0F00) {                          /* Check if the precision constant current source should be used */
    ADC_HVDIFF_SET_CURRENT_OUTPUT(HVDIFF_PCS_current); /* set current for precision constant current source */
//    }

    MLX8110x_USEC_DELAY(10);                               /* 10us settling time */

    ADC_TRIGGER_NEXT_CONVERSION();                         /* Trigger second ADC conversion */
    MLX8110x_USEC_DELAY(10);                               /* Wait ADC conversion time 10us */

    ADC_HVDIFF_RESET_CURRENT_OUTPUT();                     /* reset high precision constant current source */

    HV_INEN &= RESET_HV_INEN;                              /* Reset the HVDIFFSELx and HVENFCMx bits of the HV_INEN register */

    ANA_OUTI &= ~CP2;                                      /* reset CP2 */

    /* ------------------ end differential ADC measurement ------------------ */

    /* return value is measured reference voltage - measured differential voltage */
    return (adcResults[HVDIFFOFFSET_ADC] -adcResults[HVDIFF_ADC]);
} /* mdl_ADC_HVDIFF_Measurement */

/* ==========================================================================
 * Implementation private functions
 * ========================================================================== */

/* EOF */
