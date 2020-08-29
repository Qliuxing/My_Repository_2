/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef _LIB_MLX8110X_ADC_H_
#define _LIB_MLX8110X_ADC_H_

#include "ioports.h"

/* TR_ADCREF1 .. Register: ANA_OUTD .. Bit 0 - 6 */
#define TR_ADCREF1 (uint16)~(0x7Fu << 0)
/* TR_ADCREF2 .. Register: ANA_OUTD .. Bit 8 - 14 */
#define TR_ADCREF2 (uint16)~(0x7Fu << 8)
/* TR_ADCREF3 .. Register: ANA_OUTE .. Bit 0 - 6 */
#define TR_ADCREF3 (uint16)~(0x7Fu << 0)

/* ADCFREQ .. Register: ANA_OUTE .. Bit 14 - 15 */
#define ADCFREQ             (uint16)~(3u << 14)
#define ADC_SET_FREQ_1MHZ   (1u << 14)
#define ADC_SET_FREQ_2MHZ   (0u << 14)
#define ADC_SET_FREQ_3MHZ   (2u << 14)
#define ADC_SET_FREQ_4MHZ   (3u << 14)

/* ADC_SIN .. Bit 8 - 12 */
#define ADC_SIN_CLEARCHANNEL (uint16)~(0x1Fu << 8)
#define ADC_SIN_VS_FOURTEENTH  (0u << 8)
#define ADC_SIN_TEMP           (1u << 8)
#define ADC_SIN_VDDD           (2u << 8)
#define ADC_SIN_VDDA_HALF      (3u << 8)
#define ADC_SIN_V5V6_FOURTH    (4u << 8)
#define ADC_SIN_VAUX_HALF      (5u << 8)
#define ADC_SIN_LINAAMP        (6u << 8)
#define ADC_SIN_LINVCMO        (7u << 8)
#define ADC_SIN_D0_HALF        (8u << 8)
#define ADC_SIN_D1_HALF        (9u << 8)
#define ADC_SIN_D2_HALF        (10u << 8)
#define ADC_SIN_D3_HALF        (11u << 8)
#define ADC_SIN_D4_HALF        (12u << 8)
#define ADC_SIN_D5_HALF        (13u << 8)
#define ADC_SIN_D6_FOURTH      (14u << 8)
#define ADC_SIN_D7_HALF        (15u << 8)
#define ADC_SIN_HVIO0_HALF     (16u << 8)
#define ADC_SIN_HVIO1_HALF     (17u << 8)
#define ADC_SIN_HVIO2_HALF     (18u << 8)
#define ADC_SIN_HVIO3_HALF     (19u << 8)
#define ADC_SIN_HVIODIFF       (20u << 8)

/* ADC_HVDIFF .. Register: HV_INEN .. Bit 12 - 15 */
#define ADC_HVDIFFCLEAR (uint16)~(0xFu << 12)
#define ADC_HVDIFFSEL0           (1u << 12)
#define ADC_HVDIFFSEL1           (1u << 13)
#define ADC_HVDIFFSEL2           (1u << 14)
#define ADC_HVDIFFSEL3           (1u << 15)

/* ADC_SREF - reference voltage .. Bit 0 - 1 */
#define ADC_SREF_CLEARVOLT (uint16)~(0x3u << 0)
#define ADC_SREF_OFF    (0u << 0)
#define ADC_SREF_0V75   (1u << 0)
#define ADC_SREF_1V5    (2u << 0)
#define ADC_SREF_2V5    (3u << 0)

/* ADC_SREF - HW trigger .. Bit 4 - 7 */
#define ADC_SREF_CLEARTRIGGER (uint16)~(0xFu << 4)
#define ADC_SREF_PWM1_CNTI     (0u << 4)
#define ADC_SREF_PWM2_CNTI     (1u << 4)
#define ADC_SREF_PWM3_CNTI     (2u << 4)
#define ADC_SREF_PWM4_CNTI     (3u << 4)
#define ADC_SREF_PWM1_CMPI     (4u << 4)
#define ADC_SREF_PWM2_CMPI     (5u << 4)
#define ADC_SREF_PWM3_CMPI     (6u << 4)
#define ADC_SREF_PWM4_CMPI     (7u << 4)
#define ADC_SREF_TMR1_CMPB_IT  (8u << 4)
#define ADC_SREF_TMR1_CMPA_IT  (9u << 4)
#define ADC_SREF_TMR1_CAPB_IT  (10u << 4)
#define ADC_SREF_TMR1_CAPA_IT  (11u << 4)

/** Trim ADC->0V75 (Attention: ADC is already trimmed during startup)
 * @param
 * @return void
 * Register: ANA_OUTD, Bit TR_ADCREF1, Bit positions [6:0]
 */
#define ADC_SET_0V75TRIM(TR_ADC_REF1)                          \
do {                                                           \
       ANA_OUTD = ((ANA_OUTD & TR_ADCREF1) | (TR_ADC_REF1));   \
	} while (0)

/** Trim ADC->1V5 (Attention: ADC is already trimmed during startup)
 * @param
 * @return void
 * Register: ANA_OUTD, Bit TR_ADCREF2, Bit positions [14:8]
 */
#define ADC_SET_1V5TRIM(TR_ADC_REF2)                           \
do {                                                           \
       ANA_OUTD = ((ANA_OUTD & TR_ADCREF2) | (TR_ADC_REF2));   \
	} while (0)

/** Trim ADC->2V5 (Attention: ADC is already trimmed during startup)
 * @param
 * @return void
 * Register: ANA_OUTE, Bit TR_ADCREF3, Bit positions [6:0]
 */
#define ADC_SET_2V5TRIM(TR_ADC_REF3)                           \
do {                                                           \
       ANA_OUTE = ((ANA_OUTE & TR_ADCREF3) | (TR_ADC_REF3));   \
	} while (0)

/** Select ADC-Frequency
 * @param
 * @return void
 * Register: ANA_OUTE, Bit ADCFREQ, Bit positions [15:14]
 */
#define ADC_SELECT_ADCFREQ(ADC_SET_FREQ)                      \
do {                                                          \
       ANA_OUTE = ((ANA_OUTE & ADCFREQ) | (ADC_SET_FREQ));    \
	} while (0)

/** Select HVDIFF input for ADC
 * @param
 * @return void
 * Register: HV_INEN, Bit HVDIFFSEL, Bit positions [15:12]
 */
#define ADC_SET_HVDIFF(ADC_HVDIFFSEL)                             \
do {                                                              \
		HV_INEN = ((HV_INEN & ADC_HVDIFFCLEAR) | (ADC_HVDIFFSEL));  \
	} while (0)	

#endif /* _LIB_MLX8110X_ADC_H_ */
