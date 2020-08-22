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
#include "main.h"
#include "MotorDriver.h"
#include "MotorStall.h"															/* Only for debugging purpose */
#include "PID_Control.h"														/* PID-controller support */
#include <awdg.h>																/* Analogue Watchdog support */
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
#if _SUPPORT_LIN_UV
uint16 g_u16LinUVTimeCounter = 0;												/* LIN UV Time-counter */
#endif /* _SUPPORT_LIN_UV */
#pragma space none																/* __NEAR_SECTION__ */

/* ****************************************************************************	*
 * Timer_Init()
 *
 * Initialise the core timer (Mulan2-timer), at a periodic rate of 500us
 * ****************************************************************************	*/
void Timer_Init( void)
{
	TIMER = TMR_EN | CT_PERIODIC_RATE;											/* 500us timer */
	PRIO = (PRIO & ~(3 << 0)) | ((6 - 3) << 0);									/* Set CoreTimer priority to 6 (3..6) */
	PEND = CLR_TIMER_IT;
	MASK |= EN_TIMER_IT;														/* Enable Timer interrupt */
} /* End of Timer_Init() */

/* ****************************************************************************	*
 * Timer_SleepCompensation()
 *
 * Compensate the various timer-counters for the sleep-period
 * ****************************************************************************	*/
void Timer_SleepCompensation( uint16 u16SleepPeriod)
{
	u16SleepPeriod = muldivU16_U16byU16byU16( u16SleepPeriod, 256U, (uint16)(CT_PERIODIC_RATE*(PLL_freq/1000000U)));
	ATOMIC_CODE
	(
/*lint !e436 */
#if _SUPPORT_AMBIENT_TEMP
		g_u16SelfHeatingCounter += u16SleepPeriod;
/*lint !e436 */
#endif /* _SUPPORT_AMBIENT_TEMP */
		
/*lint !e436 */
#if _SUPPORT_LIN_BUS_ACTIVITY_CHECK
		/* MLX4 LIN-Bus activity check */
		{
/*lint !e436 */
#if (__MLX_PLTF_VERSION_MAJOR__ == 4)
			if ( (ml_GetState( ML_NOT_CLEAR) != ml_stINVALID) && ((LinStatus & ML_LIN_BUS_ACTIVITY) != 0) )
/*lint !e436 */
#else  /*((__MLX_PLTF_VERSION_MAJOR__ == 4) */
			if ( LinBusStatus & ML_LIN_BUS_ACTIVITY )
/*lint !e436 */
#endif /*((__MLX_PLTF_VERSION_MAJOR__ == 4) */
			{
				/* MLX4 has detected a SYNC field */
				g_u16Mlx4StateCheckCounter = 0;
				g_u8ErrorCommBusTimeout = FALSE;
/*lint !e436 */
#if (__MLX_PLTF_VERSION_MAJOR__ == 4)
				(void) ml_GetState( ML_CLR_LIN_BUS_ACTIVITY);
/*lint !e436 */
#else  /*((__MLX_PLTF_VERSION_MAJOR__ == 4) */
				__asm__("clrb dp:_LinBusStatus.0");									/* LinBusStatus &= ~ML_LIN_BUS_ACTIVITY; */
/*lint !e436 */
#endif /*((__MLX_PLTF_VERSION_MAJOR__ == 4) */
			}
			else
			{
				g_u16Mlx4StateCheckCounter += u16SleepPeriod;
			}
		}
/*lint !e436 */
#endif /* _SUPPORT_LIN_BUS_ACTIVITY_CHECK */

		g_u16PID_CtrlCounter += u16SleepPeriod;									/* PID Current/Speed control */
		g_u16PID_ThrshldCtrlCounter += u16SleepPeriod;							/* PID Current/Speed Threshold control */
		if ( g_u8MotorStartDelay > u16SleepPeriod )
		{
			g_u8MotorStartDelay -= (uint8) u16SleepPeriod;
		}
		else
		{
			g_u8MotorStartDelay = 0;
		}

/*lint !e436 */
#if ((LINPROT & LINXX) == LIN2X)
		if ( g_u16DiagResponseTimeoutCount > u16SleepPeriod )
		{
			g_u16DiagResponseTimeoutCount -= u16SleepPeriod;
		}
		else if ( g_u16DiagResponseTimeoutCount != 0 )
		{
			g_u16DiagResponseTimeoutCount = 0;
			if ( g_u8BufferOutID == QR_RFR_DIAG )								/* Pending response type: Diagnostic */
			{
				g_u8BufferOutID = (uint8) QR_INVALID;							/* Invalidate Diagnostics response */
			}
		}
/*lint !e436 */
#endif /* ((LINPROT & LINXX) == LIN2X) */

/*lint !e436 */
#if _SUPPORT_CHIP_TEMP_PROFILE
		g_u16TemperatureStabilityCounter += u16SleepPeriod;
/*lint !e436 */
#endif /* _SUPPORT_CHIP_TEMP_PROFILE */

/*lint !e436 */
#if _SUPPORT_LIN_UV
		if ( g_u16LinUVTimeCounter != 0 )
		{
			g_u16LinUVTimeCounter += u16SleepPeriod;
		}
/*lint !e436 */
#endif /* _SUPPORT_LIN_UV */

/*lint !e436 */
#if (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44)
		if ( (u16DegradeDelay != 0) && (u16DegradeDelay != 0xFFFF) )
		{
			if ( u16DegradeDelay >  u16SleepPeriod )
				u16DegradeDelay -= u16SleepPeriod;
			else
				u16DegradeDelay = 0;
		}
/*lint !e436 */
#endif /* (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44) */

#if _SUPPORT_AUTO_CALIBRATION
		if ( g_u16CalibPauseCounter != 0 )
		{
			if ( g_u16CalibPauseCounter > u16SleepPeriod )
				g_u16CalibPauseCounter -= u16SleepPeriod;
			else
				g_u16CalibPauseCounter = 0;
		}
#endif /* _SUPPORT_AUTO_CALIBRATION */
	);
} /* End of Timer_SleepCompensation() */ /*lint !e438 */

/* ****************************************************************************	*
 * TIMER_IT()
 *
 * Periodic Timer ISR
 * ****************************************************************************	*/
__interrupt__ void TIMER_IT(void) 
{
#if _SUPPORT_LIN_BUS_ACTIVITY_CHECK && (__MLX_PLTF_VERSION_MAJOR__ == 3)
	/* MLX4 LIN-Bus activity check, except during PWM-mode */
	/* Note: For (__MLX_PLTF_VERSION_MAJOR__ == 4) the periodic check has to be performed in the main-loop.
	 * (ml-functions can not be used in ISR's)
	 */
	if ( LinBusStatus & ML_LIN_BUS_ACTIVITY )
	{
		/* MLX4 has detected a SYNC field */
		g_u16Mlx4StateCheckCounter = 0;
		g_u8ErrorCommBusTimeout = FALSE;
		__asm__("clrb dp:_LinBusStatus.0");									/* LinBusStatus &= ~ML_LIN_BUS_ACTIVITY; */
	}
	else
	{
		g_u16Mlx4StateCheckCounter++;										/* State check counter */
	}
#endif /* _SUPPORT_LIN_BUS_ACTIVITY_CHECK && (__MLX_PLTF_VERSION_MAJOR__ == 3) */

	if ( g_u8MotorStopDelay != 0 )
	{
		if ( (--g_u8MotorStopDelay == 0) && ((g_e8MotorStatusMode & (uint8) ~C_MOTOR_STATUS_DEGRADED) == (uint8) C_MOTOR_STATUS_STOP) )
		{
			DRVCFG_DIS_UVWT();
			DRVCFG_DIS();
		}
	}

	if ( g_u8MotorStartDelay != 0 )
	{
		g_u8MotorStartDelay--;
	}

	g_u16PID_CtrlCounter++;														/* PID Current/Speed control */
	g_u16PID_ThrshldCtrlCounter++;												/* PID Threshold control */
#if _SUPPORT_AMBIENT_TEMP
	g_u16SelfHeatingCounter++;													/* Self-heating compensation (ambjient temperature) */
#endif /* _SUPPORT_AMBIENT_TEMP */

#if ((LINPROT & LINXX) == LIN2X)												/* LIN 2.x */
	if ( g_u16LinAATicker != 0 )												/* LIN-AA on going */
	{
		if ( --g_u16LinAATicker == 0 )											/* LIN-AutoAddresing time-out counter (seconds) */
		{
			if ( --g_u8LinAATimeout == 0 )
			{
				LinAATimeoutControl();
			}
			else	
			{
				g_u16LinAATicker = PI_TICKS_PER_SECOND;
			}
		}
	}

	if ( g_u16DiagResponseTimeoutCount != 0 )
	{
		--g_u16DiagResponseTimeoutCount;
		if ( g_u16DiagResponseTimeoutCount == 0 )								/* One second time-out */
		{
			if ( g_u8BufferOutID == QR_RFR_DIAG )								/* Pending response type: Diagnostic */
			{
				g_u8BufferOutID = (uint8) QR_INVALID;							/* Invalidate Diagnostics response */
			}
		}
	}
#endif /* ((LINPROT & LINXX) == LIN2X) */

#if WATCHDOG == ENABLED
#if _SUPPORT_LIN_AA
	if ( g_u8AutoAddressingFlags & WAITINGFORBREAK )
	{
		/* Start performing "background" watchdog acknowledges, during LIN auto-addressing (waiting for the LIN break-pulse */
		WDG_Manager();
	}
#endif /* _SUPPORT_LIN_AA */
#endif /* WATCHDOG == ENABLED */

#if _SUPPORT_CHIP_TEMP_PROFILE
	g_u16TemperatureStabilityCounter++;
#endif /* _SUPPORT_CHIP_TEMP_PROFILE */

#if _SUPPORT_LIN_UV
	if ( g_u16LinUVTimeCounter != 0 )
	{
		g_u16LinUVTimeCounter++;
	}
#endif /* _SUPPORT_LIN_UV */

#if (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44)
	if ( (u16DegradeDelay != 0) && (u16DegradeDelay != 0xFFFF) )
	{
		u16DegradeDelay--;
	}
#endif /* (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44) */

#if _SUPPORT_AUTO_CALIBRATION
	if ( g_u16CalibPauseCounter != 0 )
		g_u16CalibPauseCounter--;
#endif /* _SUPPORT_AUTO_CALIBRATION */
} /* End of TIMER_IT() */

/* EOF */
