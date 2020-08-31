/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Software Platform
 *
 */



#include <mmu_api.h>
#include "task1.h"
#include "task2.h"


/* Prototypes */
void ADC_PressureIsr (void)      __attribute__ ((interrupt));
void ADC_EndOfSequenceIsr (void) __attribute__ ((interrupt));


/** ----------------------------------------------------------------------------
 *  ADC Pressure ISR
 *
 *  \note
 *      1. Connected to MMC16 EXT0_IT
 *      2. Interrupt is triggered when pressure channel conversion is finished
 */
void ADC_PressureIsr (void)         /* SW prio: 3; CPU prio inside ISR: 2 */
{

    MMU_RunAs (MMU_TASK1_ID,
               Task1_DSP_ProcessPressure,
               1000,
               TASK1_DSP_PRESURE_STACK_SIZE);


    MMU_RunAs (MMU_TASK2_ID, 
               Task2_DSP_ProcessPressure,
               2000,
               TASK2_DSP_PRESURE_STACK_SIZE);

}


/** ----------------------------------------------------------------------------
 * ADC End of Sequence interrupt
 *
 * \note
 *      1. Interrupt is triggered when all ADC conversion from the sequence 
 *         are finished
 */
void ADC_EndOfSequenceIsr (void)    /* SW prio: 5; CPU prio inside ISR: 4 */
{

    MMU_RunAs (MMU_TASK1_ID, 
               Task1_DSP_ProcessTemperature,
               3000,
               TASK1_DSP_TEMPERATURE_STACK_SIZE);


    MMU_RunAs (MMU_TASK2_ID, 
               Task2_DSP_ProcessTemperature,
               4000,
               TASK2_DSP_TEMPERATURE_STACK_SIZE);

}

/* EOF */
