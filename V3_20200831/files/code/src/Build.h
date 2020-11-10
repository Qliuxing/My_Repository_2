/*! ----------------------------------------------------------------------------
 * \file		Build.h
 * \brief		MLX81310 Project building file
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
 *
 * MELEXIS Microelectronic Integrated Systems
 * 
 * Copyright (C) 2012-2015 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * Note: To configure the correct setup/motor combination, set below market sections 1 through 5 correctly;
 *	Section #1:	(LIN) Communication protocol
 *	Section #2: Usage of NVRAM or Code Constants
 *	Section #3: Motor type
 *	Section #4: Application type
 *	Section #5: Special parameters
 *	Section #6: Debug support
 * DO NOT FORGET THE "lin2b_romtbl.S" !!!
 *
 * ****************************************************************************	*/

#ifndef _BUILD_
#define _BUILD_

#include <syslib.h>

/* Define some hardware settings */

/************************ Type of compiler ************************/
/* User defines */
#define COMPILER	GCC			/* select the compiler */

/* Automatic defines*/
#define GCC		1				/* the 2 compilers we can select*/
#define COSMIC 	2

/* Software build version */
#define C_SW_BUILD					2											/* Build version identifier */

/* Variable storage definition*/
#if COMPILER == GCC
#define __TINY_SECTION__	 		space dp
#define __NEAR_SECTION__	 		space ep
#define __BUILTIN__			 		__inline__
#define INTERRUPT(nameisr)			void __attribute__((interrupt)) (nameisr)(void) 
 
#else
#define __TINY_SECTION__	 		space extern [] @tiny
#define __NEAR_SECTION__	 		space extern [] @near
#define __BUILTIN__			 		@builtin
#define INTERRUPT(nameisr) 			@interrupt void nameisr( void ) 
#endif

#define FALSE						0u
#define TRUE						1u											/* Should be not-FALSE */

#define DISABLED					FALSE
#define ENABLED						TRUE

#define WATCHDOG					ENABLED

#define C_TEMPOFF					60											/* Temperature range: -60 to +195 (-C_TEMPOFF to +(255-C_TEMPOFF) */

#define PLL_freq					(MCU_PLL_MULT * 250000UL)					/* PLL frequency RCOsc * 250kHz */

/* NOTE: Don't use below marco's with local stack-variables; The C-compiler doesn't "see" the psup/pop 
 * It's better to use ATOMIC_CODE() */
#define BEGIN_CRITICAL_SECTION()	__asm__("psup #0")	/*!< Set to priority 0 to block all interrupts */
#define END_CRITICAL_SECTION()		__asm__("pop M")	/*!< Restore priority */

/* *** Section #1: Communication *** */						
/* *** NOTE: rename the lib_lin to lib (LIN) or lib_pwm to lib (PWM) *** */
#define LIN_COMM					TRUE						/* <<<1<<< */	/* LIN-interface: FALSE: Disabled; TRUE: Enabled */

#define NOLIN						(0x00U << 8U)
#define LIN13						(0x13U << 8U)								/* LIN protocol according LIN 1.3 */
#define LIN20						(0x20U << 8U)								/* LIN protocol according LIN 2.0 */
#define LIN21						(0x21U << 8U)								/* LIN protocol according LIN 2.1 */
#define LIN22						(0x22U << 8U)								/* LIN protocol according LIN 2.2 */
#define LIN2J						(0x2AU << 8U)								/* LIN protocol according LIN 2.x/SAE-J2602 */
#define LIN2X						(0x2FU << 8U)								/* LIN protocol according LIN 2.x */
#define LIN2						(0x20U << 8U)								/* Any LIN 2.x protocol, including SAE-J2602 */
#define LINX						(0xF0U << 8U)								/* Any LIN x.x protocol, including SAE-J2602 */
#define LINXX						(0xFFU << 8U)

#define APP_ACT_VALVE_GM			0x21U										/* 23 = GM (Water)Valve V1.0 (07-APR-2016) */

#define LIN2J_VALVE_GM				(LIN2J | APP_ACT_VALVE_GM)					/* LIN protocol according SAE J2602/GM valve 1.0 */
#define LIN21_VALVE_NEXT			(LIN21 | APP_ACT_VALVE_GM)					/* LIN protocol according SAE J2602/GM valve 1.0 */


#if LIN_COMM
/* LIN communication protocol */
#define LINPROT						LIN21_VALVE_NEXT							/* Select LIN 2.0/SAE J2602 GM Valve V1.0 protocol */

#define _SUPPORT_DIG_LIN			FALSE										/* FALSE: Only LIN/PWM; TRUE: LIN as Low/High Digital pin */
#else  /* LIN_COMM */
#define LINPROT						NOLIN
#endif /* LIN_COMM */

#define PWM_COMM_NONE				0											/* No PWM-IN */
#define PWM_COMM_TC1				1											/* PWM via LV TC1   (MLX81300B) */
#define PWM_COMM_TC2				2											/* PWM via LV TC2   (MLX81300B) */
#define PWM_COMM_IO3				3											/* PWM via HV IO[3] (MLX81300D) */
#define PWM_COMM_LIN				8											/* PWM via HV LIN   (MLX81300D) */
#define PWM_COMM_IN 				PWM_COMM_NONE								/* No PWM-Input */	
#define PWM_COMM_OUT				PWM_COMM_NONE								/* No PWM-Output */

#define MCU_ASSP_MODE				TRUE

/* Motor family	*/
#define MF_TYPE						0xF0										/* Upper 4 bits is Family Type */
#define MF_SUBTYPE					0x0F										/* Lower 4 bits is Sub-type */
#define MF_DC						0x10										/* DC */
#define MF_DC_T0					0x10										/* DC, Pos: None */
#define MF_DC_T1					0x11										/* DC, Pos: 3-wire (Phase1, P, Phase2) */
#define	MF_DC_T2					0x12										/* DC, Pos: 3-wires (3V3, P, GND) */
#define MF_BLDC						0x20										/* BEMF, with startup in Stepper mode */
#define MF_STEPPER					0x30										/* Stepper */
#define MF_RELUCTANCE				0x40										/* Reluctance motor */

/* *** Section #2: Usage of NVRAM or Code Constants *** */
/* debug mode:NVRAM,release mode:const */
#define MP_CONST					0											/* Use fixed constants for UniROM code (Shortest code size) */
#define MP_NVRAM					1											/* Use NVRAM parameters for UniROM code (Most flexible) */
#define MOTOR_PARAMS				MP_CONST					/* <<<2<<< */
#define _SUPPORT_CODE_PARAMS		FALSE										/* FALSE: Use NVRAM (if valid); TRUE: Use MotorParams.h settings */

/* *** Section #3: Motor type *** */
/* 0xxx = MLX81300/MLX81310 
 * x12x = Sanhua HAVC
 */
#define MT_SANHAU_EXVALVE			0x0121										/* Sanhua 4-phase Bi-Polar Expansion Valve (22R) */
#define MT_SANHAU_WVALVE			0x0122										/* Sanhua 4-phase Bi-Polar Water-value */
/* Active motor-type */
#define MOTOR_TYPE					MT_SANHAU_WVALVE			/* <<<3<<< */

/* *** Section #5: Special *** */									/* <<<5<<< */
#define USE_MULTI_PURPOSE_BUFFER			TRUE								/* FALSE: Separate buffers; TRUE: Space saving (RAM) */

#define _SUPPORT_MLX_DEBUG_MODE				FALSE								/* Melexis Debug-mode */
#define _SUPPORT_WD_RST_RECOVERY			TRUE								/* Support Watchdog reset fast recovery */
#define _SUPPORT_LINCMD_WD_RST				FALSE								/* Support LIN-command to simulate Watchdog reset */
#define _SUPPORT_CRASH_RECOVERY				TRUE								/* Support crash recovery */
#define _SUPPORT_LINCMD_CRASH				FALSE								/* Support LIN-command to simulate application crash */
#define _SUPPORT_TESTMODE_OFF				TRUE								/* MLX81300DC support test-mode switching */
#define _SUPPORT_TWO_LINE_TEMP_INTERPOLATION	TRUE							/* FALSE: One line linear interpolation; TRUE: Two lines lineair interpolation */
#define _SUPPORT_AMBIENT_TEMP				FALSE								/* FALSE: Use chip temperature for compensation; TRUE: Use estimated ambient temperature for compensation */
#define _SUPPORT_LINNETWORK_LOADER			TRUE								/* FALSE: Flash-loading via point-to-point (0x7F); TRUE: Network Flash-loading support (NAD) */
#define _SUPPORT_MLX16_HALT					FALSE								/* FALSE: MLX16 doesn't HALT; TRUE: MLX16 enters HALT during holding mode (power-safe) */
#define _SUPPORT_VSFILTERED					FALSE								/* FALSE: Unfiltered Vs (ADC Channel 0); TRUE: Filtered Vs (ADC Channel 4) (MLX81310A) */
#define _SUPPORT_VSMFILTERED				TRUE								/* FALSE: Unfiltered Vsm (ADC Channel 14); TRUE: Filtered Vsm (ADC Channel 4) (MLX81310C) */
#define _SUPPORT_AUTO_BAUDRATE				TRUE								/* FALSE: Fixed baudrate; TRUE: Auto-detection of baudrate */
#define _SUPPORT_MOTOR_SELFTEST				FALSE								/* FALSE: No motor driver check at POR; TRUE: Motor driver check at POR */
#define _SUPPORT_LIN_UV						FALSE								/* FALSE: No LIN UV check; TRUE: LIN UV check (reset Bus-time-out) */

/* bipolar mode */
#define BIPOLAR_MODE_UV_WT					0									/* Coil between U & V, and second coil between W & T */
#define BIPOLAR_MODE_UT_VW					1									/* Coil between U & T, and second coil between V & W (P11847) */
#define BIPOLAR_MODE_UW_VT					2									/* Coil between U & W, and second coil between V & T (P11770) */
#define _SUPPORT_BIPOLAR_MODE				BIPOLAR_MODE_UW_VT
#define BIPOLAR_PWM_DOUBLE_MIRROR			0									/* Each coil double PWM mirror */
																				/* Phase 1 & 3: |	  +---------------+		|
																				 * 				|	  |				  |		|
																				 * 				|-----+				  +-----|
																				 * Phase 2 & 4:	|			+---+			|
																				 * 				|			|	|			|
																				 * 				|-----------+	+-----------|
																				 */
#define BIPOLAR_PWM_SINGLE_MIRROR_VSM		1									/* Each coil single PWM mirror, other HIGH */
																				/* Phase 1 & 3: |	  +---------------+		|
																				 * 				|	  |				  |		|
																				 * 				|-----+				  +-----|
																				 * Phase 2 & 4: |---------------------------|
																				 * 				|							|
																				 * 				|							|
																				 */
#define BIPOLAR_PWM_SINGLE_MIRROR_GND		2									/* Each coil single PWM mirror, other HIGH (MMP150312-1) */
																				/* Phase 1 & 3: |	  +---------------+		|
																				 * 				|	  |				  |		|
																				 * 				|-----+				  +-----|
																				 * Phase 2 & 4: |							|
																				 * 				|							|
																				 * 				|---------------------------|
																				 * 							  ^
																				 * ADC:						 Im
																				 * 							 50%
																				 * Note: VDS-Monitor is not possible!
																				 */
#define BIPOLAR_PWM_SINGLE_INDEPENDED_VSM	3									/* Each coil single PWM mirror/inverse mirror, other HIGH */
																				/* Phase 1	|	  +---------------+		|  Phase 3:	|----+				   +----|
																				 * 			|	  |				  |		|			|	 |				   |	|
																				 * 			|-----+				  +-----|			|	 +-----------------+	|
																				 * Phase 2: |---------------------------|  Phase 4:	|---------------------------|
																				 * 			|							|			|							|
																				 * 			|							|			|							|
																				 */
#define BIPOLAR_PWM_SINGLE_INDEPENDED_GND	4									/* Each coil single PWM mirror/inverse mirror, other HIGH (MMP150312-1) */
																				/* Phase 1	|	  +---------------+		|  Phase 3:	|----+				   +----|
																				 * 			|	  |				  |		|			|	 |				   |	|
																				 * 			|-----+				  +-----|			|	 +-----------------+	|
																				 * Phase 2: |							|			|							|
																				 * 			|							|			|							|
																				 * 			|---------------------------|  Phase 4:	|---------------------------|
																				 * 						  ^														^
																				 * ADC:					 Im														Im
																				 * 						 50%		   100%						 50%		   100%
																				 * Note: VDS-Monitor is not possible!
																				 */
#define BIPOLAR_PWM_SINGLE_MIRRORSPECIAL	5									/* Each coil single PWM mirror, other HIGH/LOW */
																				/* Phase 1	|	  +---------------+		|  Phase 3:	|	 +-----------------+	|
																				 * 			|	  |				  |		|			|	 |				   |	|
																				 * 			|-----+				  +-----|			|----+	 			   +----|
																				 * Phase 2: |---------------------------|  Phase 4:	|							|
																				 * 			|							|			|							|
																				 * 			|							|			|---------------------------|

																			 	*/
/* PWM mode */																				 	
#define _SUPPORT_PWM_MODE					BIPOLAR_PWM_SINGLE_INDEPENDED_GND
#define _SUPPORT_DOUBLE_MOTOR_CURRENT		TRUE								/* FALSE: Motor Current x 1; TRUE: Motor Current x 2 */
#define _SUPPORT_DOUBLE_USTEP				FALSE								/* FALSE: 32 or 48 micro-steps; TRUE: 64 or 96 micro-steps */
#define _SUPPORT_PWM_DC_RAMPUP				FALSE								/* FALSE: Constant mPWM-DC; TRUE: Increasing mPWM-DC at ramp-up */
#define _SUPPORT_PWM_DC_RAMPDOWN			TRUE								/* FALSE: Constant mPWM-DC; TRUE: Decrease mPWM-DC at ramp-down */
#define _SUPPORT_PWM_100PCT					TRUE								/* FALSE: max. PWM duty-cycle: 97.5%; TRUE: max. PWM duty-cycle: 100% */

/* *** Section #6: Debug *** */									/* <<<6<<< */
#define _DEBUG								FALSE								/* Set TRUE when one of below _DEBUG items is set TRUE */
#define _DEBUG_SPI							FALSE								/* FALSE: I/O-toggling; TRUE: SPI data */
#define DEBUG_SPI_BAUDRATE					1000000UL							/* SPI Baudrate: 1.0MBaud (100k-1MBaud) */
#define _DEBUG_COMMUT_ISR					FALSE

#define _DEBUG_FATAL						FALSE								/* Save MLX-state in case of Fatal-error occurs in RAM */
#define _DEBUG_MOTOR_CURRENT_FLT			FALSE								/* Motor current filter debug-buffers */
#define _DEBUG_VOLTAGE_COMPENSATION			FALSE								/* Motor voltage compensation */

#define _DEBUG_NVRAM_ERRORLOG				FALSE
/* *** Section #7: Coolant Valve  *** */	
/* debug mode:NVRAM,release mode:const */
#define VP_CONST                            0
#define VP_NVRAM                            1
#define VALVE_PARAMS                        VP_NVRAM
/* valve type */
#define VT_CV_GEELY_3Z                      0x0101
#define VT_CV_GM                            0x0202
#define VT_EXV_DAIMLER                      0x0303

#define VALVE_TYPE                          VT_CV_GM

/* diagnostic:motor */
#define _SUPPORT_COIL_RESISTANCE_CHECK		FALSE		
#define _SUPPORT_REWIND_STALL_FUNCTION		FALSE								/* FALSE: rewind stall disable; TRUE: rewind stall enable */
/* diagnostic:over current,over temperature */
#define _SUPPORT_DIAG_OC					TRUE								/* FALSE: Ignore OC; TRUE: Handle OC */
#define _SUPPORT_DIAG_OVT					TRUE								/* FALSE: Ignore OVT; TRUE: Handle OVT */
#define _SUPPORT_PHASE_SHORT_DET			FALSE								/* FALSE: No phase-short-to-GND detection support; TRUE: Phase-short-to-GND detection supported */
#define _SUPPORT_CHIP_TEMP_PROFILE			FALSE								/* FALSE: No chip temperature profile check; TRUE: Chip temperature profile check (dT/dt) */

/* diagnostic:bus activity check */
#define _SUPPORT_LIN_BUS_ACTIVITY_CHECK		TRUE								/* FALSE: No MLX4/LIN-Bus activity check; TRUE: LIN-Bus activity check */
#define _SUPPORT_DEGRADE_DELAY				FALSE								/* FALSE: Resume immediately from UV/OV; TRUE: Add (variable) delay before resume from UV/OV (MMP150128-1) */
/* diagnostic:stall detection */
#define _SUPPORT_STALLDET_A                 FALSE								/* stall detect based on current amplitude */
#define _SUPPORT_STALLDET_B					FALSE								/* stall detect based on bemf */
#define _SUPPORT_STALLDET_O					FALSE								/* Stall detector based on current oscillations */
#define _SUPPORT_STALLDET_H					TRUE 								/* Stall detector based on hall-sensor info (See also: _SUPPORT_HALL_SENSOR) (MMP160420-1) */
#define _SUPPORT_STALLDET                   TRUE
/* diagnostic:drift check */
#define _SUPPORT_DRIFT_CHECK				FALSE								/* drift detect based on hall-sensor */
/* diagnostic:system */	
#define _SUPPORT_IOREG_CHECK				TRUE								/* FALSE: No critical IO-Register check; TRUE: Critical IO-register check */

/* functions */
#define _SUPPORT_SPEED_AUTO					FALSE								/* FALSE: No auto-speed support; Auto speed supported based on voltage and temperature */
#define _SUPPORT_AUTO_CALIBRATION			FALSE								/* FALSE: No POR calibration; TRUE: Calibration at POR */
#define _SUPPORT_HALL_SENSOR				TRUE								/* FALSE: Sensor-less; TRUE: Hall-sensor for stall (See also: _SUPPORT_STALLDET_H) (MMP160420-1) */
#define _SUPPORT_ENDSTOP_DETECTION			TRUE   
#define _SUPPORT_BUSTIMEOUT_SLEEP			TRUE								/* FALSE: Only EmRun (if enabled), TRUE: EmRun (if enabled) followed by SLEEP (GM: 6.5.1) */
#define _SUPPORT_BUSTIMEOUT					TRUE								/* FALSE: Do not move to emergency position after bus-timeout; TRUE: Move to emergency position after bus-timeout */

/* NVRAM */
#define _SUPPORT_NVRAM_RECOVER_CYCLE_ONCE		TRUE							/* FALSE: Recover until okay; TRUE: Recover once */
#define _SUPPORT_UNLIMITED_NVRAM_WRITE_CYCLES	TRUE							/* FALSE: Limit write-cycles to 65000/page; TRUE: Unlimited write-cycles */

/* *** Section #8: Version Control  *** */	
/* LIN variant */
//#define C_VARIANT							0xB1U

/* SVN version */
//#define C_SVN								2630
/* hardware & software version */
#define C_SW_REF							0x89U								/* UniROM SW revision 1.1 */
#define C_HW_REF							0x10U								/* HW Revision 1.0 */
/* project ID */
#define C_PROJECT_ID						0xCBU								/* Sanhua GM (Water)VALVE */

#endif /*_BUILD_*/
