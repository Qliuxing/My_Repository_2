/*! \file		MotorDriver.h
 *  \brief		MLX81300 Motor Driver handling
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
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

#ifndef MOTOR_DRIVER_H_
#define MOTOR_DRIVER_H_

#include "Build.h"
#include <syslib.h>
#include <plib.h>
#include "MotorParams.h"
#include "NVRAM_UserPage.h"

/* Motor Status mode (g_e8MotorStatusMode) */
#define C_MOTOR_STATUS_STOP			0x00U										/* Actuator stopped */
#define C_MOTOR_STATUS_RUNNING		0x01U										/* bit 0: Actuator Running */
#define C_MOTOR_STATUS_INIT			0x02U										/* bit 1: Actuator initialisation (Only internal state) */
#define C_MOTOR_STATUS_SELFTEST		0x04U										/* bit 2: Actuator Self-test (Only internal state) */
#define C_MOTOR_STATUS_STOPPING		(0x08U | C_MOTOR_STATUS_RUNNING)			/* bit 3+0: (Going to) stopping but still running */
#define C_MOTOR_STATUS_APPL_STOP	0x40U										/* bit 6: Application stopped */
#define C_MOTOR_STATUS_DEGRADED		0x80U										/* bit 7: Actuator in degraded mode (Only Cooling 2.3 and only Status) */

/* Motor Control Request */
#define C_MOTOR_CTRL_STOP			0x00
#define C_MOTOR_CTRL_START          0x01

enum MOTOR_STARTUP_MODE
{
	MSM_STOP = 0,
	MSM_STEPPER_A,																/* Stepper-Acceleration */
	MSM_STEPPER_D,																/* Stepper-Deceleration */
	MSM_STEPPER_C,																/* Stepper-Constant */
	MSM_BEMF
};

enum MOTOR_STOP_MODE
{
	C_STOP_RAMPDOWN = 0,														/* Stop actuator with ramp-down speed */
	C_STOP_HOLD,																/* Stop actuator with holding current */
	C_STOP_IMMEDIATE,															/* Stop actuator immediately, without start-up delay */
	C_STOP_EMERGENCY,															/* Stop actuator immediately, with start-up delay */
	C_STOP_SLEEP																/* Stop actuator w/o holding current */
};

enum TACHO_MODES
{
	C_TACHO_NONE = 0,															/* No Tacho output */
	C_TACHO_60DEG_ELECTRIC,														/* Tacho output toggles every 60 degrees of an electric rotation */
	C_TACHO_180DEG_ELECTRIC,													/* Tacho output toggles every 180 degrees of an electric rotation */
	C_TACHO_180DEG_MECHANICAL													/* Tacho output toggles every 180 degrees of a mechanical rotation */
};

#define VSUP2PWM			TRUE												/* Motor driver operates between Vsup to PWM */

#define STARTUP_OPEN_LOOP			0u											/* Start-up in Open-Loop */
#define STARTUP_CLOSED_LOOP_SRP		1u											/* Start-up in Closed-Loop (SRP) */
#define STARTUP_STEPPER				2u											/* Start-up in Stepper */
#define STARTUP_MICRO_STEPPER		3u											/* Start-up in Micro-stepper (8x) */

/* Motor speeds (g_u8MotorCtrlSpeed/g_u8MotorStatusSpeed) */
#define C_MOTOR_SPEED_STOP			0U											/* NVRAM_MIN_SPEED */
#define C_MOTOR_SPEED_LOW			1U											/* NVRAM_SPEED_TORQUE_BOOST */
#define C_MOTOR_SPEED_MID_LOW		2U											/* NVRAM_SPEED0 */
#define C_MOTOR_SPEED_MID			3U											/* NVRAM_SPEED1 */
#define C_MOTOR_SPEED_MID_HIGH		4U											/* NVRAM_SPEED2 */
#define C_MOTOR_SPEED_HIGH 			5U											/* NVRAM_SPEED3 */
#define C_DEFAULT_MOTOR_SPEED		C_MOTOR_SPEED_MID

/* Motor rotational direction (e8MotorDirectionCCW) */
#define C_MOTOR_DIR_CW				0u											/* Continueos */
#define C_MOTOR_DIR_CCW				1u											/* Continueos */
#define C_MOTOR_DIR_UNKNOWN			2u

/* Zero-crossing detector state */
#define ZC_RESET	0
#define ZC_START	1
#define ZC_FOUND	2

/* Stall detector levels */
#define M_STALL_MODE				0x0F										/* bit 6:3: Stall-mode */
#define C_STALL_FOUND_A				0x01										/* bit 6: Current(Amps) stall detected */
#define C_STALL_FOUND_B				0x02										/* bit 5: BEMF-based stall detected */
#define C_STALL_FOUND_H				0x04										/* bit 4: hall-sensor stall detected */
#define C_STALL_FOUND_O				0x08										/* bit 3: Current Oscillation stall detected */

#define EXT_PHASE_DEADTIME	(1 * (PLL_freq/(1000000*CYCLES_PER_INSTR*2)) + 1)	/* 1us: External P/N-FET dead-time (4) */

#if (MOTOR_PARAMS == MP_NVRAM)
/* ****************************************************************************	*
 * NVRAM (UniROM) parameters
 * ****************************************************************************	*/
#define NVRAM_CONFIGURATION_ID				((uint16) MotorParams.ConfigurationID)

#define NVRAM_VSUP_REF						((uint16) ((uint16)MotorParams.VsupRef * 25u) >> 1u)
#define NVRAM_LEAD_ANGLE					((uint16) MotorParams.LeadAngle)
#define NVRAM_BROWNOUT_LEVEL				((uint16) MotorParams.BrownoutLevel)
#define NVRAM_VDS_THRESHOLD					((uint16) ((uint16)MotorParams.VdsThreshold * 25u) >> 1u)	/* MMP140428-1 */

#define NVRAM_POLE_PAIRS					((uint16) MotorParams.PolePairs)
#define NVRAM_MOTOR_PHASES					((uint16) MotorParams.MotorPhases)
#define NVRAM_MOTOR_CONSTANT				((uint16) MotorParams.MotorConstant)
#define NVRAM_MOTOR_COIL_RTOT				((uint16) MotorParams.MotorCoilRtot)

#define NVRAM_MICRO_STEPS					((uint16) 1u << MotorParams.MicroSteps)
#define NVRAM_MOTOR_DIR_INV					((uint16) MotorParams.MotorDirectionINV)
#define NVRAM_TACHO_MODE					((uint16) MotorParams.TachoMode)

/* Mechanical */
#define NVRAM_GEARBOX_RATIO					((uint16) MotorParams.GearBoxRatio)
#define NVRAM_DEF_TRAVEL					((uint16) MotorParams.DefTravel)										/* Full-stroke */
#define NVRAM_DEF_TRAVEL_TOLERANCE_HI		((uint16) MotorParams.DefTravelToleranceHi)										/* Deviation: 180 degrees of electric rotation */
#define NVRAM_DEF_TRAVEL_TOLERANCE_LO		((uint16) MotorParams.DefTravelToleranceLo)
#define NVRAM_DEF_EMRUN_POS					((uint16) MotorParams.EmergencyRunPos)

/* four speed points */
#define NVRAM_SPEED0						((uint16) MotorParams.Speed_0)
#define NVRAM_SPEED1						((uint16) MotorParams.Speed_1)
#define NVRAM_SPEED2						((uint16) MotorParams.Speed_2)
#define NVRAM_SPEED3						((uint16) MotorParams.Speed_3)

#define NVRAM_MIN_SPEED						((uint16) MotorParams.MinimumSpeed)
#define NVRAM_ACCELERATION_CONST			((uint16) MotorParams.AccelerationConst)
#define NVRAM_ACCELERATION_POINTS			((uint16) MotorParams.AccelerationPoints)
#define NVRAM_DECELERATION_STEPS			((uint16) MotorParams.DecelerationSteps + 1) /* MMP130819-1 */


#define NVRAM_RUNNING_CURR_LEVEL			((uint16) MotorParams.RunningTorqueCurrent * 2u)
#define NVRAM_HOLDING_CURR_LEVEL			((uint16) MotorParams.HoldingTorqueCurrent * 2u)

#define NVRAM_CURRTHRSHLD_TEMP_1			( (int16) MotorParams.CurrThrshldTemp1 - C_TEMPOFF)
#define NVRAM_CURRTHRSHLD_TEMP_2			( (int16) MotorParams.CurrThrshldTemp2 - C_TEMPOFF)
#define NVRAM_CURRTHRSHLD_TEMP_3			( (int16) MotorParams.CurrThrshldTemp3 - C_TEMPOFF)
#define NVRAM_CURRTHRSHLD_TEMP_4			( (int16) MotorParams.CurrThrshldTemp4 - C_TEMPOFF)
#define NVRAM_CURRTHRSHLD_RATIO_1			((uint16) MotorParams.CurrThrshldRatio1)
#define NVRAM_CURRTHRSHLD_RATIO_2			((uint16) MotorParams.CurrThrshldRatio2)
#define NVRAM_CURRTHRSHLD_RATIO_3			((uint16) MotorParams.CurrThrshldRatio3)
#define NVRAM_CURRTHRSHLD_RATIO_4			((uint16) MotorParams.CurrThrshldRatio4)
#define NVRAM_CURRTHRSHLD_ZONE_1			((uint16) MotorParams.CurrThrshldZone1)
#define NVRAM_CURRTHRSHLD_ZONE_2			((uint16) MotorParams.CurrThrshldZone2)
#define NVRAM_CURRTHRSHLD_ZONE_3			((uint16) MotorParams.CurrThrshldZone3)
#define NVRAM_CURRTHRSHLD_ZONE_4			((uint16) MotorParams.CurrThrshldZone4)
#define NVRAM_CURRTHRSHLD_ZONE_5			((uint16) MotorParams.CurrThrshldZone5)

/* stall detection */
#define NVRAM_STALL_B_THRSHLD				((uint16) MotorParams.StallBemfThreshold)
#define NVRAM_STALL_B_DELAY					( (uint8) MotorParams.StallBemfDelay)

#define NVRAM_STALL_O_THRSHLD				((uint16) MotorParams.StallOscThreshold)
#define NVRAM_STALL_O_WIDTH					((uint16) MotorParams.StallOscWidth)
#define NVRAM_STALL_O_OFFSET				((uint16) C_STALL_O_OFFSET)						/* MMP140428-1 */

#define NVRAM_STALL_CURR_THRSHLD			((uint16) MotorParams.StallCurrentThreshold)
#define NVRAM_STALL_SPEED_DEPENDED			((uint16) MotorParams.StallSpeedDepended)

#define NVRAM_STALL_DETECTOR_DELAY			((uint16) MotorParams.StallDetectorDelay << 3u)

/* diagnostic */
#define NVRAM_APPL_UTEMP					((int16) MotorParams.ApplUnderTemperature - C_TEMPOFF)
#define NVRAM_APPL_OTEMP_WARN				((int16) MotorParams.ApplOverTemperatureWarn - C_TEMPOFF)
#define NVRAM_APPL_OTEMP_SHUT				((int16) MotorParams.ApplOverTemperatureShut - C_TEMPOFF)
#define NVRAM_APPL_UVOLT					((int16) ((MotorParams.ApplUnderVoltage * 25u) >> 1u))
#define NVRAM_APPL_OVOLT					((int16) ((MotorParams.ApplOverVoltage * 25u) >> 1u))

/* PID */
#define NVRAM_PID_RUNNINGCTRL_PER			((uint16) MotorParams.PidRunningCtrlPeriod)
#define NVRAM_PID_HOLDINGCTRL_PER			((uint16) MotorParams.PidHoldingCtrlPeriod << 2u)
#define NVRAM_PID_COEF_P					((uint16) MotorParams.PidCoefP)
#define NVRAM_PID_COEF_I					((uint16) MotorParams.PidCoefI)
#define NVRAM_PID_COEF_D					((uint16) MotorParams.PidCoefD)
#define NVRAM_PID_THRSHLDCTRL_PER			((uint16) MotorParams.PidThrshldCtrlPer << 7u)	/* MMP140911-1 */
#define NVRAM_MIN_HOLDCORR_RATIO			((uint16) (((uint32)PWM_REG_PERIOD * MotorParams.PidLowerHoldingLimit) >> (4u - PWM_PRESCALER_N)))
#define NVRAM_MIN_CORR_RATIO				((uint16) (((uint32)PWM_REG_PERIOD * MotorParams.PidLowerLimit) >> (4u - PWM_PRESCALER_N)))
#define NVRAM_MAX_CORR_RATIO				((uint16) (((uint32)PWM_REG_PERIOD * ((uint32)MotorParams.PidUpperLimit + 1u)) >> (4u - PWM_PRESCALER_N)))
#define NVRAM_PID_RAMP_DOWN					((uint16) (MotorParams.PidRampDown << 2u))
#define NVRAM_PID_RAMP_UP					((uint16) (MotorParams.PidRampUp << 2u))

#else  /* (MOTOR_PARAMS == MP_NVRAM) */

/* ****************************************************************************	*
 * Fixed constants (iso NVRAM)
 * ****************************************************************************	*/
#define NVRAM_CONFIGURATION_ID				CONFIGURATION_ID

#define NVRAM_VSUP_REF						((uint16) (C_VSUP_REF * 25) >> 1)
#define NVRAM_LEAD_ANGLE					((uint16) C_LEAD_ANGLE)
#define NVRAM_BROWNOUT_LEVEL				((uint16) C_BROWN_OUT_LEVEL)
#define NVRAM_VDS_THRESHOLD					((uint16) (C_VDS_THRESHOLD * 12.5))				/* MMP140428-1 */
#define NVRAM_GEARBOX_RATIO					((uint16) MOTOR_GEAR_BOX_RATIO)

#define NVRAM_POLE_PAIRS					((uint16) MOTOR_POLE_PAIRS)
#define NVRAM_MOTOR_PHASES					((uint16) (MOTOR_PHASES == 3) ? 1 : 0)
#define NVRAM_MOTOR_CONSTANT				((uint16) C_MOTOR_CONST_10MV_PER_RPS)
#define NVRAM_MOTOR_COIL_RTOT				((uint16) C_COILS_RTOT)

#define NVRAM_MICRO_STEPS					((uint16) 1u << MOTOR_MICROSTEPS)				/* Must be shift-factor */
#define NVRAM_MOTOR_DIR_INV					((uint16) MOTOR_DIR_INV)
#define NVRAM_TACHO_MODE					((uint16) C_TACHO_MODE)

/* Mechanical */
#define NVRAM_DEF_TRAVEL					((uint16) C_DEF_TRAVEL)							/* Full-stroke */
#define NVRAM_DEF_TRAVEL_TOLERANCE_HI		((uint16) C_DEF_TRAVEL_TOLERANCE_HI)			/* Deviation: 180 degrees of electric rotation */
#define NVRAM_DEF_TRAVEL_TOLERANCE_LO		((uint16) C_DEF_TRAVEL_TOLERANCE_LO)
#define NVRAM_DEF_EMRUN_POS					((uint16) C_DEF_EMRUN_POS)

/* speed */
#define NVRAM_SPEED0						((uint16) C_SPEED_0)
#define NVRAM_SPEED1						((uint16) C_SPEED_1)
#define NVRAM_SPEED2						((uint16) C_SPEED_2)
#define NVRAM_SPEED3						((uint16) C_SPEED_3)

#define NVRAM_MIN_SPEED						((uint16) C_SPEED_MIN)
#define NVRAM_ACCELERATION_CONST			((uint16) C_ACCELERATION_CONST)
#define NVRAM_ACCELERATION_POINTS			((uint16) C_ACCELERATION_POINTS)

#define NVRAM_HOLDING_CURR_LEVEL			((uint16) C_PID_HOLDING_CURR_LEVEL)
#define NVRAM_RUNNING_CURR_LEVEL			((uint16) C_PID_RUNNING_CURR_LEVEL)

/* self-heat */
#define NVRAM_CURRTHRSHLD_TEMP_1			( (int16) C_CURRTHRSHLD_TEMP_1)
#define NVRAM_CURRTHRSHLD_TEMP_2			( (int16) C_CURRTHRSHLD_TEMP_2)
#define NVRAM_CURRTHRSHLD_TEMP_3			( (int16) C_CURRTHRSHLD_TEMP_3)
#define NVRAM_CURRTHRSHLD_TEMP_4			( (int16) C_CURRTHRSHLD_TEMP_4)
#define NVRAM_CURRTHRSHLD_RATIO_1			((uint16) C_CURRTHRSHLD_RATIO_1)
#define NVRAM_CURRTHRSHLD_RATIO_2			((uint16) C_CURRTHRSHLD_RATIO_2)
#define NVRAM_CURRTHRSHLD_RATIO_3			((uint16) C_CURRTHRSHLD_RATIO_3)
#define NVRAM_CURRTHRSHLD_RATIO_4			((uint16) C_CURRTHRSHLD_RATIO_4)
#define NVRAM_CURRTHRSHLD_ZONE_1			((uint16) C_CURRTHRSHLD_AREA_1)
#define NVRAM_CURRTHRSHLD_ZONE_2			((uint16) C_CURRTHRSHLD_AREA_2)
#define NVRAM_CURRTHRSHLD_ZONE_3			((uint16) C_CURRTHRSHLD_AREA_3)
#define NVRAM_CURRTHRSHLD_ZONE_4			((uint16) C_CURRTHRSHLD_AREA_4)
#define NVRAM_CURRTHRSHLD_ZONE_5			((uint16) C_CURRTHRSHLD_AREA_5)

/* stall detection */
#define NVRAM_STALL_B_THRSHLD				((uint16) C_STALL_B_THRESHOLD)
#define NVRAM_STALL_B_DELAY					((uint16) C_STALL_B_DELAY)

#define NVRAM_STALL_O_THRSHLD				((uint16) C_STALL_O_THRESHOLD)
#define NVRAM_STALL_O_WIDTH					((uint16) C_STALL_O_WIDTH)
#define NVRAM_STALL_O_OFFSET				((uint16) C_STALL_O_OFFSET)					/* MMP140428-1 */
#define NVRAM_STALL_O_OFFSET				((uint16) C_STALL_O_OFFSET)					/* MMP140428-1 */

#define NVRAM_STALL_CURR_THRSHLD			((uint16) C_STALL_A_THRESHOLD)
#define NVRAM_STALL_SPEED_DEPENDED			((uint16) C_STALL_SPEED_DEPENDED)

#define NVRAM_STALL_DETECTOR_DELAY			((uint16) C_DETECTOR_DELAY)


#define NVRAM_APPL_UTEMP					( (int16) C_APPL_UTEMP)
#define NVRAM_APPL_OTEMP_WARN				( (int16) C_APPL_OTEMP_WARN)
#define NVRAM_APPL_OTEMP_SHUT				( (int16) C_APPL_OTEMP_SHUT)
#define NVRAM_APPL_UVOLT					( (int16) (C_APPL_UVOLT * 12.5))
#define NVRAM_APPL_OVOLT					( (int16) (C_APPL_OVOLT * 12.5))

#define NVRAM_PID_RUNNINGCTRL_PER			((uint16) C_PID_RUNNINGCTRL_PERIOD)
#define NVRAM_PID_HOLDINGCTRL_PER			((uint16) (C_PID_HOLDINGCTRL_PERIOD << 2))
#define NVRAM_PID_COEF_P					((uint16) C_PID_COEF_P)
#define NVRAM_PID_COEF_I					((uint16) C_PID_COEF_I)
#define NVRAM_PID_COEF_D					((uint16) C_PID_COEF_D)
#define NVRAM_PID_THRSHLDCTRL_PER			((uint16) (C_PID_THRSHLDCTRL_PERIOD << 7u))
#define NVRAM_MIN_HOLDCORR_RATIO			((uint16) ((((uint32)PWM_REG_PERIOD) * C_MIN_HOLDCORR_RATIO) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_MIN_CORR_RATIO				((uint16) ((((uint32)PWM_REG_PERIOD) * C_MIN_CORR_RATIO) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_MAX_CORR_RATIO				((uint16) ((((uint32)PWM_REG_PERIOD) * (C_MAX_CORR_RATIO + 1)) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_PID_RAMP_DOWN					((uint16) (C_PID_RAMP_DOWN << 2))
#define NVRAM_PID_RAMP_UP					((uint16) (C_PID_RAMP_UP << 2))
#endif /* (MOTOR_PARAMS == MOTOR_NVRAM) */

/* micro-steps per full step */
#define C_MICROSTEP_PER_FULLSTEP			NVRAM_MICRO_STEPS						   /* Number of micro-steps per full-step (1, 2, 4 , 8 or 16) */

/* HALL stall configuration */
#define C_MOTOR_HALL_STALLDET_STEP	    	(12u * C_MICROSTEP_PER_FULLSTEP)   		   /* 24 full steps for hall stall detection */
#define C_MOTOR_HALL_REBOUND_STEP_MIN		(1u * C_MICROSTEP_PER_FULLSTEP)
#define C_MOTOR_HALL_REBOUND_STEO_MAX		(3u * C_MICROSTEP_PER_FULLSTEP)

/* Motor running average filter length:4FS */
#define C_MOVAVG_SZ							((uint16)1u << C_MOVAVG_SSZ)		

/* MLX81310/15 Motor-Driver FETs (P+N-FET) */
#define C_FETS_RTOT					1u											/* 1R */

/* 16bits register */
typedef struct
{
	uint16 UV			:1;				/* Under Voltage */
	uint16 OV			:1;				/* Over Voltage */
	uint16 OPEN			:1;				/* Open Load */
	uint16 SHORT		:1;				/* Short Load */
	uint16 TW			:1;				/* Thermal Diagnostic */
	uint16 TS			:1;				/* Thermal Shutdown */
	uint16 ST			:4;				/* Stall */
	uint16 DRIFT		:1;				/* Drift */
	uint16 RSV			:5;				/* Reserved */
}Motor_FaultStatus;

typedef struct
{
	uint16 MotorCtrl;
	uint16 TgtPos;
	uint16 ActPos;
	uint16 Irun;
	uint16 Ihold;
	uint16 SpdRPM;
}Motor_ControlParams;

typedef struct
{
	uint8			  Mode;
	Motor_FaultStatus Fault;
	uint16 			  ActPos;
	uint16			  TgtPos;
	uint8			  Direction;
}Motor_RuntimeStatus;

/* ****************************************************************************	*
 *	public variables												*
 * ****************************************************************************	*/
#pragma space dp
extern volatile uint16 g_u16CorrectionRatio;									/* Motor correction ratio, depend on temperature and voltage */
extern uint16 g_u16MicroStepIdx;												/* (Micro)step index */
extern uint16 g_u16CommutTimerPeriod;											/* (Actual) commutation timer period (Commutation-ISR) */
extern uint16 g_u16TargetCommutTimerPeriod;										/* Target commutation timer period (target speed) */
extern uint16 g_u16StartupDelay;
extern uint16 g_u16MotorCurrentMovAvgxN;										/* Sum of last 16 motor-currents (x 4..16) */
extern uint16 g_u16MotorCurrentLPFx64;											/* Low-pass filter (IIR-1) motor-current x 64 */
extern uint8 g_u8MotorStopDelay;												/* Delay between drive stage from LS to TRI-STATE */
/* MMP151118-2 */
extern uint8 g_e8MotorDirectionCCW;												/* Control/Status-flag motor rotational direction Counter Clock-wise */
extern volatile uint16 g_u16ActuatorActPos;										/* (Motor-driver) Actual motor-rotor position [WD] */
extern volatile uint16 g_u16ActuatorTgtPos;										/* (Motor-driver) Target motor-rotor position [WD] */

extern volatile Motor_FaultStatus g_sMotorFault;								/* Motor Fault Status */

#pragma space none

#pragma space nodp
extern uint8 g_u8MotorHoldingCurrState;											/* Motor Holding Current State */
extern uint16 g_u16MotorSpeedRPS;												/* Target motor-speed [RPS] */
extern uint16 g_u16MotorMicroStepsPerElecRotation;								/* Number of micro-steps per electric rotation */
extern uint16 g_u16MotorMicroStepsPerMechRotation;								/* Number of micro-steps per mechanical rotation */
extern uint16 g_au16MotorSpeedCommutTimerPeriod[];
extern uint16 g_au16MotorSpeedRPS[];
extern uint16 l_u16MotorCurrentRawIdx;
extern uint16 l_au16MotorCurrentRaw[C_MOVAVG_SZ];
extern uint8 g_u8MotorStartupMode;

extern uint8 g_u8MotorStatusSpeed;						/* (Status) Actual motor-speed */

extern uint16 g_u16falg;
extern uint16 g_u16PosFlag;

/* debug motor current filter */
#if _DEBUG_MOTOR_CURRENT_FLT

#if _SUPPORT_STALLDET_O
#define C_MOTOR_CURR_SZ		(16*32)												/* 16 electric rotations of 32 uSteps */
#else  /* _SUPPORT_STALLDET_O */
#define C_MOTOR_CURR_SZ		(20*32)												/* 20 electric rotations of 32 uSteps */
#endif /* _SUPPORT_STALLDET_O */

extern uint8 l_au8MotorCurrRaw[C_MOTOR_CURR_SZ];								/* Raw (unfiltered) motor current measurement */
extern uint16 l_u16MotorCurrIdx;												/* Motor current measurement index */

#endif /* _DEBUG_MOTOR_CURRENT_FLT */

#pragma space none

/* ****************************************************************************	*
 *	public functions											*
 * ****************************************************************************	*/
extern void MotorDriver_MainFunction(void);
extern void MotorDriverInit( void);
extern void MotorDriverSetParams(Motor_ControlParams params);
extern void MotorDriverGetStatus(Motor_RuntimeStatus *pstatus);
extern void MotorDriverClearFaultStatus(void);


#endif /* MOTOR_DRIVER_H_ */

/* EOF */
