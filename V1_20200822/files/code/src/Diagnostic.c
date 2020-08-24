/*! ----------------------------------------------------------------------------
 * \file		Diagnostic.c
 * \brief		MLX81300 Diagnostic handling
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	DiagnosticsInit()
 *				EXT4_IT()
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
#include "Diagnostic.h"
#include "main.h"
#include "ADC.h"																/* ADC support */
#include "ErrorCodes.h"															/* Error logging support */

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION (@TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp
#pragma space none

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	(@NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp
uint8 g_u8HallSwitchState = 0xFF;
uint16 g_u16HallMicroStepIdx = 0xFFFF;
#pragma space none

/* ****************************************************************************	*
 * Diagnostic initialisation
 *
 * Enable motor-driver automatically shut-off in case of over-current.
 * Don't automatically shut-off motor-driver on over- or under-voltage, or over-temperature.
 * Diagnostic ISR priority: 3 
 * ****************************************************************************	*/
void DiagnosticsInit( void)
{
	DRVCFG = (DRVCFG | (DIS_OC | DIS_OT | DIS_OV | DIS_UV));					/* Disable over-current, over-temperature, over-voltage, under-voltage */
	ANA_OUTI &= ~SEL_UV_VS;														/* Enable UV & OV debounce circuitry */
	ANA_OUTG = (ANA_OUTG & 0xFCFF) | (NVRAM_BROWNOUT_LEVEL << 8);				/* Brown-out UV-level = 6V + n * 1V; */
	/* MMP141212-1: Important note: Any OV/UV or OVT at power-up of the chip,
	 * will be cleared below and therefore not given an IRQ. OC should not
	 * happen as driver is not enabled. UV can also be caused by a slow ramp-up
	 * of the supply-voltage!! */
#if (_SUPPORT_DIAG_OVT == FALSE)
	ANA_OUTG |= INACTIVE_OVT;													/* MMP150409-2 */
#endif /* (_SUPPORT_DIAG_OVT == FALSE) */
#if (_SUPPORT_HALL_SENSOR)
	XI4_PEND = (C_DIAG_MASK | XI4_IO5);
	XI4_MASK |= (C_DIAG_MASK | XI4_IO5);									/* Enable second-level diagnostic interrupts and Hall-switch @ IO[0] */
	g_u8HallSwitchState = IO_IN & XI4_IO5;
	if ( g_u8HallSwitchState )
	{
		/* IO[0] is high; Set IRQ-event on falling-edge */
		IO_CFG |= FRB_IO5;
	}
	else
	{
		/* IO[0] is low; Set IRQ-event on rising-edge */
		IO_CFG &= ~FRB_IO5;
	}
#else  /* (_SUPPORT_HALL_SENSOR) */
	XI4_PEND = C_DIAG_MASK;
	XI4_MASK |= C_DIAG_MASK;
#endif /* (_SUPPORT_HALL_SENSOR) */
	/* PRIO = (PRIO & ~(3U << 14)) | ((3U - 3U) << 14); */						/* EXT4_IT Priority: 3 (3..6) */
	PRIO = (PRIO & ~(3U << 14));												/* EXT4_IT Priority: 3 (3..6) */
	PEND = CLR_EXT4_IT;
	MASK |= EN_EXT4_IT;															/* Enable Diagnostic Interrupt */

	/* MMP141212-1: Check for OVT and OV. Perform Diagnostics handling if required */
	{
#if (_SUPPORT_DIAG_OVT == FALSE)
		uint16 u16DiagnosticEvent = (ANA_INA & XI4_OV);
#else  /* (_SUPPORT_DIAG_OVT == FALSE) */
		uint16 u16DiagnosticEvent = (ANA_INA & (XI4_OVT | XI4_OV));
#endif /* (_SUPPORT_DIAG_OVT == FALSE) */
		if ( u16DiagnosticEvent != 0 )
		{
			HandleDiagnosticEvent( u16DiagnosticEvent);
		}
	}

} /* End of DiagnosticsInit() */

/* ****************************************************************************	*
 * HandleDiagnosticEvent()
 *
 * Handle Diagnostic Events
 * ****************************************************************************	*/
void HandleDiagnosticEvent( uint16 u16Event)
{
	if ( u16Event & (XI4_OC_DRV | XI4_OVT) )
	{
		/* In case over-current or over-temperature, switch off motor */
#if _SUPPORT_DIAG_OC
		if ( u16Event & XI4_OC_DRV )
		{
			/* Chip over-current */
			/* The over-current may occur then chips has entered test-mode. The test-mode freezes the
			 * complete digital part of the chip, including the Motor PWM and driver. This may result
			 * in a phase H and a phase L, having a DC-current flow!!
			 */
			if ( (g_e8MotorStatusMode == (uint8) C_MOTOR_STATUS_RUNNING) || (g_u8MotorHoldingCurrState != FALSE) )
			{
				/* Average between two driver-current measurements */
				NopDelay( DELAY_mPWM); /*lint !e522 */							/* Wait for ESD pulse to be gone and a new ADC measurement have been take place */
				g_i16Current = GetMotorDriverCurrent();
				if ( g_i16Current > 1400 )
				{
					//g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_YES;
					g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;// Ban, stop the motor in case of coil short
					MotorDriverStop( (uint16) C_STOP_EMERGENCY);				/* Over-current */
					//g_u16TargetPosition = g_u16ActualPosition;					/* 9.5.3.3 */
					SetLastError( (uint8) C_ERR_DIAG_OVER_CURRENT);
					g_e8ErrorCoil = (uint8) C_ERR_SELFTEST_B;
				}
			}
			else
			{
				//g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_YES;
				g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;// Ban, stop the motor in case of coil short
				MotorDriverStop( (uint16) C_STOP_EMERGENCY);					/* Over-current */
				//g_u16TargetPosition = g_u16ActualPosition;						/* 9.5.3.3 */
				SetLastError( (uint8) C_ERR_DIAG_OVER_CURRENT);
				g_e8ErrorCoil = (uint8) C_ERR_SELFTEST_B;
			}
		}
#endif /* _SUPPORT_DIAG_OC */
		if ( u16Event & XI4_OVT )
		{
			/* Chip over-temperature */
			if ( (g_e8MotorStatusMode == (uint8) C_MOTOR_STATUS_RUNNING) || (g_u8MotorHoldingCurrState != FALSE) )
			{
				ResetChipTemperature();
				NopDelay( DELAY_mPWM); /*lint !e522 */
				GetChipTemperature( FALSE);										/* MMP131020-1 */
			}
			else
			{
				MeasureVsupplyAndTemperature();
				GetChipTemperature( FALSE);										/* MMP131020-1 */
			}
			if ( (g_i16ChipTemperature > (int16) C_CHIP_OVERTEMP_LEVEL) && (g_e8ErrorOverTemperature != (uint8) C_ERR_OTEMP_YES))
			{
				g_u8OverTemperatureCount++;
				if ( g_u8OverTemperatureCount >= (uint8) C_OVERTEMP_TO_PERMDEFECT_THRSHLD )
				{
					g_e8ErrorOverTemperature = (uint8) C_ERR_OTEMP_YES;
					SetLastError( (uint8) C_ERR_DIAG_OVER_TEMP);
					//g_u8OverTemperatureCount = 0;
				}
			}
		}
	}
	if ( u16Event & (XI4_UV | XI4_OV) )
	{
		/* Chip under- or over-voltage */
		if ( (g_e8MotorStatusMode == (uint8) C_MOTOR_STATUS_RUNNING) || (g_u8MotorHoldingCurrState != FALSE) )
		{
			/* Average between two supply-voltage measurements */
			NopDelay( DELAY_mPWM); /*lint !e522 */								/* Wait for ESD pulse to be gone and a new ADC measurement have been take place */
			GetVsupply();
		}
		else
		{
			MeasureVsupplyAndTemperature();
			GetVsupply();
		}
		{
			uint8 e8DiagVoltage = (uint8) C_ERR_VOLTAGE_IN_RANGE;
			if ( g_i16SupplyVoltage < (int16)((6 + NVRAM_BROWNOUT_LEVEL) * 100) )
			{
				/* Chip under-voltage */
				e8DiagVoltage = (uint8) C_ERR_VOLTAGE_UNDER;
				SetLastError( (uint8) C_ERR_DIAG_UNDER_VOLT);
			}
			else if ( g_i16SupplyVoltage > 2800 )
			{
				/* Chip over-voltage */
				e8DiagVoltage = (uint8) C_ERR_VOLTAGE_OVER;
				SetLastError( (uint8) C_ERR_DIAG_OVER_VOLT);
			}
			if ( e8DiagVoltage != (uint8) C_ERR_VOLTAGE_IN_RANGE)
			{
				g_e8ErrorVoltage = e8DiagVoltage;								/* 9.5.3.4 */
				g_e8ErrorVoltageComm = g_e8ErrorVoltage;
				if ( g_e8MotorRequest != C_MOTOR_REQUEST_NONE )					/* MMP150313-3 - Begin */
				{
					g_e8DegradedMotorRequest = g_e8MotorRequest;
					g_e8MotorRequest = C_MOTOR_REQUEST_NONE;
					MotorDriverStop( (uint16) C_STOP_EMERGENCY);				/* Under/Over-voltage */
				}
				else if ( g_e8MotorStatusMode == (uint8) C_MOTOR_STATUS_RUNNING )	/* MMP150313-3 - End */
				{
					/* Enter degraded-mode; Stop motor and resume when voltage decreases below upper-application threshold or raise above lower-application threshold */
					g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_START;
					MotorDriverStop( (uint16) C_STOP_EMERGENCY);				/* Under/Over-voltage */
				}
				else if ( g_e8DegradedMotorRequest == (uint8) C_MOTOR_REQUEST_NONE )
				{
					g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_STOP;
					MotorDriverStop( (uint16) C_STOP_EMERGENCY);				/* Degraded-mode */
				}
				g_e8MotorStatusMode |= (uint8) C_MOTOR_STATUS_DEGRADED;
			}
		}
	}
} /* End of HandleDiagnosticEvent() */

/* ****************************************************************************	*
 * EXT4_IT()
 *
 * Diagnostic & IO Interrupt Service Routine.
 * ****************************************************************************	*/
__interrupt__ void EXT4_IT(void)
{
	uint16 u16Pending = (XI4_PEND & XI4_MASK);									/* Copy interrupt requests which are not masked   */
	do
	{
		XI4_PEND = u16Pending;													/* Clear requests which are going to be processed */
	} while (XI4_PEND & u16Pending);

	/* Multiple diagnostics events is most likely caused by Rinnen/ESD-pulse */
	if ( ((u16Pending & XI4_UV) != 0) && ((u16Pending & (XI4_OC_DRV | XI4_OVT | XI4_OV)) != 0) )
	{
		/* Under-voltage together with any other diagnostic event is strange */
	}
	else
	{
		HandleDiagnosticEvent( u16Pending);

		if ( (u16Pending & XI4_IO5) != 0 )
		{
			g_u8HallSwitchState = IO_IN & XI4_IO5;
			if ( g_u8HallSwitchState )
			{
				/* IO[0] is high; Set IRQ-event on falling-edge */
				IO_CFG |= FRB_IO5;
			}
			else
			{
				/* IO[0] is low; Set IRQ-event on rising-edge */
				IO_CFG &= ~FRB_IO5;
			}
			//g_u16HallMicroStepIdx = g_u16MicroStepIdx;
			g_u16HallMicroStepIdx = g_u16ActuatorActPos;//Ban
		}
	}
} /* EXT4_IT() */

/* EOF */
