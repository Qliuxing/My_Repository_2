/*! \file		MotorParams.h
 *  \brief		MLX81310 Motor Parameters
 *
 * \note		project MLX81310
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
 *	Configuration ID	CID	[Hex]	Motor description
 *	UniROM Basic - G	UCG	0x5458	Sanhua BiPolar (4-phase) Expansion-Valve
 *	UniROM Basic - P	UCP 0x5460	Sanhua BiPolar (4-phase) Water-Valve
 *
 * ****************************************************************************	*/

#ifndef MOTOR_PARAMS_H_
#define MOTOR_PARAMS_H_

#include "syslib.h"
#include "Build.h"

#define C_BROWN_OUT_LEVEL			1											/* 0 = 6V, 1 = 7V, 2 = 8V & 3 = 9V +/- 0.5V */
#define C_HALL_LATCHES				0											/* No Hall-Latches */

//#define C_AUTO_RECALIBRATE		1											/* Enable Auto re-calibration */
//#define C_LIN_UV					2											/* LIN UV: 7.0V (6.0V + n * 0.V) (MMP131216-1) */
#define C_VDS_RUN					65											/* voltage drop Vds running  [10mV/LSB] */
#define C_VDS_STOP					35											/* Vds stop  */
#define C_VOLTAGE_HYS				50											/* Voltage hysteric:  0.5V (  50 x 10mV) */
#define C_TEMPERATURE_HYS			5											/* Temperature hysteric: 5 degrees Celsius */

/* system configuration and calibration info: */
typedef struct
{
	
	/* Actuator (VALVE): 0x10+[6] */
	uint16 DefTravel;										/* 0x00: Default Travel */
	uint16 DefTravelToleranceLo	 :8;						/* 0x02: Default Travel Tolerance (Lower) */
	uint16 DefTravelToleranceHi  :8;						/* 0x03: Default Travel Tolerance (Upper) */
	uint16 EmergencyRunPos;									/* 0x04: Emergency Run stop position  */
	
	/* Motor: 0x16+[8] */
	uint16 ConfigurationID;									/* 0x00: Configuration ID */
	uint16 MotorFamily			: 8;						/* 0x02: Motor Family */
	uint16 MotorPhases			: 4;						/* 0x03 [3:0]: Number of motor-phases: 1 = 3-phase, 0 = Bi-polar */
	uint16 PolePairs			: 4;						/* 0x03 [7:4]: Number of pole-pairs + 1 */
	uint16 MotorConstant		: 8;						/* 0x04: Motor Constant BEMF [10mV/rps] */
	uint16 MotorCoilRtot		: 8;						/* 0x05: Motor coil resistance (total) */
	uint16 MicroSteps			: 4;						/* 0x06 [3:0]: Number of micro-steps: 2^n (or 1 << n) */
	uint16 MotorDirectionINV	: 4;						/* 0x06 [7:4]: Motor rotational direction: 0=CW, 1=CCW */
	uint16 Reserved1D			: 8;						/* 0x07: Reserved */

	/* current/voltage: 0x1E+[8] */
	uint16 HoldingTorqueCurrent	: 8;						/* 0x00: Holding Torque current threshold */
	uint16 RunningTorqueCurrent : 8;						/* 0x01: Running Torque current threshold */
	uint16 LeadAngle			: 6;						/* 0x02 [5:0]: Lead Angle */
	uint16 BrownoutLevel		: 2;						/* 0x02 [7:6]: Brown-out level */
	uint16 VsupRef				: 8;						/* 0x03: Vsupply reference [1/8V] */
	uint16 VdsThreshold			: 8;						/* 0x04: Vds Threshold (MMP140428-1) [1/8V] */
	uint16 Reserved23			: 8;						/* 0x05: Reserved */
	uint16 GearBoxRatio;									/* 0x06: Gear-box ratio, e.g. 600:1 */ 

	/* stall:0x26 + [8]*/
	uint16 StallBemfDelay		: 8;						/* 0x00.: Stall-detector "B" delay */
	uint16 StallBemfThreshold	: 8;						/* 0x01.: Stall-detector "B" threshold (BEMF) */
	uint16 StallOscWidth		: 8;						/* 0x02.: Stall-detector "O" Width */
	uint16 StallOscThreshold	: 8;						/* 0x03.: Stall-detector "O" threshold (Oscillation) */
	uint16 StallCurrentThreshold: 8;						/* 0x04.: Stall current threshold ratio */
	uint16 StallSpeedDepended	: 8;						/* 0x05: Stall speed depended */
	uint16 StallDetectorDelay	: 8;						/* 0x06: Stall detector delay */
	uint16 Reserved31			: 8;						/* 0x07: Reserved */

	/* Four speed points:0x2E+[8] */
	uint16 Speed_0;											/* 0x00: Speed_0 */
	uint16 Speed_1;											/* 0x02: Speed_1 */
	uint16 Speed_2;											/* 0x04: Speed_2 */
	uint16 Speed_3;											/* 0x06: Speed_3 */

	/* speed:acceleration/deceleration: 0x36+[6] */
	uint16 MinimumSpeed;									/* 0x00: Minimum speed */
	uint16 AccelerationConst;								/* 0x02: Acceleration-constant */
	uint16 AccelerationPoints	: 6;						/* 0x04 [5:0]: Acceleration-points */
	uint16 TachoMode			: 2;						/* 0x04 [7:6]: Tacho-mode */
	uint16 DecelerationSteps	: 8;						/* 0x05: Deceleration-(u)Steps (MMP130819-1) */

	/* Application diagnostic levels: 0x3C+[6] */
	uint16 ApplUnderTemperature		: 8;					/* 0x00: Application under-temperature */
	uint16 ApplOverTemperatureWarn	: 8;					/* 0x01: Application over-temperature warning */
	uint16 ApplOverTemperatureShut  : 8;					/* 0x02: Application over-temperature shut */
	uint16 ApplUnderVoltage			: 8;					/* 0x03: Application under-voltage */
	uint16 ApplOverVoltage			: 8;					/* 0x04: Application over-voltage */
	uint16 ApplReserved41			: 8; 					/* 0x05: Reserved */
	
	/* Current vs. Temperature compensation:0x42+[10] */
	uint16 CurrThrshldTemp1		: 8;						/* 0x00: Current threshold temperature #1 */
	uint16 CurrThrshldRatio1	: 8;						/* 0x01: Current threshold ratio #1 */
	uint16 CurrThrshldTemp2		: 8;						/* 0x02: Current threshold temperature #2 */
	uint16 CurrThrshldRatio2	: 8;						/* 0x03: Current threshold ratio #2 */
	uint16 CurrThrshldTemp3		: 8;						/* 0x04: Current threshold temperature #3 */
	uint16 CurrThrshldRatio3	: 8;						/* 0x05: Current threshold ratio #3 */
	uint16 CurrThrshldTemp4		: 8;						/* 0x06: Current threshold temperature #4 */
	uint16 CurrThrshldRatio4	: 8;						/* 0x07: Current threshold ratio #4 */
	uint16 CurrThrshldZone1		: 1;						/* 0x08.[0]: Zone I */ 
	uint16 CurrThrshldZone2		: 2;						/* 0x08.[2:1]: Zone II */ 
	uint16 CurrThrshldZone3		: 2;						/* 0x08.[4:3]: Zone III */ 
	uint16 CurrThrshldZone4		: 2;						/* 0x08.[6:5]: Zone IV */ 
	uint16 CurrThrshldZone5		: 1;						/* 0x08.[7]: Zone V */ 
	uint16 Reserved4B			: 8;						/* 0x09: Reserved */
	
	/* PID Control: 0x4C+[12] */
	uint16 PidRunningCtrlPeriod	: 8;						/* 0x00: PID running control-period */
	uint16 PidHoldingCtrlPeriod	: 8;						/* 0x01: PID holding control-period */
	uint16 PidCoefP				: 8;						/* 0x02: PID-Coefficient P */
	uint16 PidCoefI				: 8;						/* 0x03: PID-Coefficient I */
	uint16 PidCoefD				: 8;						/* 0x04: PID-Coefficient D */
	uint16 PidThrshldCtrlPer	: 8;						/* 0x05: PID-period for threshold control */
	uint16 PidLowerHoldingLimit	: 8;						/* 0x06: PID Lower-limit Holding (output) */
	uint16 PidLowerLimit		: 8;						/* 0x07: PID Lower-limit Running (output) */
	uint16 PidUpperLimit		: 8;						/* 0x08: PID Upper-limit (output) */
	uint16 PidRampDown			: 8;						/* 0x09: PID Ramp-down limitation */
	uint16 PidRampUp			: 8;						/* 0x0A: PID Ramp-up limitation */
	uint16 Reserved57			: 8;						/* 0x0B: reserved */
}MOTOR_CALIBPARAMS;

#if (MOTOR_TYPE == MT_SANHAU_EXVALVE)
/* ****************************************************************************	*
 * Sanhua 4-phase "Valve" Motor (240 FS between end-stops)
 * Pole-pairs: 10
 * Motor coil resistance: 23R, Motor coil inductance: 30mH (production)
 * Motor coil resistance: 22R, Motor coil inductance: 30mH (new)
 * Max. Current: Icoil = 150mA
 * BEMF: ?mVpp/RPS-m, ?mVrms/RPS-m (?mVpp/eHz, ?mVrms/eHz): ?Vrms, ?Vpp
 * Target Speed: ?RPM-m, ?eHz
 *
 * Motor PWM Frequency is based on:
 *	maximum speed of 5.00RPM (external) or ?? RPM (internal), 6 pole-pairs and 32 micro-steps.
 *  The Motor-PWM frequency should above 18kHz (above human-audio)
 * ****************************************************************************	*/
#define CONFIGURATION_ID			((('U'-'@')<<10)|(('C'-'@')<<5)|('G'-'@'))	/* UniROM Basic 'G' (Sanhua 4-Phase Valve) */
#define MOTOR_FAMILY				MF_STEPPER
#define MOTOR_POLE_PAIRS			10
#define MOTOR_PHASES				4
#define MOTOR_STEP_PER_PHASE		1
#define MOTOR_MICROSTEPS			3											/* 8 (1 << 3) steps per phase-step */
#define C_TACHO_MODE				0											/* 0: No tacho, 1: 60-deg commut, 2: 180-deg commut, 3: 180-deg mech-rotattion */
#define C_MOTOR_DIR_INV				1

#define C_DEF_TRAVEL				4125										/* Full-stroke */
#define C_DEF_TRAVEL_TOLERANCE_HI	48											/* Deviation: 180 degrees of electric rotation */
#define C_DEF_TRAVEL_TOLERANCE_LO	48

#define MOTOR_GEAR_BOX_RATIO		1											/* Gear-box ratio: 1:5	0 */

#define C_MOVAVG_SSZ				6											/* Four commutation periods of 16 micro-steps */

#define C_DETECTOR_DELAY			128											/* Number of steps before stall-detection becomes active */

#define C_STALLDETECTOR_MODE		C_STALL_FOUND_A								/* Stall detection mode to turn-off motor and report stall */
#define C_STALL_A_THRESHOLD			(((14 * 128) + 50)/100)						/* Stall "A" threshold: 14% */
#define C_STALL_O_THRESHOLD			((uint8) ((6 * 256)/100))					/* Stall "O" threshold: 6% */
#define C_STALL_O_WIDTH				(8u - 1u)										/* Stall "O" width */
#define C_STALL_B_THRESHOLD			20											/* Stall "B" threshold:[10mV/LSB] */
#define C_STALL_B_DELAY				0											/* Stall "B" delay: */
#define C_STALL_SPEED_DEPENDED		0											/* 0 = Fixed stall-threshold; 1 = Speed depended threshold */
#define C_RESTALL_POR				0											/* Restall after POR: disabled */
#define C_MOTOR_CONST_10MV_PER_RPS	145											/* 1.45 Vpp/RPS-m */
#define C_COIL_R					22											/* 22R */
#define C_COILS_RTOT				C_COIL_R									/* BiPolar */
#define C_PID_HOLDING_CURR_LEVEL	60											/* Motor holding current threshold [mA] */
#define C_PID_RUNNING_CURR_LEVEL	300											/* Motor running current threshold [mA] */
#define C_PID_BOOST_CURR_LEVEL		0											/* Motor running current threshold in torque-boost mode [mA] */
#define SUPPORT_MOTOR_POSITION		FALSE										/* No motor-rotor position readback */
#define C_REWIND_STEPS				0											/* Rewinding steps with/out holding current */
/* ***
 * Speed
 * ***/
#define C_SPEED_MIN					  60
#define C_SPEED_0					  90										/* Motor RPM's (2.500 RPS = 15 eHz;  60 FS/s) */
#define C_SPEED_1					 120										/* Motor RPM's (3.333 RPS = 20 eHz;  80 FS/s) */
#define C_SPEED_2					 150										/* Motor RPM's (4.167 RPS = 25 eHz; 100 FS/s) */
#define C_SPEED_3					 180										/* Motor RPM's (5.000 RPS = 30 eHz; 120 FS/s) */
#define C_SPEED_TORQUE_BOOST		  60										/* Motor RPM's (1.667 RPS = 10 eHz;  40 FS/s) */
#define C_ACCELERATION_CONST		1500										/* Max: 575 RPM */
#define C_ACCELERATION_POINTS		(2 - 1)
#define C_DECELERATION_STEPS		1
/*
 *	4-line/5-zone Current threshold vs. temperature curve.
 *
 *				|Zone1|   Zone2   |   Zone3   |   Zone4   | Zone5
 *	RATIO_1	----+-----*
 *	(1.35)		|		\
 *	RATIO_4	----+		  \								  *-------
 *	(1.20)		|			\						   /
 *				|			  \					    /
 *				|				\				 /
 *	RATIO_2/3 --+				  *-----------*
 *	(1.00)		|
 *				+-----+-----------+-----------+-----------+---------+-------
 *					  |			  |			  |			  |			|
 *					TEMP1		TEMP2		TEMP3		TEMP4	C_APPL_OVERTEMPERATURE
 *					(-34)		( +8)		(+38)		(+62)	  (+80)
 */
#define C_CURRTHRSHLD_TEMP_1		-34
#define C_CURRTHRSHLD_TEMP_2		 +8
#define C_CURRTHRSHLD_TEMP_3		+38
#define C_CURRTHRSHLD_TEMP_4		+62
#define C_CURRTHRSHLD_TEMP_HYS		2											/* Hysteric arround points */
#define C_CURRTHRSHLD_RATIO_1		((uint16) (1.02*128))						/* +2 */
#define C_CURRTHRSHLD_RATIO_2		((uint16) (1.00*128))						/* 0% */
#define C_CURRTHRSHLD_RATIO_3		((uint16) (1.00*128))						/* 0% */
#define C_CURRTHRSHLD_RATIO_4		((uint16) (1.12*128))						/* +12% */
#define C_CURRTHRSHLD_AREA_1		1											/* As _1 */
#define C_CURRTHRSHLD_AREA_2		3											/* Interpolate between _1 and _2 */
#define C_CURRTHRSHLD_AREA_3		1											/* As _2 */
#define C_CURRTHRSHLD_AREA_4		3											/* Interpolate between _3 and _4 */
#define C_CURRTHRSHLD_AREA_5		1											/* As _4 */
/* ***
 * Application diagnostic levels
 * ***/
#define C_APPL_UTEMP				-40											/* GM: 3.1 */
#define C_APPL_OTEMP_WARN			125											/* GM: 3.1:over temperature warning */
#define C_APPL_OTEMP_SHUT			140											/* GM: 3.1:over temperature shutdown */
#define C_APPL_UVOLT				((uint16)(8.0 * 8))							/* GM: 6.6 */
#define C_APPL_OVOLT				((uint16)(18.0 * 8))						/* GM: 6.6 */
/* ***
 * PID Control
 * ***/
#define C_PID_RUNNINGCTRL_PERIOD	10											/* Every 10ms (= 20 x 500us); Range: 0.5..125 ms */
#define C_PID_HOLDINGCTRL_PERIOD	25											/* Every 50ms (= (25<<2) x 500us); Range: 2..500 ms */
#define C_PID_COEF_P				50											/* COEF_P = 50/256 */
#define C_PID_COEF_I				30											/* COEF_I = 30/256 */
#define C_PID_COEF_D				30											/* COEF_D = 30/256 */
#define C_LOADDUMP_VOLT				100											/* 1.00 V change (approx. 7.5-8%) */
#define C_VSUP_REF					((uint16)(13.5 * 8))						/* Reference voltage for PWM Duty-cycle control */
#define C_PID_THRSHLDCTRL_PERIOD	16											/* Every 1s (= (16<<7) x 500us); Range: 64ms..16s */
#define C_MIN_HOLDCORR_RATIO		((uint8)(((10 * 256) + 50)/100))			/* HOLDING: 10% = 26/256 */
#define C_MIN_CORR_RATIO			((uint8)(((10 * 256) + 50)/100))			/* BiPolar: 10% = 26/256 */
#define C_MAX_CORR_RATIO			((uint8)(((98.4 * 256)/100) - 1))			/* BiPolar: 98.4% = 230/256 */
#define C_MAX_CORR_RATIO_SP_BOOST	((uint8)((0 * 256)/100))					/* BiPolar: 0% = 0/256 */
#define C_MAX_CORR_RATIO_TQ_BOOST	((uint8)((0 * 256)/100))					/* BiPolar: 0% = 0/256 */
#define C_PID_RAMP_UP				0
#define C_PID_RAMP_DOWN				0


#define C_VDS_THRESHOLD				((uint16)(3.0 * 8.0))						/* Vds = 3.0V */
#define C_STALL_O					1											/* Stall "O": Enabled */
#define C_STALL_O_OFFSET			1											/* Stall "O" Offset: 16 */

#define C_LEAD_ANGLE				0											/* 0 degrees (= N * 1.875 degrees) */
#endif /* (MOTOR_TYPE == MT_SANHAU_EXVALVE) */

#if (MOTOR_TYPE == MT_SANHAU_WVALVE)
/* ****************************************************************************	*
 * Sanhua 4-phase "Valve" Motor (? FS between end-stops)
 * Pole-pairs: ?
 * Motor coil resistance: 18R, Motor coil inductance: 10mH
 * Max. Current: Icoil = ?mA
 * BEMF: ?mVpp/RPS-m, ?mVrms/RPS-m (?mVpp/eHz, ?mVrms/eHz): ?Vrms, ?Vpp
 * Target Speed: ?RPM-m, ?eHz
 *
 * Motor PWM Frequency is based on:
 *	maximum speed of 3.93RPM (external) or 1250 RPM (internal), 6 pole-pairs and 32 micro-steps.
 *  The Motor-PWM frequency should above 18kHz (above human-audio)
 * ****************************************************************************	*/
#define CONFIGURATION_ID			(((uint16)('U'-'@')<<10u)|((uint16)('C'-'@')<<5u)|(uint16)('P'-'@'))	/* UniROM Basic 'P' (Sanhua 4-Phase Water-Valve) */
#define MOTOR_FAMILY				MF_STEPPER
#define MOTOR_POLE_PAIRS			6u
#define MOTOR_PHASES				4u
#define C_MOTOR_CONST_10MV_PER_RPS	30u											/* 0.46 Vpp/RPS-m */
#define C_COIL_R					17u											/* 18R */
#define C_COILS_RTOT				C_COIL_R									/* BiPolar */

#define MOTOR_STEP_PER_PHASE		1u
#define MOTOR_MICROSTEPS			3u											/* 8 (1 << 3) steps per phase-step */
#define C_TACHO_MODE				0u											/* 0: No tacho, 1: 60-deg commut, 2: 180-deg commut, 3: 180-deg mech-rotattion */
#define MOTOR_DIR_INV				0u											/* Motor rotational direction inverse: 0=CW, 1=CCW */

#define C_DEF_TRAVEL				3100u										/* Full-stroke:Geely 3240;GM:3100 */
#define C_DEF_TRAVEL_TOLERANCE_HI	200u										/* Deviation: 180 degrees of electric rotation */
#define C_DEF_TRAVEL_TOLERANCE_LO	200u
#define C_DEF_EMRUN_POS				0x00u

#define MOTOR_GEAR_BOX_RATIO		318u										/* Gear-box ratio: 1:5	0 */

#define C_MOVAVG_SSZ				5u											/* Four commutation periods of 8 micro-steps:2^5=32micro steps */

#define C_DETECTOR_DELAY			128u										/* Number of steps before stall-detection becomes active */
#define C_STALLDETECTOR_MODE		C_STALL_FOUND_A								/* Stall detection mode to turn-off motor and report stall */
#define C_STALL_A_THRESHOLD			(((16u * 128u) + 50u)/100u)					/* Stall "A" threshold: 16.0% */
#define C_STALL_O_THRESHOLD			((uint8) ((31.25 * 256.0)/100.0))				/* Stall "O" threshold: 31.25% */
#define C_STALL_O_WIDTH				(8u - 1u)									/* Stall "O" width */
#define C_STALL_B_THRESHOLD			20u											/* Stall "B" threshold:[10mV/LSB] */
#define C_STALL_B_DELAY				0u											/* Stall "B" delay: */
#define C_STALL_SPEED_DEPENDED		0u											/* 0 = Fixed stall-threshold; 1 = Speed depended threshold */
#define C_PID_HOLDING_CURR_LEVEL	100u										/* Motor holding current threshold [mA] */
#define C_PID_RUNNING_CURR_LEVEL	400u										/* Motor running current threshold [mA] */
#define C_PID_BOOST_CURR_LEVEL		0u											/* Motor running current threshold in torque-boost mode [mA] */

/* ***
 * Speed
 * ***/
#define C_SPEED_MIN					 125u										/* Motor RPM min:50FPS */
#define C_SPEED_0					 388u										/* Motor RPM's boost100%,155FPS */
//#define C_SPEED_1					 950u										/* Motor RPM's,380FPS GM */
#define C_SPEED_1					 950u										/* Motor RPM's,380FPS NEXT */
#define C_SPEED_2					1250u										/* Motor RPM's */
#define C_SPEED_3					1250u										/* Motor RPM's */
#define C_SPEED_TORQUE_BOOST		1250u										/* Motor RPM's */
#define C_ACCELERATION_CONST		3250u										/* Max: 3000 RPM */
#define C_ACCELERATION_POINTS		(32u - 1u)
#define C_DECELERATION_STEPS		1u
/*
 *	4-line/5-zone Current threshold vs. temperature curve.
 *
 *				|Zone1|   Zone2   |   Zone3   |   Zone4   | Zone5
 *	RATIO_1	----+-----*
 *	(1.35)		|		\
 *	RATIO_4	----+		  \								  *-------
 *	(1.20)		|			\						   /
 *				|			  \					    /
 *				|				\				 /
 *	RATIO_2/3 --+				  *-----------*
 *	(1.00)		|
 *				+-----+-----------+-----------+-----------+---------+-------
 *					  |			  |			  |			  |			|
 *					TEMP1		TEMP2		TEMP3		TEMP4	C_APPL_OVERTEMPERATURE
 *					(-34)		( +8)		(+38)		(+62)	  (+80)
 */
#define C_CURRTHRSHLD_TEMP_1		-34
#define C_CURRTHRSHLD_TEMP_2		 +8
#define C_CURRTHRSHLD_TEMP_3		+38
#define C_CURRTHRSHLD_TEMP_4		+62
#define C_CURRTHRSHLD_TEMP_HYS		2											/* Hysteric arround points */
#define C_CURRTHRSHLD_RATIO_1		((uint16) (1.00*128.0))						/* +2 */
#define C_CURRTHRSHLD_RATIO_2		((uint16) (1.00*128.0))						/* 0% */
#define C_CURRTHRSHLD_RATIO_3		((uint16) (1.00*128.0))						/* 0% */
#define C_CURRTHRSHLD_RATIO_4		((uint16) (1.00*128.0))						/* +12% */
#define C_CURRTHRSHLD_AREA_1		1u											/* As _1 */
#define C_CURRTHRSHLD_AREA_2		3u											/* Interpolate between _1 and _2 */
#define C_CURRTHRSHLD_AREA_3		1u											/* As _2 */
#define C_CURRTHRSHLD_AREA_4		3u											/* Interpolate between _3 and _4 */
#define C_CURRTHRSHLD_AREA_5		1u											/* As _4 */
/* ***
 * Application diagnostic levels
 * ***/
#define C_APPL_UTEMP				-40											/* GM: 3.1 */
#define C_APPL_OTEMP_WARN			130											/* GM: 3.1:over temperature warning */
#define C_APPL_OTEMP_SHUT			140											/* GM: 3.1:over temperature shutdown */
#define C_APPL_UVOLT				((int16)(9.0 * 8.0))						/* GM: 6.6 */
#define C_APPL_OVOLT				((int16)(19.0 * 8.0))						/* GM: 6.6 */
/* ***
 * PID Control
 * ***/
#define C_PID_RUNNINGCTRL_PERIOD	10u											/* Every 5ms (= 20 x 500us); Range: 0.5..125 ms */
#define C_PID_HOLDINGCTRL_PERIOD	25u											/* Every 50ms (= (25<<2) x 500us); Range: 2..500 ms */
#define C_PID_COEF_P				50u											/* COEF_P = 50/256 */
#define C_PID_COEF_I				30u											/* COEF_I = 30/256 */
#define C_PID_COEF_D				30u											/* COEF_D = 30/256 */
#define C_LOADDUMP_VOLT				100u										/* 1.00 V change (approx. 7.5-8%) */
#define C_VSUP_REF					((uint16)(13.5 * 8.0))						/* Reference voltage for PWM Duty-cycle control */
#define C_PID_THRSHLDCTRL_PERIOD	16u											/* Every 1s (= (16<<7) x 500us); Range: 64ms..16s */
#define C_MIN_HOLDCORR_RATIO		((uint8)(((10u * 256u) + 50u)/100u))		/* HOLDING: 10% = 26/256 */
#define C_MIN_CORR_RATIO			((uint8)(((10u * 256u) + 50u)/100u))		/* BiPolar: 10% = 26/256 */
#define C_MAX_CORR_RATIO			((uint8)(((98.4 * 256.0)/100.0) - 1.0))	/* BiPolar: 98.4% = 230/256 */
#define C_MAX_CORR_RATIO_SP_BOOST	((uint8)((0 * 256)/100))					/* BiPolar: 0% = 0/256 */
#define C_MAX_CORR_RATIO_TQ_BOOST	((uint8)((0 * 256)/100))					/* BiPolar: 0% = 0/256 */
#define C_VDS_THRESHOLD				((uint16)(3.0 * 8.0))						/* Vds = 3.0V */
#define C_STALL_O					0u											/* Stall "O": Enabled */
#define C_STALL_O_OFFSET			1u											/* Stall "O" Offset: 16 */

#define C_LEAD_ANGLE				0u											/* 0 degrees (= N * 1.875 degrees) */
#endif /* (MOTOR_TYPE == MT_SANHAU_WVALVE) */

/* ****************************************************************************	*
 * Motor parameters
 * ****************************************************************************	*/
//#define SZ_MICRO_VECTOR_TABLE_4PH	80		/* 2*2*16uStep * (1 + 1/4) */
#define SZ_MICRO_VECTOR_TABLE_4PH	40		/* 2*2*8uStep*(1 + 1/4) */

#pragma space nodp
extern MOTOR_CALIBPARAMS MotorParams;

#pragma space none

#endif /* MOTOR_PARAMS_H_ */

/* EOF */
