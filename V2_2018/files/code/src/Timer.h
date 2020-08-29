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

#ifndef SYSLIB_H_
#include "syslib.h"
#endif /* SYSLIB_H_ */

/* *** Definitions *** */
#define CT_PERIODIC_RATE			500U											/*!< Periodic interrupt at 500us rate */
#define	PI_TICKS_PER_SECOND			(1000000U/CT_PERIODIC_RATE)
#define	PI_TICKS_PER_HALF_SECOND	(500000U/CT_PERIODIC_RATE)
#define PI_TICKS_PER_MILLISECOND	(1000U/CT_PERIODIC_RATE)

#define C_PI_TICKS_10MS				( 10 * PI_TICKS_PER_MILLISECOND)
#define C_PI_TICKS_20MS				( 20 * PI_TICKS_PER_MILLISECOND)
#define C_PI_TICKS_125MS			(125 * PI_TICKS_PER_MILLISECOND)
#define C_PI_TICKS_250MS			(250 * PI_TICKS_PER_MILLISECOND)
#define C_PI_TICKS_PWM_INACTIVE		(  4 * PI_TICKS_PER_SECOND)
#define C_PI_TICKS_STABILISE		( 50 * PI_TICKS_PER_MILLISECOND)			/* Max. 255 */
#define C_PI_TICKS_STABILISE_CALIB	(100 * PI_TICKS_PER_MILLISECOND)

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
extern void Timer_Init( void);													/*!< Initialise the core timer (Mulan2-timer), at a periodic rate of 500us */
extern void Timer_SleepCompensation( uint16 u16SleepPeriod);					/*!< Compensate the various timer-counters for the sleep-period */

#if _SUPPORT_LIN_UV
extern uint16 g_u16LinUVTimeCounter;											/* LIN UV Time-counter */
#endif /* _SUPPORT_LIN_UV */

#endif /* TIMER_H_ */

/* EOF */
