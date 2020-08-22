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

#ifndef MDL_ADC_HVDIFF_H_
#define MDL_ADC_HVDIFF_H_
/* ==========================================================================
 * Public defines
 * ========================================================================== */

#define ADC_HVDIFF_ADC_REF   ADC_SREF_1V5                             /* Select ADC Reference voltage used during the HVDIFF measurement */
                                                                      /* ADC_SREF_1V5 => 1.5V ADC reference voltage */
                                                                      /* ADC_SREF_2V5 => 2.5V ADC reference voltage */

/* mdl_ADC_HVDIFF_Measurement() parameter  HV_pin_config values for using the precision current source (PCS) */
#define ADC_HVDIFF_HV0_PCS   0x1100                                   /* select HVDIFFSEL0 and HVENFCM0 for mdl_ADC_HVDIFF_Measurement() parameter HV_pin_config */
#define ADC_HVDIFF_HV1_PCS   0x2200                                   /* select HVDIFFSEL1 and HVENFCM1 for mdl_ADC_HVDIFF_Measurement() parameter HV_pin_config */
#define ADC_HVDIFF_HV2_PCS   0x4400                                   /* select HVDIFFSEL2 and HVENFCM2 for mdl_ADC_HVDIFF_Measurement() parameter HV_pin_config */
#define ADC_HVDIFF_HV3_PCS   0x8800                                   /* select HVDIFFSEL3 and HVENFCM3 for mdl_ADC_HVDIFF_Measurement() parameter HV_pin_config */

/* mdl_ADC_HVDIFF_Measurement() parameter  HV_pin_config values without using the precision current source (PCS) */
#define ADC_HVDIFF_HV0   0x1000                                       /* select HVDIFFSEL0 for mdl_ADC_HVDIFF_Measurement() parameter HV_pin_config */
#define ADC_HVDIFF_HV1   0x2000                                       /* select HVDIFFSEL1 for mdl_ADC_HVDIFF_Measurement() parameter HV_pin_config */
#define ADC_HVDIFF_HV2   0x4000                                       /* select HVDIFFSEL2 for mdl_ADC_HVDIFF_Measurement() parameter HV_pin_config */
#define ADC_HVDIFF_HV3   0x8000                                       /* select HVDIFFSEL3 for mdl_ADC_HVDIFF_Measurement() parameter HV_pin_config */

#if (ADC_HVDIFF_ADC_REF == ADC_SREF_2V5)
#define ADC_HVDIFF_CURRENT_100uA    (EEP_CURRDAC_4mA_35 * 1 / 20)     /* 100uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_200uA    (EEP_CURRDAC_4mA_35 * 2 / 20)     /* 200uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_300uA    (EEP_CURRDAC_4mA_35 * 3 / 20)     /* 300uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_400uA    (EEP_CURRDAC_4mA_35 * 4 / 20)     /* 400uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_500uA    (EEP_CURRDAC_4mA_35 * 5 / 20)     /* 500uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_600uA    (EEP_CURRDAC_4mA_35 * 6 / 20)     /* 600uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_700uA    (EEP_CURRDAC_4mA_35 * 7 / 20)     /* 700uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_800uA    (EEP_CURRDAC_4mA_35 * 8 / 20)     /* 800uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_900uA    (EEP_CURRDAC_4mA_35 * 9 / 20)     /* 900uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_1000uA   (EEP_CURRDAC_4mA_35 * 10 / 20)    /* 1000uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_1100uA   (EEP_CURRDAC_4mA_35 * 11 / 20)    /* 1100uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_1200uA   (EEP_CURRDAC_4mA_35 * 12 / 20)    /* 1200uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_1300uA   (EEP_CURRDAC_4mA_35 * 13 / 20)    /* 1300uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_1400uA   (EEP_CURRDAC_4mA_35 * 14 / 20)    /* 1400uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_1500uA   (EEP_CURRDAC_4mA_35 * 15 / 20)    /* 1500uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_1600uA   (EEP_CURRDAC_4mA_35 * 16 / 20)    /* 1600uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_1700uA   (EEP_CURRDAC_4mA_35 * 17 / 20)    /* 1700uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_1800uA   (EEP_CURRDAC_4mA_35 * 18 / 20)    /* 1800uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_1900uA   (EEP_CURRDAC_4mA_35 * 19 / 20)    /* 1900uA output current in case ADC ref. 2.5V was selected  */
#define ADC_HVDIFF_CURRENT_2000uA   (EEP_CURRDAC_4mA_35 * 20 / 20)    /* 2000uA output current in case ADC ref. 2.5V was selected  */

#elif (ADC_HVDIFF_ADC_REF == ADC_SREF_1V5)
#define ADC_HVDIFF_CURRENT_100uA    (EEP_CURRDAC_4mA_35 * 1 / 12)     /* 100uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_200uA    (EEP_CURRDAC_4mA_35 * 2 / 12)     /* 200uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_300uA    (EEP_CURRDAC_4mA_35 * 3 / 12)     /* 300uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_400uA    (EEP_CURRDAC_4mA_35 * 4 / 12)     /* 400uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_500uA    (EEP_CURRDAC_4mA_35 * 5 / 12)     /* 500uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_600uA    (EEP_CURRDAC_4mA_35 * 6 / 12)     /* 600uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_700uA    (EEP_CURRDAC_4mA_35 * 7 / 12)     /* 700uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_800uA    (EEP_CURRDAC_4mA_35 * 8 / 12)     /* 800uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_900uA    (EEP_CURRDAC_4mA_35 * 9 / 12)     /* 900uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_1000uA   (EEP_CURRDAC_4mA_35 * 10 / 12)    /* 1000uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_1100uA   (EEP_CURRDAC_4mA_35 * 11 / 12)    /* 1100uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_1200uA   (EEP_CURRDAC_4mA_35 * 12 / 12)    /* 1200uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_1300uA   (EEP_CURRDAC_4mA_35 * 13 / 12)    /* 1300uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_1400uA   (EEP_CURRDAC_4mA_35 * 14 / 12)    /* 1400uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_1500uA   (EEP_CURRDAC_4mA_35 * 15 / 12)    /* 1500uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_1600uA   (EEP_CURRDAC_4mA_35 * 16 / 12)    /* 1600uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_1700uA   (EEP_CURRDAC_4mA_35 * 17 / 12)    /* 1700uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_1800uA   (EEP_CURRDAC_4mA_35 * 18 / 12)    /* 1800uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_1900uA   (EEP_CURRDAC_4mA_35 * 19 / 12)    /* 1900uA output current in case ADC ref. 1.5V was selected  */
#define ADC_HVDIFF_CURRENT_2000uA   (EEP_CURRDAC_4mA_35 * 20 / 12)    /* 2000uA output current in case ADC ref. 1.5V was selected  */

#else /* if (ADC_HVDIFF_ADC_REF == ADC_SREF_2V5) */
#error "Selected ADC reference voltage is not supported"
#endif /* ADC_HVDIFF_ADC_REF  */

/* ==========================================================================
 * Public variables
 * ========================================================================== */

/* ==========================================================================
 * Public functions
 * ========================================================================== */
extern int16 mdl_ADC_HVDIFF_Measurement(uint16 HV_pin_config, uint16 HVDIFF_PCS_current);

#endif /* MDL_ADC_HVDIFF_H_ */
