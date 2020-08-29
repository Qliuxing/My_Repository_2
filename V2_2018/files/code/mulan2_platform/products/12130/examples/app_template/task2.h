/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef TASK2_H_
#define TASK2_H_


#define TASK2_INIT_STACK_SIZE                 0     /* in words, TBD ! */
#define TASK2_DSP_PRESURE_STACK_SIZE         60     /* in words, TBD ! */
#define TASK2_DSP_TEMPERATURE_STACK_SIZE     60     /* in words, TBD ! */


extern void Task2_Init (uint16_t arg);
extern void Task2_DSP_ProcessPressure (uint16_t arg);
extern void Task2_DSP_ProcessTemperature (uint16_t arg);

extern volatile uint16_t task2_v_pr __attribute__((dp));
extern volatile uint16_t task2_v_tm __attribute__((dp));

#endif /* TASK2_H_ */
