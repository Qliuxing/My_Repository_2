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
#include "MotorParams.h"
#include "NVRAM_UserPage.h"

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
	C_STOP_IMMEDIATE,															/* Stop actuator immediately, without start-up delay */
	C_STOP_EMERGENCY,															/* Stop actuator immediately, with start-up delay */
	C_STOP_SLEEP																/* Stop actuator w/o holding current */
};

#if 0
enum TACHO_MODES
{
	C_TACHO_NONE = 0,															/* No Tacho output */
	C_TACHO_60DEG_ELECTRIC,														/* Tacho output toggles every 60 degrees of an electric rotation */
	C_TACHO_180DEG_ELECTRIC,													/* Tacho output toggles every 180 degrees of an electric rotation */
	C_TACHO_180DEG_MECHANICAL													/* Tacho output toggles every 180 degrees of a mechanical rotation */
};
#endif /* 0 */

#define VSUP2PWM			TRUE												/* Motor driver operates between Vsup to PWM */

#define STARTUP_OPEN_LOOP			0											/* Start-up in Open-Loop */
#define STARTUP_CLOSED_LOOP_SRP		1											/* Start-up in Closed-Loop (SRP) */
#define STARTUP_STEPPER				2											/* Start-up in Stepper */
#define STARTUP_MICRO_STEPPER		3											/* Start-up in Micro-stepper (8x) */

#define EXT_PHASE_DEADTIME	(1 * (PLL_freq/(1000000*CYCLES_PER_INSTR*2)) + 1)	/* 1us: External P/N-FET dead-time (4) */

#if _SUPPORT_DOUBLE_USTEP
#define C_MICROSTEP_PER_FULLSTEP	16
#else  /* _SUPPORT_DOUBLE_USTEP */
#define C_MICROSTEP_PER_FULLSTEP	8
#endif /* _SUPPORT_DOUBLE_USTEP */

#if (MOTOR_PARAMS == MP_NVRAM)
/* ****************************************************************************	*
 * NVRAM (UniROM) parameters
 * ****************************************************************************	*/
#define NVRAM_PWMIN_FREQ					((uint16) ((4096 + g_NvramUser.PwmInFreqBase) << g_NvramUser.PwmInFreqExp) - 4096)
#define NVRAM_PWMIN_DIV						((uint16) g_NvramUser.PwmInDivider)
#define NVRAM_PWMIN_TOLERANCE				((uint16) g_NvramUser.PwmInFreqTolerance)
#define NVRAM_PWMIN_DC_SPEED_1				((uint16) g_NvramUser.PwmInSpeed1)
#define NVRAM_PWMIN_DC_SPEED_2				((uint16) g_NvramUser.PwmInSpeed2)
#define NVRAM_PWMIN_DC_SPEED_3				((uint16) g_NvramUser.PwmInSpeed3)
#define NVRAM_PWMIN_DC_SPEED_4				((uint16) g_NvramUser.PwmInSpeed4)
#define NVRAM_PWMOUT_PERIOD					((uint16) g_NvramUser.PwmOutPeriod)
#define NVRAM_PWMOUT_IDLE					((uint16) g_NvramUser.PwmOutIdle)
#define NVRAM_PWMOUT_DIAG_OK				((uint16) g_NvramUser.PwmOutDiagOk)
#define NVRAM_PWMOUT_DIAG_OVER_TEMP			((uint16) g_NvramUser.PwmOutDiagOverTemp)
#define NVRAM_PWMOUT_DIAG_STALL				((uint16) g_NvramUser.PwmOutDiagStall)
#define NVRAM_PWMOUT_DIAG_MECH_ERR			((uint16) g_NvramUser.PwmOutDiagMechErr)
#define NVRAM_PWMOUT_DIAG_ELEC_ERR			((uint16) g_NvramUser.PwmOutDiagElecErr)
#define NVRAM_PWMOUT_DIAG_VSUP				((uint16) g_NvramUser.PwmOutDiagVsup)
#define NVRAM_PWMOUT_DIAG_RESET				((uint16) g_NvramUser.PwmOutDiagReset)
#define NVRAM_PWMOUT_DIAG_CUST_ERR			((uint16) g_NvramUser.PwmOutDiagCustomErr)
#define NVRAM_VSUP_REF						((uint16) (g_NvramUser.VsupRef * 25) >> 1)
#define NVRAM_VSUP_TEMP_COEF				( (int16) g_NvramUser.VsupRefTempCoef)
#define NVRAM_VSUP_TEMP_COEF2				( (int16) g_NvramUser.VsupRefTempCoef2)
#define NVRAM_LEAD_ANGLE					((uint16) g_NvramUser.LeadAngle)
#define NVRAM_BROWNOUT_LEVEL				((uint16) g_NvramUser.BrownoutLevel)
#define NVRAM_GEARBOX_RATIO					((uint16) g_NvramUser.GearBoxRatio)
#define NVRAM_POLE_PAIRS					((uint16) g_NvramUser.PolePairs + 1)
#define NVRAM_MICRO_STEPS					((uint16) g_NvramUser.MicroSteps)
#define NVRAM_MOTOR_PHASES					((uint16) g_NvramUser.MotorPhases)
#define NVRAM_STARTUP_STALL_DELAY			((uint16) g_NvramUser.StallStartupDelay)
#define NVRAM_STARTUP_STALL_THRSHLD			((uint16) g_NvramUser.StallStartupThreshold)
#if _SUPPORT_STALLDET_B
#define NVRAM_STALL_B_THRSHLD				((uint16) g_NvramUser.StallBemfThreshold)
#define NVRAM_STALL_B_DELAY					( (uint8) g_NvramUser.StallBemfDelay)
#endif /* _SUPPORT_STALLDET_B */
#if _SUPPORT_STALLDET_O
#define NVRAM_STALL_O_THRSHLD				((uint16) g_NvramUser.StallOscThreshold)
#define NVRAM_STALL_O_WIDTH					((uint16) g_NvramUser.StallOscWidth + 1)
#endif /* _SUPPORT_STALLDET_O */
#define NVRAM_STALL_CURR_THRSHLD			((uint16) g_NvramUser.StallCurrentThreshold)
#define NVRAM_STALL_SPEED_DEPENDED			((uint16) g_NvramUser.StallSpeedDepended)
#define NVRAM_RESTALL_POR					((uint16) g_NvramUser.RestallPor)
#define NVRAM_VDS_THRESHOLD					((uint16) (g_NvramUser.VdsThreshold * 25) >> 1)
#define NVRAM_SPEED0						((uint16) g_NvramUser.Speed_0)
#define NVRAM_SPEED1						((uint16) g_NvramUser.Speed_1)
#define NVRAM_SPEED2						((uint16) g_NvramUser.Speed_2)
#define NVRAM_SPEED3						((uint16) g_NvramUser.Speed_3)
#define NVRAM_SPEED_TORQUE_BOOST			((uint16) g_NvramUser.SpeedTorqueBoost)
#define NVRAM_AUTOSPEED_TEMP_1				( (int16) g_NvramUser.AutoSpeedTemperature1 - C_TEMPOFF)
#define NVRAM_AUTOSPEED_TEMP_2				( (int16) g_NvramUser.AutoSpeedTemperature2 - C_TEMPOFF)
#define NVRAM_AUTOSPEED_TEMP_3				( (int16) g_NvramUser.AutoSpeedTemperature3 - C_TEMPOFF)
#define NVRAM_AUTOSPEED_TEMPZONE_1			((uint16) g_NvramUser.AutoSpeedZone1)
#define NVRAM_AUTOSPEED_TEMPZONE_2			((uint16) g_NvramUser.AutoSpeedZone2)
#define NVRAM_AUTOSPEED_TEMPZONE_3			((uint16) g_NvramUser.AutoSpeedZone3)
#define NVRAM_AUTOSPEED_TEMPZONE_4			((uint16) g_NvramUser.AutoSpeedZone4)
#define NVRAM_AUTOSPEED_VOLT_1				( (int16) (g_NvramUser.AutoSpeedVoltage1 * 25) >> 1)
#define NVRAM_AUTOSPEED_VOLT_2				( (int16) (g_NvramUser.AutoSpeedVoltage2 * 25) >> 1)
#define NVRAM_AUTOSPEED_VOLT_3				( (int16) (g_NvramUser.AutoSpeedVoltage3 * 25) >> 1)
#define NVRAM_AUTOSPEED_VOLTZONE_1			((uint16) g_NvramUser.AutoSpeedVoltageZone1)
#define NVRAM_AUTOSPEED_VOLTZONE_2			((uint16) g_NvramUser.AutoSpeedVoltageZone2)
#define NVRAM_AUTOSPEED_VOLTZONE_3			((uint16) g_NvramUser.AutoSpeedVoltageZone3)
#define NVRAM_AUTOSPEED_VOLTZONE_4			((uint16) g_NvramUser.AutoSpeedVoltageZone4)
#define NVRAM_MIN_SPEED						((uint16) g_NvramUser.MinimumSpeed)
#define NVRAM_ACCELERATION_CONST			((uint16) g_NvramUser.AccelerationConst)
#define NVRAM_ACCELERATION_POINTS			((uint16) g_NvramUser.AccelerationPoints)
#define NVRAM_TACHO_MODE					((uint16) g_NvramUser.TachoMode)
#define NVRAM_MOTOR_CONSTANT				((uint16) g_NvramUser.MotorConstant)
#define NVRAM_MOTOR_COIL_RTOT				((uint16) g_NvramUser.MotorCoilRtot)
#define NVRAM_HOLDING_CURR_LEVEL			((uint16) g_NvramUser.HoldingTorqueCurrent)
#if (defined __MLX81315_A__) && _SUPPORT_QUADRUPLE_MOTOR_CURRENT
#define NVRAM_RUNNING_CURR_LEVEL			((uint16) (4 * g_NvramUser.RunningTorqueCurrent))
#define NVRAM_BOOST_CURR_LEVEL				((uint16) (4 * g_NvramUser.BoostTorqueCurrent))
#elif (_SUPPORT_DOUBLE_MOTOR_CURRENT != FALSE)
#define NVRAM_RUNNING_CURR_LEVEL			((uint16) (2 * g_NvramUser.RunningTorqueCurrent))
#define NVRAM_BOOST_CURR_LEVEL				((uint16) (2 * g_NvramUser.BoostTorqueCurrent))
#else  /* (_SUPPORT_DOUBLE_MOTOR_CURRENT != FALSE) */
#define NVRAM_RUNNING_CURR_LEVEL			((uint16) g_NvramUser.RunningTorqueCurrent)
#define NVRAM_BOOST_CURR_LEVEL				((uint16) g_NvramUser.BoostTorqueCurrent)
#endif /* (_SUPPORT_DOUBLE_MOTOR_CURRENT != FALSE) */
#define NVRAM_CURRTHRSHLD_TEMP_1			( (int16) g_NvramUser.CurrThrshldTemp1 - C_TEMPOFF)
#define NVRAM_CURRTHRSHLD_TEMP_2			( (int16) g_NvramUser.CurrThrshldTemp2 - C_TEMPOFF)
#define NVRAM_CURRTHRSHLD_TEMP_3			( (int16) g_NvramUser.CurrThrshldTemp3 - C_TEMPOFF)
#define NVRAM_CURRTHRSHLD_TEMP_4			( (int16) g_NvramUser.CurrThrshldTemp4 - C_TEMPOFF)
#define NVRAM_CURRTHRSHLD_RATIO_1			((uint16) g_NvramUser.CurrThrshldRatio1)
#define NVRAM_CURRTHRSHLD_RATIO_2			((uint16) g_NvramUser.CurrThrshldRatio2)
#define NVRAM_CURRTHRSHLD_RATIO_3			((uint16) g_NvramUser.CurrThrshldRatio3)
#define NVRAM_CURRTHRSHLD_RATIO_4			((uint16) g_NvramUser.CurrThrshldRatio4)
#define NVRAM_CURRTHRSHLD_ZONE_1			((uint16) g_NvramUser.CurrThrshldZone1)
#define NVRAM_CURRTHRSHLD_ZONE_2			((uint16) g_NvramUser.CurrThrshldZone2)
#define NVRAM_CURRTHRSHLD_ZONE_3			((uint16) g_NvramUser.CurrThrshldZone3)
#define NVRAM_CURRTHRSHLD_ZONE_4			((uint16) g_NvramUser.CurrThrshldZone4)
#define NVRAM_CURRTHRSHLD_ZONE_5			((uint16) g_NvramUser.CurrThrshldZone5)
#define NVRAM_APPL_UTEMP					( (int16) g_NvramUser.ApplUnderTemperature - C_TEMPOFF)
#define NVRAM_APPL_OTEMP					( (int16) g_NvramUser.ApplOverTemperature - C_TEMPOFF)
#define NVRAM_APPL_UVOLT					( (int16) (g_NvramUser.ApplUnderVoltage * 25) >> 1)
#define NVRAM_APPL_OVOLT					( (int16) (g_NvramUser.ApplOverVoltage * 25) >> 1)
#define NVRAM_PID_RUNNINGCTRL_PER			((uint16) g_NvramUser.PidRunningCtrlPeriod)
#define NVRAM_PID_HOLDINGCTRL_PER			((uint16) (g_NvramUser.PidHoldingCtrlPeriod << 2))
#define NVRAM_PID_COEF_P					((uint16) g_NvramUser.PidCoefP)
#define NVRAM_PID_COEF_I					((uint16) g_NvramUser.PidCoefI)
#define NVRAM_PID_COEF_D					((uint16) g_NvramUser.PidCoefD)
#define NVRAM_PID_THRSHLDCTRL_PER			((uint16) ((g_NvramUser.PidThrshldCtrlPer << 8) >> 1))
#define NVRAM_MIN_HOLDCORR_RATIO			((uint16) ((((uint32)PWM_REG_PERIOD) * g_NvramUser.PidLowerHoldingLimit) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_MIN_CORR_RATIO				((uint16) ((((uint32)PWM_REG_PERIOD) * g_NvramUser.PidLowerLimit) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_MAX_CORR_RATIO				((uint16) ((((uint32)PWM_REG_PERIOD) * (g_NvramUser.PidUpperLimit + 1)) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_MAX_CORR_RATIO_SP_BOOST		((uint16) ((((uint32)PWM_REG_PERIOD) * (g_NvramUser.PidUpperLimitSpBoost + 1)) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_MAX_CORR_RATIO_TQ_BOOST		((uint16) ((((uint32)PWM_REG_PERIOD) * (g_NvramUser.PidUpperLimitTqBoost + 1)) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_PID_RAMP_DOWN					((uint16) (g_NvramUser.PidRampDown << 2))
#define NVRAM_PID_RAMP_UP					((uint16) (g_NvramUser.PidRampUp << 2))
#define NVRAM_REWIND_STEPS					((uint16) g_NvramUser.RewindSteps)
#define NVRAM_SRP_ADC_RATE_PERIOD			((uint16) g_NvramUser.SrpAdcRatePeriod)
#define NVRAM_STARTUP_CORR_RATIO			((uint16) ((((uint32)PWM_REG_PERIOD) * g_NvramUser.StartupLimit) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_SPEED_TOLERANCE				((uint16) g_NvramUser.SpeedTolerance)
#define NVRAM_STALL_DETECTOR_DELAY			((uint16) (g_NvramUser.StallDetectorDelay << 3))
#define NVRAM_LIN_UV						((uint16) g_NvramUser.LinUV)
#define NVRAM_DECELERATION_STEPS			((uint16) g_NvramUser.DecelerationSteps + 1)
#define NVRAM_AUTO_RECALIBRATE				((uint16) g_NvramUser.AutoRecalibration)
#define NVRAM_BUSTIMEOUT_SLEEP				((uint16) g_NvramUser.BusTimeOutSleep)
#define NVRAM_STALL_O						((uint16) g_NvramUser.StallO)
#define NVRAM_STALL_O_OFFSET				((uint16) C_STALL_O_OFFSET)

#else  /* (MOTOR_PARAMS == MP_NVRAM) */

/* ****************************************************************************	*
 * Fixed constants (iso NVRAM)
 * ****************************************************************************	*/
#define NVRAM_PWMIN_FREQ					((uint16) C_PWM_PERIOD_SEL)
#define NVRAM_PWMIN_DIV						((uint16) C_PWM_PERIOD_DIV)
#define NVRAM_PWMIN_TOLERANCE				((uint16) C_PWMIN_TOLERANCE)
#define NVRAM_PWMOUT_PERIOD					((uint16) C_PWMOUT_PERIOD)
#define NVRAM_PWMOUT_IDLE					((uint16) C_PWMOUT_IDLE)
#define NVRAM_PWMOUT_DIAG_OK				((uint16) C_PWMOUT_DIAG_OK)
#define NVRAM_PWMOUT_DIAG_OVER_TEMP			((uint16) C_PWMOUT_DIAG_OVERTEMP)
#define NVRAM_PWMOUT_DIAG_STALL				((uint16) C_PWMOUT_DIAG_STALL)
#define NVRAM_PWMOUT_DIAG_MECH_ERR			((uint16) C_PWMOUT_DIAG_MECH_ERR)
#define NVRAM_PWMOUT_DIAG_ELEC_ERR			((uint16) C_PWMOUT_DIAG_ELEC_ERR)
#define NVRAM_PWMOUT_DIAG_VSUP				((uint16) C_PWMOUT_DIAG_VSUP)
#define NVRAM_PWMOUT_DIAG_RESET				((uint16) C_PWMOUT_DIAG_RESET)
#define NVRAM_PWMOUT_DIAG_CUST_ERR			((uint16) C_PWMOUT_DIAG_CUST_ERR)
#define NVRAM_VSUP_REF						((uint16) (C_VSUP_REF * 25) >> 1)
#define NVRAM_VSUP_TEMP_COEF				( (int16) C_VSUP_TEMP_COEF)
#define NVRAM_VSUP_TEMP_COEF2				( (int16) C_VSUP_TEMP_COEF2)
#define NVRAM_BROWNOUT_LEVEL				((uint16) C_BROWN_OUT_LEVEL)
#define NVRAM_GEARBOX_RATIO					((uint16) MOTOR_GEAR_BOX_RATIO)
#define NVRAM_POLE_PAIRS					((uint16) MOTOR_POLE_PAIRS)
#define NVRAM_MICRO_STEPS					((uint16) MOTOR_MICROSTEPS)	/* Must be shift-factor */
#define NVRAM_MOTOR_PHASES					((uint16) (MOTOR_PHASES == 3) ? 1 : 0)
#define NVRAM_STALL_B_THRSHLD				((uint16) C_STALL_B_THRESHOLD)
#define NVRAM_STALL_B_DELAY					( (uint8) C_STALL_B_DELAY)
#define NVRAM_STALL_CURR_THRSHLD			((uint16) C_STALL_COEF)
#define NVRAM_STALL_SPEED_DEPENDED			((uint16) C_STALL_SPEED_DEPENDED)
#define NVRAM_RESTALL_POR					((uint16) C_RESTALL_POR)
#define NVRAM_VDS_THRESHOLD					((uint16) (C_VDS_THRESHOLD * 12.5))
#define NVRAM_SPEED0						((uint16) C_SPEED_0)
#define NVRAM_SPEED1						((uint16) C_SPEED_1)
#define NVRAM_SPEED2						((uint16) C_SPEED_2)
#define NVRAM_SPEED3						((uint16) C_SPEED_3)
#define NVRAM_SPEED_TORQUE_BOOST			((uint16) C_SPEED_TORQUE_BOOST)
#define NVRAM_AUTOSPEED_TEMP_1				((uint16) C_AUTOSPEED_TEMP_1)
#define NVRAM_AUTOSPEED_TEMP_2				((uint16) C_AUTOSPEED_TEMP_2)
#define NVRAM_AUTOSPEED_TEMP_3				((uint16) C_AUTOSPEED_TEMP_3)
#define NVRAM_AUTOSPEED_TEMPZONE_1			((uint16) C_AUTOSPEED_TEMPZONE_1)
#define NVRAM_AUTOSPEED_TEMPZONE_2			((uint16) C_AUTOSPEED_TEMPZONE_2)
#define NVRAM_AUTOSPEED_TEMPZONE_3			((uint16) C_AUTOSPEED_TEMPZONE_3)
#define NVRAM_AUTOSPEED_TEMPZONE_4			((uint16) C_AUTOSPEED_TEMPZONE_4)
#define NVRAM_AUTOSPEED_VOLT_1				( (int16) (C_AUTOSPEED_VOLT_1 * 12.5))
#define NVRAM_AUTOSPEED_VOLT_2				( (int16) (C_AUTOSPEED_VOLT_2 * 12.5))
#define NVRAM_AUTOSPEED_VOLT_3				( (int16) (C_AUTOSPEED_VOLT_3 * 12.5))
#define NVRAM_AUTOSPEED_VOLTZONE_1			((uint16) C_AUTOSPEED_VOLTZONE_1)
#define NVRAM_AUTOSPEED_VOLTZONE_2			((uint16) C_AUTOSPEED_VOLTZONE_2)
#define NVRAM_AUTOSPEED_VOLTZONE_3			((uint16) C_AUTOSPEED_VOLTZONE_3)
#define NVRAM_AUTOSPEED_VOLTZONE_4			((uint16) C_AUTOSPEED_VOLTZONE_4)
#define NVRAM_MIN_SPEED						((uint16) C_SPEED_MIN)
#define NVRAM_ACCELERATION_CONST			((uint16) C_ACCELERATION_CONST)
#define NVRAM_ACCELERATION_POINTS			((uint16) C_ACCELERATION_POINTS)
#define NVRAM_TACHO_MODE					((uint16) C_TACHO_MODE)
#define NVRAM_MOTOR_CONSTANT				((uint16) C_MOTOR_CONST_10MV_PER_RPS)
#define NVRAM_MOTOR_COIL_RTOT				((uint16) C_COILS_RTOT)
#define NVRAM_HOLDING_CURR_LEVEL			((uint16) C_PID_HOLDING_CURR_LEVEL)
#define NVRAM_RUNNING_CURR_LEVEL			((uint16) C_PID_RUNNING_CURR_LEVEL)
#define NVRAM_BOOST_CURR_LEVEL				((uint16) C_PID_BOOST_CURR_LEVEL)
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
#define NVRAM_APPL_UTEMP					( (int16) C_APPL_UTEMP)
#define NVRAM_APPL_OTEMP					( (int16) C_APPL_OTEMP)
#define NVRAM_APPL_UVOLT					( (int16) (C_APPL_UVOLT * 12.5))
#define NVRAM_APPL_OVOLT					( (int16) (C_APPL_OVOLT * 12.5))
#define NVRAM_PID_RUNNINGCTRL_PER			((uint16) C_PID_RUNNINGCTRL_PERIOD)
#define NVRAM_PID_HOLDINGCTRL_PER			((uint16) (C_PID_HOLDINGCTRL_PERIOD << 2))
#define NVRAM_PID_COEF_P					((uint16) C_PID_COEF_P)
#define NVRAM_PID_COEF_I					((uint16) C_PID_COEF_I)
#define NVRAM_PID_COEF_D					((uint16) C_PID_COEF_D)
#define NVRAM_PID_THRSHLDCTRL_PER			((uint16) (C_PID_THRSHLDCTRL_PERIOD << 7))
#define NVRAM_MIN_HOLDCORR_RATIO			((uint16) ((((uint32)PWM_REG_PERIOD) * C_MIN_HOLDCORR_RATIO) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_MIN_CORR_RATIO				((uint16) ((((uint32)PWM_REG_PERIOD) * C_MIN_CORR_RATIO) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_MAX_CORR_RATIO				((uint16) ((((uint32)PWM_REG_PERIOD) * (C_MAX_CORR_RATIO + 1)) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_MAX_CORR_RATIO_SP_BOOST		((uint16) ((((uint32)PWM_REG_PERIOD) * (C_MAX_CORR_RATIO_SP_BOOST + 1)) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_MAX_CORR_RATIO_TQ_BOOST		((uint16) ((((uint32)PWM_REG_PERIOD) * (C_MAX_CORR_RATIO_TQ_BOOST + 1)) >> (4 - PWM_PRESCALER_N)))
#define NVRAM_PID_RAMP_DOWN					((uint16) (C_PID_RAMP_DOWN << 2))
#define NVRAM_PID_RAMP_UP					((uint16) (C_PID_RAMP_UP << 2))
#define NVRAM_REWIND_STEPS					((uint16) C_REWIND_STEPS)
#define NVRAM_SRP_ADC_RATE_PERIOD			((uint16) C_SRP_ADC_RATE_PERIOD)
#define NVRAM_STARTUP_CORR_RATIO			((uint16) ((((uint32)PWM_REG_PERIOD) * C_STARTUP_CORR_RATIO) >> (4 - PWM_prescaler_N)))
#define NVRAM_SPEED_TOLERANCE				((uint16) C_SPEED_TOLERANCE)
#define NVRAM_STALL_DETECTOR_DELAY			((uint16) C_DETECTOR_DELAY)
#define NVRAM_LIN_UV						((uint16) C_LIN_UV)
#define NVRAM_STALL_O						((uint16) C_STALL_O)
#define NVRAM_STALL_O_OFFSET				((uint16) C_STALL_O_OFFSET)
#endif /* (MOTOR_PARAMS == MOTOR_NVRAM) */

/* Motor running average filter length */
#define C_MOVAVG_SZ					(1 << C_MOVAVG_SSZ)		

/* MLX81310/15 Motor-Driver FETs (P+N-FET) */
#if (defined __MLX81315_A__)
#define C_FETS_RTOT					1											/* 1R */
#else  /* (defined __MLX81315_A__) */
#define C_FETS_RTOT					6											/* 6R */
#endif /* (defined __MLX81315_A__) */

/* Enable the driver and the PWM phase W, V and U */
#define DRVCFG_DIS()		{DRVCFG = (DRVCFG | DIS_DRV);}
/* 3-Phase: U, V and W */
#define DRVCFG_DIS_UVW()	{DRVCFG = (DRVCFG & ~(DIS_DRV|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U));}
#define DRVCFG_DIS_UVWz()	{DRVCFG = (DRVCFG & ~(DRV_CFG_W|DRV_CFG_V|DRV_CFG_U));}
#define DRVCFG_PWM_UVW()	{DRVCFG = ((DRVCFG & ~(DIS_DRV|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U)) | (DRV_CFG_W_PWM|DRV_CFG_V_PWM|DRV_CFG_U_PWM));}
#define DRVCFG_GND_UVW()	{DRVCFG = (DRVCFG & ~(DRV_CFG_W|DRV_CFG_V|DRV_CFG_U)) | (DRV_CFG_W_0|DRV_CFG_V_0|DRV_CFG_U_0);}
#define DRVCFG_VSUP_UVW()	{DRVCFG = (DRVCFG & ~(DRV_CFG_W|DRV_CFG_V|DRV_CFG_U)) | (DRV_CFG_W_1|DRV_CFG_V_1|DRV_CFG_U_1);}
#define DRVCFG_CNFG_UVW(x)	{DRVCFG = ((DRVCFG & ~(DIS_DRV|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U)) | x);}
/* 4-Phase: U, V, W and T */
#define DRVCFG_DIS_UVWT()	{DRVCFG =  (DRVCFG & ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U));}
#define DRVCFG_PWM_UVWT()	{DRVCFG = ((DRVCFG & ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U)) | (DRV_CFG_T_PWM|DRV_CFG_W_PWM|DRV_CFG_V_PWM|DRV_CFG_U_PWM));}
#define DRVCFG_GND_UVWT()	{DRVCFG = ((DRVCFG & ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U)) | (DRV_CFG_T_0|DRV_CFG_W_0|DRV_CFG_V_0|DRV_CFG_U_0));}
#define DRVCFG_VSUP_UVWT()	{DRVCFG = ((DRVCFG & ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U)) | (DRV_CFG_T_1|DRV_CFG_W_1|DRV_CFG_V_1|DRV_CFG_U_1));}
#define DRVCFG_CNFG_UVWT(x)	{DRVCFG = ((DRVCFG & ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U)) | x);}
#define DRVCFG_DIS_T()		{DRVCFG =  (DRVCFG & ~(DIS_DRV|DRV_CFG_T));}
#define DRVCFG_PWM_T()		{DRVCFG = ((DRVCFG & ~(DIS_DRV|DRV_CFG_T)) | (DRV_CFG_T_PWM));}

#define DRVCFG_PWM_VW()		{DRVCFG = ((DRVCFG & ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U)) | (DRV_CFG_W_PWM|DRV_CFG_V_PWM));}
#define DRVCFG_PWM_UT()		{DRVCFG = ((DRVCFG & ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U)) | (DRV_CFG_T_PWM|DRV_CFG_U_PWM));}

/* Zero-crossing detector state */
#define ZC_RESET	0
#define ZC_START	1
#define ZC_FOUND	2

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
extern void MotorDriverInit( void);
extern void MotorDriverSelfTest( void);
extern void MotorDriverStart( void);
extern void MotorDriverStop( uint16 u16Immediate);
extern void MotorDriverCurrentMeasureInit( void);
extern void MotorDriverCurrentMeasure( uint16 u16RunningMode);
extern void MotorDriver_4PhaseStepper( void);
extern void MotorDriver_EmergencyDrive( void);									/* Emergency motor driver firmware */

/* ****************************************************************************	*
 *	P u b l i c   v a r i a b l e s												*
 * ****************************************************************************	*/
#pragma space dp
extern volatile uint16 g_u16CorrectionRatio;									/* Motor correction ratio, depend on temperature and voltage */
extern uint16 g_u16MicroStepIdx;													/* (Micro)step index */
extern uint16 g_u16CommutTimerPeriod;											/* (Actual) commutation timer period (Commutation-ISR) */
extern uint16 g_u16TargetCommutTimerPeriod;										/* Target commutation timer period (target speed) */
extern uint16 g_u16StartupDelay;
extern uint16 g_u16MotorCurrentMovAvgxN;										/* Sum of last 16 motor-currents (x 4..16) */
extern uint16 g_u16MotorCurrentLPFx64;											/* Low-pass filter (IIR-1) motor-current x 64 */
extern uint8 g_u8MotorStopDelay;												/* Delay between drive stage from LS to TRI-STATE */

extern volatile uint16 g_u16ActuatorActPos __attribute__ ((section(".dp.noinit")));	/* (Motor-driver) Actual motor-rotor position [WD] */
extern uint16 g_u16ActuatorTgtPos __attribute__ ((section(".dp.noinit")));		/* (Motor-driver) Target motor-rotor position [WD] */
#pragma space none

extern uint8 g_u8MotorHoldingCurrState;											/* Motor Holding Current State */
extern uint16 g_u16MotorSpeedRPS;												/* Target motor-speed [RPS] */
extern uint16 g_u16MotorMicroStepsPerElecRotation;								/* Number of micro-steps per electric rotation */
extern uint16 g_u16MotorMicroStepsPerMechRotation;								/* Number of micro-steps per mechanical rotation */
extern uint16 g_u16MotorRewindSteps;											/* Number of Rewind-uSteps */
extern uint16 g_au16MotorSpeedCommutTimerPeriod[];
extern uint16 g_au16MotorSpeedRPS[];
extern int16 g_i16VoltStepIdx;
extern uint16 l_u16MotorCurrentRawIdx;
extern uint16 l_au16MotorCurrentRaw[C_MOVAVG_SZ];

#if _DEBUG_MOTOR_CURRENT_FLT
#if (_SUPPORT_DOUBLE_USTEP == FALSE)
#if _SUPPORT_STALLDET_O
#define C_MOTOR_CURR_SZ		(20*32)												/* 20 electric rotations of 32 uSteps */
#else  /* _SUPPORT_STALLDET_O */
#define C_MOTOR_CURR_SZ		(24*32)												/* 24 electric rotations of 32 uSteps */
#endif /* _SUPPORT_STALLDET_O */
#else  /* (_SUPPORT_DOUBLE_USTEP == FALSE) */
#if _SUPPORT_STALLDET_O
#define C_MOTOR_CURR_SZ		(10*64)												/* 10 electric rotations of 64 uSteps */
#else  /* _SUPPORT_STALLDET_O */
#define C_MOTOR_CURR_SZ		(12*64)												/* 12 electric rotations of 64 uSteps */
#endif /* _SUPPORT_STALLDET_O */
#endif /* (_SUPPORT_DOUBLE_USTEP == FALSE) */
extern uint8 l_au8MotorCurrRaw[C_MOTOR_CURR_SZ];								/* Raw (unfiltered) motor current measurement */
extern uint16 l_u16MotorCurrIdx;												/* Motor current measurement index */
#endif /* _DEBUG_MOTOR_CURRENT_FLT */

#endif /* MOTOR_DRIVER_H_ */

/* EOF */
