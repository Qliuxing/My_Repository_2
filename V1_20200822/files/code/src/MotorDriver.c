/*! ----------------------------------------------------------------------------
 * \file		MotorDriver.c
 * \brief		MLX81300 Motor driver handling
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	MotorDriverInit()
 *				MotorDriverSelfTest()
 *				MotorDriverCurrentMeasureInit()
 *				MotorDriverCurrentMeasure()
 *				MotorDriver_InitialPwmDutyCycle()
 *				MotorDriver_4PhaseStepper()
 *				MotorDriverStart()
 *				MotorDriverStop()
 *				Commutation_ISR()
 *
 * MELEXIS Microelectronic Integrated Systems
 * 
 * Copyright (C) 2012-2014 Melexis N.V.
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
#include <syslib.h>
#include "MotorDriver.h"
#include "main.h"
#include "ADC.h"																/* ADC support */
#include "ErrorCodes.h"															/* Error logging support */
#include "MotorStall.h"															/* Motor stall detector support */
#include "MotorDriverTables.h"
#include "NVRAM_UserPage.h"														/* NVRAM User-page support */
#include "PID_Control.h"														/* PID-controller support */
#include "Timer.h"																/* Periodic IRQ Timer support */
#include <awdg.h>																/* Analogue Watchdog support */
#include "private_mathlib.h"
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */
#if _DEBUG_SPI
#include "SPI_Debug.h"
#include "Diagnostic.h"
#endif /* _DEBUG_SPI */

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION (@TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp
volatile uint16 g_u16CorrectionRatio;											/* Motor correction ratio, depend on temperature and voltage */
uint16 g_u16MicroStepIdx;														/* (Micro)step index */
uint16 g_u16CommutTimerPeriod;													/* Commutation timer period */
uint16 g_u16TargetCommutTimerPeriod;											/* Target commutation timer period (target speed) */
uint16 g_u16StartupDelay = 2*C_MOVAVG_SZ;
uint16 g_u16MotorCurrentMovAvgxN;												/* Moving average current (4..16 samples) */
uint16 g_u16MotorCurrentLPFx64;													/* Low-pass filter (IIR-1) motor-current x 64 */
uint8 g_u8MotorStopDelay = 0;													/* Delay between drive stage from LS to TRI-STATE */
uint8 l_u8VTIdx = 0;
/* MMP151118-2 */
volatile uint16 g_u16ActuatorActPos __attribute__ ((section(".dp.noinit")));	/* (Motor-driver) Actual motor-rotor position [WD] */
uint16 g_u16ActuatorTgtPos __attribute__ ((section(".dp.noinit")));				/* (Motor-driver) Target motor-rotor position [WD] */
#pragma space none

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	(@NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp																/* __NEAR_SECTION__ */
uint8 g_u8MotorStartupMode = (uint8) MSM_STOP;									/* 4: Motor STOP state */
uint8 g_u8MotorHoldingCurrState = FALSE;										/* Motor Holding Current State */
uint16 g_u16MotorSpeedRPS;														/* 4: Target motor-speed [RPS] */
int16 g_i16VoltStepIdx = -1;

uint16 l_au16MotorCurrentRaw[C_MOVAVG_SZ];
uint16 l_u16MotorCurrentRawIdx;
uint16 l_u16StartupDelayInit = 2*C_MOVAVG_SZ;									/* Initial startup-delay [uSteps] */

uint16 l_u16SpeedRPM;															/* MMP160606-1 */
uint32 l_u32Temp;																/* MMP160606-1 */
uint16 l_u16LowSpeedPeriod;														/* MMP160606-1 */

uint16 g_au16MotorSpeedCommutTimerPeriod[8];
uint16 g_au16MotorSpeedRPS[8];
uint16 g_u16MotorMicroStepsPerElecRotation;										/* Number of micro-steps per electric rotation */
uint16 g_u16MotorMicroStepsPerMechRotation;										/* Number of micro-steps per mechanical rotation */
uint16 g_u16MotorRewindSteps;
uint16 l_u16CoilZeroCurrCountA = 0;
uint16 l_u16CoilZeroCurrCountB = 0;
uint16 l_u16CoilCurrentStartDelay = 0;

#if (USE_MULTI_PURPOSE_BUFFER == FALSE)
uint16 l_au16VelocityTimer[VT_BUF_SZ];
#endif /* (USE_MULTI_PURPOSE_BUFFER == FALSE) */

#if _DEBUG_MOTOR_CURRENT_FLT
uint8 l_au8MotorCurrRaw[C_MOTOR_CURR_SZ];										/* Raw (un-filtered) motor current measurement */
uint16 l_u16MotorCurrIdx;														/* Motor current measurement index */
#endif /* _DEBUG_MOTOR_CURRENT_FLT */
#pragma space none																/* __NEAR_SECTION__ */

#define C_MIN_COIL_CURRENT			10		/* ADC-LSB */
#define C_COIL_ZERO_CURRENT_COUNT	32		/* 32 micro-steps */
#define C_COIL_OVER_CURRENT_COUNT	32		/* 32 micro-steps */
#define C_COIL_CURRENT_START_DELAY	128
extern uint16 l_u16CurrentZeroOffset;

/* ****************************************************************************	*
 *	L o c a l   f u n c t i o n s												*
 * ****************************************************************************	*/
void MotorDriver_InitialPwmDutyCycle( uint16 u16CurrentLevel, uint16 u16MotorSpeed);

/* ****************************************************************************	*
 * MotorDriverInit()
 *
 * Initialise Motor Driver
 * ****************************************************************************	*/
void MotorDriverInit( void)
{
#if _SUPPORT_DOUBLE_USTEP
	uint16 u16MotorMicroStepsPerFullStep = (1 << (NVRAM_MICRO_STEPS + 1));		/* Number of micro-steps per full-step (2, 4, 8 or 16) */
#else  /* _SUPPORT_DOUBLE_USTEP */
	uint16 u16MotorMicroStepsPerFullStep = (1 << NVRAM_MICRO_STEPS);			/* Number of micro-steps per full-step (1, 2, 4 or 8) */
#endif /* _SUPPORT_DOUBLE_USTEP */
	g_u16MotorMicroStepsPerElecRotation = (uint16) mulU32_U16byU16( u16MotorMicroStepsPerFullStep, (NVRAM_MOTOR_PHASES + 2) << 1);
	g_u16MotorMicroStepsPerMechRotation = (uint16) mulU32_U16byU16( NVRAM_POLE_PAIRS, g_u16MotorMicroStepsPerElecRotation);
	{
		uint16 u16ConstAccelaration = NVRAM_ACCELERATION_CONST;
		if ( u16ConstAccelaration != 0 )
		{
			l_u32Temp = divU32_U32byU16( (TIMER_CLOCK * 60), g_u16MotorMicroStepsPerMechRotation);

			g_au16MotorSpeedCommutTimerPeriod[0] = divU16_U32byU16( l_u32Temp, NVRAM_MIN_SPEED) - 1;
			g_au16MotorSpeedCommutTimerPeriod[1] = divU16_U32byU16( l_u32Temp, NVRAM_SPEED_TORQUE_BOOST) - 1;
			g_au16MotorSpeedCommutTimerPeriod[2] = divU16_U32byU16( l_u32Temp, NVRAM_SPEED0) - 1;
			g_au16MotorSpeedCommutTimerPeriod[3] = divU16_U32byU16( l_u32Temp, NVRAM_SPEED1) - 1;
			g_au16MotorSpeedCommutTimerPeriod[4] = divU16_U32byU16( l_u32Temp, NVRAM_SPEED2) - 1;
			g_au16MotorSpeedCommutTimerPeriod[5] = divU16_U32byU16( l_u32Temp, NVRAM_SPEED3) - 1;
			g_au16MotorSpeedCommutTimerPeriod[6] = g_au16MotorSpeedCommutTimerPeriod[1];
			g_au16MotorSpeedCommutTimerPeriod[7] = g_au16MotorSpeedCommutTimerPeriod[0];
			g_u16TargetCommutTimerPeriod = g_au16MotorSpeedCommutTimerPeriod[2];	/* Target commutation timer period (target speed) */
			g_u16CommutTimerPeriod = g_au16MotorSpeedCommutTimerPeriod[2];

			g_au16MotorSpeedRPS[0] = divU16_U32byU16( (uint32)(uint16)(NVRAM_MIN_SPEED + 30U), 60);
			g_au16MotorSpeedRPS[1] = divU16_U32byU16( (uint32)(uint16)(NVRAM_SPEED_TORQUE_BOOST + 30U), 60);
			g_au16MotorSpeedRPS[2] = divU16_U32byU16( (uint32)(uint16)(NVRAM_SPEED0 + 30U), 60);
			g_au16MotorSpeedRPS[3] = divU16_U32byU16( (uint32)(uint16)(NVRAM_SPEED1 + 30U), 60);
			g_au16MotorSpeedRPS[4] = divU16_U32byU16( (uint32)(uint16)(NVRAM_SPEED2 + 30U), 60);
			g_au16MotorSpeedRPS[5] = divU16_U32byU16( (uint32)(uint16)(NVRAM_SPEED3 + 30U), 60);
			g_au16MotorSpeedRPS[6] = g_au16MotorSpeedRPS[1];
			g_au16MotorSpeedRPS[7] = g_au16MotorSpeedRPS[0];
		}
	}

	g_u16CorrectionRatio = NVRAM_MIN_CORR_RATIO;
	/* BLDC motor Commutation/Stepper timer */
	g_u16MicroStepIdx = 0;
	TMR1_CTRL = C_TMRx_CTRL_MODE0;												/* Timer mode 0 */
	TMR1_REGB = g_u16CommutTimerPeriod;											/* Will be overwritten by MotorDriverStart() */
	XI0_PEND = CLR_T1_INT4;														/* Clear (potentially) Timer1 second level interrupts (T1_INT4) */
	XI0_MASK |= EN_T1_INT4;														/* Enable Timer1, CompareB (T1_INT4) */
	PRIO = (PRIO & ~(3 << 6)) | ((4 - 3) << 6);									/* Set Timer1 priority to 4 (3..6) */
	PEND = CLR_EXT0_IT;
	MASK |= EN_EXT0_IT;

	g_u16MotorRewindSteps = (uint16) mulU32_U16byU16( NVRAM_REWIND_STEPS, u16MotorMicroStepsPerFullStep);

	/* Setup Motor PWM */	
	PWM1_CTRL = 0;																/* Disable master */
	PWM2_CTRL = 0;																/* Disable Slave 1 */
	PWM3_CTRL = 0;																/* Disable Slave 2 */
	PWM4_CTRL = 0;																/* Disable Slave 3 */
	PWM5_CTRL = 0;																/* Disable Slave 4 */
	PWM1_PSCL = PWM_PRESCALER;													/* Initialise the master pre-scaler ratio (Fck/8) */
	PWM1_PER = PWM_REG_PERIOD;
	PWM2_PER = PWM_REG_PERIOD;													/* -=#=- Probably not needed to set slave period too */
	PWM3_PER = PWM_REG_PERIOD;													/* -=#=- Probably not needed to set slave period too */
	PWM4_PER = PWM_REG_PERIOD;													/* -=#=- Probably not needed to set slave period too */
	PWM5_PER = PWM_REG_PERIOD;													/* -=#=- Probably not needed to set slave period too */
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR)
#if _SUPPORT_PHASE_SHORT_DET
	/*
	 * (Double PWM)	25%(7.9us)	44%(7.5us)	62%(7.9us)	81%(7.9us)	100%(10.4us)		(7.5us/ADC-conversion)
	 * MF_STEPPER:	Imotor		Vphase		Vs-filt		Temperature	Vsm
	 */
	PWM1_CMP = (((25L * PWM_REG_PERIOD) + 50)/100);		/*  8.0us */			/* 25% of period */
	PWM2_CMP = (((44L * PWM_REG_PERIOD) + 50)/100);		/*  7.5us */			/* 44% of period */
	PWM3_CMP = (((62L * PWM_REG_PERIOD) + 50)/100);		/*  8.0us */			/* 62% of period */
	PWM4_CMP = (((81L * PWM_REG_PERIOD) + 50)/100);		/*  8.0us */			/* 81% of period */
#else  /* _SUPPORT_PHASE_SHORT_DET */
	/* (Double PWM)	25%			50%			75%			100%							(10.5us/ADC-conversion)
	 * MF_STEPPER:	Imotor		Vs-filt		Temperature	Vsm
	 */
	PWM1_CMP = (((25L * PWM_REG_PERIOD) + 50)/100);		/* 10.5us */			/* 25% of period: (PWM_REG_PERIOD+2)/4 */
	PWM2_CMP = (((50L * PWM_REG_PERIOD) + 50)/100);		/* 10.5us */			/* 50% of period */
	PWM3_CMP = (((75L * PWM_REG_PERIOD) + 50)/100);		/* 10.5us */			/* 75% of period */
#endif /* _SUPPORT_PHASE_SHORT_DET */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM)
#if _SUPPORT_PHASE_SHORT_DET
	/* (Single PWM)	20%			40%			60%			80%			100%				(8.33us/ADC-conversion)
	 * MF_STEPPER:	Vphase		Vs-filt		Temperature	Vsm-unfilt	Imotor
	 */
	PWM1_CMP = (((20L * PWM_REG_PERIOD) + 50)/100);		/* 8.33us */			/* 20% of period */
	PWM2_CMP = (((40L * PWM_REG_PERIOD) + 50)/100);								/* 40% of period */
	PWM3_CMP = (((60L * PWM_REG_PERIOD) + 50)/100);								/* 60% of period */
	PWM4_CMP = (((80L * PWM_REG_PERIOD) + 50)/100);								/* 80% of period */
#else  /* _SUPPORT_PHASE_SHORT_DET */
	/* (Single PWM)	25%			50%			75%			100%							(10.5us/ADC-conversion)
	 * MF_STEPPER:	Vs-filt		Temperature	Vsm-unfilt	Imotor
	 */
	PWM1_CMP = (((25L * PWM_REG_PERIOD) + 50)/100);		/* 10.5us */			/* 25% of period */
	PWM2_CMP = (((50L * PWM_REG_PERIOD) + 50)/100);		/* 10.5us */			/* 50% of period */
	PWM3_CMP = (((75L * PWM_REG_PERIOD) + 50)/100);		/* 10.5us */			/* 75% of period */
#endif /* _SUPPORT_PHASE_SHORT_DET */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND)
#if _SUPPORT_PHASE_SHORT_DET
	/* (Single PWM)	17%			33%			50%			67%			83%			100%				(8.33us/ADC-conversion)
	 * MF_STEPPER:	Vs-unfilt	Vphase		Imotor		Vsm-filt	-			Temperature
	 */
	PWM1_CMP = (((17L * PWM_REG_PERIOD) + 50)/100);		/* 8.33us */			/* 17% of period */
	PWM2_CMP = (((33L * PWM_REG_PERIOD) + 50)/100);								/* 33% of period */
	PWM3_CMP = (((50L * PWM_REG_PERIOD) + 50)/100);								/* 50% of period */
	PWM4_CMP = (((67L * PWM_REG_PERIOD) + 50)/100);								/* 67% of period */
	PWM5_CMP = (((83L * PWM_REG_PERIOD) + 50)/100);								/* 83% of period */
#else  /* _SUPPORT_PHASE_SHORT_DET */
	/* (Single PWM)	25%			50%			75%			100%							(10.5us/ADC-conversion)
	 * MF_STEPPER:	Vs-unfilt	Imotor		Vsm-filt	Temperature
	 */
	PWM1_CMP = (((25L * PWM_REG_PERIOD) + 50)/100);		/* 10.5us */			/* 25% of period */
	PWM2_CMP = (((50L * PWM_REG_PERIOD) + 50)/100);		/* 10.5us */			/* 50% of period */
	PWM3_CMP = (((75L * PWM_REG_PERIOD) + 50)/100);		/* 10.5us */			/* 75% of period */
#endif /* _SUPPORT_PHASE_SHORT_DET */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
#if _SUPPORT_PHASE_SHORT_DET
	/* (Single PWM)	17%			33%			50%			67%			83%			100%	(7.0us/ADC-conversion)
	 * MF_STEPPER:	Temperature	Vs-filt		Imotor1		Vs-phase	Vsm-unfilt	Imotor2
	 */
	PWM1_CMP = (((17L * PWM_REG_PERIOD) + 50)/100);		/*  7.0us */			/* 17% of period */
	PWM2_CMP = (((33L * PWM_REG_PERIOD) + 50)/100);		/*  7.0us */			/* 33% of period */
	PWM3_CMP = (((50L * PWM_REG_PERIOD) + 50)/100);		/*  7.0us */			/* 50% of period */
	PWM4_CMP = (((67L * PWM_REG_PERIOD) + 50)/100);		/*  7.0us */			/* 67% of period */
	PWM5_CMP = (((83L * PWM_REG_PERIOD) + 50)/100);		/*  7.0us */			/* 83% of period */
#else  /* _SUPPORT_PHASE_SHORT_DET */
	/* (Single PWM)	17%			33%			50%			75%			100%		(7.0us/ADC-conversion)
	 * MF_STEPPER:	Temperature	Vs-filt		Imotor1		Vsm-unfilt	Imotor2
	 */
	PWM1_CMP = (((17L * PWM_REG_PERIOD) + 50)/100);		/*  7.0us */			/* 17% of period */
	PWM2_CMP = (((33L * PWM_REG_PERIOD) + 50)/100);		/*  7.0us */			/* 33% of period */
	PWM3_CMP = (((50L * PWM_REG_PERIOD) + 50)/100);		/*  10.5us */			/* 50% of period */
	PWM4_CMP = (((75L * PWM_REG_PERIOD) + 50)/100);		/*  10.5us */			/* 75% of period */
#endif /* _SUPPORT_PHASE_SHORT_DET */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */

#if (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) && (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_GND)
	/* Mirror mode */
	PWM1_CTRL = (MODE | EBLK | ECI | EPI);										/* Initialise the master control register - CMPI and PWMI enabled */
	PWM2_CTRL = (MODE | ECI | EXT | EBLK);										/* initialise the slave 1 control register - CMPI enabled */
	PWM3_CTRL = (MODE | ECI | EXT | EBLK);										/* initialise the slave 2 control register - CMPI enabled */
	PWM4_CTRL = (MODE | ECI | EXT | EBLK);										/* initialise the slave 3 control register - CMPI enabled */
	PWM5_CTRL = (MODE | EXT | EBLK);											/* Initialise the slave 4 control register - CMPI disabled */
#else  /* (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) && (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_GND) */
	/* In-depended mode */
	PWM1_CTRL = (EBLK | ECI | EPI);												/* Initialise the master control register - CMPI and PWMI enabled */
	PWM2_CTRL = (ECI | EXT | EBLK);												/* Initialise the slave 1 control register - CMPI enabled */
	PWM3_CTRL = (ECI | EXT | EBLK);												/* Initialise the slave 2 control register - CMPI enabled */
	PWM4_CTRL = (ECI | EXT | EBLK);												/* Initialise the slave 3 control register - CMPI enabled */
	PWM5_CTRL = (ECI | EXT | EBLK);												/* Initialise the slave 4 control register - CMPI enabled */
#endif /* (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) && (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_GND) */
	PWM1_CTRL |= EBLK;															/* Start PWM in application mode */

} /* End of MotorDriverInit */

#if (_SUPPORT_MOTOR_SELFTEST != FALSE)
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
#define FET_SETTING (((10*PLL_freq)/(1000000*CYCLES_PER_INSTR*2)) + 1)			/* 10us: 10us*PLL-freq/(10000000us/s * #cycles/instruction) * instructions */
void MotorDriverSelfTest( void)
{
	uint16 u16SelfTestIdx;
	uint16 u16VdsThreshold;														/* MMP130919-1/MMP140403-1 */
	T_ADC_SELFTEST_4PH adcMotorSelfTest4Ph;
	uint16 u16Pwm2Storage = PWM2_CMP;											/* MMP150219-2: Save PWM2 ADC trigger CMP time */
	PWM2_CMP = (((50L * PWM_REG_PERIOD) + 50)/100);								/* MMP150219-2: Set PWM2 ADC trigger CMP time at 50% or period */

	g_e8ErrorCoil = 0;//init

	MeasureVsupplyAndTemperature();												/* MMP130919-1 - Begin */
	GetVsupplyMotor();
	if ( NVRAM_VDS_THRESHOLD != 0 )
	{
		u16VdsThreshold = NVRAM_VDS_THRESHOLD;
	}
	else
	{
		u16VdsThreshold = 200U;
	}																			/* MMP130919-1 - End */

	/* Test for FET shortages; Note: Diagnostics configuration will switch off driver at over-current */
	for ( u16SelfTestIdx = 0; (g_e8ErrorElectric == (uint8) C_ERR_ELECTRIC_NO) && (u16SelfTestIdx < (sizeof(c_au8DrvCfgSelfTestA)/sizeof(c_au8DrvCfgSelfTestA[0]))); u16SelfTestIdx++ )
	{
		int16 i16DriverCurrent = 0;												/* MMP140403-1 */

		DRVCFG_CNFG_UVWT( (uint16) c_au8DrvCfgSelfTestA[u16SelfTestIdx]);		/* MMP130904-1 */

		MeasurePhaseVoltage( (uint16)c_au16DrvAdcSelfTestA[u16SelfTestIdx>>1]);	/* MMP140403-1/MMP130919-1 - Begin */
		if ( (u16SelfTestIdx & 1) == 0 )
		{
			/* Even-index (0,2,4,6) are phase to ground: Check current too (< 20 mA) */
			MeasureMotorCurrent();
			i16DriverCurrent = GetMotorDriverCurrent();
		}
		/* Even-index (0,2,4,6) are phase to ground: Vphase < Vds; Odd-index (1,3,5,7) are phase to supply: Vphase > (Vsup - Vds) */
		if ( (g_e8ErrorElectric != (uint8) C_ERR_ELECTRIC_NO) ||
			(((u16SelfTestIdx & 1) == 0) && ((g_i16PhaseVoltage > (int16)u16VdsThreshold) || (i16DriverCurrent > 20))) ||
			(((u16SelfTestIdx & 1) != 0) && (g_i16PhaseVoltage < (int16)(g_i16MotorVoltage - u16VdsThreshold))) )
		{																		/* MMP130919-1 - End */
			/* Over-current trigger; Phase makes short with supply or Ground */
			g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
			SetLastError( (uint8) C_ERR_SELFTEST_A);
			g_e8ErrorCoil = (uint8) C_ERR_SELFTEST_A;//Ban, FET short with ground or supply
			break;
		}
	}

	/* Convert Vds-voltage (10mV units) to ADC-LSB */
	u16VdsThreshold = muldivU16_U16byU16byU16( u16VdsThreshold, C_GVOLTAGE_DIV, EE_GADC);

	for ( u16SelfTestIdx = 0; (g_e8ErrorElectric == (uint8) C_ERR_ELECTRIC_NO) && (u16SelfTestIdx < (sizeof(c_au8DrvCfgSelfTestB4)/sizeof(c_au8DrvCfgSelfTestB4[0]))); u16SelfTestIdx++ )
	{
		/* Test for open connections, damaged coil(s) */
		uint16 u16Vsm;
		uint16 u16VphH;
		uint16 u16VphL;
		uint16 u16Vds;
		uint16 u16MotorCoilCurrent;
		register uint16 u16DC;
		if ( u16SelfTestIdx & 0x02 )
		{
			/* Phase LOW + phase -PWM */
			u16DC = (PWM_REG_PERIOD >> 3);
		}
		else
		{
			/* Phase HIGH + phase PWM */
			u16DC = PWM_REG_PERIOD - (PWM_REG_PERIOD >> 3);						/* Approx. 12.5% */
		}
#if (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) && (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_GND)
		PWM2_LT = u16DC;														/* Copy the results into the PWM register for phase U */
		PWM3_LT = u16DC;														/* Copy the results into the PWM register for phase V */
		PWM4_LT = u16DC;														/* Copy the results into the PWM register for phase W */
		PWM5_LT = u16DC;														/* Copy the results into the PWM register for phase T */
		PWM1_LT = u16DC;														/* Master must be modified at last (value is not important) */
#else  /* (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) && (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_GND) */
		u16DC = u16DC/2;
		PWM2_LT = u16DC;														/* Copy the results into the PWM register for phase U */
		PWM2_HT = PWM_REG_PERIOD - u16DC;
		PWM3_LT = u16DC;														/* Copy the results into the PWM register for phase V */
		PWM3_HT = PWM_REG_PERIOD - u16DC;
		PWM4_LT = u16DC;														/* Copy the results into the PWM register for phase W */
		PWM4_HT = PWM_REG_PERIOD - u16DC;
		PWM5_LT = u16DC;														/* Copy the results into the PWM register for phase T */
		PWM5_HT = PWM_REG_PERIOD - u16DC;
		PWM1_LT = u16DC;														/* Master must be modified at last (value is not important) */
#endif /* (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) && (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_GND) */

		DRVCFG_CNFG_UVWT( (uint16) c_au8DrvCfgSelfTestB4[u16SelfTestIdx]);

		ADC_Stop();																/* clear the ADC control register */
		if ( u16SelfTestIdx & 2 )
		{
			ADC_SBASE = (uint16) tAdcSelfTest4B;								/* Phase = Low */
		}
		else
		{
			ADC_SBASE = (uint16) tAdcSelfTest4A;								/* Phase = High */
		}
		ADC_DBASE = (uint16) &adcMotorSelfTest4Ph;
		ADC_CTRL |= (ADC_TRIG_SRC | ADC_SYNC_SOC);								/* Single cycle of conversion is done */
		ADC_CTRL |= ADC_START;													/* Start ADC */

		/* This takes about 4 Motor PWM-periods per self-test */
		while (ADC_CTRL & ADC_START) /* lint -e{722} */ ;						/* Wait for ADC result (Time-out?) */

		DRVCFG_GND_UVWT();

		if ( g_e8ErrorElectric != (uint8) C_ERR_ELECTRIC_NO )					/* Over-current ? */
		{
			/* Over-current trigger; Phase makes short with other phase */
			g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
			SetLastError( C_ERR_SELFTEST_B);
			g_e8ErrorCoil = (uint8) C_ERR_SELFTEST_B;//Ban, phase shot with other phase
			break;
		}

		if ( u16SelfTestIdx & 2 )
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

		switch ( u16SelfTestIdx & ~2 )
		{
		default:
		case 0:
			u16VphH = adcMotorSelfTest4Ph.PhaseU_HighVoltage;
			u16VphL = adcMotorSelfTest4Ph.PhaseU_LowVoltage;
			break;
		case 1:
			u16VphH = adcMotorSelfTest4Ph.PhaseV_HighVoltage;
			u16VphL = adcMotorSelfTest4Ph.PhaseV_LowVoltage;
			break;
		case 4:
			u16VphH = adcMotorSelfTest4Ph.PhaseW_HighVoltage;
			u16VphL = adcMotorSelfTest4Ph.PhaseW_LowVoltage;
			break;
		case 5:
			u16VphH = adcMotorSelfTest4Ph.PhaseT_HighVoltage;
			u16VphL = adcMotorSelfTest4Ph.PhaseT_LowVoltage;
			break;
		}

		if ( u16Vsm > u16VphH )
			u16Vds = u16Vsm - u16VphH;
		else
			u16Vds = u16VphH - u16Vsm;
#if _SUPPORT_COIL_RESISTANCE_CHECK //Ban disable coil resistance check
		if ( u16Vds > u16VdsThreshold )
		{
			g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
			SetLastError( C_ERR_SELFTEST_D);
			g_e8ErrorCoil = (uint8) C_ERR_SELFTEST_D;//Ban, phase resistance is too small
			break;
		}
		if ( u16VphL > u16VdsThreshold )
		{
			g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
			SetLastError( C_ERR_SELFTEST_E);
			g_e8ErrorCoil = (uint8) C_ERR_SELFTEST_E;//Ban, phase resistance is too small
			break;
		}
#endif // _SUPPORT_COIL_RESISTANCE_CHECK
		extern uint16 l_u16CurrentZeroOffset;
		if ( (int16) (u16MotorCoilCurrent - l_u16CurrentZeroOffset) < C_MIN_COIL_CURRENT )
		{
			/* No current (less than 10 LSB's); Coil Open */
			g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
			SetLastError( C_ERR_SELFTEST_C);
			g_e8ErrorCoil = (uint8) C_ERR_SELFTEST_C;
			break;
		}
	}	

	DRVCFG_DIS_UVWT();
	PWM2_LT = PWM_SCALE_OFFSET;													/* 50% PWM duty cycle for phase U */
	PWM3_LT = PWM_SCALE_OFFSET;													/* 50% PWM duty cycle for phase V */
	PWM4_LT = PWM_SCALE_OFFSET;													/* 50% PWM duty cycle for phase W */
	PWM5_LT = PWM_SCALE_OFFSET;													/* 50% PWM duty cycle for phase T */
	PWM1_LT = PWM_SCALE_OFFSET;													/* Master must be modified at last (value is not important) */
	DRVCFG_DIS();																/* MMP140903-1 */
	PWM2_CMP = u16Pwm2Storage;													/* MMP150219-2: Restore PWM2 ADC trigger CMP time */

} /* End of MotorDriverSelfTest() */
#endif /* (_SUPPORT_MOTOR_SELFTEST != FALSE) */

/* ****************************************************************************	*
 * MotorDriverCurrentMeasureInit()
 *
 * Initialise for motor driver current measurement
 * Performance: <10us
 * ****************************************************************************	*/
void MotorDriverCurrentMeasureInit( void)
{
	uint16 u16Count;															/* MMP140331-2 - Begin */
	l_u16StartupDelayInit = 64 * NVRAM_ACCELERATION_POINTS;						/* Calculate the startup-delay, based on acceleration steps */
	if ( l_u16StartupDelayInit < (2*C_MOVAVG_SZ) )
	{
		l_u16StartupDelayInit = (2*C_MOVAVG_SZ);								/* Minimum of twice the moving-average filter size */
	}
	else if ( l_u16StartupDelayInit > NVRAM_STALL_DETECTOR_DELAY )
	{
		l_u16StartupDelayInit = NVRAM_STALL_DETECTOR_DELAY;						/* Maximum of NVRAM specified */
	}																			/* MMP140331-2 - End */

	ATOMIC_CODE
	(
		g_u16StartupDelay = l_u16StartupDelayInit;								/* (MMP140331-2) */
		g_u16MotorCurrentLPFx64 = 0;											/* Low-pass Filtered motor-current (x 64) */
		l_u16MotorCurrentRawIdx = 0;											/* Raw current moving average index */
		g_u16MotorCurrentMovAvgxN = 0;											/* Moving average motor-current (x 4..16) */
		l_au16MotorCurrentRaw[0] = 0;
	);
	{
		uint16 *pStallCurrentRaw = &l_au16MotorCurrentRaw[1];
		for ( u16Count = 1; u16Count < C_MOVAVG_SZ; u16Count++ )
		{
			*pStallCurrentRaw = 0;
			pStallCurrentRaw++;
		}
	}

} /* End of MotorDriverCurrentMeasureInit() */

/* ****************************************************************************	*
 * MotorDriverCurrentMeasure()
 *
 * Measure a average motor current, based on ADC current's
 * Performance: Approximate: 10us @ 20MHz
 * ****************************************************************************	*/
void MotorDriverCurrentMeasure( void)
{
#if (C_MOVAVG_SSZ < 6)
	uint16 u16MotorCurrentAcc;
#endif /* (C_MOVAVG_SSZ < 6 ) */
	uint16 u16MicroStepMotorCurrent = GetRawMotorDriverCurrent();
#if _DEBUG_SPI
	SpiDebugWriteFirst(g_u16PidRunningThreshold|0x8000);
	SpiDebugWriteNext(g_u16MotorCurrentMovAvgxN);
	//SpiDebugWriteFirst(g_u16HallMicroStepIdx);
	//SpiDebugWriteNext(u16MicroStepMotorCurrent);
#endif /* _DEBUG_SPI */

	/* Moving average (sum) of motor-driver current */
	uint16 *pu16MotorCurrentElement = &l_au16MotorCurrentRaw[l_u16MotorCurrentRawIdx];
	uint16 u16PrevMotorCurrent = *pu16MotorCurrentElement;
	l_u16MotorCurrentRawIdx = (l_u16MotorCurrentRawIdx + 1) & (C_MOVAVG_SZ - 1);
	if ( (g_u16StartupDelay != 0) || (u16MicroStepMotorCurrent < (u16PrevMotorCurrent << 1)) )	/* Check for valid motor-driver current (at least smaller than 2x previous current)  */
	{
		g_u16MotorCurrentMovAvgxN -= u16PrevMotorCurrent;						/* Subtract oldest raw motor-driver current */
		g_u16MotorCurrentMovAvgxN += u16MicroStepMotorCurrent;					/* Add newest raw motor-driver current */
		*pu16MotorCurrentElement = u16MicroStepMotorCurrent;					/* Overwrite oldest with newest motor-driver current */
	}

	/* During twice the moving-average-buffer size and during acceleration of the motor, LPF should follow
	   lowest value of LPF or Motor-current. As the speed is increasing so also is the BEMF also increasing,
	   which causes the current to decrease. Otherwise a first order (IIR-1) LPF is used. */
#if (C_MOVAVG_SSZ < 6)
	u16MotorCurrentAcc = (g_u16MotorCurrentMovAvgxN << (6 - C_MOVAVG_SSZ));
	if ( (g_u16StartupDelay > (l_u16StartupDelayInit - (2*C_MOVAVG_SZ))) || (g_u8MotorStartupMode == (uint8) MSM_STEPPER_D) || ((g_u8MotorStartupMode == (uint8) MSM_STEPPER_A) && (u16MotorCurrentAcc < g_u16MotorCurrentLPFx64)) )
	{
		g_u16MotorCurrentLPFx64 = u16MotorCurrentAcc;
#endif /* (C_MOVAVG_SSZ < 6 ) */
#if (C_MOVAVG_SSZ == 6 )
	if ( (g_u16StartupDelay > (l_u16StartupDelayInit - (2*C_MOVAVG_SZ))) || (g_u8MotorStartupMode == (uint8) MSM_STEPPER_D) || ((g_u8MotorStartupMode == (uint8) MSM_STEPPER_A) && (g_u16MotorCurrentMovAvgxN < g_u16MotorCurrentLPFx64)) )
	{
		g_u16MotorCurrentLPFx64 = g_u16MotorCurrentMovAvgxN;
#endif /* (C_MOVAVG_SSZ == 6 ) */
	}
	else
	{
#if (MOTOR_MICROSTEPS < 3)
		/* LPF_B: IIR of 0.9921875 (127/128) & 0.0078125 (1/128) */
		g_u16MotorCurrentLPFx64 = (g_u16MotorCurrentLPFx64 - ((g_u16MotorCurrentLPFx64 + 63) >> 7)) + ((g_u16MotorCurrentMovAvgxN + (1 << C_MOVAVG_SSZ)) >> (1 + C_MOVAVG_SSZ));
#else  /* (MOTOR_MICROSTEPS < 3) */
		/* LPF_B: IIR of 0.99609375 (255/256) & 0.00390625 (1/256) */
		g_u16MotorCurrentLPFx64 = (g_u16MotorCurrentLPFx64 - ((g_u16MotorCurrentLPFx64 + 63) >> 8)) + ((g_u16MotorCurrentMovAvgxN + (1 << (1 + C_MOVAVG_SSZ))) >> (2 + C_MOVAVG_SSZ));
#endif /* (MOTOR_MICROSTEPS < 3) */
	}

	if ( g_u16StartupDelay > 0 )
	{
		g_u16StartupDelay--;
	}

#if _DEBUG_MOTOR_CURRENT_FLT
	l_au8MotorCurrRaw[l_u16MotorCurrIdx] = (uint8) ((u16MicroStepMotorCurrent >> 1) & 0xFF);
	if ( ++l_u16MotorCurrIdx >= C_MOTOR_CURR_SZ )
		l_u16MotorCurrIdx = 0;
#endif /* _DEBUG_MOTOR_CURRENT_FLT */
} /* End of MotorDriverCurrentMeasure() */

/* ****************************************************************************	*
 * MotorDriver_InitialPwmDutyCycle()
 *
 * Calculate Motor PWM (initial) Duty-cycle, based on current threshold level and speed
 * ****************************************************************************	*/
void MotorDriver_InitialPwmDutyCycle( uint16 u16CurrentLevel, uint16 u16MotorSpeed)
{
	if ( u16MotorSpeed == 0 )														/* MMP140228-1 - Begin */
	{
		g_u16CorrectionRatio  = ((NVRAM_MOTOR_COIL_RTOT + 2 * C_FETS_RTOT) * u16CurrentLevel);
		g_u16CorrectionRatio /= 4;
//		g_u16CorrectionRatio = (5 * g_u16CorrectionRatio);							/* Divided by 5/16 */
//		g_u16PidCtrlRatio = muldivU16_U16byU16byU16( g_u16CorrectionRatio, PWM_REG_PERIOD << PWM_PRESCALER_N, NVRAM_VSUP_REF);
	}																				/* MMP140228-1 - End */
	else
	{
		/* Ohmic losses: Ur-losses = (0.5 * R[ohm] * I[mA])/10 [10mV] = (R[ohm] * I[mA])/20 [10mV]
		 * FET losses: Ufet-losses = (Rfet * I[mA])/10 [10mV] = (2 * Rfet * I[mA])/20 [10mV]*/
		g_u16CorrectionRatio  = ((NVRAM_MOTOR_COIL_RTOT + (2 * C_FETS_RTOT)) * u16CurrentLevel);
		g_u16CorrectionRatio /= 20;													/* Divided by 20 */
		g_u16CorrectionRatio += (NVRAM_MOTOR_CONSTANT * u16MotorSpeed);				/* BEMF = Kmotor[10mV/RPS] * Speed[RPS] */
	}
	g_u16PidCtrlRatio =  muldivU16_U16byU16byU16( g_u16CorrectionRatio << 3, PWM_REG_PERIOD << (1 + PWM_PRESCALER_N), NVRAM_VSUP_REF);
	g_u16PID_I = g_u16PidCtrlRatio;
	if ( g_i16MotorVoltage > 0 )
	{
		g_u16CorrectionRatio = muldivU16_U16byU16byU16( g_u16CorrectionRatio << 3, PWM_REG_PERIOD << (1 + PWM_PRESCALER_N), (uint16) g_i16MotorVoltage);
	}
	else
	{
		g_u16CorrectionRatio = g_u16PidCtrlRatio;
	}
	g_i16PID_D = 0;
	g_i16PID_E = 0;
	g_u16PID_CtrlCounter = 0;													/* Re-start Current-control PID */
} /* End of MotorDriver_InitialPwmDutyCycle() */

/* ****************************************************************************	*
 * MotorDriver_4PhaseStepper
 *
 * Performance: 13.5us @ 28MHz (BIPOLAR_PWM_SINGLE_INDEPENDED_GND)
 *
 * Based on a 32-step c_ai16MicroStepVector4PH-table!!
 * ****************************************************************************	*/
void MotorDriver_4PhaseStepper( void)
{
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR)
	register int16 iPwm = (int16) (mulI32_I16byU16( c_ai16MicroStepVector4PH[g_u16MicroStepIdx], g_u16CorrectionRatio) >> (20 + PWM_PRESCALER_N));
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
	PWM4_LT = (uint16) ((int16) PWM_SCALE_OFFSET + iPwm);	/* U */
	PWM2_LT = (uint16) ((int16) PWM_SCALE_OFFSET - iPwm);	/* W */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
	PWM3_LT = (uint16) ((int16) PWM_SCALE_OFFSET + iPwm);	/* V */
	PWM2_LT = (uint16) ((int16) PWM_SCALE_OFFSET - iPwm);	/* W */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	iPwm = (int16) (mulI32_I16byU16( c_ai16MicroStepVector4PH[g_u16MicroStepIdx + C_MICROSTEP_PER_FULLSTEP], g_u16CorrectionRatio) >> (20 + PWM_PRESCALER_N));
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
	PWM3_LT = (uint16) ((int16) PWM_SCALE_OFFSET + iPwm);	/* V */
	PWM5_LT = (uint16) ((int16) PWM_SCALE_OFFSET - iPwm);	/* T */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
	PWM4_LT = (uint16) ((int16) PWM_SCALE_OFFSET + iPwm);	/* U */
	PWM5_LT = (uint16) ((int16) PWM_SCALE_OFFSET - iPwm);	/* T */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM)
	/* Sines */
	register int16 iPwm = (int16) (mulI32_I16byU16( c_ai16MicroStepVector4PH[u16MicroStepIdx], g_u16CorrectionRatio) >> (19 + PWM_PRESCALER_N));
	if ( iPwm >= 0 )
	{
		/* 1st and 2nd Quadrant */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* U-phase: HIGH, W-Phase: PWM */
		PWM4_LT = PWM_REG_PERIOD;							/* U = HIGH */
		PWM2_LT = (uint16) ((int16) PWM_REG_PERIOD - iPwm);	/* W = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* T-phase: HIGH, W-Phase: PWM */
		PWM5_LT = PWM_REG_PERIOD;							/* T = HIGH */
		PWM2_LT = (uint16) ((int16) PWM_REG_PERIOD - iPwm);	/* W = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* V-phase: HIGH, W-Phase: PWM */
		PWM3_LT = PWM_REG_PERIOD;							/* V = HIGH */
		PWM2_LT = (uint16) ((int16) PWM_REG_PERIOD - iPwm);	/* W = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	}
	else
	{
		/* 3rd and 4th Quadrant */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* W-phase: HIGH, U-Phase: PWM */
		PWM2_LT = PWM_REG_PERIOD;							/* W = HIGH */
		PWM4_LT = (uint16) ((int16) PWM_REG_PERIOD + iPwm);	/* U = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* W-phase: HIGH, T-Phase: PWM */
		PWM2_LT = PWM_REG_PERIOD;							/* W = HIGH */
		PWM5_LT = (uint16) ((int16) PWM_REG_PERIOD + iPwm);	/* T = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* W-phase: HIGH, V-Phase: PWM */
		PWM2_LT = PWM_REG_PERIOD;							/* W = HIGH */
		PWM3_LT = (uint16) ((int16) PWM_REG_PERIOD + iPwm);	/* V = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	}
	/* Cosines */
	iPwm = (int16) (mulI32_I16byU16( c_ai16MicroStepVector4PH[u16MicroStepIdx + C_MICROSTEP_PER_FULLSTEP], g_u16CorrectionRatio) >> (19 + PWM_PRESCALER_N));
	if ( iPwm >= 0 )
	{
		/* 1st and 4th Quadrant */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* V-phase: HIGH, T-Phase: PWM */
		PWM3_LT = PWM_REG_PERIOD;							/* V = HIGH */
		PWM5_LT = (uint16) ((int16) PWM_REG_PERIOD - iPwm);	/* T = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* U-phase: HIGH, V-Phase: PWM */
		PWM4_LT = PWM_REG_PERIOD;							/* U = HIGH */
		PWM3_LT = (uint16) ((int16) PWM_REG_PERIOD - iPwm);	/* V = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* U-phase: HIGH, T-Phase: PWM */
		PWM4_LT = PWM_REG_PERIOD;							/* U = HIGH */
		PWM5_LT = (uint16) ((int16) PWM_REG_PERIOD - iPwm);	/* T = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	}
	else
	{
		/* 2st and 3th Quadrant */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* T-phase: HIGH, V-Phase: PWM */
		PWM5_LT = PWM_REG_PERIOD;							/* T = HIGH */
		PWM3_LT = (uint16) ((int16) PWM_REG_PERIOD + iPwm);	/* V = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* V-phase: HIGH, U-Phase: PWM */
		PWM3_LT = PWM_REG_PERIOD;							/* V = HIGH */
		PWM4_LT = (uint16) ((int16) PWM_REG_PERIOD + iPwm);	/* U = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* T-phase: HIGH, U-Phase: PWM */
		PWM5_LT = PWM_REG_PERIOD;							/* T = HIGH */
		PWM4_LT = (uint16) ((int16) PWM_REG_PERIOD + iPwm);	/* U = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	}
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND)
	/* Sines */
	register int16 iPwm = (int16) (mulI32_I16byU16( c_ai16MicroStepVector4PH[g_u16MicroStepIdx], g_u16CorrectionRatio) >> (19 + PWM_PRESCALER_N));
	if ( iPwm >= 0 )
	{
		/* 1st and 2nd Quadrant */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* U-phase: HIGH, W-Phase: PWM */
		PWM4_LT = 0;										/* U = LOW */
		PWM2_LT = (uint16) iPwm;							/* W = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* T-phase: HIGH, W-Phase: PWM */
		PWM5_LT = 0;										/* T = LOW */
		PWM2_LT = (uint16) iPwm;							/* W = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* V-phase: HIGH, W-Phase: PWM */
		PWM3_LT = 0;										/* V = LOW */
		PWM2_LT = (uint16) iPwm;							/* W = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	}
	else
	{
		/* 3rd and 4th Quadrant */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* W-phase: HIGH, U-Phase: PWM */
		PWM2_LT = 0;										/* W = LOW */
		PWM4_LT = (uint16) ((int16) 0 - iPwm);				/* U = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* W-phase: HIGH, T-Phase: PWM */
		PWM2_LT = 0;										/* W = LOW */
		PWM5_LT = (uint16) ((int16) 0 - iPwm);				/* T = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* W-phase: HIGH, V-Phase: PWM */
		PWM2_LT = 0;										/* W = LOW */
		PWM3_LT = (uint16) ((int16) 0 - iPwm);				/* V = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	}
	/* Cosines */
	iPwm = (int16) (mulI32_I16byU16( c_ai16MicroStepVector4PH[g_u16MicroStepIdx + C_MICROSTEP_PER_FULLSTEP], g_u16CorrectionRatio) >> (19 + PWM_PRESCALER_N));
	if ( iPwm >= 0 )
	{
		/* 1st and 4th Quadrant */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* V-phase: HIGH, T-Phase: PWM */
		PWM3_LT = 0;										/* V = LOW */
		PWM5_LT = (uint16) iPwm;							/* T = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* U-phase: HIGH, V-Phase: PWM */
		PWM4_LT = 0;										/* U = LOW */
		PWM3_LT = (uint16) iPwm;							/* V = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* U-phase: HIGH, T-Phase: PWM */
		PWM4_LT = 0;										/* U = LOW */
		PWM5_LT = (uint16) iPwm;							/* T = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	}
	else
	{
		/* 2st and 3th Quadrant */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* T-phase: HIGH, V-Phase: PWM */
		PWM5_LT = 0;										/* T = LOW */
		PWM3_LT = (uint16) ((int16) 0 - iPwm);				/* V = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* V-phase: HIGH, U-Phase: PWM */
		PWM3_LT = 0;										/* V = LOW */
		PWM4_LT = (uint16) ((int16) 0 - iPwm);				/* U = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* T-phase: HIGH, U-Phase: PWM */
		PWM5_LT = 0;										/* T = LOW */
		PWM4_LT = (uint16) ((int16) 0 - iPwm);				/* U = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	}
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM)
	/* EMC CE/RE reduction */
	int16 iPwm1, iPwm2;
	int16 *pi16Vector = (int16 *) &c_ai16MicroStepVector4PH[g_u16MicroStepIdx];
	if (PWM_REG_PERIOD >= (128U << (4U - PWM_PRESCALER_N)))						/* (((PWM_REG_PERIOD * 256U) >> (4U - PWM_PRESCALER_N)) > 32767U) */
	{
		iPwm1 = (int16) (mulI32_I16byU16( *pi16Vector, g_u16CorrectionRatio) >> (20 + PWM_PRESCALER_N));
		pi16Vector += C_MICROSTEP_PER_FULLSTEP;
		iPwm2 = (int16) (mulI32_I16byU16( *pi16Vector, g_u16CorrectionRatio) >> (20 + PWM_PRESCALER_N));
	}
	else
	{
#if (PWM_PRESCALER_N == 0)
		iPwm1 = mulI16_I16byI16Shft4( *pi16Vector, (int16) g_u16CorrectionRatio);	/* U */
		pi16Vector += C_MICROSTEP_PER_FULLSTEP;
		iPwm2 = mulI16_I16byI16Shft4( *pi16Vector, (int16) g_u16CorrectionRatio);	/* V */
#else
		i16PwmU = (int16) (mulI16_I16byI16( *pi16Vector, (int16) g_u16CorrectionRatio) >> (4 + PWM_PRESCALER_N));	/* U */
		pi16Vector += C_MICROSTEP_PER_FULLSTEP;
		i16PwmV = (int16) (mulI16_I16byI16( *pi16Vector, (int16) g_u16CorrectionRatio) >> (4 + PWM_PRESCALER_N));	/* V */
#endif
	}
	if ( u16MicroStepIdx & (2*C_MICROSTEP_PER_FULLSTEP) )
	{
		/* 3rd and 4th Quadrant (Pwm1) */
		iPwm1 = (0 - iPwm1);
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* W = HIGH */
		PWM2_LT = 0;
		PWM2_HT = PWM_REG_PERIOD + 1;											/* MMP150603-1 */

		/* U = PWM */
		PWM4_LT = (uint16) iPwm1;
		PWM4_HT = (uint16) (PWM_REG_PERIOD - iPwm1);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* W = HIGH */
		PWM2_LT = 0;
		PWM2_HT = PWM_REG_PERIOD + 1;											/* MMP150603-1 */

		/* T = PWM */
		PWM5_LT = (uint16) iPwm1;
		PWM5_HT = (uint16) (PWM_REG_PERIOD - iPwm1);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* W = HIGH */
		PWM2_LT = 0;
		PWM2_HT = PWM_REG_PERIOD + 1;											/* MMP150603-1 */

		/* V = PWM */
		PWM3_LT = (uint16) iPwm1;
		PWM3_HT = (uint16) (PWM_REG_PERIOD - iPwm1);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */

		if ( iPwm2 == 0 )														/* MMP141119-1 - Begin */
		{
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
			/* V = HIGH */
			PWM3_LT = 0;
			PWM3_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* T = HIGH */
			PWM5_LT = 0;
			PWM5_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
			/* U = HIGH */
			PWM4_LT = 0;
			PWM4_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* V = HIGH */
			PWM3_LT = 0;
			PWM3_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
			/* U  = HIGH */
			PWM4_LT = 0;
			PWM4_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* T = HIGH */
			PWM5_LT = 0;
			PWM5_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
		}
		else if ( u16MicroStepIdx & C_MICROSTEP_PER_FULLSTEP )					/* MMP141119-1 - End */
		{
			/* 4th Quadrant (Pwm2) */
			iPwm2 = ((int16) PWM_SCALE_OFFSET - iPwm2);
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
			/* V = HIGH */
			PWM3_LT = 0;
			PWM3_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* T = PWM */
			PWM5_HT = (uint16) iPwm2;
			PWM5_LT = (uint16) (PWM_REG_PERIOD - iPwm2);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
			/* U = HIGH */
			PWM4_LT = 0;
			PWM4_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* V = PWM */
			PWM3_HT = (uint16) iPwm2;
			PWM3_LT = (uint16) (PWM_REG_PERIOD - iPwm2);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
			/* U  = HIGH */
			PWM4_LT = 0;
			PWM4_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* T = PWM */
			PWM5_HT = (uint16) iPwm2;
			PWM5_LT = (uint16) (PWM_REG_PERIOD - iPwm2);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
		}
		else
		{
			/* 3rd Quadrant (Pwm2) */
			iPwm2 = (PWM_SCALE_OFFSET + iPwm2);
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
			/* T = HIGH */
			PWM5_LT = 0;
			PWM5_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* V = PWM */
			PWM3_LT = (uint16) iPwm2;
			PWM3_HT = (uint16) (PWM_REG_PERIOD - iPwm2);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
			/* V = HIGH */
			PWM3_LT = 0;
			PWM3_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* U = PWM */
			PWM4_HT = (uint16) iPwm2;
			PWM4_LT = (uint16) (PWM_REG_PERIOD - iPwm2);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
			/* T = HIGH */
			PWM5_LT = 0;
			PWM5_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* U = PWM */
			PWM4_HT = (uint16) iPwm2;
			PWM4_LT = (uint16) (PWM_REG_PERIOD - iPwm2);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
		}
	}
	else
	{
		/* 1st and 2nd Quadrant (Pwm1)*/
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* U = HIGH */
		PWM4_LT = 0;
		PWM4_HT = PWM_REG_PERIOD + 1;											/* MMP150603-1 */

		/* W = PWM */
		PWM2_LT = (uint16) iPwm1;
		PWM2_HT = (uint16) (PWM_REG_PERIOD - iPwm1);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* T = HIGH */
		PWM5_LT = 0;
		PWM5_HT = PWM_REG_PERIOD + 1;											/* MMP150603-1 */

		/* W = PWM */
		PWM2_LT = (uint16) iPwm1;
		PWM2_HT = (uint16) (PWM_REG_PERIOD - iPwm1);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* V= HIGH */
		PWM3_LT = 0;
		PWM3_HT = PWM_REG_PERIOD + 1;											/* MMP150603-1 */

		/* W = PWM */
		PWM2_LT = (uint16) iPwm1;
		PWM2_HT = (uint16) (PWM_REG_PERIOD - iPwm1);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */

		if ( iPwm2 == 0 )														/* MMP141119-1 - Begin */
		{
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
			/* T = HIGH */
			PWM5_LT = 0;
			PWM5_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* V = HIGH */
			PWM3_LT = 0;
			PWM3_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
			/* V = HIGH */
			PWM3_LT = 0;
			PWM3_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* U = HIGH */
			PWM4_LT = 0;
			PWM4_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
			/* T = HIGH */
			PWM5_LT = 0;
			PWM5_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* U = HIGH */
			PWM4_LT = 0;
			PWM4_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
		}
		else if ( u16MicroStepIdx & C_MICROSTEP_PER_FULLSTEP )					/* MMP141119-1 - End */
		{
			/* 2nd Quadrant */
			iPwm2 = (PWM_SCALE_OFFSET + iPwm2);
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
			/* T = HIGH */
			PWM5_LT = 0;
			PWM5_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* V = PWM */
			PWM3_LT = (uint16) iPwm2;
			PWM3_HT = (uint16) (PWM_REG_PERIOD - iPwm2);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
			/* V = HIGH */
			PWM3_LT = 0;
			PWM3_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* U = PWM */
			PWM4_HT = (uint16) iPwm2;
			PWM4_LT = (uint16) (PWM_REG_PERIOD - iPwm2);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
			/* T = HIGH */
			PWM5_LT = 0;
			PWM5_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* U = PWM */
			PWM4_HT = (uint16) iPwm2;
			PWM4_LT = (uint16) (PWM_REG_PERIOD - iPwm2);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
		}
		else
		{
			/* 1st Quadrant */
			iPwm2 = ((int16) PWM_SCALE_OFFSET - iPwm2);
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
			/* V = HIGH */
			PWM3_LT = 0;
			PWM3_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* T = PWM */
			PWM5_HT = (uint16) iPwm2;
			PWM5_LT = (uint16) (PWM_REG_PERIOD - iPwm2);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
			/* U = HIGH */
			PWM4_LT = 0;
			PWM4_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* V = PWM */
			PWM3_HT = (uint16) iPwm2;
			PWM3_LT = (uint16) (PWM_REG_PERIOD - iPwm2);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
			/* U = HIGH */
			PWM4_LT = 0;
			PWM4_HT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */

			/* T = PWM */
			PWM5_HT = (uint16) iPwm2;
			PWM5_LT = (uint16) (PWM_REG_PERIOD - iPwm2);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
		}
	}
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND)					/* MMP150515-1 */
	/* EMC CE/RE reduction */
	int16 iPwm1, iPwm2;
	int16 *pi16Vector = (int16 *) &c_ai16MicroStepVector4PH[g_u16MicroStepIdx];
#if (PWM_REG_PERIOD >= (128U << (4U - PWM_PRESCALER_N)))						/* (((PWM_REG_PERIOD * 256U) >> (4U - PWM_PRESCALER_N)) > 32767U) */
	iPwm1 = (int16) (mulI32_I16byU16( *pi16Vector, g_u16CorrectionRatio) >> (20 + PWM_PRESCALER_N));
	pi16Vector += C_MICROSTEP_PER_FULLSTEP;
	iPwm2 = (int16) (mulI32_I16byU16( *pi16Vector, g_u16CorrectionRatio) >> (20 + PWM_PRESCALER_N));
#elif (PWM_PRESCALER_N == 0)
	iPwm1 = mulI16_I16byI16Shft4( *pi16Vector, (int16) g_u16CorrectionRatio);	/* U */
	pi16Vector += C_MICROSTEP_PER_FULLSTEP;
	iPwm2 = mulI16_I16byI16Shft4( *pi16Vector, (int16) g_u16CorrectionRatio);	/* V */
#else
	i16PwmU = (int16) (mulI16_I16byI16( *pi16Vector, (int16) g_u16CorrectionRatio) >> (4 + PWM_PRESCALER_N));	/* U */
	pi16Vector += C_MICROSTEP_PER_FULLSTEP;
	i16PwmV = (int16) (mulI16_I16byI16( *pi16Vector, (int16) g_u16CorrectionRatio) >> (4 + PWM_PRESCALER_N));	/* V */
#endif
	if ( g_u16MicroStepIdx & (2*C_MICROSTEP_PER_FULLSTEP) )
	{
		/* 3rd and 4th Quadrant (Pwm1) */
		iPwm1 = (PWM_SCALE_OFFSET + iPwm1);
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* U = PWM */
		PWM4_LT = (uint16) iPwm1;
		PWM4_HT = (uint16) (PWM_REG_PERIOD - iPwm1);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* T = PWM */
		PWM5_LT = (uint16) iPwm1;
		PWM5_HT = (uint16) (PWM_REG_PERIOD - iPwm1);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* V = PWM */
		PWM3_LT = (uint16) iPwm1;
		PWM3_HT = (uint16) (PWM_REG_PERIOD - iPwm1);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */

		/* W = LOW */
		PWM2_HT = 0;
		PWM2_LT = PWM_REG_PERIOD + 1;											/* MMP150603-1 */;
	}
	else
	{
		/* 1st and 2nd Quadrant (Pwm1)*/
		/* W = PWM */
		iPwm1 = ((int16) PWM_SCALE_OFFSET - iPwm1);
		PWM2_LT = (uint16) iPwm1;
		PWM2_HT = (uint16) (PWM_REG_PERIOD - iPwm1);

#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* U = LOW */
		PWM4_HT = 0;
		PWM4_LT = PWM_REG_PERIOD + 1;											/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* T = LOW */
		PWM5_HT = 0;
		PWM5_LT = PWM_REG_PERIOD + 1;											/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* V= LOW */
		PWM3_HT = 0;
		PWM3_LT = PWM_REG_PERIOD + 1;											/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	}

	{
		uint16 u16Quad = g_u16MicroStepIdx & (3*C_MICROSTEP_PER_FULLSTEP);
		if ( (u16Quad == 0) || (u16Quad == (3*C_MICROSTEP_PER_FULLSTEP)) )
		{
			/* 1st and 4th Quadrant (Pwm2) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
			/* T = PWM */
			PWM5_HT = (uint16) iPwm2;
			PWM5_LT = (uint16) (PWM_REG_PERIOD - iPwm2);

			/* V = LOW */
			PWM3_HT = 0;
			PWM3_LT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
			/* V = PWM */
			PWM3_HT = (uint16) iPwm2;
			PWM3_LT = (uint16) (PWM_REG_PERIOD - iPwm2);

			/* U = LOW */
			PWM4_HT = 0;
			PWM4_LT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
			/* T = PWM */
			PWM5_HT = (uint16) iPwm2;
			PWM5_LT = (uint16) (PWM_REG_PERIOD - iPwm2);

			/* U  = LOW */
			PWM4_HT = 0;
			PWM4_LT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
		}
		else
		{
			/* 3rd Quadrant (Pwm2) */
			iPwm2 = (0 - iPwm2);
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
			/* V = PWM */
			PWM3_HT = (uint16) iPwm2;
			PWM3_LT = (uint16) (PWM_REG_PERIOD - iPwm2);

			/* T = LOW */
			PWM5_HT = 0;
			PWM5_LT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
			/* U = PWM */
			PWM4_HT = (uint16) iPwm2;
			PWM4_LT = (uint16) (PWM_REG_PERIOD - iPwm2);

			/* V = LOW */
			PWM3_HT = 0;
			PWM3_LT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
			/* U = PWM */
			PWM4_HT = (uint16) iPwm2;
			PWM4_LT = (uint16) (PWM_REG_PERIOD - iPwm2);

			/* T = LOW */
			PWM5_HT = 0;
			PWM5_LT = PWM_REG_PERIOD + 1;										/* MMP150603-1 */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
		}
	}
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
	register int16 iPwm = (int16) (mulI32_I16byU16( c_ai16MicroStepVector4PH[u16MicroStepIdx], g_u16CorrectionRatio) >> (19 + PWM_PRESCALER_N));
	if ( iPwm >= 0 )
	{
		/* 1st and 2nd Quadrant */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* U-phase: HIGH, W-Phase: PWM */
		PWM4_LT = PWM_REG_PERIOD;							/* U = HIGH */
		PWM2_LT = (uint16) ((int16) PWM_REG_PERIOD - iPwm);	/* W = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* T-phase: HIGH, W-Phase: PWM */
		PWM5_LT = PWM_REG_PERIOD;							/* T = HIGH */
		PWM2_LT = (uint16) ((int16) PWM_REG_PERIOD - iPwm);	/* W = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* V-phase: HIGH, W-Phase: PWM */
		PWM3_LT = PWM_REG_PERIOD;							/* V = HIGH */
		PWM2_LT = (uint16) ((int16) PWM_REG_PERIOD - iPwm);	/* W = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	}
	else
	{
		/* 3rd and 4th Quadrant */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* W-phase: HIGH, U-Phase: PWM */
		PWM2_LT = PWM_REG_PERIOD;							/* W = HIGH */
		PWM4_LT = (uint16) ((int16) PWM_REG_PERIOD + iPwm);	/* U = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* W-phase: HIGH, T-Phase: PWM */
		PWM2_LT = PWM_REG_PERIOD;							/* W = HIGH */
		PWM5_LT = (uint16) ((int16) PWM_REG_PERIOD + iPwm);	/* T = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* W-phase: HIGH, V-Phase: PWM */
		PWM2_LT = PWM_REG_PERIOD;							/* W = HIGH */
		PWM3_LT = (uint16) ((int16) PWM_REG_PERIOD + iPwm);	/* V = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	}
	iPwm = (int16) (mulI32_I16byU16( c_ai16MicroStepVector4PH[u16MicroStepIdx + C_MICROSTEP_PER_FULLSTEP], g_u16CorrectionRatio) >> (19 + PWM_PRESCALER_N));
	if ( iPwm >= 0 )
	{
		/* 1st and 4th Quadrant */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* V-phase: HIGH, T-Phase: PWM */
		PWM3_LT = 0;										/* V = LOW */
		PWM5_LT = (uint16) iPwm;							/* T = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* U-phase: HIGH, V-Phase: PWM */
		PWM4_LT = 0;										/* U = LOW */
		PWM3_LT = (uint16) iPwm;							/* V = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* U-phase: HIGH, T-Phase: PWM */
		PWM4_LT = 0;										/* U = LOW */
		PWM5_LT = (uint16) iPwm;							/* T = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	}
	else
	{
		/* 2st and 3th Quadrant */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* T-phase: HIGH, V-Phase: PWM */
		PWM5_LT = 0;										/* T = LOW */
		PWM3_LT = (uint16) (0 - iPwm);						/* V = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* V-phase: HIGH, U-Phase: PWM */
		PWM3_LT = 0;										/* V = LOW */
		PWM4_LT = (uint16) (0 - iPwm);						/* U = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* T-phase: HIGH, U-Phase: PWM */
		PWM5_LT = 0;										/* T = LOW */
		PWM4_LT = (uint16) (0 - iPwm);						/* U = PWM */
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
	}
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */

	PWM1_LT = PWM_SCALE_OFFSET;													/* Master must be modified at last (value is not important) */

#if _SUPPORT_PHASE_SHORT_DET
	if ( (g_u16MicroStepIdx & (C_MICROSTEP_PER_FULLSTEP-1)) == (C_MICROSTEP_PER_FULLSTEP/2) )	/* Based on 4-phase, 32 micro-steps (per 8 micro-steps) */
	{
		ADC_SetupShortDetection( 0);
	}
#endif /* _SUPPORT_PHASE_SHORT_DET */
} /* End of MotorDriver_4PhaseStepper() */

/* ****************************************************************************	*
 * MotorDriverStart()
 *
 * Start Motor Driver
 * ****************************************************************************	*/
void MotorDriverStart( void)
{
	if ( g_e8ErrorElectric == (uint8) C_ERR_ELECTRIC_PERM )						/* Don't start motor in case of permanent electric failure */
	{
		return;
	}

#if USE_MULTI_PURPOSE_BUFFER
	/* Fill multi-purpose buffer with acceleration-data */
	{
		l_u16SpeedRPM = NVRAM_MIN_SPEED;
		l_u32Temp = divU32_U32byU16( (TIMER_CLOCK * 60U), g_u16MotorMicroStepsPerMechRotation);
		l_u16LowSpeedPeriod = divU16_U32byU16( l_u32Temp, l_u16SpeedRPM) - 1;
	}
#endif /* USE_MULTI_PURPOSE_BUFFER */

	if ( g_u8RewindFlags & (uint8) C_REWIND_STALL_DETECT )
	{
		if ( (g_u16MotorRewindSteps != 0) &&									/* MMP140331-1 */
			(((g_u8RewindFlags & (uint8) C_REWIND_DIRECTION_CCW) == g_e8MotorDirectionCCW) || (g_u8RewindFlags & C_REWIND_DIRECTION_AUTO)) )
		{
			/* Start rewind-function, with "rewinding" */
			g_u8RewindFlags = (uint8) (C_REWIND_ACTIVE | C_REWIND_REWIND);		/* Start rewind-process (MMP140331-1) */
			g_u16TargetPositionRewind = g_u16TargetPosition;
			if ( g_e8MotorDirectionCCW )
			{
				if ( g_u16ActualPosition <= (uint16) (C_MAX_POS - g_u16MotorRewindSteps) )
				{
					g_u16TargetPosition = g_u16ActualPosition + g_u16MotorRewindSteps;
					g_e8MotorDirectionCCW = FALSE;								/* MMP140331-3 */
				}
				else
				{
					g_u8RewindFlags = 0;										/* No rewind possible */
				}
			}
			else
			{
				if ( g_u16ActualPosition >= g_u16MotorRewindSteps )
				{
					g_u16TargetPosition = g_u16ActualPosition - g_u16MotorRewindSteps;
					g_e8MotorDirectionCCW = TRUE;								/* MMP140331-3 */
				}
				else
				{
					g_u8RewindFlags = 0;										/* No rewind possible */
				}
			}
		}
		else if ( (g_u8RewindFlags & (uint8) C_REWIND_DIRECTION_CCW) != g_e8MotorDirectionCCW ) /* MMP140331-1 - Begin */
		{
			g_u8RewindFlags = 0;												/* Clear previous detected stall flags */
		}																		/* MMP140331-1 - End */
	}

	g_u16ActuatorActPos = g_u16ActualPosition;
	g_u16ActuatorTgtPos = g_u16TargetPosition;
	//g_u8StallOcc = FALSE; TODO,Ban
	//g_u8MechError = FALSE;

	/* Clear motor-driver current measurement */
	MotorDriverCurrentMeasureInit();
#if _DEBUG_MOTOR_CURRENT_FLT
	l_u16MotorCurrIdx = 0;
#endif /* _DEBUG_MOTOR_CURRENT_FLT */

	g_u8MotorStartupMode = (uint8) MSM_STEPPER_A;								/* Start-up in Acceleration stepper mode */
	/* (MMP140331-2) g_u16StartupDelay = NVRAM_STALL_DETECTOR_DELAY; */
	MotorStallInitA();
#if _SUPPORT_STALLDET_O
	MotorStallInitO();
#endif /* _SUPPORT_STALLDET_O */
#if _SUPPORT_STALLDET_H
	MotorStallInitH();
#endif /* _SUPPORT_STALLDET_H */

#if _DEBUG_VOLTAGE_COMPENSATION
	u16MotorVoltIdx = 0;
#endif /* _DEBUG_VOLTAGE_COMPENSATION */
	l_u16CoilZeroCurrCountA = 0;
	l_u16CoilZeroCurrCountB = 0;
	l_u16CoilCurrentStartDelay = C_COIL_CURRENT_START_DELAY;

	/* Connect drivers */
	/* Stepper 4-phase/32-steps */

	{
		MotorDriver_InitialPwmDutyCycle( g_u16PidRunningThreshold, g_au16MotorSpeedRPS[1]);	/* MMP140822-1 - Begin */
	}
	MotorDriver_4PhaseStepper();
#if (_SUPPORT_PWM_DC_RAMPUP == FALSE)											/* MMP140903-2 - Begin */
	if ( g_u16MotorSpeedRPS > g_au16MotorSpeedRPS[1] )
	{
		MotorDriver_InitialPwmDutyCycle( g_u16PidRunningThreshold, g_u16MotorSpeedRPS);
	}																			/* MMP140822-1 - End */
#endif /* (_SUPPORT_PWM_DC_RAMPUP == FALSE) */									/* MMP140903-2 - End */
	DRVCFG_PWM_UVWT();															/* Enable the driver and the PWM phase W, V, U and T */
	g_u8MotorHoldingCurrState = FALSE;

	/* Setup ADC for Motor Current/Voltage measurements */
#if _SUPPORT_PHASE_SHORT_DET
	ADC_Start( 0);
#else  /* _SUPPORT_PHASE_SHORT_DET */
	ADC_Start();
#endif /* _SUPPORT_PHASE_SHORT_DET */

	l_u8VTIdx = 0;
	if ( g_u8MotorStartupMode == (uint8) MSM_STEPPER_A )
	{
		if ( g_u16TargetCommutTimerPeriod < l_u16LowSpeedPeriod )
		{
			/* Target speed too fast for motor to start-up with */
			g_u16CommutTimerPeriod = l_u16LowSpeedPeriod;						/* Initial start-up speed */
		}
		else
		{
			/* Target speed is slower than maximum motor start-up speed */
			g_u16CommutTimerPeriod = g_u16TargetCommutTimerPeriod;
		}
	}
	TMR1_REGB = g_u16CommutTimerPeriod;
	TMR1_CTRL = C_TMRx_CTRL_MODE0 | TMRx_START;									/* Start Timer mode 0 */
	g_e8MotorStatusMode = (uint8) C_MOTOR_STATUS_RUNNING;
	g_u8MotorStopDelay = 0;

	if ( (g_u8RewindFlags & (uint8) (C_REWIND_ACTIVE | C_REWIND_REWIND)) == (uint8) C_REWIND_ACTIVE )
	{
		g_u8RewindFlags &= (uint8) ~C_REWIND_ACTIVE;							/* Rewind-function is finished */
	}

} /* End of MotorDriverStart */

/* ****************************************************************************	*
 * MotorDriverStop()
 *
 * uit16 u16Immediate:	C_STOP_RAMPDOWN  : Ramp-down
 *						C_STOP_IMMEDIATE : Immediate stop (without ramp-down)
 *						C_STOP_EMERGENCY : Immediate stop (without ramp-down) + delay
 *
 * Stop Motor Driver
 * ****************************************************************************	*/
void MotorDriverStop( uint16 u16Immediate)
{
	if ( (g_e8MotorStatusMode & (uint8) ~C_MOTOR_STATUS_DEGRADED) != (uint8) C_MOTOR_STATUS_STOP )
	{
		/* Not STOP status */
		if ( (u16Immediate == (uint16) C_STOP_RAMPDOWN) && (l_u8VTIdx > 1) ) /*lint !e845 */	/* MMP150922-1 */
		{
			TMR1_CTRL = C_TMRx_CTRL_MODE0 | TMRx_START;							/* Start timer mode 0 */

			/* Request to ramp-down */
			if ( (g_e8MotorStatusMode & (uint8) ~C_MOTOR_STATUS_DEGRADED) == (uint8) C_MOTOR_STATUS_STOPPING )
			{
				/* Already stopping */
				g_u8MotorStartDelay = (uint8) C_PI_TICKS_10MS;					/* Motor status change to STOP soon */
				return;
			}

			/* Set TargetPos near CurrentPos, including ramp-down */
			{
				if ( g_u16ActuatorActPos > g_u16ActuatorTgtPos )
				{
					uint32 u32DeltaPos = g_u16ActuatorActPos - g_u16ActuatorTgtPos;
					if ( u32DeltaPos > l_u8VTIdx )
					{
						g_u16ActuatorTgtPos = g_u16ActuatorActPos - l_u8VTIdx;
					}
				}
				else
				{
					uint32 u32DeltaPos = g_u16ActuatorTgtPos - g_u16ActuatorActPos;
					if ( u32DeltaPos > l_u8VTIdx )
					{
						g_u16ActuatorTgtPos = g_u16ActuatorActPos + l_u8VTIdx;
					}
				}
			}

			g_e8MotorStatusMode = ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED) | (uint8) C_MOTOR_STATUS_STOPPING);
			g_u8MotorStartDelay = (uint8) C_PI_TICKS_20MS;						/* Motor status change to STOP when ramp-down is finished */
			return;
		}
		else if ( u16Immediate == (uint16) C_STOP_EMERGENCY )
		{
			g_u8MotorStartDelay = (uint8) C_PI_TICKS_10MS;						/* Wait 10 (up to 20 ms) before continue */
			if ( g_u8MotorStatusSpeed > (uint8) C_MOTOR_SPEED_MID )
			{
				g_u8MotorStartDelay = (uint8) C_PI_TICKS_20MS;
			}
		}
		else
		{
			g_u8MotorStartDelay = 0;
		}
	}

	/* Re-stall code */
	if ( (g_u8StallOcc != FALSE) && ((g_u8RewindFlags & (uint8) C_REWIND_ACTIVE) == 0) )
	{
		/* 'Restore' actual-position in case of re-stall without rewind; MMP140331-4 - Begin */
		if ( (NVRAM_REWIND_STEPS == 0) && ((g_u8RewindFlags & (uint8) C_REWIND_STALL_DETECT) != 0) )
		{
			/* Stall detected before (no rewind support) */
			if ( g_e8MotorDirectionCCW )
			{
				if ( (g_u8RewindFlags & (uint8) C_REWIND_DIRECTION_CCW) != 0 )
				{
					/* Stall in same direction; 'Restore' actual-position */
					g_u16ActuatorActPos += (l_u16StartupDelayInit + (C_MICROSTEP_PER_FULLSTEP << NVRAM_STALL_O_OFFSET));		/* MMP140428-1 */
				}
			}
			else
			{
				if ( (g_u8RewindFlags & (uint8) C_REWIND_DIRECTION_CCW) == 0 )
				{
					/* Stall in same direction; 'Restore' actual-position */
					if ( g_u16ActuatorActPos > (l_u16StartupDelayInit + (C_MICROSTEP_PER_FULLSTEP << NVRAM_STALL_O_OFFSET)) )	/* MMP140428-1 */
					{
						g_u16ActuatorActPos -= (l_u16StartupDelayInit + (C_MICROSTEP_PER_FULLSTEP << NVRAM_STALL_O_OFFSET));	/* MMP140428-1 */
					}
					else
					{
						g_u16ActuatorActPos = 0;
					}
				}
			}
		}																		/* MMP140331-4 - End */

		/* Set re-wind active */
		g_u8RewindFlags |= (uint8) C_REWIND_STALL_DETECT;
		if ( g_e8MotorDirectionCCW )
		{
			g_u8RewindFlags |= (uint8) C_REWIND_DIRECTION_CCW;
		}
		else
		{
			g_u8RewindFlags &= (uint8) ~C_REWIND_DIRECTION_CCW;
		}
	}

	/* First stop ADC, before stopping motor (trigger-event) */
	ADC_Stop();
	g_u8MotorStartupMode = (uint8) MSM_STOP;									/* Stop mode */
	g_e8MotorStatusMode = ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED) | (uint8) C_MOTOR_STATUS_STOP);
	if ( bistResetInfo != C_CHIP_STATE_WATCHDOG_RESET )
	{
		/* make target-position same as actual position, except in case of WD-reset */
		g_u16ActuatorTgtPos = g_u16ActuatorActPos;							/* Stop: Target = Actual */
	}
	g_u16ActualPosition = g_u16ActuatorActPos;
	g_u8MotorStatusSpeed = (uint8) C_MOTOR_SPEED_STOP;							/* Stop */

#if (LINPROT == LIN2J_VALVE_GM)
	if ( g_e8CalibrationStep == (uint8) C_CALIB_DONE )
	{
		/* Check for mechanical defect (1 = calculation rounding) */
		if ( (g_u16ActualPosition <= 1) || (g_u16ActualPosition >= ((g_u16CalibTravel + (2*C_PERC_OFFSET)) - 1)) )
		{
			g_u8MechError = TRUE;
		}

		/* Round actual position */
		if ( g_u16ActualPosition < (C_PERC_OFFSET + C_HALFPERC_OFFSET) )
		{
			g_u16ActualPosition = C_PERC_OFFSET;
		}
		else if ( g_u16ActualPosition > ((g_u16CalibTravel + C_PERC_OFFSET) - C_HALFPERC_OFFSET) )
		{
			g_u16ActualPosition = (g_u16CalibTravel + C_PERC_OFFSET);
		}
	}
#endif /* (LINPROT == LIN2J_VALVE_GM) */


	if ( (g_u8MotorHoldingCurrEna != FALSE) &&									/* Holding mode enabled */
		(g_e8ErrorElectric != (uint8) C_ERR_ELECTRIC_PERM) && (g_e8ErrorVoltage == (uint8) C_ERR_VOLTAGE_IN_RANGE) && (u16Immediate != (uint16) C_STOP_SLEEP) ) /*lint !e845 */
	{
		/* Keep Motor driver active with a specified amount of current (unless permanent electric error) */
		MotorDriver_InitialPwmDutyCycle( g_u16PidHoldingThreshold, 0);
	
		MotorDriver_4PhaseStepper();
		DRVCFG_PWM_UVWT();														/* Enable the driver and the PWM phase W, V and U */
		g_u8MotorHoldingCurrState = TRUE;

		g_u16MotorCurrentLPFx64 = (g_u16PidHoldingThreshold << 6);				/* Low-pass Filtered motor-current (x 64) */
#if _SUPPORT_PHASE_SHORT_DET
		ADC_Start( 0);															/* Start measuring motor current */
#else  /* _SUPPORT_PHASE_SHORT_DET */
		ADC_Start();
#endif /* _SUPPORT_PHASE_SHORT_DET */
	}
	else
	{
		/* Disconnect drivers */
		if ( g_e8ErrorElectric != (uint8) C_ERR_ELECTRIC_PERM )					/* MMP130919-1 - Begin */
		{
			if ( g_e8ErrorElectric != (uint8) C_ERR_ELECTRIC_YES )				/* MMP150217-1 - Begin */
			{
				DRVCFG_GND_UVWT();												/* Make Low-side active, for a short time (recycle current) */
			}
			else
			{
				DRVCFG_VSUP_UVWT();												/* Make High-side active, for a short time (recycle current) */
			}																	/* MMP150217-1 - End */
		}
		else
		{
			/* In case of a permanent error, don't connect drivers anymore */
			DRVCFG_DIS_UVWT();
			DRVCFG_DIS();														/* MMP140903-1 */
		}																		/* MMP130919-1 - End */
		g_u8MotorHoldingCurrState = FALSE;

		g_u8MotorStopDelay = 200;												/* 200x 0.5ms = 100ms delay before driver is disconnected */
	}

	TMR1_CTRL &= ~TMRx_START;													/* Stop "commutation timer" */
	XI0_PEND = CLR_T1_INT4;														/* Clear (potentially pending) Timer1 second level interrupts (T1_INT4) */
	PEND = CLR_EXT0_IT;															/* ... and first level interrupt */
#if USE_MULTI_PURPOSE_BUFFER
	g_MPBuf.u8Usage = (uint8) C_MP_BUF_FREE;									/* Motor-stopped: Multi-purpose buffer is free for others */
#endif /* USE_MULTI_PURPOSE_BUFFER */

	/* Re-stall code */
	if ( (g_u8RewindFlags & (uint8) (C_REWIND_ACTIVE | C_REWIND_REWIND)) == (uint8) (C_REWIND_ACTIVE | C_REWIND_REWIND) )
	{
		g_u8RewindFlags &= (uint8) ~C_REWIND_REWIND;							/* Rewinding of the Rewind-function is finished */
		g_u16TargetPosition = g_u16TargetPositionRewind;
		g_u8MotorStopDelay = 0;													/* Cancel stop delay */
		if ( g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_CALIBRATION )
		{
			if ( g_e8CalibrationStep == (uint8) C_CALIB_CHECK_HI_ENDPOS )
				g_e8CalibrationStep = C_CALIB_SETUP_HI_ENDPOS;
			else if ( g_e8CalibrationStep == (uint8) C_CALIB_CHECK_LO_ENDPOS )
				g_e8CalibrationStep = C_CALIB_SETUP_LO_ENDPOS;
		}
		else
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_START;
	}
	else if ( (g_u8RewindFlags & (uint8) C_REWIND_STALL_DETECT) == 0 )
	{
		g_u8RewindFlags = 0;													/* Clear all other flags in case no STALL have been detected */
	}

#if (_SUPPORT_BUSTIMEOUT_SLEEP != FALSE)										/* MMP130626-8 */
	if ( (g_u8EmergencyRunOcc != FALSE) && (g_u8ErrorCommBusTimeout != FALSE) &&
		 (g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_NONE) && (g_e8DegradedMotorRequest == (uint8) C_MOTOR_REQUEST_NONE) )
	{
		g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_SLEEP;	
	}
#endif /* (_SUPPORT_BUSTIMEOUT_SLEEP != FALSE) */

} /* End of MotorDriverStop */

/* ****************************************************************************	*
 * Commutation_ISR()
 *
 * MotorDriver Commutation Interrupt.
 * Perform next motor step. If motor speed not reached, accelerate.
 * For STEPPER/3-PHASE this ISR takes approximate 48us (longer then one Motor PWM period !!)
 *
 * Note: The Commutation_ISR() not be interrupted by LIN communication, also not by LIN status request.
 * The g_u8StallOcc flags can be set in this Commutation_ISR(), but cleared in the MotorDriverStop(),
 * before it is communicated back to the ECU/Master.
 * ****************************************************************************	*/
#define Commutation_ISR	EXT0_IT
__interrupt__ void Commutation_ISR(void)
{
#if (_DEBUG_COMMUT_ISR != FALSE)
	DEBUG_SET_IO_B();
#endif /* (_DEBUG_COMMUT_ISR != FALSE) && (_DEBUG_HALLLATCH_ISR == FALSE) */

	uint16 pending = XI0_PEND & XI0_MASK;										/* Copy interrupt requests which are not masked   */
	do
	{
		XI0_PEND = pending;														/* Clear requests which are going to be processed */
	} while (XI0_PEND & pending);

	if ( (g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) == 0 )
	{
		return;		/* Used for CPU wake-up */
	}

	if ( g_e8MotorDirectionCCW )
	{
		g_u16ActuatorActPos--;													/* Closing */
	}
	else
	{
		g_u16ActuatorActPos++;													/* Opening */
	}

	{
		int32 i32DeltaPosition = (int32)g_u16ActuatorActPos - (int32)g_u16ActuatorTgtPos;
		if ( i32DeltaPosition == 0 )
		{
			MotorDriverStop( (uint16) C_STOP_IMMEDIATE);
			return;
		}
		if ( i32DeltaPosition < 0 )
		{
			i32DeltaPosition = -i32DeltaPosition;
		}
		if ( i32DeltaPosition <= (int16) l_u8VTIdx )
		{
			/* Decelerate motor speed (almost at target-position) */
			g_u16StartupDelay = (uint16) i32DeltaPosition;
			g_u16TargetCommutTimerPeriod = l_u16LowSpeedPeriod;
			g_e8MotorStatusMode |= (uint8) C_MOTOR_STATUS_STOPPING;
		}
	}

	/* Current measurement used for Stall-detector "A" and current control (PID) */
	MotorDriverCurrentMeasure();
	/* Coil current check */
	if(l_u16CoilCurrentStartDelay == 0)
	{
		if(g_u16CurrentMotorCoilA < (l_u16CurrentZeroOffset + C_MIN_COIL_CURRENT))
		{
			l_u16CoilZeroCurrCountA++;
		}
		else if ( l_u16CoilZeroCurrCountA != 0 )
		{
			l_u16CoilZeroCurrCountA--;
		}

		if(g_u16CurrentMotorCoilB < (l_u16CurrentZeroOffset + C_MIN_COIL_CURRENT))
		{
			l_u16CoilZeroCurrCountB++;
		}
		else if ( l_u16CoilZeroCurrCountB != 0 )
		{
			l_u16CoilZeroCurrCountB--;
		}

		if((l_u16CoilZeroCurrCountA >= C_COIL_ZERO_CURRENT_COUNT) || (l_u16CoilZeroCurrCountB >= C_COIL_ZERO_CURRENT_COUNT))
		{
			MotorDriverStop( (uint16) C_STOP_IMMEDIATE);
			g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
			SetLastError( (uint8) C_ERR_COIL_ZERO_CURRENT);
			g_e8ErrorCoil = (uint8) C_ERR_SELFTEST_C;//coil open
			return;
		}
	}else if(l_u16CoilCurrentStartDelay > 0){
		l_u16CoilCurrentStartDelay--;
	}else{
		l_u16CoilCurrentStartDelay = 0;
	}



	/* Update micro-step index */
	{
		uint16 u16MicroStepIdx = g_u16MicroStepIdx;
		if ( g_e8MotorDirectionCCW )
		{
			/* Counter Clock-wise (Closing) */
			if ( u16MicroStepIdx == 0 )
			{
				u16MicroStepIdx = g_u16MotorMicroStepsPerElecRotation;
			}
			u16MicroStepIdx--;													/* Decrement the PWM vector pointer */
		}
		else
		{
			/* Clock-wise (Opening) */
			u16MicroStepIdx++;													/* Increment the PWM vectors pointer */
			if ( u16MicroStepIdx >= g_u16MotorMicroStepsPerElecRotation )		/* Test the PWM vectors pointer: 48 usteps per electrical period */
			{
				u16MicroStepIdx = 0;											/* Re-initialise the PWM vectors pointer to 0 */
			}
		}
		g_u16MicroStepIdx = u16MicroStepIdx;
	}

	/* Check for speed update required */
	if ( g_u16CommutTimerPeriod == g_u16TargetCommutTimerPeriod )
	{
		g_u8MotorStartupMode = (uint8) MSM_STEPPER_C;
	}
	else
	{
		/* Update speed */
		uint16 u16Compensation = l_u16SpeedRPM;							//MMP160606-1
		if ( g_u16CommutTimerPeriod < g_u16TargetCommutTimerPeriod )
		{
			/* Deceleration per micro-step */
			g_u8MotorStartupMode = (uint8) MSM_STEPPER_D;					/* Too fast, decelerate */
			l_u16SpeedRPM = l_u16SpeedRPM - divU16_U32byU16( (uint32) 2*NVRAM_ACCELERATION_CONST, l_u16SpeedRPM);	/* MMP160606-1 */
			g_u16CommutTimerPeriod = divU16_U32byU16( l_u32Temp, l_u16SpeedRPM) - 1;	/* MMP160606-1 */
			l_u8VTIdx--;
			if ( g_u16StartupDelay < l_u8VTIdx )
			{
				g_u16StartupDelay = l_u16StartupDelayInit;						/* MMP130627-1/MMP140331-2: Speed reduction, stall detection post-poned */
			}
			if ( g_u16CommutTimerPeriod > g_u16TargetCommutTimerPeriod )
			{
				g_u16CommutTimerPeriod = g_u16TargetCommutTimerPeriod;
			}
			TMR1_REGB = g_u16CommutTimerPeriod;
#if (_SUPPORT_PWM_DC_RAMPDOWN != FALSE)											/* MMP140903-2 - Begin */
			g_u16PidCtrlRatio = muldivU16_U16byU16byU16( g_u16PidCtrlRatio, l_u16SpeedRPM, u16Compensation);	/* MMP160606-2 */
			g_u16PID_I = g_u16PidCtrlRatio;
#endif /* (_SUPPORT_PWM_DC_RAMPDOWN != FALSE) */								/* MMP140903-2 - Begin */

//			g_u16CommutTimerPeriod = VELOCITY_TIMER[l_u8VTIdx];
//			if ( g_u16CommutTimerPeriod > g_u16TargetCommutTimerPeriod )
//			{
//				g_u16CommutTimerPeriod = g_u16TargetCommutTimerPeriod;
//			}
//			else if ( l_u8VTIdx != 0 )
//			{
//				l_u8VTIdx--;
//				if ( g_u16StartupDelay < l_u8VTIdx )
//				{
//					g_u16StartupDelay = l_u16StartupDelayInit;					/* MMP130627-1/MMP140331-2: Speed reduction, stall detection post-poned */
//				}
//			}
//			TMR1_REGB = g_u16CommutTimerPeriod;
//			g_u8MotorStartupMode = (uint8) MSM_STEPPER_D;						/* Too fast, decelerate */
//#if (_SUPPORT_PWM_DC_RAMPDOWN != FALSE)											/* MMP140903-2 - Begin */
//			/* Reduce the PWM-duty cycle to avoid current increase (wrong stall detection) (254/256) */
//			g_u16PidCtrlRatio = (uint16) (mulU32_U16byU16( g_u16PidCtrlRatio, 254) >> 8);
//			g_u16PID_I = g_u16PidCtrlRatio;
//#endif /* (_SUPPORT_PWM_DC_RAMPDOWN != FALSE) */								/* MMP140903-2 - Begin */
		}
		else if ( (g_u16MicroStepIdx == 0) || ((g_u16MicroStepIdx > NVRAM_ACCELERATION_POINTS) && ((g_u16MicroStepIdx & NVRAM_ACCELERATION_POINTS) == 0)) )
		{
			/* Acceleration per acceleration_points ((multiple) full-step) */
			g_u8MotorStartupMode = (uint8) MSM_STEPPER_A;					/* Too slow, accelerate */
			l_u16SpeedRPM = l_u16SpeedRPM + divU16_U32byU16( (uint32) 2*NVRAM_ACCELERATION_CONST, l_u16SpeedRPM);	/* MMP160606-1 */
			g_u16CommutTimerPeriod = divU16_U32byU16( l_u32Temp, l_u16SpeedRPM) - 1;	/* MMP160606-1 */
			l_u8VTIdx++;
			if ( g_u16CommutTimerPeriod < g_u16TargetCommutTimerPeriod )	/* MMP150923-1 */
			{
				g_u16CommutTimerPeriod = g_u16TargetCommutTimerPeriod;
			}
			TMR1_REGB = g_u16CommutTimerPeriod;
#if (_SUPPORT_PWM_DC_RAMPUP != FALSE)											/* MMP140903-2 - Begin */
			g_u16PidCtrlRatio = muldivU16_U16byU16byU16( g_u16PidCtrlRatio, l_u16SpeedRPM, u16Compensation);	/* MMP160606-2 */
			g_u16PID_I = g_u16PidCtrlRatio;
#endif /* (_SUPPORT_PWM_DC_RAMPUP != FALSE) */									/* MMP140903-2 - End */

//			g_u16CommutTimerPeriod = VELOCITY_TIMER[l_u8VTIdx];
//			if ( g_u16CommutTimerPeriod < g_u16TargetCommutTimerPeriod )
//			{
//				g_u16CommutTimerPeriod = g_u16TargetCommutTimerPeriod;
//			}
//			else if ( l_u8VTIdx < ((sizeof(VELOCITY_TIMER)/sizeof(VELOCITY_TIMER[0])) - 1) )
//			{
//				l_u8VTIdx++;
//#if (_SUPPORT_PWM_DC_RAMPUP != FALSE)											/* MMP140903-2 - Begin */
//				if ( g_u16MotorSpeedRPS > g_au16MotorSpeedRPS[1] )
//				{
//					g_u16PidCtrlRatio = g_u16PID_I = g_u16PidCtrlRatio + (g_u16PidCtrlRatio >> 7);
//				}
//#endif /* (_SUPPORT_PWM_DC_RAMPUP != FALSE) */									/* MMP140903-2 - Begin */
//			}
//			TMR1_REGB = g_u16CommutTimerPeriod;
//			g_u8MotorStartupMode = (uint8) MSM_STEPPER_A;						/* Too slow, accelerate */
		}
	}

	VoltageCorrection();

	MotorDriver_4PhaseStepper();
	if ( MotorStallCheckA() != (uint16) C_STALL_NOT_FOUND )						/* Stall-detector "A" */
	{
		g_u8StallTypeComm |= (uint8) C_STALL_FOUND_A;
		if ( g_e8StallDetectorEna & ((uint8) C_STALLDET_A | (uint8) C_STALLDET_CALIB))	/* MMP130916-1 */
		{
			g_u8StallOcc = TRUE;												/* Report stall and ...  */
			MotorDriverStop( (uint16) C_STOP_EMERGENCY);						/* ... stop motor (Stall) */
		}
	}
#if _SUPPORT_STALLDET_O
	else if ( (NVRAM_STALL_O != 0) && (MotorStallCheckO() != C_STALL_NOT_FOUND) )	/* Stall-detector "O" (MMP140428-1) */
	{
		g_u8StallTypeComm |= (uint8) C_STALL_FOUND_O;
		if ( g_e8StallDetectorEna & ((uint8) C_STALLDET_O | (uint8) C_STALLDET_CALIB))
		{
			g_u8StallOcc = TRUE;												/* Report stall and ...  */
			MotorDriverStop( (uint16) C_STOP_EMERGENCY);						/* ... stop motor (Stall) */
		}
	}
#endif /* _SUPPORT_STALLDET_O */
#if _SUPPORT_STALLDET_H
	/* Stall detection based on hall-sensor(s) */
	else if ( MotorStallCheckH() != (uint8) C_STALL_NOT_FOUND )					/* Stall-detector "H" */
	{
		g_u8StallTypeComm |= (uint8) C_STALL_FOUND_H;
		if ( g_e8StallDetectorEna & ((uint8) C_STALLDET_H | (uint8) C_STALLDET_CALIB))									/* MMP130916-1 */
		{
			g_u8StallOcc = TRUE;												/* Report stall and ...  */
			MotorDriverStop( (uint16) C_STOP_EMERGENCY);						/* ... stop motor (Stall) */
		}
	}
#endif /* _SUPPORT_STALLDET_H */

#if _DEBUG_COMMUT_ISR
	DEBUG_CLR_IO_B();
#endif /* _DEBUG_COMMUT_ISR */
} /* End of Commutation_ISR() */

/* EOF */
