/*! ----------------------------------------------------------------------------
 * \file		Timer.c
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
 * \functions	Timer_Init()
 *				Timer_Stop()
 *				Timer_SleepCompensation()
 *				TIMER_IT()
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

#include "Build.h"
#include "Timer.h"																/* Periodic IRQ Timer support */
#include <plib.h>																/* Use Melexis MLX813xx library (WDG_Manager) */
#include "private_mathlib.h"
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION (@TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp 																/* __TINY_SECTION__ */
#pragma space none																/* __TINY_SECTION__ */

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	(@NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp																/* __NEAR_SECTION__ */
uint16 l_u16Timer[MAX_TIMER];
#pragma space none																/* __NEAR_SECTION__ */


/* ****************************************************************************	*
 * Timer_Init()
 *
 * Initialise the core timer (Mulan2-timer), at a periodic rate of 500us
 * ****************************************************************************	*/
void Timer_Init( void)
{
	uint16 i;
	for(i = 0; i < (uint16)MAX_TIMER; i++)
	{
		l_u16Timer[i] = 0;
	}
	/* System Tick Timer - Core Timer  */
	TIMER =  TMR_EN | CT_PERIODIC_RATE;											/* 500us timer */

} /* End of Timer_Init() */

/* ****************************************************************************	*
 * Timer_Start()
 *
 * Set the timer by timer period and timer id
 * ****************************************************************************	*/
void Timer_Start(TIMER_ID id,uint16 TimerPeriod)
{
	if(id < MAX_TIMER)
	{
		l_u16Timer[id] = TimerPeriod;
	}
}

/* ****************************************************************************	*
 * Is_Timer_Timeout()
 *
 * Get the timer status by timer id
 * ****************************************************************************	*/
uint8 Timer_IsExpired(TIMER_ID id)
{
	if(l_u16Timer[id] == 0u)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/* ****************************************************************************	*
 * Timer_SleepCompensation()
 *
 * Compensate the various timer-counters for the sleep-period
 * ****************************************************************************	*/
void Timer_SleepCompensation( uint16 u16SleepPeriod)
{
	uint16 i;
	
	u16SleepPeriod = muldivU16_U16byU16byU16( u16SleepPeriod, 256U, (uint16)(CT_PERIODIC_RATE*(PLL_freq/1000000U)));
	ATOMIC_CODE
	(
		for(i = 0u; i < (uint16)MAX_TIMER; i++)
		{
			if(l_u16Timer[i] > u16SleepPeriod)
			{
				l_u16Timer[i] -= u16SleepPeriod;
			}
			else
			{
				l_u16Timer[i] = 0;
			}		
		}	
	);
} /* End of Timer_SleepCompensation() */


/* ****************************************************************************	*
 * TIMER_IT()
 *
 * Periodic Timer ISR
 * ****************************************************************************	*/
__interrupt__ void TIMER_IT(void) 
{
	uint16 i;
	
	for(i = 0; i < (uint16)MAX_TIMER; i++)
	{	
		if(l_u16Timer[i] > 0u)
		{
			l_u16Timer[i]--;
		}
	}
}


/* EOF */
