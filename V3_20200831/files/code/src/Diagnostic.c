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
#include "ADC.h"																/* ADC support */
#include "ErrorCodes.h"															/* Error logging support */
#include "MotorDriver.h"
#include "MotorDriverTables.h"
#include "Timer.h"
#include <Private_mathlib.h>


/* Debounce error filter; An error has to be detected twice in a row */
#define C_DEBFLT_ERR_NONE					0x00U
#define C_DEBFLT_ERR_PHASE_SHORT			0x01U								/* Bit 0: Phase short to ground error */
#define C_DEBFLT_ERR_OVT					0x02U								/* Bit 1: Over-Temperature error */
#define C_DEBFLT_ERR_UV						0x04U								/* Bit 2: Under-Voltage error */
#define C_DEBFLT_ERR_OV						0x08U								/* Bit 3: Over-Voltage error */
#define C_DEBFLT_ERR_TEMP_PROFILE			0x10U								/* Bit 4: Chip Temperature profile error */
#define C_DEBFLT_ERR_OVTS					0x20U								/* Bit 5: Over-Temperature shutdown error */



/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION (@TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp
#pragma space none

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	(@NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp
uint16 g_u16HallMicroStepIdx = 0xFFFFu;

uint16 l_u16HallSwitchState = 0xFFu;
uint8  l_u8DriftCheckCount = 0u;

uint8  l_e8ErrorDebounceFilter = C_DEBFLT_ERR_NONE;								/* Debounce filter */
uint8  l_u8UOVoltageCount = 0u;													/* Under/Over temperature count */
uint8  l_u8OverTemperatureWarnCount = 0u;										/* Number of over-temperature events */
uint8  l_u8OverTemperatureShutCount = 0u;

uint16 l_u16CoilZeroCurrCountA = 0u;
uint16 l_u16CoilZeroCurrCountB = 0u;
uint16 l_u16CoilCurrentStartDelay = 0u;



#pragma space none


void HandleDiagnosticEvent( uint16 u16Event);


void MotorDiagnosticDrift(void)
{
	if ( g_u8MotorStartupMode == (uint8) MSM_STOP ) 
	{
		l_u8DriftCheckCount++;
		if (l_u8DriftCheckCount >= C_DRIFT_DEBOUNCE_THR)
		{
			l_u8DriftCheckCount = 0u;
			g_sMotorFault.DRIFT = 1u;
		}
	}
    else
    {
		l_u8DriftCheckCount = 0u;
    }

}


/* ****************************************************************************	*
 * Diagnostic initialisation
 *
 * Enable motor-driver automatically shut-off in case of over-current.
 * Don't automatically shut-off motor-driver on over- or under-voltage, or over-temperature.
 * Diagnostic ISR priority: 3 
 * ****************************************************************************	*/
void DiagnosticsInit( void)
{
	DRVCFG |= (DIS_OC | DIS_OT | DIS_OV | DIS_UV);					/* Disable over-current, over-temperature, over-voltage, under-voltage */
	ANA_OUTI &= ~SEL_UV_VS;											/* Enable UV & OV debounce circuitry */
	ANA_OUTG &= 0xFCFFu;												/* Brown-out UV-level = 6V + n * 1V; */
	ANA_OUTG |= (NVRAM_BROWNOUT_LEVEL << 8u);				
	/* MMP141212-1: Important note: Any OV/UV or OVT at power-up of the chip,
	 * will be cleared below and therefore not given an IRQ. OC should not
	 * happen as driver is not enabled. UV can also be caused by a slow ramp-up
	 * of the supply-voltage!! */
#if (_SUPPORT_DIAG_OVT == FALSE)
	ANA_OUTG |= INACTIVE_OVT;													/* MMP150409-2 */
#endif /* (_SUPPORT_DIAG_OVT == FALSE) */

#if (_SUPPORT_HALL_SENSOR)
	XI4_PEND = (C_DIAG_MASK | XI4_IO5);
	XI4_MASK |= (C_DIAG_MASK | XI4_IO5);										/* Enable second-level diagnostic interrupts and Hall-switch @ IO[0] */
	l_u16HallSwitchState = (IO_IN & XI4_IO5);
	if ( l_u16HallSwitchState != 0u)
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
	/* PRIO = (PRIO & ~(3U << 14)) | ((3U - 3U) << 14); */							/* EXT4_IT Priority: 3 (3..6) */
	PRIO &= ~((uint16)3u << 14u);												/* EXT4_IT Priority: 3 (3..6) */
	PRIO |= ((uint16)(3u - 3u) << 14u);
	PEND = CLR_EXT4_IT;
	MASK |= EN_EXT4_IT;															/* Enable Diagnostic Interrupt */

	/* MMP141212-1: Check for OVT and OV. Perform Diagnostics handling if required */
	{
#if (_SUPPORT_DIAG_OVT == FALSE)
		uint16 u16DiagnosticEvent = (ANA_INA & XI4_OV);
#else  /* (_SUPPORT_DIAG_OVT == FALSE) */
		uint16 u16DiagnosticEvent = (ANA_INA & (XI4_OVT | XI4_OV));
#endif /* (_SUPPORT_DIAG_OVT == FALSE) */
		if ( u16DiagnosticEvent != 0u )
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
	if ( (u16Event & (uint16)(XI4_OC_DRV | XI4_OVT)) != 0u )
	{
		/* In case over-current or over-temperature, switch off motor */
#if _SUPPORT_DIAG_OC
		if ( (u16Event & (uint16)XI4_OC_DRV) != 0u )
		{
			/* Chip over-current */
			/* The over-current may occur then chips has entered test-mode. The test-mode freezes the
			 * complete digital part of the chip, including the Motor PWM and driver. This may result
			 * in a phase H and a phase L, having a DC-current flow!!
			 */
			if (g_u8MotorStartupMode != (uint8) MSM_STOP)
			{
				/* Average between two driver-current measurements */
				NopDelay( DELAY_mPWM); /*lint !e522 */								/* Wait for ESD pulse to be gone and a new ADC measurement have been take place */
				g_i16Current = GetMotorDriverCurrent();
				if ( g_i16Current > 1400 )
				{
					DRVCFG_DIS_UVWT();											/* Over Current */
					DRVCFG_DIS();																															
					g_sMotorFault.SHORT = 1u;
					SetLastError( (uint8) C_ERR_DIAG_OVER_CURRENT);
				}
			}
			else
			{
				DRVCFG_DIS_UVWT();												/* Over-current,holding current mode? */
				DRVCFG_DIS();	
				g_sMotorFault.SHORT = 1u;
				SetLastError( (uint8) C_ERR_DIAG_OVER_CURRENT);
			}
		}
#endif /* _SUPPORT_DIAG_OC */
		if ( (u16Event & (uint16)XI4_OVT) != 0u )
		{
			DRVCFG_DIS_UVWT();													/* Over-temperature shutdown,170+-15 */
			DRVCFG_DIS();
			g_sMotorFault.TS = 1u;
			SetLastError( (uint8) C_ERR_DIAG_OVER_TEMP);

			ResetChipTemperature();
			NopDelay( DELAY_mPWM); /*lint !e522 */
		}
	}
	
	if ( (u16Event & (XI4_UV | XI4_OV)) != 0u )
	{

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
	} while ((XI4_PEND & u16Pending) != 0u);

	/* Multiple diagnostics events is most likely caused by Rinnen/ESD-pulse */
	if ( ((u16Pending & XI4_UV) != 0u) && ((u16Pending & (XI4_OC_DRV | XI4_OVT | XI4_OV)) != 0u) )
	{
		/* Under-voltage together with any other diagnostic event is strange */
	}
	else
	{
		HandleDiagnosticEvent( u16Pending);

		if ( (u16Pending & XI4_IO5) != 0u )
		{
			l_u16HallSwitchState = (IO_IN & XI4_IO5);
			if ( l_u16HallSwitchState != 0u )
			{
				/* IO[0] is high; Set IRQ-event on falling-edge */
				IO_CFG |= FRB_IO5;
			}
			else
			{
				/* IO[0] is low; Set IRQ-event on rising-edge */
				IO_CFG &= ~FRB_IO5;
			}

#if _SUPPORT_DRIFT_CHECK
			MotorDiagnosticDrift();	
#endif	/* _SUPPORT_DRIFT_CHECK */

			g_u16HallMicroStepIdx = g_u16ActuatorActPos;
		}
	}
} /* EXT4_IT() */

void MotorDiagnosticCheckInit(void)
{
	l_u16CoilCurrentStartDelay = 2u * C_MOVAVG_SZ;
	l_u16CoilZeroCurrCountA = 0u;
	l_u16CoilZeroCurrCountB = 0u;
}

uint8  MotorDiagnosticOpenCheck(void)
{
	
	if(l_u16CoilCurrentStartDelay == 0u)
	{
		if( g_u16CurrentMotorCoilA <  C_MIN_COIL_CURRENT )
		{
			l_u16CoilZeroCurrCountA++;
		}
		else
		{
			if ( l_u16CoilZeroCurrCountA > 0u )
			{
				l_u16CoilZeroCurrCountA--;
			}
		}

		if( g_u16CurrentMotorCoilB < C_MIN_COIL_CURRENT  )
		{
			l_u16CoilZeroCurrCountB++;
		}
		else
		{
			if ( l_u16CoilZeroCurrCountB > 0u )
			{
				l_u16CoilZeroCurrCountB--;
			}
		}

		if((l_u16CoilZeroCurrCountA >= C_COIL_ZERO_CURRENT_COUNT) || (l_u16CoilZeroCurrCountB >= C_COIL_ZERO_CURRENT_COUNT))
		{
			g_sMotorFault.OPEN = 1u;
			SetLastError( (uint8) C_ERR_COIL_ZERO_CURRENT);
			return 1;
		}
	}
	else
	{
		if(l_u16CoilCurrentStartDelay > 0u)
		{
			l_u16CoilCurrentStartDelay--;
		}
	}

	return 0;
}

void MotorDiagnosticVsupplyAndTemperature(void)
{
	
	int16 voltage_raw;

	/* Perform Vbat/Temperature measurement incase motor is stopped */
	if(g_u8MotorStartupMode == (uint8) MSM_STOP)	
	{
		MeasureVsupplyAndTemperature();
	}
	/***************************************************
	 * p. Motor over/under temperature
	***************************************************/
	/* Calculate Voltage (100LSB/V) [10mV] */
	GetVsupply();
	GetVsupplyMotor();
	/* supply voltage is compensated with the shorty diode voltage drop and inductor voltage drop 
		 * in motor run(current peak) and motor stop(current valley) conditions.
	  */
	if (g_u8MotorStartupMode  != (uint8) MSM_STOP )
	{
		voltage_raw = g_i16MotorVoltage + C_VDS_RUN;
	}
	else
	{
		voltage_raw = g_i16MotorVoltage + C_VDS_STOP;
	}

	if ( voltage_raw < (NVRAM_APPL_UVOLT - C_VOLTAGE_HYS) )
	{
		voltage_raw = (NVRAM_APPL_UVOLT - C_VOLTAGE_HYS);
		/* First time application under-voltage error */					/* MMP150128-1 - Begin */
		if ( (l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_UV) == 0x00u )
		{
			/* Need twice a under-voltage detection, to avoid ESD-pulses disturbance will cause degraded mode entering */
			l_e8ErrorDebounceFilter |= (uint8) C_DEBFLT_ERR_UV;
			l_u8UOVoltageCount = 0u;
		}
		else
		{
			l_u8UOVoltageCount++;
			if(l_u8UOVoltageCount >= C_UOV_DEBOUNCE_THR)
			{
				l_u8UOVoltageCount = C_UOV_DEBOUNCE_THR;
				g_sMotorFault.UV = 1u; 								/* 9.5.3.4 */
			}
		}
	}
	else if ( voltage_raw > (NVRAM_APPL_OVOLT + C_VOLTAGE_HYS) )
	{
		/* First time application over-voltage error */
		if ( (l_e8ErrorDebounceFilter & C_DEBFLT_ERR_OV) == 0x00u )
		{
			/* Need twice a over-voltage detection, to avoid ESD-pulses disturbance will cause degraded mode entering */
			l_e8ErrorDebounceFilter |= (uint8) C_DEBFLT_ERR_OV;
			l_u8UOVoltageCount = 0u;
		}
		else
		{
			l_u8UOVoltageCount++;
			if(l_u8UOVoltageCount >= C_UOV_DEBOUNCE_THR)
			{
				l_u8UOVoltageCount = C_UOV_DEBOUNCE_THR;
				g_sMotorFault.OV = 1u;									/* 9.5.3.4 */
			}
		}
	}
	else if ( (voltage_raw >= NVRAM_APPL_UVOLT) && (voltage_raw <= NVRAM_APPL_OVOLT) )
	{
		if((l_e8ErrorDebounceFilter & (C_DEBFLT_ERR_UV | C_DEBFLT_ERR_OV)) != 0u)
		{
			l_e8ErrorDebounceFilter &= (uint8) ~(C_DEBFLT_ERR_UV | C_DEBFLT_ERR_OV);
			l_u8UOVoltageCount = 0u;
		}
		else
		{
			l_u8UOVoltageCount++;
			if(l_u8UOVoltageCount >= C_UOV_DEBOUNCE_THR)
			{
				l_u8UOVoltageCount = C_UOV_DEBOUNCE_THR;
				g_sMotorFault.UV = 0u;
				g_sMotorFault.OV = 0u;
			}
		}
	}
	else
	{
	
	}
	
	/***************************************************
	 * p. Motor over temperature check 
	***************************************************/
	/* Calculate Chip internal temperature (1LSB/C) [C] */
	GetChipTemperature( FALSE)												/* MMP131020-1 */;
	/* Thermal Shutdown */
	
	if ( g_i16ChipTemperature > NVRAM_APPL_OTEMP_SHUT )
    {
		if ( (l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_OVTS) == 0x00u )
		{
			/* Need twice a over-temperature detection, to avoid ESD-pulses disturbance will cause degraded mode entering */
			l_e8ErrorDebounceFilter |= (uint8) C_DEBFLT_ERR_OVTS;
			l_u8OverTemperatureShutCount = 0u;
		}
		else
		{
			l_u8OverTemperatureShutCount++;
			if ( l_u8OverTemperatureShutCount >= (uint8) C_OVT_DEBOUNCE_THR )
			{
				l_u8OverTemperatureShutCount = C_OVT_DEBOUNCE_THR;
				g_sMotorFault.TS = 0x01u;
				SetLastError( (uint8) C_ERR_APPL_OVER_TEMP);
			}
		}
	}
	else if ( g_i16ChipTemperature < (int16) (NVRAM_APPL_OTEMP_SHUT - C_TEMPERATURE_HYS) )
	{
		if(l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_OVTS)
		{
			l_e8ErrorDebounceFilter &= (uint8) ~C_DEBFLT_ERR_OVTS;
			l_u8OverTemperatureShutCount = 0u;
		}
		else
		{
			l_u8OverTemperatureShutCount++;
			if(l_u8OverTemperatureShutCount >= C_OVT_DEBOUNCE_THR)
			{
				l_u8OverTemperatureShutCount = C_OVT_DEBOUNCE_THR;
				g_sMotorFault.TS = 0u;	
			}
		}
	}
	else
	{

	}

	
	
	/* Thermal Warning */
	if ( g_i16ChipTemperature > NVRAM_APPL_OTEMP_WARN )
    {
		if ( (l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_OVT) == 0x00 )
		{
			/* Need twice a over-temperature detection, to avoid ESD-pulses disturbance will cause degraded mode entering */
			l_e8ErrorDebounceFilter |= (uint8) C_DEBFLT_ERR_OVT;
			l_u8OverTemperatureWarnCount = 0u;
		}
		else
		{
			l_u8OverTemperatureWarnCount++;
			if ( l_u8OverTemperatureWarnCount >= (uint8) C_OVT_DEBOUNCE_THR )
			{
				l_u8OverTemperatureWarnCount = C_OVT_DEBOUNCE_THR;
				g_sMotorFault.TW = 0x01u;
			}
		}
	}
	else if ( g_i16ChipTemperature < (int16) (NVRAM_APPL_OTEMP_WARN - C_TEMPERATURE_HYS) )
	{
		if(l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_OVT)
		{
			l_e8ErrorDebounceFilter &= (uint8) ~C_DEBFLT_ERR_OVT;
			l_u8OverTemperatureWarnCount = 0u;
		}
		else
		{
			l_u8OverTemperatureWarnCount++;
			if(l_u8OverTemperatureWarnCount >= C_OVT_DEBOUNCE_THR)
			{
				l_u8OverTemperatureWarnCount = C_OVT_DEBOUNCE_THR;
				g_sMotorFault.TW = 0u;
			}
		}
	}
	else
	{

	}

	/* *********************************************** */
	/* *** p. Motor-phase shortage to ground check *** */
	/* *********************************************** */
#if _SUPPORT_PHASE_SHORT_DET
	
#endif /* _SUPPORT_PHASE_SHORT_DET */
}

#if _SUPPORT_MOTOR_SELFTEST
/* ****************************************************************************	*
 * MotorDriverSelfTest()
 *
 * Self test Motor Driver
 * Note: Diagnostics must be initialised before calling this self-test.
 *
 * 1. Test FET short with Ground or Vsupply
 * 2. Test Motor-phase short
 * 3. Test Open connection with motor-phase
 * 4. Test BEMF Voltage levels
 * ****************************************************************************	*/
void MotorDiagnosticSelfTest( void)
{
	uint16 u16SelfTestIdx;
	uint16 u16VdsThreshold;														/* MMP130919-1/MMP140403-1 */
	T_ADC_SELFTEST_4PH adcMotorSelfTest4Ph;
	uint16 u16Pwm2Storage = PWM2_CMP;											/* MMP150219-2: Save PWM2 ADC trigger CMP time */

	PWM2_CMP = (((50L * PWM_REG_PERIOD) + 50)/100);								/* MMP150219-2: Set PWM2 ADC trigger CMP time at 50% or period */

	MeasureVsupplyAndTemperature();												/* MMP130919-1 - Begin */
	GetVsupplyMotor();
	if ( NVRAM_VDS_THRESHOLD != 0u )
	{
		u16VdsThreshold = NVRAM_VDS_THRESHOLD;
	}
	else
	{
		u16VdsThreshold = 200U;
	}																			/* MMP130919-1 - End */

	/* Test for FET shortages; Note: Diagnostics configuration will switch off driver at over-current */
	for ( u16SelfTestIdx = 0u; (g_sMotorFault.SHORT == 0u) && (u16SelfTestIdx < (sizeof(c_au8DrvCfgSelfTestA)/sizeof(c_au8DrvCfgSelfTestA[0]))); u16SelfTestIdx++ )
	{
		int16 i16DriverCurrent = 0;												/* MMP140403-1 */

		DRVCFG_CNFG_UVWT((uint16) c_au8DrvCfgSelfTestA[u16SelfTestIdx]);		/* MMP130904-1 */

		MeasurePhaseVoltage( (uint16)c_au16DrvAdcSelfTestA[u16SelfTestIdx>>1u]);	/* MMP140403-1/MMP130919-1 - Begin */
		if ( (u16SelfTestIdx & 1u) == 0u )
		{
			/* Even-index (0,2,4,6) are phase to ground: Check current too (< 20 mA) */
			MeasureMotorCurrent();
			i16DriverCurrent = GetMotorDriverCurrent();
		}
		/* Even-index (0,2,4,6) are phase to ground: Vphase < Vds; Odd-index (1,3,5,7) are phase to supply: Vphase > (Vsup - Vds) */
		if ( (g_sMotorFault.SHORT != 0u) ||
			(((u16SelfTestIdx & 1u) == 0u) && ((g_i16PhaseVoltage > (int16)u16VdsThreshold) || (i16DriverCurrent > 20))) ||
			(((u16SelfTestIdx & 1u) != 0u) && (g_i16PhaseVoltage < (int16)(g_i16MotorVoltage - u16VdsThreshold))) )
		{																		/* MMP130919-1 - End */
			/* Over-current trigger; Phase makes short with supply or Ground */
			g_sMotorFault.SHORT = 1u;
			SetLastError( (uint8) C_ERR_SELFTEST_A);
			break;
		}
	}

	/* Convert Vds-voltage (10mV units) to ADC-LSB */
	u16VdsThreshold = muldivU16_U16byU16byU16( u16VdsThreshold, C_GVOLTAGE_DIV, EE_GADC);

	for ( u16SelfTestIdx = 0; (g_sMotorFault.SHORT == 0u) && (u16SelfTestIdx < (sizeof(c_au8DrvCfgSelfTestB4)/sizeof(c_au8DrvCfgSelfTestB4[0]))); u16SelfTestIdx++ )
	{
		/* Test for open connections, damaged coil(s) */
		uint16 u16Vsm;
		uint16 u16VphH;
		uint16 u16VphL;
		uint16 u16Vds;
		uint16 u16MotorCoilCurrent;
		register uint16 u16DC;
		
		if ( (u16SelfTestIdx & 0x02u) != 0u )
		{
			/* Phase LOW + phase -PWM */
			u16DC = (PWM_REG_PERIOD >> 3u);
		}
		else
		{
			/* Phase HIGH + phase PWM */
			u16DC = PWM_REG_PERIOD - (PWM_REG_PERIOD >> 3u);					/* Approx. 12.5% */
		}
		
		u16DC = u16DC / 2u;
		PWM2_LT = u16DC;														/* Copy the results into the PWM register for phase U */
		PWM2_HT = PWM_REG_PERIOD - u16DC;
		PWM3_LT = u16DC;														/* Copy the results into the PWM register for phase V */
		PWM3_HT = PWM_REG_PERIOD - u16DC;
		PWM4_LT = u16DC;														/* Copy the results into the PWM register for phase W */
		PWM4_HT = PWM_REG_PERIOD - u16DC;
		PWM5_LT = u16DC;														/* Copy the results into the PWM register for phase T */
		PWM5_HT = PWM_REG_PERIOD - u16DC;
		PWM1_LT = u16DC;														/* Master must be modified at last (value is not important) */

		/* the test table depends on the connection type,GM CV is UW-VT */
		DRVCFG_CNFG_UVWT( (uint16) c_au8DrvCfgSelfTestB4[u16SelfTestIdx]);
		
		if ( (u16SelfTestIdx & 0x02u) != 0u )
		{
			MeasureSelfTest4PH((uint16) tAdcSelfTest4B, &adcMotorSelfTest4Ph);	/* Phase = Low */							
		}
		else
		{
			MeasureSelfTest4PH((uint16) tAdcSelfTest4A, &adcMotorSelfTest4Ph);	/* Phase = High */							
		}
		
		DRVCFG_GND_UVWT();

		if ( g_sMotorFault.SHORT != 0u )										/* Over-current ? */
		{
			/* Over-current trigger; Phase makes short with other phase */
			g_sMotorFault.SHORT = 1u;											/* Ban, phase coil short */
			SetLastError( C_ERR_SELFTEST_B);
			break;
		}

		if ( (u16SelfTestIdx & 0x02u) != 0u )
		{
			/* Use tAdcSelfTest4B */
			u16Vsm = adcMotorSelfTest4Ph.UnfilteredDriverCurrent;				/* Current becomes voltage */
			u16MotorCoilCurrent = adcMotorSelfTest4Ph.FilteredSupplyVoltage;
		}
		else
		{
			/* Use tAdcSelfTest4A */
			u16Vsm = adcMotorSelfTest4Ph.FilteredSupplyVoltage;
			u16MotorCoilCurrent = adcMotorSelfTest4Ph.UnfilteredDriverCurrent;
		}
		/* phase voltage */
		switch ( u16SelfTestIdx & ~0x02u )
		{
		case 0:
			u16VphH = adcMotorSelfTest4Ph.PhaseU_HighVoltage;					/* Phase U-PWM */
			u16VphL = adcMotorSelfTest4Ph.PhaseU_LowVoltage;
			break;
		case 1:
			u16VphH = adcMotorSelfTest4Ph.PhaseV_HighVoltage;					/* Phase V-PWM */
			u16VphL = adcMotorSelfTest4Ph.PhaseV_LowVoltage;
			break;
		case 4:
			u16VphH = adcMotorSelfTest4Ph.PhaseW_HighVoltage;					/* Phase W-PWM */
			u16VphL = adcMotorSelfTest4Ph.PhaseW_LowVoltage;
			break;
		case 5:
			u16VphH = adcMotorSelfTest4Ph.PhaseT_HighVoltage;					/* Phase T-PWM */
			u16VphL = adcMotorSelfTest4Ph.PhaseT_LowVoltage;
			break;
		case 8:
		case 9:
			u16VphH = adcMotorSelfTest4Ph.PhaseU_HighVoltage;					/* Phase U-PWM */
			u16VphL = adcMotorSelfTest4Ph.PhaseU_LowVoltage;
		default:
			break;
		}
		
		/* voltage drop on the FET's */
		if ( u16Vsm > u16VphH )
		{
			u16Vds = u16Vsm - u16VphH;
		}
		else
		{
			u16Vds = u16VphH - u16Vsm;
		}
		
#if _SUPPORT_COIL_RESISTANCE_CHECK
		/* Vds phase phase resistance check */
		if(u16Vds > u16VdsThreshold)
		{
			g_sMotorFault.OPEN = 1u;											
			SetLastError( C_ERR_SELFTEST_D);									/* phase (upper) resistance too big */
			break;
		}

		if(u16VphL > u16VdsThreshold)
		{
			g_sMotorFault.OPEN = 1u;											
			SetLastError( C_ERR_SELFTEST_E);									/* phase (lower) resistance too small */
			break;
		}
#endif	
		extern uint16 l_u16CurrentZeroOffset;
		if ( u16MotorCoilCurrent  < (l_u16CurrentZeroOffset + C_MIN_COIL_CURRENT) )
		{
			/* No current (less than 10 LSB's); Coil Open */
			g_sMotorFault.OPEN = 1u;
			SetLastError( C_ERR_SELFTEST_C);									/* phase open */
			break;
		}
	}	

	DRVCFG_DIS_UVWT();
	DRVCFG_DIS();																/* MMP140903-1 */
	
	PWM2_LT = PWM_SCALE_OFFSET;													/* 50% PWM duty cycle for phase U */
	PWM3_LT = PWM_SCALE_OFFSET;													/* 50% PWM duty cycle for phase V */
	PWM4_LT = PWM_SCALE_OFFSET;													/* 50% PWM duty cycle for phase W */
	PWM5_LT = PWM_SCALE_OFFSET;													/* 50% PWM duty cycle for phase T */
	PWM1_LT = PWM_SCALE_OFFSET;													/* Master must be modified at last (value is not important) */
	
	PWM2_CMP = u16Pwm2Storage;													/* MMP150219-2: Restore PWM2 ADC trigger CMP time */

} /* End of MotorDriverSelfTest() */
#endif /* _SUPPORT_MOTOR_SELFTEST */


/* EOF */
