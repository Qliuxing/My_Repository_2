/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef TASK1_H_
#define TASK1_H_


#define TASK1_INIT_STACK_SIZE                 0     /* in words, TBD ! */
#define TASK1_DSP_PRESURE_STACK_SIZE         60     /* in words, TBD ! */
#define TASK1_DSP_TEMPERATURE_STACK_SIZE     60     /* in words, TBD ! */


extern void Task1_Init (uint16_t arg);
extern void Task1_DSP_ProcessPressure (uint16_t arg);
extern void Task1_DSP_ProcessTemperature (uint16_t arg);


extern volatile uint16_t task1_v_pr __attribute__((dp));
extern volatile uint16_t task1_v_tm __attribute__((dp));


#endif /* TASK1_H_ */
