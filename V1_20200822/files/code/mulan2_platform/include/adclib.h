/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef ADCLIB_H_
#define ADCLIB_H_

#include <ioports.h>
#include "syslib.h"

/* Triggering source */
#define ADC_SOFTWARE_TRIGGER      0
#define ADC_HARDWARE_TRIGGER      1

/* Conversion mode */
#define ADC_SINGLE_SEQUNCE_MODE       0
#define ADC_CONTINUOUS_MODE           1

#define ADC_CFG_END_MARKER      0xFFFF

/* ----------------------------------------------------------------------------
 * Initializes ADC unit
 *
 * Parameters:
 *      p_config : pointer to configuration (could point to RAM or ROM)
 *      p_result : pointer to result buffer (must point to RAM)
 *      trigger  : ADC_SOFTWARE_TRIGGER or ADC_HARDWARE_TRIGGER
 *      mode : conversion mode (ADC_SINGLE_SEQUNCE_MODE
 *             or ADC_CONTINUOUS_MODE)
 * NOTES:
 *  1. ADC is initialized to detect first hard triggering pulse. i.e.
 *     ADC_SYNC_SOC = 1 [MM16 v1.1]
 */
#define ADC_INIT(p_config, p_result, trigger, mode) \
do {                                                \
    ADC_DBASE = (uint16)(p_result);                 \
    ADC_SBASE = (uint16)(p_config);                 \
    ADC_CTRL  = (((trigger) & 1) << 1)              \
                 | (((mode) & 1) << 2)              \
                 | ADC_SYNC_SOC;                    \
} while(0)


/* ----------------------------------------------------------------------------
 * Starts ADC conversion sequence
 * 
 * During conversion sequence all channels specified in configuration will be
 * processed. Conversion sequence can be terminated by calling ADC_STOP_SEQUENCE.
 * At the end of conversion sequence an ADC EOC interrupt will be signaled (if
 * enabled). Also polling of ADC_IS_SEQUENCE_COMPLETED() can be used.
 */
#define ADC_START_SEQUENCE()   \
    (ADC_CTRL |= ADC_START)
    

/* ----------------------------------------------------------------------------
 * Terminates ongoing ADC conversion sequence
 *
 * The sequence is terminated _after_ the current channel conversion is finished.
 * (the result of the current conversion will be written in memory)
 */
#define ADC_STOP_SEQUENCE()   \
    (ADC_CTRL &= ~ADC_START)

    
/* ----------------------------------------------------------------------------
 * Returns status of ongoing conversion sequence.
 * 
 * Applicable only for ADC_SINGLE_SEQUNCE_MODE. For ADC_CONTINUOUS_MODE
 * interrupt notification should be used instead
 */
#define ADC_IS_SEQUENCE_COMPLETED()    \
    ((ADC_CTRL & ADC_START) == 0)

    
/* ----------------------------------------------------------------------------
 * Enables ADC end-of-conversion sequence interrupt notification and sets its
 * priority
 * prio: 3 .. 6
 *
 * PRIO[3:2], MASK[6], PEND[6]
 */
#define ADC_INT_ENABLE(prio)            \
do {                                    \
    PRIO = ((((prio) - 3u) & 3) << 2)   \
               | (PRIO & ~(3u << 2));   \
    PEND  = CLR_ADC_IT;                 \
    MASK |= EN_ADC_IT;                  \
} while(0)


/* ----------------------------------------------------------------------------
 * Disables (mask) ADC interrupt
 */
#define ADC_INT_DISABLE()   \
    (MASK &= ~EN_ADC_IT)

/* ----------------------------------------------------------------------------
 * Clears pending ADC interrupt
 */
#define ADC_INT_CLEAR()   \
    (PEND = CLR_ADC_IT)

    
/* ----------------------------------------------------------------------------
 * Returns status of ADC interrupt
 *  0 : disabled
 *  1 : enabled
 */
#define ADC_INT_GET_STATUS()    \
    (MASK & EN_ADC_IT)


/* ----------------------------------------------------------------------------
 * Starts (triggers by software) conversion of next channel within conversion
 * sequence.
 * 
 * NOTES:
 *  Software triggering should be selected (ADC_SOFTWARE_TRIGGER) during
 *  ADC_INIT otherwise function call has no effect
 */
#if 0
/* applicable for MMC16 >= 1.3 */
#define ADC_TRIGGER_NEXT_CONVERSION()  \
    (ADC_CTRL |= ADC_SOFT_TRIG)
#else
/* applicable for MMC16 < 1.3 and MULAN2 */
#define ADC_TRIGGER_NEXT_CONVERSION()   \
    do {                                \
            PEND = CLR_ADC_IT;          \
            ADC_CTRL |= ADC_SOFT_TRIG;  \
    } while (0)
#endif

/* ----------------------------------------------------------------------------
 * Status of current conversion
 */
/*
 * Notes:
 *  1. Some delay exists between ADC start and ADC_EOC clearing (see issue MMC16-42).
 *     So, ADC_EOC=1 might be prematurely sampled as indicating end of conversion.
 *     Thus ADC_EOC=1 must be checked together with ADC_SOFT_TRIG=0. Note, that
 *     function ADC_TRIGGER_NEXT_CONVERSION should be called before ADC_IS_BUSY.
 *     This is valid for for MMC16 >= 1.3. For previous MMC16 core version ADC
 *     pending bit should be used to check end of conversion.
 */
#if 0
/* applicable for MMC16 >= 1.3 */
#define ADC_IS_BUSY()               ((ADC_CTRL & (ADC_EOC | ADC_SOFT_TRIG )) != ADC_EOC)     /* Conversion is done when EOC=1 && SOFT_TRIG = 0 (see issue MMC16-42) */
#else
/* applicable for MMC16 < 1.3 and MULAN2 */
#define ADC_IS_BUSY()               ((PEND & CLR_ADC_IT) == 0)  /* pending bit = 0 while conversion is in progress */
#endif

#define ADC_IS_OVERFLOW()           (ADC_CTRL & ADC_OVF)


#endif /* ADCLIB_H_ */
