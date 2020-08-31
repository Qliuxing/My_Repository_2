/*! ----------------------------------------------------------------------------
 * \file		PID_Control.c
 * \brief		MLX81310 PID Controller handling
 *
 * \note		project MLX81310
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	ThresholdControl()
 *				PID_Init()
 *				VoltageCorrection()
 *				PID_Control()
 *				SelfHeatCompensation()
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
 * ****************************************************************************	*
 * Resources:
 *	Timer1 as Commutation-timer.
 *	PWM 1 through 4 (Master + 3x Slave for U, V & W phase)
 *	IO[4] & IO[5] only in case of 4-phase
 *
 * ****************************************************************************	*/

#include "Build.h"
#include "Timer.h"
#include "PID_Control.h"
#include "ADC.h"
#include "MotorDriver.h"
#include "NVRAM_UserPage.h"														/* NVRAM User-page support */
#include "private_mathlib.h"
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION (@TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp 																/* __TINY_SECTION__ */
uint16 g_u16PID_I;
#pragma space none																/* __TINY_SECTION__ */

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	(@NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp																/* __NEAR_SECTION__ */
uint16 l_u16MaxPidCtrlRatio;
uint16 g_u16PidCtrlRatio;
int16 g_i16PID_D = 0;
int16 g_i16PID_E = 0;
uint16 g_u16PidHoldingThreshold;												/* 5: Motor holding current threshold */
uint16 g_u16PidHoldingThresholdADC;												/* 5: Motor holding current threshold (ADC) */
uint16 g_u16PidRunningThreshold;												/* 5: Motor current threshold (running) */
uint16 g_u16PidRunningThresholdADC;												/* 5: Motor current threshold (running) (ADC) */
uint16 l_u16MinCorrectionRatio;													/* MMP150509-2 */
uint16 l_u16MaxCorrectionRatio;													/* MMP150509-2 */
#if _SUPPORT_AMBIENT_TEMP
uint16 g_u16SelfHeatingCounter = 0;
uint32 l_u32SelfHeatingIntegrator = 0;
#endif /* _SUPPORT_AMBIENT_TEMP */
uint16 g_u16MotorRefVoltage = 1200;												/* Motor reference voltage:not used */
uint16 l_u16MotorRefVoltageADC = (uint16) ((12*1024)/(2.5*14));					/* 12.00V [ADC-LSB] */

#if _DEBUG_VOLTAGE_COMPENSATION
int16 l_ai16MotorVolt[SZ_MOTOR_VOLT_COMP];
uint16 u16MotorVoltIdx = 0;
#endif /* _DEBUG_VOLTAGE_COMPENSATION */
#pragma space none																/* __NEAR_SECTION__ */

/* ***
 * PID_Init()
 * ***/
void PID_Init( void)
{
	/* MMP141209-1: Convert [mA] to [ADC-lsb] */
	uint16 u16MCurrgain = EE_GMCURR;
	g_u16PidHoldingThreshold = NVRAM_HOLDING_CURR_LEVEL;
	g_u16PidHoldingThresholdADC = muldivU16_U16byU16byU16( g_u16PidHoldingThreshold, C_GMCURR_DIV, u16MCurrgain);	/* MMP141209-1 */
	g_u16PidRunningThreshold = NVRAM_RUNNING_CURR_LEVEL;
	g_u16PidRunningThresholdADC = muldivU16_U16byU16byU16( g_u16PidRunningThreshold, C_GMCURR_DIV, u16MCurrgain);	/* MMP141209-1/MMP131219-1 */
	g_u16MotorRefVoltage = NVRAM_VSUP_REF;
#if _SUPPORT_VSMFILTERED
	l_u16MotorRefVoltageADC = muldivU16_U16byU16byU16( NVRAM_VSUP_REF, C_GVOLTAGE_DIV, EE_GVOLTAGE) + EE_OVOLTAGE;
#else  /* _SUPPORT_VSMFILTERED */
	l_u16MotorRefVoltageADC = muldivU16_U16byU16byU16( NVRAM_VSUP_REF, C_GVOLTAGE_DIV, EE_GADC) + EE_OADC;
#endif /* _SUPPORT_VSMFILTERED */

	l_u16MinCorrectionRatio = NVRAM_MIN_CORR_RATIO;								/* MMP150509-2 */
	l_u16MaxCorrectionRatio = NVRAM_MAX_CORR_RATIO;								/* MMP150509-2 */
} /* End of PID_Init() */

/* ***
 * VoltageCorrection()
 *
 *	Compensate Motor PWM Duty Cycle for voltage changes
 * Performance: 7.5us @ 20Mz
 * ***/
void VoltageCorrection( void)
{
	uint16 u16MotorVoltageADC = GetRawVsupplyMotor();
#if _DEBUG_VOLTAGE_COMPENSATION
	l_ai16MotorVolt[u16MotorVoltIdx] = g_i16MotorVoltage;
	u16MotorVoltIdx = (u16MotorVoltIdx + 1u) & (SZ_MOTOR_VOLT_COMP - 1u);
#endif /* _DEBUG_VOLTAGE_COMPENSATION */
	if ( (u16MotorVoltageADC > 0u) && (l_u16MotorRefVoltageADC > 0u) )
	{
		/* Correct Motor PWM duty cycle instantly based on change of supply voltage */
		uint16 u16NewCorrectionRatio = divU16_U32byU16( mulU32_U16byU16( g_u16PidCtrlRatio, l_u16MotorRefVoltageADC), u16MotorVoltageADC);
		if ( g_u8MotorStartupMode !=  (uint8)MSM_STOP  )
		{
			if ( u16NewCorrectionRatio < l_u16MinCorrectionRatio )
			{
				/* Underflow */
				u16NewCorrectionRatio = l_u16MinCorrectionRatio;
			}
			else if ( u16NewCorrectionRatio > l_u16MaxCorrectionRatio )
			{
				/* Overflow */
				u16NewCorrectionRatio = l_u16MaxCorrectionRatio;
			}
			else
			{
				
			}
		}
		else if ( u16NewCorrectionRatio < NVRAM_MIN_HOLDCORR_RATIO )
		{
			/* Underflow */
			u16NewCorrectionRatio = NVRAM_MIN_HOLDCORR_RATIO;
		}
		else
		{
			
		}
		g_u16CorrectionRatio = u16NewCorrectionRatio;
	}
	else
	{
		g_u16CorrectionRatio = g_u16PidCtrlRatio;
	}
} /* End of VoltageCorrection() */

/* ***
 * PID_Control()
 *
 *	DC: Motor PWM Duty Cycle; 
 *	Stepper: current-control;
 *	BEMF: speed-control
 * ***/
void PID_Control( void)
{

	{
		/* Periodic update (PWM-DC/Current/Speed-Control) */
		int16 i16ControlError;
		int16 i16PID_Ipart;
		int16 i16PID_Ppart;
		uint16 u16PidCtrlRatio;

		/* Current Control */
		uint16 u16MotorCurrentLPFFraction = ((g_u16MotorCurrentLPFx64 + 32u) >> 6u);	/* MMP140911-1 */
		
		if ( g_u8MotorStartupMode !=  (uint8)MSM_STOP )
		{
			i16ControlError = (int16) (g_u16PidRunningThresholdADC - u16MotorCurrentLPFFraction);	/* MMP140911-1 */
			l_u16MaxPidCtrlRatio = l_u16MaxCorrectionRatio;
		}
		else
		{
			i16ControlError = (int16) (g_u16PidHoldingThresholdADC - u16MotorCurrentLPFFraction);	/* MMP140911-1 */
		}

		/* Derivative-part */
		g_i16PID_D = mulI16_I16byI16RndDiv64( (i16ControlError - g_i16PID_E), (int16)NVRAM_PID_COEF_D);
		g_i16PID_E = i16ControlError;
		/* Integral-part */
		i16PID_Ipart = mulI16_I16byI16RndDiv64( i16ControlError, (int16)NVRAM_PID_COEF_I);
		if ( (i16PID_Ipart < 0) && (g_u16PID_I < ((uint16) -i16PID_Ipart)) )
		{
			g_u16PID_I = 0u;
		}
		else
		{
			g_u16PID_I = (uint16) ((int16)g_u16PID_I + i16PID_Ipart);
		}
		/* Proportional-part */
		i16PID_Ppart = mulI16_I16byI16RndDiv64( i16ControlError, (int16) NVRAM_PID_COEF_P);
		if ( (i16PID_Ppart < 0) && (g_u16PID_I < ((uint16) -i16PID_Ppart)) )
		{
			if ( g_u8MotorStartupMode !=  (uint8)MSM_STOP )
			{
				u16PidCtrlRatio = l_u16MinCorrectionRatio;
			}
			else
			{
				u16PidCtrlRatio = NVRAM_MIN_HOLDCORR_RATIO;
			}
			g_u16PID_I = 0u;
		}
		else
		{
			u16PidCtrlRatio = (uint16) ((int16)g_u16PID_I + i16PID_Ppart + g_i16PID_D);
			if ( g_u8MotorStartupMode !=  (uint8)MSM_STOP )
			{
				if ( u16PidCtrlRatio < l_u16MinCorrectionRatio )
				{
					/* Underflow */
					u16PidCtrlRatio = l_u16MinCorrectionRatio;
				}
				else if ( u16PidCtrlRatio >= l_u16MaxPidCtrlRatio )
				{
					/* Overflow */
					u16PidCtrlRatio = l_u16MaxPidCtrlRatio;
					g_u16PID_I = u16PidCtrlRatio;								/* MMP140617-1 */
				}
				else
				{
				
				}
			}
			else if ( u16PidCtrlRatio < NVRAM_MIN_HOLDCORR_RATIO )
			{
				/* Underflow */
				u16PidCtrlRatio = NVRAM_MIN_HOLDCORR_RATIO;
			}
			else
			{
				
			}
		}
		g_u16PidCtrlRatio = u16PidCtrlRatio;
	}
	VoltageCorrection();

	/* Update motor-driver PWM duty-cycle in case of holding-mode with coil-current */
} /* End of PID_Control() */

/* ***
 * ThresholdControl()
 *
 *	DC-Motor: Motor Power
 *	Stepper: Current Threshold Control
 *	BEMF: Nothing
 * ***/
void ThresholdControl( void)
{
		uint16 u16CurrThrshldRatio;
		int16 i16TemperatureBgn = NVRAM_CURRTHRSHLD_TEMP_1;
		uint16 u16CurrThrshldRatioBgn = NVRAM_CURRTHRSHLD_RATIO_1;


#if _SUPPORT_AMBIENT_TEMP
		if ( g_i16AmbjTemperature < (i16TemperatureBgn - C_CURRTHRSHLD_TEMP_HYS) )
#else  /* _SUPPORT_AMBIENT_TEMP */
		if ( g_i16ChipTemperature < (i16TemperatureBgn - C_CURRTHRSHLD_TEMP_HYS) )
#endif /* _SUPPORT_AMBIENT_TEMP */
		{
			if ( NVRAM_CURRTHRSHLD_ZONE_1 != 0u )
			{
				u16CurrThrshldRatio = u16CurrThrshldRatioBgn;					/* Same as point _1 */
			}
			else
			{
				u16CurrThrshldRatio = 0u;										/* Shutdown motor */
			}
		}
		else
		{
			uint8 u8CurrThrshldCtrlType = (uint8)NVRAM_CURRTHRSHLD_ZONE_2;				/* Get current threshold compensation-type */
			uint16 u16CurrThrshldRatioEnd = NVRAM_CURRTHRSHLD_RATIO_2;			/* Get zone end point (_2) */
			int16 i16TemperatureEnd = NVRAM_CURRTHRSHLD_TEMP_2;						
#if _SUPPORT_AMBIENT_TEMP
			if ( g_i16AmbjTemperature > i16TemperatureEnd )						/* Temperature above second zone ? */
#else  /* _SUPPORT_AMBIENT_TEMP */
			if ( g_i16ChipTemperature > i16TemperatureEnd )						/* Temperature above second zone ? */
#endif /* _SUPPORT_AMBIENT_TEMP */
			{
				i16TemperatureBgn = i16TemperatureEnd;							/* Next zone; begin point (_2) */
				u16CurrThrshldRatioBgn = u16CurrThrshldRatioEnd;
				u8CurrThrshldCtrlType = (uint8)NVRAM_CURRTHRSHLD_ZONE_3;
				u16CurrThrshldRatioEnd = NVRAM_CURRTHRSHLD_RATIO_3;				/* Get zone end point (_3) */
				i16TemperatureEnd = NVRAM_CURRTHRSHLD_TEMP_3;
#if _SUPPORT_AMBIENT_TEMP
				if ( g_i16AmbjTemperature > i16TemperatureEnd )					/* Temperature above third zone ? */
#else  /* _SUPPORT_AMBIENT_TEMP */
				if ( g_i16ChipTemperature > i16TemperatureEnd )					/* Temperature above third zone ? */
#endif /* _SUPPORT_AMBIENT_TEMP */
				{
					i16TemperatureBgn = i16TemperatureEnd;						/* Next zone; begin point (_3) */
					u16CurrThrshldRatioBgn = u16CurrThrshldRatioEnd;
					u8CurrThrshldCtrlType = (uint8)NVRAM_CURRTHRSHLD_ZONE_4;
					u16CurrThrshldRatioEnd = NVRAM_CURRTHRSHLD_RATIO_4;			/* Get zone end point (_3) */
					i16TemperatureEnd = NVRAM_CURRTHRSHLD_TEMP_4;
#if _SUPPORT_AMBIENT_TEMP
					if ( g_i16AmbjTemperature > i16TemperatureEnd )				/* Temperature above fourth zone ? */
#else  /* _SUPPORT_AMBIENT_TEMP */
					if ( g_i16ChipTemperature > i16TemperatureEnd )				/* Temperature above fourth zone ? */
#endif /* _SUPPORT_AMBIENT_TEMP */
					{
						u8CurrThrshldCtrlType = (uint8)NVRAM_CURRTHRSHLD_ZONE_5;
					}
				}
			}
			if ( u8CurrThrshldCtrlType == 1u )
			{
				u16CurrThrshldRatio = u16CurrThrshldRatioBgn;
			}
			else if ( u8CurrThrshldCtrlType == 2u )
			{
				u16CurrThrshldRatio = u16CurrThrshldRatioEnd;
			}
			else if ( u8CurrThrshldCtrlType == 3u )
			{
				i16TemperatureEnd = i16TemperatureEnd - i16TemperatureBgn;
#if _SUPPORT_AMBIENT_TEMP
				u16CurrThrshldRatio = (uint16) (muldivI16_I16byI16byI16( ((int16) u16CurrThrshldRatioEnd - (int16) u16CurrThrshldRatioBgn), (g_i16AmbjTemperature - i16TemperatureBgn), i16TemperatureEnd) + (int16)u16CurrThrshldRatioBgn);
#else  /* _SUPPORT_AMBIENT_TEMP */
				u16CurrThrshldRatio = (uint16) (muldivI16_I16byI16byI16( ((int16) u16CurrThrshldRatioEnd - (int16) u16CurrThrshldRatioBgn), (g_i16ChipTemperature - i16TemperatureBgn), i16TemperatureEnd) + (int16)u16CurrThrshldRatioBgn);
#endif /* _SUPPORT_AMBIENT_TEMP */
			}
			else
			{
				u16CurrThrshldRatio = 0u;										/* Shutdown motor */
			}
		}
		{
			uint16 u16MCurrgain = EE_GMCURR;
			g_u16PidHoldingThresholdADC = muldivU16_U16byU16byU16( g_u16PidHoldingThreshold, u16CurrThrshldRatio, u16MCurrgain);		/* MMP141209-1/MMP131219-1 */
			g_u16PidRunningThresholdADC = muldivU16_U16byU16byU16( g_u16PidRunningThreshold, u16CurrThrshldRatio, u16MCurrgain);		/* MMP141209-1/MMP131219-1 */
		}

} /* End of ThresholdControl */


#if _SUPPORT_AMBIENT_TEMP
/* ***
 * SelfHeatCompensation()
 *
 *	The self-heating compensation is split into two parts:
 *	1. Chip self-heating based on supply-voltage level (3-6C).
 *	2. Chip self-heating based on motor-current through FET's (up to 10-15C).
 * ***/
void SelfHeatCompensation( void)
{
	if ( g_u16SelfHeatingCounter >= 500u )
	{
		uint16 u16SelfHeating;

		g_u16SelfHeatingCounter -= 500u;

		l_u32SelfHeatingIntegrator = (uint32) (mulU32hi_U32byU16( l_u32SelfHeatingIntegrator, 64600U) + ((g_u16MotorCurrentMovAvgxN + 8u) >> 4u));
		u16SelfHeating = (uint16) mulU32hi_U32byU16( l_u32SelfHeatingIntegrator, 52U);

		/* Chip-self-heating based on supply-level: Approx 1C per 3V */
		g_i16AmbjTemperature = (g_i16ChipTemperature - (int16)u16SelfHeating) - (g_i16SupplyVoltage >> 8);
	}
} /* End of SelfHeatCompensation() */
#endif /* _SUPPORT_AMBIENT_TEMP */

/* EOF */
