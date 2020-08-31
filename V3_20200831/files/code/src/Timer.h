/*! ----------------------------------------------------------------------------
 * \file		Timer.h
 * \brief		MLX81310 Core Timer handling
 *
 * \note		project MLX81310
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-18
 *   
 * \version 	1.0 - preliminary
 *
 *				
 *
 * MELEXIS Microelectronic Integrated Systems
 * 
 * Copyright (C) 2012-2013 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * ****************************************************************************	*/

#ifndef TIMER_H_
#define TIMER_H_

#include <syslib.h>
#include "Build.h"
#include "lib_mlx315_misc.h"

#define C_SLEEP_PERIOD				((uint16)((PLL_freq/256U) * 0.05))			/* Sleep period of 50 ms */

/* *** Definitions *** */
#define CT_PERIODIC_RATE			500U										/*!< Periodic interrupt at 500us rate */
#define	PI_TICKS_PER_SECOND			(1000000U/CT_PERIODIC_RATE)
#define	PI_TICKS_PER_HALF_SECOND	(500000U/CT_PERIODIC_RATE)
#define PI_TICKS_PER_MILLISECOND	(1000U/CT_PERIODIC_RATE)

#define C_PI_TICKS_10MS				( 10u * PI_TICKS_PER_MILLISECOND)
#define C_PI_TICKS_20MS				( 20u * PI_TICKS_PER_MILLISECOND)
#define C_PI_TICKS_125MS			(125u * PI_TICKS_PER_MILLISECOND)
#define C_PI_TICKS_250MS			(250u * PI_TICKS_PER_MILLISECOND)
#define C_PI_TICKS_500MS			(500u * PI_TICKS_PER_MILLISECOND)
#define C_PI_TICKS_PWM_INACTIVE		(  4u * PI_TICKS_PER_SECOND)
#define C_PI_TICKS_STABILISE		( 50u * PI_TICKS_PER_MILLISECOND)			/* Max. 255 */
#define C_PI_TICKS_STABILISE_CALIB	(100u * PI_TICKS_PER_MILLISECOND)

#define DELAY_4us	(uint16)(((    4U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*   4us delay */
#define DELAY_7us	(uint16)(((    7U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*   7us delay */
#define DELAY_10us	(uint16)(((   10U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*  10us delay */
#define DELAY_50us	(uint16)(((   50U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*  50us delay */
#define DELAY_60us	(uint16)(((   60U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*  60us delay */
#define DELAY_100us	(uint16)(((  100U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/* 100us delay */
#define DELAY_1ms	(uint16)((( 1000U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*   1ms delay */
#define DELAY_1m25s	(uint16)((( 1250U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/* 1m25s delay */
#define DELAY_10ms	(uint16)(((10000U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*  10ms delay */
#define DELAY_16ms	(uint16)(((16000U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*  16ms delay (maximum at 32 MHz PLL) */
#if (PLL_freq <= 20000000)
#define DELAY_25ms	(uint16)(((25000U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/* 25ms delay (maximum at 20 MHz PLL) */
#endif /* (PLL_freq <= 20000000) */
#define DELAY_mPWM	(uint16)((PLL_freq/((uint32)PWM_FREQ*2*CYCLES_PER_INSTR))-1)	/* Motor-PWM period delay */
#define ADC_DELAY	(uint16)((PLL_freq/((uint32)PWM_FREQ*2*4*CYCLES_PER_INSTR))+1)	/* 25% of PLL-freq/PWM-Freq/(#cycles/instruction)/#instructions */

typedef enum
{
   /* Insert here the project specific timers */
#if _SUPPORT_LIN_BUS_ACTIVITY_CHECK
   MLX4_STATUS_CHECK_TIMER,    	/* g_u16Mlx4StateCheckCounter */
#endif   
   MOTOR_STOP_DELAY_TIMER,   		/* g_u8MotorStopDelay */
   MOTOR_START_DELAY_TIMER,    	/* g_u8MotorStartDelay */
   CV_INIT_BLOCK_DLY_TIMER,    	/* g_u16CVInitializeBlockDly */
   PID_CTRL_TIMER,    				/* g_u16PID_CtrlCounter */
   PID_THRSHLD_CTRL_TIMER,    		/* g_u16PID_ThrshldCtrlCounter */
#if _SUPPORT_AMBIENT_TEMP
   SELF_HEATING_TIMER,    			/* g_u16SelfHeatingCounter */
#endif   
   DIAG_RESPONSE_TIMER,    		/* g_u16DiagResponseTimeoutCount */
#if _SUPPORT_CHIP_TEMP_PROFILE
   TEMPERATURE_STABILITY_TIMER,    /* g_u16TemperatureStabilityCounter */
#endif   
#if _SUPPORT_LIN_UV
   LIN_UV_TIMER,    				/* g_u16LinUVTimeCounter */
#endif   
   CALIB_PAUSE_TIMER,    			/* g_u16CalibPauseCounter */
   FAULT_HOLD_TIMER,

  MAX_TIMER,
} TIMER_ID;

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
extern void Timer_Init( void);														/*!< Initialize the core timer (Mulan2-timer), at a periodic rate of 500us */
extern void Timer_SleepCompensation( uint16 u16SleepPeriod);					/*!< Compensate the various timer-counters for the sleep-period */
extern void Timer_Start(TIMER_ID id,uint16 TimerPeriod);						/* Set timer by timer ID*/
extern uint8 Timer_IsExpired(TIMER_ID id);									/* Get the status of timer*/

static __inline__ void NopDelay(uint16 u16DelayCount)
{
	for (; u16DelayCount > 0u; u16DelayCount-- )
	{
		NOP();
	}
}



#endif /* TIMER_H_ */

/* EOF */
