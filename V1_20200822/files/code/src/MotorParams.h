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

#ifndef SYSLIB_H_
#include "syslib.h"
#endif /* SYSLIB_H_ */

#include "Build.h"

#define C_BROWN_OUT_LEVEL			1											/* 0 = 6V, 1 = 7V, 2 = 8V & 3 = 9V +/- 0.5V */
#define C_HALL_LATCHES				0											/* No Hall-Latches */
#define C_BUSTIMEOUT_SLEEP			0
#define C_AUTO_RECALIBRATE			1											/* Enable Auto re-calibration */
#define C_LIN_UV					2											/* LIN UV: 7.0V (6.0V + n * 0.V) (MMP131216-1) */

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
#define MOTOR_FAMILY				MF_STEPPER
#define CONFIGURATION_ID			((('U'-'@')<<10)|(('C'-'@')<<5)|('G'-'@'))	/* UniROM Basic 'G' (Sanhua 4-Phase Valve) */
#define MOTOR_POLE_PAIRS			12
#define MOTOR_PHASES				4
#define MOTOR_STEP_PER_PHASE		1
#define MOTOR_MICROSTEPS			3											/* 8 (1 << 3) steps per phase-step */
#define MOTOR_GEAR_BOX_RATIO		1											/* Gear-box ratio: 1:5	0 */
#define C_TACHO_MODE				0											/* 0: No tacho, 1: 60-deg commut, 2: 180-deg commut, 3: 180-deg mech-rotattion */
#define C_DEF_TRAVEL				312											/* Full-stroke */
#define C_DEF_TRAVEL_TOLERANCE		10											/* Deviation: 180 degrees of electric rotation */
#define C_ENDSTOP_PAUSETIME			50
#define C_PERC_OFFSET				((( 5 * C_DEF_TRAVEL) + 50)/100)			/* 5% */
#define C_HALFPERC_OFFSET			((( 1 * C_DEF_TRAVEL) + 50)/200)			/* 0.5% */

#if _SUPPORT_DOUBLE_USTEP
#define C_MOVAVG_SSZ				6											/* Four commutation periods of 16 micro-steps */
#else  /* _SUPPORT_DOUBLE_USTEP */
#define C_MOVAVG_SSZ				5											/* Four commutation periods of 8 micro-steps */
#endif /* _SUPPORT_DOUBLE_USTEP */
#define C_DETECTOR_DELAY			128											/* Number of steps before stall-detection becomes active */

#define C_STALLDETECTOR_MODE		C_STALL_FOUND_A								/* Stall detection mode to turn-off motor and report stall */
#define C_STALL_A_THRESHOLD			(((14 * 128) + 50)/100)						/* Stall "A" threshold: 14% */
#define C_STALL_O_THRESHOLD			((uint8) ((6 * 256)/100))					/* Stall "O" threshold: 6% */
#define C_STALL_O_WIDTH				(8 - 1)										/* Stall "O" width */
#define C_STALL_SPEED_DEPENDED		0											/* 0 = Fixed stall-threshold; 1 = Speed depended threshold */
#define C_RESTALL_POR				0											/* Restall after POR: disabled */
#define C_MOTOR_CONST_10MV_PER_RPS	145											/* 1.45 Vpp/RPS-m */
#define C_COIL_R					23											/* 22R */
#define C_COILS_RTOT				C_COIL_R									/* BiPolar */
#define C_PID_HOLDING_CURR_LEVEL	60											/* Motor holding current threshold [mA] */
#define C_PID_RUNNING_CURR_LEVEL	300											/* Motor running current threshold [mA] */
#define C_PID_BOOST_CURR_LEVEL		0											/* Motor running current threshold in torque-boost mode [mA] */
#define SUPPORT_MOTOR_POSITION		FALSE										/* No motor-rotor position readback */
#define C_REWIND_STEPS				0											/* Rewinding steps with/out holding current */
/* ***
 * Speed
 * ***/
#define C_SPEED_MIN					  75
#define C_SPEED_0					  75										/* Motor RPM's 60/48*60 = 75RPM */
#define C_SPEED_1					  75										/* Motor RPM's 60/48*60 = 75RPM */
#define C_SPEED_2					  75										/* Motor RPM's 60/48*60 = 75RPM */
#define C_SPEED_3					  75										/* Motor RPM's 60/48*60 = 75RPM */
#define C_SPEED_TORQUE_BOOST		  75										/* Motor RPM's 60/48*60 = 75RPM */
#define C_ACCELERATION_CONST		1500										/* Max: 575 RPM */
#define C_ACCELERATION_POINTS		(2 - 1)
#define C_DECELERATION_STEPS		1
#if _SUPPORT_SPEED_AUTO
#define C_AUTOSPEED_TEMP_1			-40
#define C_AUTOSPEED_TEMP_2			-15
#define C_AUTOSPEED_TEMP_3			+60
#define C_AUTOSPEED_TEMPZONE_1		0
#define C_AUTOSPEED_TEMPZONE_2		1
#define C_AUTOSPEED_TEMPZONE_3		3
#define C_AUTOSPEED_TEMPZONE_4		1
#define C_AUTOSPEED_VOLT_1			((uint16)(10.5 * 8))
#define C_AUTOSPEED_VOLT_2			((uint16)(11.75 * 8))
#define C_AUTOSPEED_VOLT_3			0xFF
#define C_AUTOSPEED_VOLTZONE_1		0
#define C_AUTOSPEED_VOLTZONE_2		1
#define C_AUTOSPEED_VOLTZONE_3		3
#define C_AUTOSPEED_VOLTZONE_4		0
#endif /* _SUPPORT_SPEED_AUTO */
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
#define C_APPL_OTEMP				105											/* GM: 3.1 */
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
#define C_VDS_THRESHOLD				((uint16)(3.0 * 8))							/* Vds = 3.0V */
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
#define MOTOR_FAMILY				MF_STEPPER
#define CONFIGURATION_ID			((('U'-'@')<<10)|(('C'-'@')<<5)|('P'-'@'))	/* UniROM Basic 'P' (Sanhua 4-Phase Water-Valve) */
#define MOTOR_POLE_PAIRS			6
#define MOTOR_PHASES				4
#define MOTOR_STEP_PER_PHASE		1
#define MOTOR_MICROSTEPS			3											/* 8 (1 << 3) steps per phase-step */
#define MOTOR_GEAR_BOX_RATIO		318											/* Gear-box ratio: 1:5	0 */
#define C_TACHO_MODE				0											/* 0: No tacho, 1: 60-deg commut, 2: 180-deg commut, 3: 180-deg mech-rotattion */
#define C_DEF_TRAVEL				2950										/* Full-stroke */
#define C_DEF_TRAVEL_TOLERANCE		25											/* Deviation: 180 degrees of electric rotation */
#define C_ENDSTOP_PAUSETIME			50
#define C_PERC_OFFSET				(uint16)((( 5UL * C_DEF_TRAVEL) + 50)/100)	/* 5% */
#define C_HALFPERC_OFFSET			(uint16)((( 1UL * C_DEF_TRAVEL) + 50)/200)	/* 0.5% */

#if _SUPPORT_DOUBLE_USTEP
#define C_MOVAVG_SSZ				6											/* Four commutation periods of 16 micro-steps */
#else  /* _SUPPORT_DOUBLE_USTEP */
#define C_MOVAVG_SSZ				5											/* Four commutation periods of 8 micro-steps */
#endif /* _SUPPORT_DOUBLE_USTEP */
#define C_DETECTOR_DELAY			128											/* Number of steps before stall-detection becomes active */

#define C_STALLDETECTOR_MODE		C_STALL_FOUND_A								/* Stall detection mode to turn-off motor and report stall */
#define C_STALL_A_THRESHOLD			(((16 * 128) + 50)/100)						/* Stall "A" threshold: 16.0% */
#define C_STALL_O_THRESHOLD			((uint8) ((31.25 * 256)/100))				/* Stall "O" threshold: 31.25% */
#define C_STALL_O_WIDTH				(8 - 1)										/* Stall "O" width */
#define C_STALL_SPEED_DEPENDED		0											/* 0 = Fixed stall-threshold; 1 = Speed depended threshold */
#define C_RESTALL_POR				0											/* Restall after POR: disabled */
#define C_MOTOR_CONST_10MV_PER_RPS	2											/* 0.46 Vpp/RPS-m */
#define C_COIL_R					7											/* 18R */
#define C_COILS_RTOT				C_COIL_R									/* BiPolar */
#define C_PID_HOLDING_CURR_LEVEL	100											/* Motor holding current threshold [mA] */
#define C_PID_RUNNING_CURR_LEVEL	300											/* Motor running current threshold [mA] */
#define C_PID_BOOST_CURR_LEVEL		0											/* Motor running current threshold in torque-boost mode [mA] */
#define SUPPORT_MOTOR_POSITION		FALSE										/* No motor-rotor position readback */
#define C_REWIND_STEPS				0											/* Rewinding steps with/out holding current */
/* ***
 * Speed
 * ***/
#define C_SPEED_MIN					 125
#define C_SPEED_0					1250										/* Motor RPM's */
#define C_SPEED_1					1250										/* Motor RPM's */
#define C_SPEED_2					1250										/* Motor RPM's */
#define C_SPEED_3					1250										/* Motor RPM's */
#define C_SPEED_TORQUE_BOOST		1250										/* Motor RPM's */
#define C_ACCELERATION_CONST		7500										/* Max: 3000 RPM */
#define C_ACCELERATION_POINTS		(64 - 1)
#define C_DECELERATION_STEPS		1
#if _SUPPORT_SPEED_AUTO
#define C_AUTOSPEED_TEMP_1			-40
#define C_AUTOSPEED_TEMP_2			-15
#define C_AUTOSPEED_TEMP_3			+60
#define C_AUTOSPEED_TEMPZONE_1		0
#define C_AUTOSPEED_TEMPZONE_2		1
#define C_AUTOSPEED_TEMPZONE_3		3
#define C_AUTOSPEED_TEMPZONE_4		1
#define C_AUTOSPEED_VOLT_1			((uint16)(10.5 * 8))
#define C_AUTOSPEED_VOLT_2			((uint16)(11.75 * 8))
#define C_AUTOSPEED_VOLT_3			0xFF
#define C_AUTOSPEED_VOLTZONE_1		0
#define C_AUTOSPEED_VOLTZONE_2		1
#define C_AUTOSPEED_VOLTZONE_3		3
#define C_AUTOSPEED_VOLTZONE_4		0
#endif /* _SUPPORT_SPEED_AUTO */
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
#define C_APPL_OTEMP				140											/* GM: 3.1 */
#define C_APPL_UVOLT				((uint16)(9.0 * 8))							/* GM: 6.6 */
#define C_APPL_OVOLT				((uint16)(19.0 * 8))						/* GM: 6.6 */
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
#define C_VDS_THRESHOLD				((uint16)(3.0 * 8))							/* Vds = 3.0V */
#define C_STALL_O					1											/* Stall "O": Enabled */
#define C_STALL_O_OFFSET			1											/* Stall "O" Offset: 16 */

#define C_LEAD_ANGLE				0											/* 0 degrees (= N * 1.875 degrees) */
#endif /* (MOTOR_TYPE == MT_SANHAU_WVALVE) */

/* ****************************************************************************	*
 * Motor parameters
 * ****************************************************************************	*/
#if _SUPPORT_DOUBLE_USTEP
#define SZ_MICRO_VECTOR_TABLE_4PH	80		/* 2*2 * 16uStep * (1 + 1/4) */
#else  /* _SUPPORT_DOUBLE_USTEP */
#define SZ_MICRO_VECTOR_TABLE_4PH	40		/* 2*2 * 8uStep * (1 + 1/4) */
#endif /* _SUPPORT_DOUBLE_USTEP */

#endif /* MOTOR_PARAMS_H_ */

/* EOF */
