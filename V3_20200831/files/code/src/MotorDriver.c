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
#include "ADC.h"																/* ADC support */
#include "ErrorCodes.h"															/* Error logging support */
#include "Diagnostic.h"															/* Motor diagnostic support */
#include "MotorStall.h"															/* Motor stall detector support */
#include "MotorDriverTables.h"
#include "NVRAM_UserPage.h"														/* NVRAM User-page support */
#include "PID_Control.h"														/* PID-controller support */
#include "Timer.h"																/* Periodic IRQ Timer support */
#include "private_mathlib.h"
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */
#include "lib_mlx315_misc.h"

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION ( TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp
uint8 l_u8VTIdx = 0u;
volatile uint16 g_u16CorrectionRatio;											/* Motor correction ratio, depend on temperature and voltage */
uint16 g_u16MicroStepIdx;														/* (Micro)step index */
uint16 g_u16CommutTimerPeriod;													/* Commutation timer period */
uint16 g_u16TargetCommutTimerPeriod;											/* Target commutation timer period (target speed) */
uint16 g_u16StartupDelay = 2u * C_MOVAVG_SZ;
uint16 g_u16MotorCurrentMovAvgxN;												/* Moving average current (4..16 samples) */
uint16 g_u16MotorCurrentLPFx64;													/* Low-pass filter (IIR-1) motor-current x 64 */
uint8 g_u8MotorStopDelay = 0u;													/* Delay between drive stage from LS to TRI-STATE */

uint8 g_e8MotorDirectionCCW;													/* Control/Status-flag motor rotational direction Counter Clock-wise */
volatile uint16 g_u16ActuatorActPos;
volatile uint16 g_u16ActuatorTgtPos;

uint16 l_u16ActuatorBufferedActPos;
uint16 l_u16ActuatorBufferedTgtPos;

uint16 l_u16ActuatorBufferedSpdRPM;

volatile Motor_FaultStatus g_sMotorFault = 
{
	0,				/* Under Voltage */
	0,				/* Over Voltage */
	0,				/* Open Load */
	0,				/* Short Load */
	0,				/* Thermal Warning */
	0,				/* Thermal Shutdown */
	0,				/* Stall */
	0,				/* Drift */
	0,				/* Reserved */
};

/* MMP151118-2 */
#pragma space none

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	( NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp																/* __NEAR_SECTION__ */
uint8 g_u8MotorStartupMode = (uint8) MSM_STOP;									/* 4: Motor STOP state */
uint8 g_u8MotorHoldingCurrState = FALSE;										/* Motor Holding Current State */
uint16 g_u16MotorSpeedRPS;														/* 4: Target motor-speed [RPS] */

uint16 l_au16MotorCurrentRaw[C_MOVAVG_SZ];
uint16 l_u16MotorCurrentRawIdx;
uint16 l_u16StartupDelayInit = 2u*C_MOVAVG_SZ;									/* Initial startup-delay [uSteps] */

uint16 l_u16SpeedRPM;															/* MMP160606-1 */
uint32 l_u32Temp;																/* MMP160606-1 */
uint16 l_u16LowSpeedPeriod;														/* MMP160606-1 */

uint16 g_au16MotorSpeedCommutTimerPeriod[8];
uint16 g_au16MotorSpeedRPS[8];
uint16 g_u16MotorMicroStepsPerElecRotation;										/* Number of micro-steps per electric rotation */
uint16 g_u16MotorMicroStepsPerMechRotation;										/* Number of micro-steps per mechanical rotation */

uint8 g_u8MotorStatusSpeed = (uint8) C_MOTOR_SPEED_STOP;						/* (Status) Actual motor-speed */

uint8 l_u8MotorStatus = C_MOTOR_STATUS_STOP;
uint8 l_u8MotorRequest = C_MOTOR_CTRL_STOP;

    
#if (USE_MULTI_PURPOSE_BUFFER == FALSE)
uint16 l_au16VelocityTimer[VT_BUF_SZ];
#endif /* (USE_MULTI_PURPOSE_BUFFER == FALSE) */

#if _DEBUG_MOTOR_CURRENT_FLT
uint8 l_au8MotorCurrRaw[C_MOTOR_CURR_SZ];										/* Raw (un-filtered) motor current measurement */
uint16 l_u16MotorCurrIdx;														/* Motor current measurement index */
#endif /* _DEBUG_MOTOR_CURRENT_FLT */

#if MOTOR_PARAMS == MP_NVRAM
MOTOR_CALIBPARAMS MotorParams;											/* Motor calibration params shadow RAM */
#endif


#pragma space none																/* __NEAR_SECTION__ */

#if MOTOR_PARAMS == MP_NVRAM
/* system configuration and calibration info: */
const MOTOR_CALIBPARAMS MotorCalibParamsDefault = 
{
	/* Actuator (VALVE): [6] */
	C_DEF_TRAVEL,											/* 0x10: Default Travel */
	C_DEF_TRAVEL_TOLERANCE_LO,								/* 0x12: Default Travel Tolerance (Lower) */
	C_DEF_TRAVEL_TOLERANCE_HI,								/* 0x13: Default Travel Tolerance (Upper) */
	C_DEF_EMRUN_POS,										/* 0x14: Emergency Run stop position  */
	
	/* Motor: [8] */
	CONFIGURATION_ID,										/* 0x16: Configuration ID */
	MOTOR_FAMILY,											/* 0x18: Motor Family */
	(4u - MOTOR_PHASES),									/* 0x19 [3:0]: Number of motor-phases: 1 = 3-phase, 0 = Bi-polar */
	MOTOR_POLE_PAIRS,										/* 0x19 [7:4]: Number of pole-pairs */
	C_MOTOR_CONST_10MV_PER_RPS,								/* 0x1A: Motor Constant BEMF [10mV/rps] */
	C_COILS_RTOT,											/* 0x1B: Motor coil resistance (total) */
	MOTOR_MICROSTEPS,										/* 0x1C [3:0]: Number of micro-steps: 2^n (or 1 << n) */
	MOTOR_DIR_INV,											/* 0x1C[7:4]: Motor rotational direction: 0=CW, 1=CCW */
	0xFF,													/* 0x1D: Reserved */

	/* current/voltage: [6] */
	(C_PID_HOLDING_CURR_LEVEL / 2u),						/* 0x1E: Holding Torque current threshold */
	(C_PID_RUNNING_CURR_LEVEL / 2u),						/* 0x1F: Running Torque current threshold */
	C_LEAD_ANGLE,											/* 0x20 [5:0]: Lead Angle */
	C_BROWN_OUT_LEVEL,										/* 0x20 [7:6]: Brown-out level */
	C_VSUP_REF,												/* 0x21: Vsupply reference [1/8V] */
	C_VDS_THRESHOLD,										/* 0x22: Vds Threshold (MMP140428-1) [1/8V] */
	0xFF,													/* 0x23: Reserved */
	MOTOR_GEAR_BOX_RATIO,									/* 0x24: Gear-box ratio, e.g. 600:1 */ 

	/* stall: [8]*/
	C_STALL_B_DELAY,										/* 0x26: Stall-detector "B" delay */
	C_STALL_B_THRESHOLD,									/* 0x27.: Stall-detector "B" threshold (BEMF) */
	C_STALL_O_WIDTH,										/* 0x28.: Stall-detector "O" Width */
	C_STALL_O_THRESHOLD,									/* 0x29.: Stall-detector "O" threshold (Oscillation) */
	C_STALL_A_THRESHOLD,									/* 0x2A.: Stall current threshold ratio */
	C_STALL_SPEED_DEPENDED,									/* 0x2B: Stall speed depended */
	((C_DETECTOR_DELAY + 4u) >> 3u),						/* 0x2C: Stall detector delay */
	0xFF,													/* 0x2D: Reserved */

	/* Four speed points:[8] */
	C_SPEED_0,												/* 0x2E: Speed_0 */
	C_SPEED_1,												/* 0x30: Speed_1 */
	C_SPEED_2,												/* 0x32: Speed_2 */
	C_SPEED_3,												/* 0x34: Speed_3 */

	/* speed:acceleration/deceleration: [6] */
	C_SPEED_MIN,											/* 0x36: Minimum speed */
	C_ACCELERATION_CONST,									/* 0x38: Acceleration-constant */
	C_ACCELERATION_POINTS,									/* 0x3A [5:0]: Acceleration-points */
	C_TACHO_MODE,											/* 0x3A [7:6]: Tacho-mode */
	C_DECELERATION_STEPS,									/* 0x3B: Deceleration-(u)Steps (MMP130819-1) */

	/* Application diagnostic levels: [4] */
	(uint16)(C_APPL_UTEMP + C_TEMPOFF),						/* 0x3C: Application under-temperature */
	(uint16)(C_APPL_OTEMP_WARN + C_TEMPOFF),				/* 0x3D: Application over-temperature warn */
	(uint16)(C_APPL_OTEMP_SHUT + C_TEMPOFF),				/* 0x3E: Application over-temperature shut */
	(uint16)C_APPL_UVOLT,									/* 0x3F: Application under-voltage */
	(uint16)C_APPL_OVOLT,									/* 0x40: Application over-voltage */
	0xFF,													/* 0x41: Reserved */
	
	/* Current vs. Temperature compensation:[10] */
	(uint16)(C_CURRTHRSHLD_TEMP_1 + C_TEMPOFF),				/* 0x42: Current threshold temperature #1 */
	C_CURRTHRSHLD_RATIO_1,									/* 0x43: Current threshold ratio #1 */
	(uint16)(C_CURRTHRSHLD_TEMP_2 + C_TEMPOFF),				/* 0x44: Current threshold temperature #2 */
	C_CURRTHRSHLD_RATIO_2,									/* 0x45: Current threshold ratio #2 */
	(uint16)(C_CURRTHRSHLD_TEMP_3 + C_TEMPOFF),				/* 0x46: Current threshold temperature #3 */
	C_CURRTHRSHLD_RATIO_3,									/* 0x47: Current threshold ratio #3 */
	(uint16)(C_CURRTHRSHLD_TEMP_4 + C_TEMPOFF),				/* 0x48: Current threshold temperature #4 */
	C_CURRTHRSHLD_RATIO_4,									/* 0x49: Current threshold ratio #4 */
	C_CURRTHRSHLD_AREA_1,									/* 0x4A.[0]: Zone I */ 
	C_CURRTHRSHLD_AREA_2,									/* 0x4A.[2:1]: Zone II */ 
	C_CURRTHRSHLD_AREA_3,									/* 0x4A.[4:3]: Zone III */ 
	C_CURRTHRSHLD_AREA_4,									/* 0x4A.[6:5]: Zone IV */ 
	C_CURRTHRSHLD_AREA_5,									/* 0x4A.[7]: Zone V */ 
	0xFF,													/* 0x4B: Reserved */
	
	/* PID Control:0x4C+ [12] */
	C_PID_RUNNINGCTRL_PERIOD,								/* 0x4C: PID running control-period */
	C_PID_HOLDINGCTRL_PERIOD,								/* 0x4D: PID holding control-period */
	C_PID_COEF_P,											/* 0x4E: PID-Coefficient P */
	C_PID_COEF_I,											/* 0x4F: PID-Coefficient I */
	C_PID_COEF_D,											/* 0x50: PID-Coefficient D */
	C_PID_THRSHLDCTRL_PERIOD,								/* 0x51: PID-period for threshold control */
	C_MIN_HOLDCORR_RATIO,									/* 0x52: PID Lower-limit Holding (output) */
	C_MIN_CORR_RATIO,										/* 0x53: PID Lower-limit Running (output) */
	C_MAX_CORR_RATIO,										/* 0x54: PID Upper-limit (output) */
	0,														/* 0x55: PID Ramp-down limitation */
	0,														/* 0x56: PID Ramp-up limitation */
	0xFF,													/* 0x57: reserved */
};
#endif

/* ****************************************************************************	*
 *	local functions declaration												*
 * ****************************************************************************	*/
void MotorDriverStart( void );
void MotorDriverStop( uint16 u16Immediate );
void MotorDriver_InitialPwmDutyCycle( uint16 u16CurrentLevel, uint16 u16MotorSpeed );
void MotorDriverCurrentMeasureInit( void );
void MotorDriverCurrentMeasure( void );
void MotorDriver_4PhaseStepper( void );


/* public functions implementation */

void MotorDriverSetParams(Motor_ControlParams params)
{
	l_u8MotorRequest = params.MotorCtrl;
	if( params.TgtPos != 0xFFFFu )
	{
		l_u16ActuatorBufferedTgtPos = params.TgtPos;
	}
	/* argument check:actuator actual position should not be invalid  */
	if( params.ActPos != 0xFFFFu )
	{
		g_u16ActuatorActPos = params.ActPos;
	}

	/* update target speed */
	if( params.SpdRPM != 0xFFFFu )
	{
		l_u16ActuatorBufferedSpdRPM = params.SpdRPM;
	}
}

void MotorDriverGetStatus(Motor_RuntimeStatus *pstatus)
{
	pstatus->Mode = g_u8MotorStartupMode;
	pstatus->ActPos = g_u16ActuatorActPos;
	pstatus->TgtPos = g_u16ActuatorTgtPos;
	pstatus->Fault = g_sMotorFault;
	pstatus->Direction = g_e8MotorDirectionCCW;
}

void MotorDriverClearFaultStatus(void)
{
	/* clear permanent mechanical or eletric error,latched */
	g_sMotorFault.DRIFT = 0;
	g_sMotorFault.ST = 0;
	g_sMotorFault.SHORT = 0;
	g_sMotorFault.OPEN = 0;
}

void MotorDriver_MainFunction(void)
{
	/* Diagnostic */
	MotorDiagnosticVsupplyAndTemperature();

	/* Diagnostic protection:motor transfer to degrade mode */
	if((g_sMotorFault.UV != 0u) || (g_sMotorFault.OV != 0u) || 
		(g_sMotorFault.OPEN != 0u) || (g_sMotorFault.SHORT != 0u) ||
		(g_sMotorFault.ST != 0u) || (g_sMotorFault.DRIFT != 0u) || (g_sMotorFault.TS != 0u))
	{
		l_u8MotorStatus = C_MOTOR_STATUS_DEGRADED;
		MotorDriverStop(C_STOP_EMERGENCY);
	}

	/* Stepper Motor State Machine */
	switch( l_u8MotorStatus )
	{
		case C_MOTOR_STATUS_SELFTEST:
			l_u8MotorStatus = C_MOTOR_STATUS_STOP;
			break;
		case C_MOTOR_STATUS_RUNNING:
			/* runtime current PID control  */
			if( Timer_IsExpired(PID_CTRL_TIMER) == TRUE )
			{
				PID_Control();													/* PID-control (Current) */
				Timer_Start(PID_CTRL_TIMER,(uint16)NVRAM_PID_RUNNINGCTRL_PER);
			}
			/* update target speed,may be overwitten by ISR */
			{
				uint32 u32Temp;
				
				u32Temp = divU32_U32byU16( (TIMER_CLOCK * 60U), g_u16MotorMicroStepsPerMechRotation);
				g_u16MotorSpeedRPS = divU16_U32byU16( (uint32)(uint16)(l_u16ActuatorBufferedSpdRPM + 30U), 60U);
				g_u16TargetCommutTimerPeriod = divU16_U32byU16( u32Temp, l_u16ActuatorBufferedSpdRPM) - 1U;	
			}
			/* need change new direction? */
			if(l_u8MotorRequest == C_MOTOR_CTRL_START)
			{
				uint8 u8NewMotorDirectionCCW;
				
				u8NewMotorDirectionCCW = (l_u16ActuatorBufferedTgtPos < g_u16ActuatorActPos) ? C_MOTOR_DIR_CCW : C_MOTOR_DIR_CW;
				/* update new target position:commutation if direction inversed with current direction */
				if ( u8NewMotorDirectionCCW != g_e8MotorDirectionCCW )
		     	{
		        	/* Changing direction; Stop motor first before starting in opposite direction */
		        	MotorDriverStop( (uint16) C_STOP_RAMPDOWN);					/* Change of direction */
		       	}
				else
				{
					/* only update target position */
					g_u16ActuatorTgtPos = l_u16ActuatorBufferedTgtPos;
				}
			}
			/* Motor request stop */
			if(l_u8MotorRequest == C_MOTOR_CTRL_STOP)
			{
				MotorDriverStop( (uint16) C_STOP_IMMEDIATE);
				l_u8MotorStatus = C_MOTOR_STATUS_STOP;
			}
			/* Motor reach target position,stop by commutation ISR */
			if(g_u8MotorStartupMode == (uint8)MSM_STOP)
			{
				l_u8MotorStatus = C_MOTOR_STATUS_STOP;
			}
			break;
		case C_MOTOR_STATUS_STOP:
			/* ********************************************************** */
			/* *** l. Threshold control (Stepper: Current-threshold) 			   *** */
			/* ********************************************************** */
			if(Timer_IsExpired(PID_THRSHLD_CTRL_TIMER) == TRUE)
			{
				ThresholdControl();
				Timer_Start(PID_THRSHLD_CTRL_TIMER,(uint16)NVRAM_PID_THRSHLDCTRL_PER);
			}

			/* Stop-mode & holding-current required:not support */
			if(g_u8MotorHoldingCurrState == TRUE)
			{
				/* holding current */
				if( Timer_IsExpired(PID_CTRL_TIMER) == TRUE)
				{
					Timer_Start(PID_CTRL_TIMER,(uint16)NVRAM_PID_HOLDINGCTRL_PER);
					PID_Control();
					MotorDriver_4PhaseStepper();
				}
			}
			/* start request and motor target position not equals actual position */
			if( (l_u8MotorRequest == C_MOTOR_CTRL_START) && (l_u16ActuatorBufferedTgtPos != g_u16ActuatorActPos) )
			{
				g_u16ActuatorTgtPos = l_u16ActuatorBufferedTgtPos;
				
				if(Timer_IsExpired(MOTOR_START_DELAY_TIMER) == TRUE)
				{			
					MotorDriverStart();
					l_u8MotorStatus = C_MOTOR_STATUS_RUNNING;
				}
			}
			else
			{
				MotorDriverStop(C_STOP_IMMEDIATE);
			}
			break;
		case C_MOTOR_STATUS_DEGRADED:
			/* Diagnostic shutdown protection */
			if((g_sMotorFault.UV != 0u) || (g_sMotorFault.OV != 0u) || 
				(g_sMotorFault.OPEN != 0u) || (g_sMotorFault.SHORT != 0u) ||
				(g_sMotorFault.ST != 0u) || (g_sMotorFault.DRIFT != 0u) || (g_sMotorFault.TS != 0u))
			{
				MotorDriverStop(C_STOP_EMERGENCY);
			}
			else
			{
				/* degrade mode removed,delay before motor re-start */
				l_u8MotorStatus = C_MOTOR_STATUS_STOP;
				Timer_Start(MOTOR_START_DELAY_TIMER,C_PI_TICKS_20MS);
			}
			break;
		default:
			l_u8MotorStatus = C_MOTOR_STATUS_STOP;
			break;
	}

}

/* ****************************************************************************	*
 * MotorDriverInit()
 *
 * Initialise Motor Driver
 * ****************************************************************************	*/
void MotorDriverInit( void)
{
	uint32 u32Temp;

#if MOTOR_PARAMS == MP_NVRAM
	/* data validity by configuration ID,use default if invalid */
	if(NVRAM_CONFIGURATION_ID != CONFIGURATION_ID)
	{
		/* NVRAM physical address 0x1010,in consideration of page integrity 2 word */
		(void)NVRAM_Write(0x1006, (const uint16 *)&MotorCalibParamsDefault, sizeof(MOTOR_CALIBPARAMS)/sizeof(uint16));
		/* UniROM */
		{
			uint16 *pMRAM,*pURAM;
			uint16 i;
			
			pMRAM = (uint16 *) &MotorCalibParamsDefault;
			pURAM = (uint16 *) &MotorParams;
			for(i = 0;i < sizeof(MOTOR_CALIBPARAMS)/sizeof(uint16);i++)
			{
				pURAM[i] = pMRAM[i];
			}
		}
	}
	else
	{
		(void)NVRAM_Read(0x1006, (uint16 *)&MotorParams, sizeof(MOTOR_CALIBPARAMS)/sizeof(uint16));
	}
#endif
	/* 1 electric rotation equals 4FS for bipolar stepper motor */
	g_u16MotorMicroStepsPerElecRotation = (uint16) mulU32_U16byU16( C_MICROSTEP_PER_FULLSTEP, (NVRAM_MOTOR_PHASES + 2u) << 1u);
	g_u16MotorMicroStepsPerMechRotation = (uint16) mulU32_U16byU16( NVRAM_POLE_PAIRS, g_u16MotorMicroStepsPerElecRotation);
	{
		uint16 u16ConstAccelaration = NVRAM_ACCELERATION_CONST;
		if ( u16ConstAccelaration != 0u )
		{
			u32Temp = divU32_U32byU16( (TIMER_CLOCK * 60u), g_u16MotorMicroStepsPerMechRotation);

			g_au16MotorSpeedCommutTimerPeriod[0] = divU16_U32byU16( u32Temp, NVRAM_MIN_SPEED) - 1u;
			g_au16MotorSpeedCommutTimerPeriod[1] = divU16_U32byU16( u32Temp, NVRAM_SPEED3) - 1u;
			g_au16MotorSpeedCommutTimerPeriod[2] = divU16_U32byU16( u32Temp, NVRAM_SPEED0) - 1u;
			g_au16MotorSpeedCommutTimerPeriod[3] = divU16_U32byU16( u32Temp, NVRAM_SPEED1) - 1u;
			g_au16MotorSpeedCommutTimerPeriod[4] = divU16_U32byU16( u32Temp, NVRAM_SPEED2) - 1u;
			g_au16MotorSpeedCommutTimerPeriod[5] = divU16_U32byU16( u32Temp, NVRAM_SPEED3) - 1u;
			g_au16MotorSpeedCommutTimerPeriod[6] = g_au16MotorSpeedCommutTimerPeriod[1];
			g_au16MotorSpeedCommutTimerPeriod[7] = g_au16MotorSpeedCommutTimerPeriod[0];
			g_u16TargetCommutTimerPeriod = g_au16MotorSpeedCommutTimerPeriod[2];	/* Target commutation timer period (target speed) */
			g_u16CommutTimerPeriod = g_au16MotorSpeedCommutTimerPeriod[2];

			g_au16MotorSpeedRPS[0] = divU16_U32byU16( (uint32)(uint16)(NVRAM_MIN_SPEED + 30U), 60U);
			g_au16MotorSpeedRPS[1] = divU16_U32byU16( (uint32)(uint16)(NVRAM_SPEED3 + 30U), 60U);
			g_au16MotorSpeedRPS[2] = divU16_U32byU16( (uint32)(uint16)(NVRAM_SPEED0 + 30U), 60U);
			g_au16MotorSpeedRPS[3] = divU16_U32byU16( (uint32)(uint16)(NVRAM_SPEED1 + 30U), 60U);
			g_au16MotorSpeedRPS[4] = divU16_U32byU16( (uint32)(uint16)(NVRAM_SPEED2 + 30U), 60U);
			g_au16MotorSpeedRPS[5] = divU16_U32byU16( (uint32)(uint16)(NVRAM_SPEED3 + 30U), 60U);
			g_au16MotorSpeedRPS[6] = g_au16MotorSpeedRPS[1];
			g_au16MotorSpeedRPS[7] = g_au16MotorSpeedRPS[0];
		}
	}

	g_u16CorrectionRatio = NVRAM_MIN_CORR_RATIO;

	/* BLDC motor Commutation/Stepper timer */
	g_u16MicroStepIdx = 0u;

	TMR1_CTRL = C_TMRx_CTRL_MODE0;												/* Timer mode 0 */
	TMR1_REGB = 0u;																/* Will be overwritten by MotorDriverStart() */

	/* Setup Motor PWM */	
	PWM1_CTRL = 0u;																/* Disable master */
	PWM2_CTRL = 0u;																/* Disable Slave 1 */
	PWM3_CTRL = 0u;																/* Disable Slave 2 */
	PWM4_CTRL = 0u;																/* Disable Slave 3 */
	PWM5_CTRL = 0u;																/* Disable Slave 4 */
	PWM1_PSCL = (uint8)PWM_PRESCALER;											/* Initialise the master pre-scaler ratio (Fck/8) */
	PWM1_PER = (uint16)PWM_REG_PERIOD;
	PWM2_PER = (uint16)PWM_REG_PERIOD;											/* -=#=- Probably not needed to set slave period too */
	PWM3_PER = (uint16)PWM_REG_PERIOD;											/* -=#=- Probably not needed to set slave period too */
	PWM4_PER = (uint16)PWM_REG_PERIOD;											/* -=#=- Probably not needed to set slave period too */
	PWM5_PER = (uint16)PWM_REG_PERIOD;											/* -=#=- Probably not needed to set slave period too */

	
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR)
#if _SUPPORT_PHASE_SHORT_DET
	/*
	 * (Double PWM) 25%(7.9us)	44%(7.5us)	62%(7.9us)	81%(7.9us)	100%(10.4us)		(7.5us/ADC-conversion)
	 * MF_STEPPER:	Imotor		Vphase		Vs-filt 	Temperature Vsm
	 */
	PWM1_CMP = (((25L * PWM_REG_PERIOD) + 50)/100); 	/*	8.0us */			/* 25% of period */
	PWM2_CMP = (((44L * PWM_REG_PERIOD) + 50)/100); 	/*	7.5us */			/* 44% of period */
	PWM3_CMP = (((62L * PWM_REG_PERIOD) + 50)/100); 	/*	8.0us */			/* 62% of period */
	PWM4_CMP = (((81L * PWM_REG_PERIOD) + 50)/100); 	/*	8.0us */			/* 81% of period */
#else  /* _SUPPORT_PHASE_SHORT_DET */
	/* (Double PWM) 25% 		50% 		75% 		100%							(10.5us/ADC-conversion)
	 * MF_STEPPER:	Imotor		Vs-filt 	Temperature Vsm
	 */
	PWM1_CMP = (((25L * PWM_REG_PERIOD) + 50)/100); 	/* 10.5us */			/* 25% of period: (PWM_REG_PERIOD+2)/4 */
	PWM2_CMP = (((50L * PWM_REG_PERIOD) + 50)/100); 	/* 10.5us */			/* 50% of period */
	PWM3_CMP = (((75L * PWM_REG_PERIOD) + 50)/100); 	/* 10.5us */			/* 75% of period */
#endif /* _SUPPORT_PHASE_SHORT_DET */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR) */
	
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM)
#if _SUPPORT_PHASE_SHORT_DET
	/* (Single PWM) 20% 		40% 		60% 		80% 		100%				(8.33us/ADC-conversion)
	 * MF_STEPPER:	Vphase		Vs-filt 	Temperature Vsm-unfilt	Imotor
	 */
	PWM1_CMP = (((20L * PWM_REG_PERIOD) + 50)/100); 	/* 8.33us */			/* 20% of period */
	PWM2_CMP = (((40L * PWM_REG_PERIOD) + 50)/100); 							/* 40% of period */
	PWM3_CMP = (((60L * PWM_REG_PERIOD) + 50)/100); 							/* 60% of period */
	PWM4_CMP = (((80L * PWM_REG_PERIOD) + 50)/100); 							/* 80% of period */
#else  /* _SUPPORT_PHASE_SHORT_DET */
	/* (Single PWM) 25% 		50% 		75% 		100%							(10.5us/ADC-conversion)
	 * MF_STEPPER:	Vs-filt 	Temperature Vsm-unfilt	Imotor
	 */
	PWM1_CMP = (((25L * PWM_REG_PERIOD) + 50)/100); 	/* 10.5us */			/* 25% of period */
	PWM2_CMP = (((50L * PWM_REG_PERIOD) + 50)/100); 	/* 10.5us */			/* 50% of period */
	PWM3_CMP = (((75L * PWM_REG_PERIOD) + 50)/100); 	/* 10.5us */			/* 75% of period */
#endif /* _SUPPORT_PHASE_SHORT_DET */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM) */
	
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND)
#if _SUPPORT_PHASE_SHORT_DET
	/* (Single PWM) 17% 		33% 		50% 		67% 		83% 		100%				(8.33us/ADC-conversion)
	 * MF_STEPPER:	Vs-unfilt	Vphase		Imotor		Vsm-filt	-			Temperature
	 */
	PWM1_CMP = (((17L * PWM_REG_PERIOD) + 50)/100); 	/* 8.33us */			/* 17% of period */
	PWM2_CMP = (((33L * PWM_REG_PERIOD) + 50)/100); 							/* 33% of period */
	PWM3_CMP = (((50L * PWM_REG_PERIOD) + 50)/100); 							/* 50% of period */
	PWM4_CMP = (((67L * PWM_REG_PERIOD) + 50)/100); 							/* 67% of period */
	PWM5_CMP = (((83L * PWM_REG_PERIOD) + 50)/100); 							/* 83% of period */
#else  /* _SUPPORT_PHASE_SHORT_DET */
	/* (Single PWM) 25% 		50% 		75% 		100%							(10.5us/ADC-conversion)
	 * MF_STEPPER:	Vs-unfilt	Imotor		Vsm-filt	Temperature
	 */
	PWM1_CMP = (((25L * PWM_REG_PERIOD) + 50)/100); 	/* 10.5us */			/* 25% of period */
	PWM2_CMP = (((50L * PWM_REG_PERIOD) + 50)/100); 	/* 10.5us */			/* 50% of period */
	PWM3_CMP = (((75L * PWM_REG_PERIOD) + 50)/100); 	/* 10.5us */			/* 75% of period */
#endif /* _SUPPORT_PHASE_SHORT_DET */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) */
	
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
#if _SUPPORT_PHASE_SHORT_DET
	/* (Single PWM) 17% 		33% 		50% 		67% 		83% 		100%	(7.0us/ADC-conversion)
	 * MF_STEPPER:	Temperature Vs-filt 	Imotor1 	Vs-phase	Vsm-unfilt	Imotor2
	 */
	PWM1_CMP = (((17L * PWM_REG_PERIOD) + 50)/100); 	/*	7.0us */			/* 17% of period */
	PWM2_CMP = (((33L * PWM_REG_PERIOD) + 50)/100); 	/*	7.0us */			/* 33% of period */
	PWM3_CMP = (((50L * PWM_REG_PERIOD) + 50)/100); 	/*	7.0us */			/* 50% of period */
	PWM4_CMP = (((67L * PWM_REG_PERIOD) + 50)/100); 	/*	7.0us */			/* 67% of period */
	PWM5_CMP = (((83L * PWM_REG_PERIOD) + 50)/100); 	/*	7.0us */			/* 83% of period */
#else  /* _SUPPORT_PHASE_SHORT_DET */
	/* (Single PWM) 17% 		33% 		50% 		75% 		100%		(7.0us/ADC-conversion)
	 * MF_STEPPER:	Temperature Vs-filt 	Imotor1 	Vsm-unfilt	Imotor2
	 */
	PWM1_CMP = (uint16)(((17UL * PWM_REG_PERIOD) + 50U)/100U); 	/*	7.0us */			/* 17% of period */
	PWM2_CMP = (uint16)(((33UL * PWM_REG_PERIOD) + 50U)/100U); 	/*	7.0us */			/* 33% of period */
	PWM3_CMP = (uint16)(((50UL * PWM_REG_PERIOD) + 50U)/100U); 	/*	10.5us */			/* 50% of period */
	PWM4_CMP = (uint16)(((75UL * PWM_REG_PERIOD) + 50U)/100U); 	/*	10.5us */			/* 75% of period */
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
	PWM1_CTRL = (EBLK | ECI | EPI); 											/* Initialise the master control register - CMPI and PWMI enabled */
	PWM2_CTRL = (ECI | EXT | EBLK); 											/* Initialise the slave 1 control register - CMPI enabled */
	PWM3_CTRL = (ECI | EXT | EBLK); 											/* Initialise the slave 2 control register - CMPI enabled */
	PWM4_CTRL = (ECI | EXT | EBLK); 											/* Initialise the slave 3 control register - CMPI enabled */
	PWM5_CTRL = (ECI | EXT | EBLK); 											/* Initialise the slave 4 control register - CMPI enabled */
#endif /* (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) && (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_INDEPENDED_GND) */
	PWM1_CTRL |= EBLK;															/* Start PWM in application mode */
										
} /* End of MotorDriverInit */

/* ****************************************************************************	*
 * MotorDriverStart()
 *
 * Start Motor Driver
 * ****************************************************************************	*/
void MotorDriverStart( void)
{
	/* 1)stop, 2)no error,3)target position not equals actual position */
	if( g_u8MotorStartupMode == (uint8)MSM_STOP )
	{
#if USE_MULTI_PURPOSE_BUFFER
		/* Fill multi-purpose buffer with acceleration-data */
		{
			l_u16SpeedRPM = NVRAM_MIN_SPEED;
			l_u32Temp = divU32_U32byU16( (TIMER_CLOCK * 60U), g_u16MotorMicroStepsPerMechRotation);
			l_u16LowSpeedPeriod = divU16_U32byU16( l_u32Temp, l_u16SpeedRPM) - 1U;
		}
#endif /* USE_MULTI_PURPOSE_BUFFER */

		/* Clear motor-driver current measurement */
		MotorDriverCurrentMeasureInit();
#if _DEBUG_MOTOR_CURRENT_FLT
		l_u16MotorCurrIdx = 0u;
#endif /* _DEBUG_MOTOR_CURRENT_FLT */

//		g_u8MotorStartupMode = (uint8) MSM_STEPPER_A;								/* Start-up in Acceleration stepper mode */
		/* (MMP140331-2) g_u16StartupDelay = NVRAM_STALL_DETECTOR_DELAY; */
#if _SUPPORT_STALLDET_A
		MotorStallInitA();
#endif
#if _SUPPORT_STALLDET_O
		MotorStallInitO();
#endif /* _SUPPORT_STALLDET_O */
#if _SUPPORT_STALLDET_H
		MotorStallInitH();
#endif /* _SUPPORT_STALLDET_H */

		MotorDiagnosticCheckInit();													/* initialize diagnostic */

#if _DEBUG_VOLTAGE_COMPENSATION
		u16MotorVoltIdx = 0u;
#endif /* _DEBUG_VOLTAGE_COMPENSATION */

		/* Connect drivers */
		/* Stepper 4-phase/32-steps */
		{
			MotorDriver_InitialPwmDutyCycle( g_u16PidRunningThreshold, g_au16MotorSpeedRPS[1]);	/* MMP140822-1 - Begin */
		}
		MotorDriver_4PhaseStepper();

#if (_SUPPORT_PWM_DC_RAMPUP == FALSE)												/* MMP140903-2 - Begin */
		if ( g_u16MotorSpeedRPS > g_au16MotorSpeedRPS[1] )
		{
			MotorDriver_InitialPwmDutyCycle( g_u16PidRunningThreshold, g_u16MotorSpeedRPS);
		}																			/* MMP140822-1 - End */
#endif /* (_SUPPORT_PWM_DC_RAMPUP == FALSE) */										/* MMP140903-2 - End */
		DRVCFG_PWM_UVWT();															/* Enable the driver and the PWM phase W, V, U and T */

		/* Setup ADC for Motor Temperature/Current/Voltage measurements */
		ADC_Start();

		g_u8MotorStartupMode = (uint8) MSM_STEPPER_A;								/* Start-up in Acceleration stepper mode */
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
		/* motor running direction */
		g_e8MotorDirectionCCW = (g_u16ActuatorTgtPos < g_u16ActuatorActPos) ? C_MOTOR_DIR_CCW : C_MOTOR_DIR_CW;
		/* start commutation timer */
		TMR1_REGB = g_u16CommutTimerPeriod;
		TMR1_CTRL = C_TMRx_CTRL_MODE0 | TMRx_START;									/* Start Timer mode 0 */
	}
	
	return;
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
void MotorDriverStop( uint16 u16Immediate )
{
	if(g_u8MotorStartupMode != (uint8) MSM_STOP)
	{
		if(u16Immediate == (uint16) C_STOP_RAMPDOWN) /*lint !e845 */	/* MMP150922-1 */
		{
			if(l_u8VTIdx > 1u)
			{
				TMR1_CTRL = C_TMRx_CTRL_MODE0 | TMRx_START;							/* Start timer mode 0 */
				
				/* Set TargetPos near CurrentPos, including ramp-down */
				if ( g_u16ActuatorActPos > g_u16ActuatorTgtPos )
				{
					uint16 u16DeltaPos = g_u16ActuatorActPos - g_u16ActuatorTgtPos;
					if ( u16DeltaPos > l_u8VTIdx )
					{
						g_u16ActuatorTgtPos = g_u16ActuatorActPos - l_u8VTIdx;
					}
				}
				else
				{
					uint16 u16DeltaPos = g_u16ActuatorTgtPos - g_u16ActuatorActPos;
					if ( u16DeltaPos > l_u8VTIdx )
					{
						g_u16ActuatorTgtPos = g_u16ActuatorActPos + l_u8VTIdx;
					}
				}
				
				return;
			}
		}
		/* 1.First stop ADC, before stopping motor (trigger-event) */
		ADC_Stop();
		g_u8MotorStartupMode = (uint8) MSM_STOP;								/* Stop mode */
		g_u8MotorStatusSpeed = (uint8) C_MOTOR_SPEED_STOP;						/* Stop */
		/* 3.deal with specific stop request */
		/* in case stop with holding current:not supported yet */
		if(u16Immediate == (uint16) C_STOP_HOLD)
		{
			/* Keep Motor driver active with a specified amount of current (unless permanent electric error) */
			MotorDriver_InitialPwmDutyCycle( g_u16PidHoldingThreshold, 0);
	
			MotorDriver_4PhaseStepper();
			DRVCFG_PWM_UVWT();	
			g_u16MotorCurrentLPFx64 = (g_u16PidHoldingThreshold << 6u);				/* Low-pass Filtered motor-current (x 64) */

			ADC_Start();															/* Start measuring motor current */

		}
		/* in case stop with immediately */
		if(u16Immediate == (uint16) C_STOP_IMMEDIATE)
		{
			DRVCFG_GND_UVWT();														/* Make Low-side active, for a short time (recycle current) */
		}
		/* In case of a permanent error, don't connect drivers anymore */
		if(u16Immediate == (uint16)C_STOP_EMERGENCY)
		{
			DRVCFG_DIS_UVWT();
			DRVCFG_DIS();															/* MMP140903-1 */		
																					/* MMP130919-1 - End */
		}
		/* 4.stop commutation timer */
		TMR1_CTRL &= ~TMRx_START;													/* Stop "commutation timer" */
		XI0_PEND = CLR_T1_INT4;														/* Clear (potentially pending) Timer1 second level interrupts (T1_INT4) */
		PEND = CLR_EXT0_IT;															/* ... and first level interrupt */

	}

	return;
} /* End of MotorDriverStop */


/* local functions implementation */

/* ****************************************************************************	*
 * MotorDriverCurrentMeasureInit()
 *
 * Initialise for motor driver current measurement
 * Performance: <10us
 * ****************************************************************************	*/
void MotorDriverCurrentMeasureInit( void)
{
	uint16 u16Count;															/* MMP140331-2 - Begin */
	l_u16StartupDelayInit = 64u * NVRAM_ACCELERATION_POINTS;					/* Calculate the startup-delay, based on acceleration steps */
	if ( l_u16StartupDelayInit < (2u * C_MOVAVG_SZ) )
	{
		l_u16StartupDelayInit = (2u * C_MOVAVG_SZ);								/* Minimum of twice the moving-average filter size */
	}
	else if ( l_u16StartupDelayInit > NVRAM_STALL_DETECTOR_DELAY )
	{
		l_u16StartupDelayInit = NVRAM_STALL_DETECTOR_DELAY;						/* Maximum of NVRAM specified */
	}																			/* MMP140331-2 - End */
	else
	{
		
	}
	ATOMIC_CODE
	(
		g_u16StartupDelay = l_u16StartupDelayInit;								/* (MMP140331-2) */
		g_u16MotorCurrentLPFx64 = 0u;											/* Low-pass Filtered motor-current (x 64) */
		l_u16MotorCurrentRawIdx = 0u;											/* Raw current moving average index */
		g_u16MotorCurrentMovAvgxN = 0u;											/* Moving average motor-current (x 4..16) */
		l_au16MotorCurrentRaw[0] = 0u;
	);
	{
		uint16 *pStallCurrentRaw = &l_au16MotorCurrentRaw[1];
		for ( u16Count = 1u; u16Count < (uint16)C_MOVAVG_SZ; u16Count++ )
		{
			*pStallCurrentRaw = 0u;
			pStallCurrentRaw++;
		}
	}

} /* End of MotorDriverCurrentMeasureInit() */

/* ****************************************************************************	*
 * MotorDriverCurrentMeasure()
 *
 * Measure a average motor current, based on ADC current's
 * Performance: Approximate: 10us at 20MHz
 * ****************************************************************************	*/
void MotorDriverCurrentMeasure( void)
{
	uint16 u16MotorCurrentAcc;
	uint16 u16MicroStepMotorCurrent = GetRawMotorDriverCurrent();
#if _DEBUG_SPI
	SpiDebugWriteFirst(g_u16PidRunningThreshold|0x8000u);
	SpiDebugWriteNext(g_u16MotorCurrentMovAvgxN);
#endif /* _DEBUG_SPI */

	/* Moving average (sum) of motor-driver current */
	uint16 *pu16MotorCurrentElement = &l_au16MotorCurrentRaw[l_u16MotorCurrentRawIdx];
	uint16 u16PrevMotorCurrent = *pu16MotorCurrentElement;

	l_u16MotorCurrentRawIdx = (l_u16MotorCurrentRawIdx + 1u) & (C_MOVAVG_SZ - 1u);
	if ( (g_u16StartupDelay != 0u) || (u16MicroStepMotorCurrent < (u16PrevMotorCurrent << 1u)) )	/* Check for valid motor-driver current (at least smaller than 2x previous current)  */
	{
		g_u16MotorCurrentMovAvgxN -= u16PrevMotorCurrent;						/* Subtract oldest raw motor-driver current */
		g_u16MotorCurrentMovAvgxN += u16MicroStepMotorCurrent;					/* Add newest raw motor-driver current */
		*pu16MotorCurrentElement = u16MicroStepMotorCurrent;					/* Overwrite oldest with newest motor-driver current */
	}

	/* During twice the moving-average-buffer size and during acceleration of the motor, LPF should follow
	   lowest value of LPF or Motor-current. As the speed is increasing so also is the BEMF also increasing,
	   which causes the current to decrease. Otherwise a first order (IIR-1) LPF is used. */
	u16MotorCurrentAcc = (g_u16MotorCurrentMovAvgxN << (6u - C_MOVAVG_SSZ));
	if ( (g_u16StartupDelay > (l_u16StartupDelayInit - (2u * C_MOVAVG_SZ))) || 
		(g_u8MotorStartupMode == (uint8) MSM_STEPPER_D) || 
		((g_u8MotorStartupMode == (uint8) MSM_STEPPER_A) && (u16MotorCurrentAcc < g_u16MotorCurrentLPFx64)) )
	{
		g_u16MotorCurrentLPFx64 = u16MotorCurrentAcc;
	}
	else
	{
#if (MOTOR_MICROSTEPS < 3)
		/* LPF_B: IIR of 0.9921875 (127/128) & 0.0078125 (1/128) */
		g_u16MotorCurrentLPFx64 = (g_u16MotorCurrentLPFx64 - ((g_u16MotorCurrentLPFx64 + 63u) >> 7u)) + ((u16MotorCurrentAcc + 63u) >> 7u);
#else  /* (MOTOR_MICROSTEPS < 3) */
		/* LPF_B: IIR of 0.99609375 (255/256) & 0.00390625 (1/256) */
		g_u16MotorCurrentLPFx64 = (g_u16MotorCurrentLPFx64 - ((g_u16MotorCurrentLPFx64 + 128u) >> 8u)) + ((u16MotorCurrentAcc + 128u) >> 8u);
#endif /* (MOTOR_MICROSTEPS < 3) */
	}

	/* startup delay decrement every current meassure(msp) */
	if ( g_u16StartupDelay > 0u )
	{
		g_u16StartupDelay--;
	}

#if _DEBUG_MOTOR_CURRENT_FLT
	l_au8MotorCurrRaw[l_u16MotorCurrIdx] = (uint8) ((u16MicroStepMotorCurrent >> 1u) & 0xFFu);
	if ( ++l_u16MotorCurrIdx >= C_MOTOR_CURR_SZ )
		l_u16MotorCurrIdx = 0u;
#endif /* _DEBUG_MOTOR_CURRENT_FLT */
} /* End of MotorDriverCurrentMeasure() */

/* ****************************************************************************	*
 * MotorDriver_InitialPwmDutyCycle()
 *
 * Calculate Motor PWM (initial) Duty-cycle, based on current threshold level and speed
 * ****************************************************************************	*/
void MotorDriver_InitialPwmDutyCycle( uint16 u16CurrentLevel, uint16 u16MotorSpeed)
{
	if ( u16MotorSpeed == 0u )														/* MMP140228-1 - Begin */
	{
		g_u16CorrectionRatio  = ((NVRAM_MOTOR_COIL_RTOT + (2u * C_FETS_RTOT)) * u16CurrentLevel);
		g_u16CorrectionRatio /= 4u;
	}																				/* MMP140228-1 - End */
	else
	{
		/* Ohmic losses: Ur-losses = (0.5 * R[ohm] * I[mA])/10 [10mV] = (R[ohm] * I[mA])/20 [10mV]
		 * FET losses: Ufet-losses = (Rfet * I[mA])/10 [10mV] = (2 * Rfet * I[mA])/20 [10mV]*/
		g_u16CorrectionRatio  = ((NVRAM_MOTOR_COIL_RTOT + (2u * C_FETS_RTOT)) * u16CurrentLevel);
		g_u16CorrectionRatio /= 20u;													/* Divided by 20 */
		g_u16CorrectionRatio += (NVRAM_MOTOR_CONSTANT * u16MotorSpeed);				/* BEMF = Kmotor[10mV/RPS] * Speed[RPS] */
	}
	g_u16PidCtrlRatio =  muldivU16_U16byU16byU16( g_u16CorrectionRatio << 3u, (uint16)PWM_REG_PERIOD << (1u + PWM_PRESCALER_N), NVRAM_VSUP_REF);
	g_u16PID_I = g_u16PidCtrlRatio;
	if ( g_i16MotorVoltage > 0 )
	{
		g_u16CorrectionRatio = muldivU16_U16byU16byU16( g_u16CorrectionRatio << 3u, (uint16)PWM_REG_PERIOD << (1u + PWM_PRESCALER_N), (uint16) g_i16MotorVoltage);
	}
	else
	{
		g_u16CorrectionRatio = g_u16PidCtrlRatio;
	}
	g_i16PID_D = 0;
	g_i16PID_E = 0;
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
	const int16 *pi16Vector = &c_ai16MicroStepVector4PH[g_u16MicroStepIdx];
#if (PWM_REG_PERIOD >= (128U << (4U - PWM_PRESCALER_N)))						/* (((PWM_REG_PERIOD * 256U) >> (4U - PWM_PRESCALER_N)) > 32767U) */
	iPwm1 = (int16) (mulI32_I16byU16( *pi16Vector, g_u16CorrectionRatio) >> (20 + PWM_PRESCALER_N));
	pi16Vector += C_MICROSTEP_PER_FULLSTEP;
	iPwm2 = (int16) (mulI32_I16byU16( *pi16Vector, g_u16CorrectionRatio) >> (20 + PWM_PRESCALER_N));
#elif (PWM_PRESCALER_N == 0U)
	iPwm1 = mulI16_I16byI16Shft4( *pi16Vector, (int16) g_u16CorrectionRatio);	/* U */
	pi16Vector += C_MICROSTEP_PER_FULLSTEP;
	iPwm2 = mulI16_I16byI16Shft4( *pi16Vector, (int16) g_u16CorrectionRatio);	/* V */
#else
	i16PwmU = (int16) (mulI16_I16byI16( *pi16Vector, (int16) g_u16CorrectionRatio) >> (4 + PWM_PRESCALER_N));	/* U */
	pi16Vector += C_MICROSTEP_PER_FULLSTEP;
	i16PwmV = (int16) (mulI16_I16byI16( *pi16Vector, (int16) g_u16CorrectionRatio) >> (4 + PWM_PRESCALER_N));	/* V */
#endif
	if ( (g_u16MicroStepIdx & (2u*C_MICROSTEP_PER_FULLSTEP)) != 0u )
	{
		/* 3rd and 4th Quadrant (Pwm1) */
		iPwm1 = (int16)PWM_SCALE_OFFSET + iPwm1;
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* U = PWM */
		PWM4_LT =  (uint16)iPwm1;
		PWM4_HT =  (uint16)PWM_REG_PERIOD - (uint16)iPwm1;
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
		/* T = PWM */
		PWM5_LT = iPwm1;
		PWM5_HT = (PWM_REG_PERIOD - iPwm1);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
		/* V = PWM */
		PWM3_LT = (uint16) iPwm1;
		PWM3_HT = (uint16) (PWM_REG_PERIOD - iPwm1);
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */

		/* W = LOW */
		PWM2_HT = 0U;
		PWM2_LT = (uint16)PWM_REG_PERIOD + 1U;											/* MMP150603-1 */;
	}
	else
	{
		/* 1st and 2nd Quadrant (Pwm1)*/
		/* W = PWM */
		iPwm1 = ((int16) PWM_SCALE_OFFSET - iPwm1);
		PWM2_LT = (uint16) iPwm1;
		PWM2_HT = (uint16)PWM_REG_PERIOD - (uint16)iPwm1;

#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
		/* U = LOW */
		PWM4_HT = 0U;
		PWM4_LT = (uint16)PWM_REG_PERIOD + 1U;											/* MMP150603-1 */
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
		uint16 u16Quad = g_u16MicroStepIdx & (3u * C_MICROSTEP_PER_FULLSTEP);
		if ( (u16Quad == 0u) || (u16Quad == (3u * C_MICROSTEP_PER_FULLSTEP)) )
		{
			/* 1st and 4th Quadrant (Pwm2) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
			/* T = PWM */
			PWM5_HT = (uint16) iPwm2;
			PWM5_LT = (uint16)PWM_REG_PERIOD - (uint16)iPwm2;

			/* V = LOW */
			PWM3_HT = 0U;
			PWM3_LT = (uint16)PWM_REG_PERIOD + 1U;										/* MMP150603-1 */
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
			PWM3_LT = (uint16)PWM_REG_PERIOD - (uint16)iPwm2;

			/* T = LOW */
			PWM5_HT = 0U;
			PWM5_LT = (uint16)PWM_REG_PERIOD + 1U;										/* MMP150603-1 */
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

	PWM1_LT = (uint16)PWM_SCALE_OFFSET;													/* Master must be modified at last (value is not important) */

} /* End of MotorDriver_4PhaseStepper() */

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
__interrupt__ void EXT0_IT(void)
{
#if (_DEBUG_COMMUT_ISR != FALSE)
	DEBUG_SET_IO_B();
#endif /* (_DEBUG_COMMUT_ISR != FALSE) && (_DEBUG_HALLLATCH_ISR == FALSE) */

	uint16 pending = XI0_PEND & XI0_MASK;										/* Copy interrupt requests which are not masked   */
	do
	{
		XI0_PEND = pending;														/* Clear requests which are going to be processed */
	} while ((XI0_PEND & pending) != 0u);

	if ( g_u8MotorStartupMode == (uint8) MSM_STOP )
	{
		return;		/* Used for CPU wake-up */
	}

	if ( g_e8MotorDirectionCCW == C_MOTOR_DIR_CCW)												/* Motor direction counter clockwise is true? */
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
		}
	}

	/* Current measurement used for Stall-detector "A" and current control (PID) */
	MotorDriverCurrentMeasure();

	/* open coil detection */
	(void)MotorDiagnosticOpenCheck();

	/* Update micro-step index */
	{
		/* Motor Direction Inverse */
		if(NVRAM_MOTOR_DIR_INV != 0u)
		{
			if ( g_e8MotorDirectionCCW == C_MOTOR_DIR_CCW )
			{
				/* Counter Clock-wise (Closing) */
				if ( g_u16MicroStepIdx == 0u )
				{
					g_u16MicroStepIdx = g_u16MotorMicroStepsPerElecRotation;
				}
				g_u16MicroStepIdx--;													/* Decrement the PWM vector pointer */
			}
			else
			{
				/* Clock-wise (Opening) */
				g_u16MicroStepIdx++;													/* Increment the PWM vectors pointer */
				if ( g_u16MicroStepIdx >= g_u16MotorMicroStepsPerElecRotation )			/* Test the PWM vectors pointer: 48 usteps per electrical period */
				{
					g_u16MicroStepIdx = 0u;												/* Re-initialise the PWM vectors pointer to 0 */
				}	
			}
		}
		else
		{
			if ( g_e8MotorDirectionCCW == C_MOTOR_DIR_CCW )
			{
				/* Clock-wise (Opening) */
				g_u16MicroStepIdx++;													/* Increment the PWM vectors pointer */
				if ( g_u16MicroStepIdx >= g_u16MotorMicroStepsPerElecRotation )			/* Test the PWM vectors pointer: 48 usteps per electrical period */
				{
					g_u16MicroStepIdx = 0u;												/* Re-initialise the PWM vectors pointer to 0 */
				}	
			}
			else
			{
				/* Counter Clock-wise (Closing) */
				if ( g_u16MicroStepIdx == 0u )
				{
					g_u16MicroStepIdx = g_u16MotorMicroStepsPerElecRotation;
				}
				g_u16MicroStepIdx--;													/* Decrement the PWM vector pointer */
			}
		}
	}

	/* Check for speed update required */
	if ( g_u16CommutTimerPeriod == g_u16TargetCommutTimerPeriod )
	{
		g_u8MotorStartupMode = (uint8) MSM_STEPPER_C;
	}
	else
	{
		/* Update speed */
		uint16 u16Compensation = l_u16SpeedRPM;							/* MMP160606-1 */
		if ( g_u16CommutTimerPeriod < g_u16TargetCommutTimerPeriod )
		{
			/* Deceleration per micro-step */
			g_u8MotorStartupMode = (uint8) MSM_STEPPER_D;					/* Too fast, decelerate */
			l_u16SpeedRPM = l_u16SpeedRPM - divU16_U32byU16( (uint32) 2*NVRAM_ACCELERATION_CONST, l_u16SpeedRPM);	/* MMP160606-1 */
			g_u16CommutTimerPeriod = divU16_U32byU16( l_u32Temp, l_u16SpeedRPM) - 1u;	/* MMP160606-1 */
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

		}
		else if ( (g_u16MicroStepIdx == 0u) || ((g_u16MicroStepIdx > NVRAM_ACCELERATION_POINTS) && ((g_u16MicroStepIdx & NVRAM_ACCELERATION_POINTS) == 0u)) )
		{
			/* Acceleration per acceleration_points ((multiple) full-step) */
			g_u8MotorStartupMode = (uint8) MSM_STEPPER_A;						/* Too slow, accelerate */
			l_u16SpeedRPM = l_u16SpeedRPM + divU16_U32byU16( (uint32) 2*NVRAM_ACCELERATION_CONST, l_u16SpeedRPM);	/* MMP160606-1 */
			g_u16CommutTimerPeriod = divU16_U32byU16( l_u32Temp, l_u16SpeedRPM) - 1u;	/* MMP160606-1 */
			l_u8VTIdx++;
			if ( g_u16CommutTimerPeriod < g_u16TargetCommutTimerPeriod )		/* MMP150923-1 */
			{
				g_u16CommutTimerPeriod = g_u16TargetCommutTimerPeriod;
			}
			TMR1_REGB = g_u16CommutTimerPeriod;
#if (_SUPPORT_PWM_DC_RAMPUP != FALSE)											/* MMP140903-2 - Begin */
			g_u16PidCtrlRatio = muldivU16_U16byU16byU16( g_u16PidCtrlRatio, l_u16SpeedRPM, u16Compensation);	/* MMP160606-2 */
			g_u16PID_I = g_u16PidCtrlRatio;
#endif /* (_SUPPORT_PWM_DC_RAMPUP != FALSE) */									/* MMP140903-2 - End */
		}
		else
		{
			/* MISRA C:2012 Rule-15.7:All if ... else if constructs shall be terminated with an else statement */
		}
	}

	VoltageCorrection();

	MotorDriver_4PhaseStepper();
	
#if _SUPPORT_STALLDET_A
	if ( MotorStallCheckA() != (uint16) C_STALL_NOT_FOUND )						/* Stall-detector "A" */
	{
		g_sMotorFault.ST |= (uint16)C_STALL_FOUND_A;
		MotorDriverStop( (uint16) C_STOP_EMERGENCY);							/* ... stop motor (Stall) */
	}
#endif
#if _SUPPORT_STALLDET_O
	if ( MotorStallCheckO() != (uint16)C_STALL_NOT_FOUND )						/* Stall-detector "O" (MMP140428-1) */
	{
		g_sMotorFault.ST |= (uint16)C_STALL_FOUND_O;
		MotorDriverStop( (uint16) C_STOP_EMERGENCY);							/* ... stop motor (Stall) */
	}
#endif /* _SUPPORT_STALLDET_O */
#if _SUPPORT_STALLDET_H
	/* Stall detection based on hall-sensor(s) */
	if ( MotorStallCheckH() != (uint16) C_STALL_NOT_FOUND )						/* Stall-detector "H" */
	{
		g_sMotorFault.ST |= (uint16)C_STALL_FOUND_H;
		MotorDriverStop( (uint16) C_STOP_EMERGENCY);							/* ... stop motor (Stall) */
	}
#endif /* _SUPPORT_STALLDET_H */

#if _DEBUG_COMMUT_ISR
	DEBUG_CLR_IO_B();
#endif /* _DEBUG_COMMUT_ISR */
} /* End of Commutation_ISR() */
  
/* EOF */
