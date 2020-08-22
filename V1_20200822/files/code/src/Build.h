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

#define FALSE						0
#define TRUE						1	/* Should be not-FALSE */

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
/* *** NOTE: rename the lib_lin to library (LIN) or lib_pwm to library (PWM) *** */
#define LIN_COMM					TRUE						/* <<<1<<< */	/* LIN-interface: FALSE: Disabled; TRUE: Enabled */

#define NOLIN						(0x00U << 8)
#define LIN13						(0x13U << 8)								/* LIN protocol according LIN 1.3 */
#define LIN20						(0x20U << 8)								/* LIN protocol according LIN 2.0 */
#define LIN21						(0x21U << 8)								/* LIN protocol according LIN 2.1 */
#define LIN22						(0x22U << 8)								/* LIN protocol according LIN 2.2 */
#define LIN2J						(0x2AU << 8)								/* LIN protocol according LIN 2.x/SAE-J2602 */
#define LIN2X						(0x2FU << 8)								/* LIN protocol according LIN 2.x */
#define LIN2						(0x20U << 8)								/* Any LIN 2.x protocol, including SAE-J2602 */
#define LINX						(0xF0U << 8)								/* Any LIN x.x protocol, including SAE-J2602 */
#define LINXX						(0xFFU << 8)

#define APP_ACT_COOLING_23			0xC2U										/* C2 = Cooling V2.3 */
#define APP_ACT_AIRCO_44			0xA4U										/* A4 = Lastenheft Klima-Standardaktuator mit LIN-BUS-Schnittstelle 2.x V4.8 */
#define APP_ACT_HVAC_VW				0x13U										/* 13 = VW HVAC V3.0 (10-APR-2015) */
#define APP_ACT_VALVE_VW			0x21U										/* 23 = VW (Water)Valve V1.0 (07-APR-2016) */

#define LIN2X_ACT44					(LIN2X | APP_ACT_AIRCO_44)					/* LIN protocol according LIN 2.x/Actuator 4.8 */
#define LIN2J_VALVE_VW				(LIN2J | APP_ACT_VALVE_VW)					/* LIN protocol according SAE J2602/VW valve 1.0 */

#if LIN_COMM
/* LIN communication protocol */
/* LIN2X_ACT44:		Select in lin2b_romtbl.S: #define LINPROT LIN2X_ACT44
 */
/* #define LINPROT					LIN2X_ACT44		*/	/* lin2b_romtbl.S */	/* Select LIN 2.x with Klima Std-Actuators V4.8 protocol */
#define LINPROT						LIN2J_VALVE_VW								/* Select LIN 2.0/SAE J2602 VW Valve V1.0 protocol */

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
#define MF_TYPE						0xF0U										/* Upper 4 bits is Family Type */
#define MF_SUBTYPE					0x0FU										/* Lower 4 bits is Sub-type */
#define MF_DC						0x10U										/* DC */
#define MF_DC_T0					0x10U										/* DC, Positioning: None */
#define MF_DC_T1					0x11U										/* DC, Positioning: 3-wire (Phase1, P, Phase2) */
#define	MF_DC_T2					0x12U										/* DC, Positioning: 3-wires (3V3, P, GND) */
#define MF_BLDC						0x20U										/* BEMF, with startup in Stepper mode */
#define MF_STEPPER					0x30U										/* Stepper */
#define MF_RELUCTANCE				0x40U										/* Reluctance motor */

/* *** Section #2: Usage of NVRAM or Code Constants *** */
#define MP_CONST					0											/* Use fixed constants for UniROM code (Shortest code size) */
#define MP_NVRAM					1											/* Use NVRAM parameters for UniROM code (Most flexible) */
#define MOTOR_PARAMS				MP_NVRAM					/* <<<2<<< */
#define _SUPPORT_CODE_PARAMS		FALSE										/* FALSE: Use NVRAM (if valid); TRUE: Use MotorParams.h settings */
#define _SUPPORT_BUSTIMEOUT			TRUE										/* FALSE: Do not move to emergency position after bus-timeout; TRUE: Move to emergency position after bus-timeout */

/* *** Section #3: Motor type *** */
/* 0xxx = MLX81300/MLX81310 
 * x12x = ZH
 */
#define MT_ZH_HVAC				0x0120										/* ZH 4-phase Bi-polar (123R) */
#define MT_ZH_EXVALVE			0x0121										/* ZH 4-phase Bi-Polar Expansion Valve (22R) */
#define MT_ZH_WVALVE			0x0122										/* ZH 4-phase Bi-Polar Water-value */
/* Active motor-type */
#define MOTOR_TYPE					MT_ZH_WVALVE			/* <<<3<<< */

/* *** Section #5: Special *** */								/* <<<5<<< */
#define _SUPPORT_NVRAM_BACKUP				TRUE								/* NVRAM User-page backup support */
#define _SUPPORT_STALLDET_B					FALSE
#define _SUPPORT_STALLDET_O					TRUE								/* Stall detector based on current oscillations */
#define _SUPPORT_STALLDET_H					FALSE								/* Stall detector based on hall-sensor info (See also: _SUPPORT_HALL_SENSOR) (MMP160420-1) */
#define _SUPPORT_MLX_DEBUG_MODE				TRUE								/* Melexis Debug-mode */
#define _SUPPORT_RAM_TEST					TRUE								/* Test RAM before MLX4 started (Approximate 1.67ms) */
#define _SUPPORT_RAM_TEST_3LOOPS			TRUE								/* Test RAM in 3-loops (Approximate 5ms) */
#define _SUPPORT_WD_RST_RECOVERY			TRUE								/* Support Watchdog reset fast recovery */
#define _SUPPORT_CRASH_RECOVERY				TRUE								/* Support crash recovery */
#define _SUPPORT_TESTMODE_OFF				TRUE								/* MLX81300DC support test-mode switching */
#define _SUPPORT_BUSTIMEOUT_SLEEP			FALSE		/* VW151010: TRUE */				/* FALSE: Only EmRun (if enabled), TRUE: EmRun (if enabled) followed by SLEEP (VW: 6.5.1) */
#define _SUPPORT_TWO_LINE_TEMP_INTERPOLATION	TRUE							/* FALSE: One line linear interpolation; TRUE: Two lines linear interpolation */
#define _SUPPORT_AMBIENT_TEMP				TRUE								/* FALSE: Use chip temperature for compensation; TRUE: Use estimated ambient temperature for compensation */
#define _SUPPORT_CHIP_TEMP_PROFILE			TRUE								/* FALSE: No chip temperature profile check; TRUE: Chip temperature profile check (dT/dt) */
#define _SUPPORT_LINNETWORK_LOADER			TRUE								/* FALSE: Flash-loading via point-to-point (0x7F); TRUE: Network Flash-loading support (NAD) */
#define _SUPPORT_MLX16_HALT					TRUE								/* FALSE: MLX16 doesn't HALT; TRUE: MLX16 enters HALT during holding mode (power-safe) */
#define _SUPPORT_VSMFILTERED				TRUE								/* FALSE: Unfiltered Vsm (ADC Channel 14); TRUE: Filtered Vsm (ADC Channel 4) (MLX81310C) */
#define _SUPPORT_AUTO_BAUDRATE				FALSE								/* FALSE: Fixed baudrate; TRUE: Auto-detection of baudrate */
#define _SUPPORT_MOTOR_SELFTEST				TRUE								/* FALSE: No motor driver check at POR; TRUE: Motor driver check at POR */
#define _SUPPORT_LIN_UV						TRUE								/* FALSE: No LIN UV check; TRUE: LIN UV check (reset Bus-time-out) */
#define _SUPPORT_IOREG_CHECK				TRUE								/* FALSE: No critical IO-Register check; TRUE: Critical IO-register check */
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
#define BIPOLAR_PWM_SINGLE_MIRROR_GND		2									/* Each coil single PWM mirror, other HIGH */
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
#define BIPOLAR_PWM_SINGLE_INDEPENDED_GND	4									/* Each coil single PWM mirror/inverse mirror, other HIGH */
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
#define _SUPPORT_PWM_MODE					BIPOLAR_PWM_SINGLE_INDEPENDED_GND
#define _SUPPORT_PHASE_SHORT_DET			FALSE								/* FALSE: No phase-short-to-GND detection support; TRUE: Phase-short-to-GND detection supported */
#define _SUPPORT_DOUBLE_MOTOR_CURRENT		TRUE								/* FALSE: Motor Current x 1; TRUE: Motor Current x 2 */
#if (defined __MLX81315_A__)
/* MLX81315 */
#define _SUPPORT_QUADRUPLE_MOTOR_CURRENT	TRUE								/* FALSE: See: _SUPPORT_DOUBLE_MOTOR_CURRENT; TRUE: Motor Current x 4 (MMP141209-4) */
#endif /* (defined __MLX81315_A__) */
#define _SUPPORT_DOUBLE_USTEP				TRUE								/* FALSE: 32 or 48 micro-steps; TRUE: 64 or 96 micro-steps */
#define _SUPPORT_PWM_DC_RAMPUP				TRUE								/* FALSE: Constant mPWM-DC; TRUE: Increasing mPWM-DC at ramp-up */
#define _SUPPORT_PWM_DC_RAMPDOWN			TRUE								/* FALSE: Constant mPWM-DC; TRUE: Decrease mPWM-DC at ramp-down */
#define _SUPPORT_PWM_100PCT					TRUE								/* FALSE: max. PWM duty-cycle: 97.5%; TRUE: max. PWM duty-cycle: 100% */
#define _SUPPORT_DIAG_OC					TRUE								/* FALSE: Ignore OC; TRUE: Handle OC */
#define _SUPPORT_DIAG_OVT					TRUE								/* FALSE: Ignore OVT; TRUE: Handle OVT */
#define _SUPPORT_LIN_BUS_ACTIVITY_CHECK		TRUE								/* FALSE: No MLX4/LIN-Bus activity check; TRUE: LIN-Bus activity check */
#if (LINPROT == LIN2X_ACT44)
/* Feature of HVAC 4.8 Actuator */
#define _SUPPORT_HVAC_GROUP_ADDRESS			FALSE								/* FALSE: Function ID 0x0001; TRUE: Function ID 0x0002 */
#define _SUPPORT_DEGRADE_DELAY				FALSE								/* FALSE: Resume immediately from UV/OV; TRUE: Add (variable) delay before resume from UV/OV */
#else  /* (LINPROT == LIN2X_ACT44) */
#define _SUPPORT_HVAC_GROUP_ADDRESS			FALSE								/* FALSE: Function ID 0x0001; TRUE: Function ID 0x0002 */
#define _SUPPORT_DEGRADE_DELAY				FALSE								/* FALSE: Resume immediately from UV/OV; TRUE: Add (variable) delay before resume from UV/OV */
#endif /* (LINPROT == LIN2X_ACT44) */
#define _SUPPORT_SPEED_AUTO					FALSE								/* FALSE: No auto-speed support; Auto speed supported based on voltage and temperature */
#define _SUPPORT_AUTO_CALIBRATION			FALSE								/* FALSE: No POR calibration; TRUE: Calibration at POR */
#define _SUPPORT_HALL_SENSOR				FALSE								/* FALSE: Sensor-less; TRUE: Hall-sensor for stall (See also: _SUPPORT_STALLDET_H) (MMP160420-1) */
#define _SUPPORT_UNLIMITED_NVRAM_WRITE_CYCLES	TRUE							/* FALSE: Limit write-cycles to 65000/page; TRUE: Unlimited write-cycles */
#define _SUPPORT_NVRAM_RECOVER_CYCLE_ONCE	TRUE								/* FALSE: Recover until okay; TRUE: Recover once */
#define _SUPPORT_OVT_PED					FALSE								/* FALSE: OVT will not result in PED; TRUE: 5x OVT will result in PED (Permanent Electric Defect) */

#define _SUPPORT_LOG_ERRORS					TRUE								/* FALSE: No error logging; TRUE: Error logging RAM/LIN-frame support */
#define _SUPPORT_LOG_NVRAM					FALSE								/* FALSE: Don't save errors in NVRAM; TRUE: Save serious errors in NVRAM */

/* LIN-AA Stuff */
#if (LINPROT == LIN2X_ACT44)
#define _SUPPORT_LIN_AA						TRUE								/* FALSE: No LIN-AA support; TRUE: LIN-AA support based on BSM */
#define FORCE_LINAA_OLD						TRUE								/* FALSE: Support three types of BSM; TRUE: Support only one type of BSM "old" */
#define USE_MULTI_PURPOSE_BUFFER			TRUE								/* FALSE: Separate buffers; TRUE: Space saving (RAM) */
#define LINAA_NON_CHOPPER_MODE				FALSE								/* FALSE: Chopper-mode; TRUE: Non-chopper mode */
#define LINAA_BSM_SNPD_R1p0					TRUE								/* FALSE: Cooling-mode; TRUE: LIN Bus Shunt Method Slave Node Position Detection */
#define LIN_AA_INFO							TRUE								/* Collect Ish1, Ish2 and Ish3 data for 10 measurements */
#define LIN_AA_SCREENTEST					TRUE								/* Additional screening data CM1..3 & DM1..3 */
#elif (LINPROT == LIN2J_VALVE_VW)
#define _SUPPORT_LIN_AA						FALSE								/* FALSE: No LIN-AA support; TRUE: LIN-AA support based on BSM */
#define FORCE_LINAA_OLD						TRUE								/* FALSE: Support three types of BSM; TRUE: Support only one type of BSM "old" */
#define USE_MULTI_PURPOSE_BUFFER			FALSE								/* FALSE: Separate buffers; TRUE: Space saving (RAM) */
#define LINAA_NON_CHOPPER_MODE				FALSE								/* FALSE: Chopper-mode; TRUE: Non-chopper mode */
#define LINAA_BSM_SNPD_R1p0					TRUE								/* FALSE: Cooling-mode; TRUE: LIN Bus Shunt Method Slave Node Position Detection */
#define LIN_AA_INFO							FALSE								/* Collect Ish1, Ish2 and Ish3 data for 10 measurements */
#define LIN_AA_SCREENTEST					FALSE								/* Additional screening data CM1..3 & DM1..3 */
#endif /* (LINPROT == LIN2J_VALVE_VW) */


/* *** Section #6: Debug *** */									/* <<<6<<< */
#define _DEBUG								FALSE								/* Set TRUE when one of below _DEBUG items is set TRUE */
#define _DEBUG_SPI							FALSE								/* FALSE: I/O-toggling; TRUE: SPI data */
#define DEBUG_SPI_BAUDRATE					1000000U							/* SPI Baudrate: 1.0MBaud (100k-1MBaud) */
#define _DEBUG_COMMUT_ISR					FALSE

#define _DEBUG_MOTOR_CURRENT_FLT			FALSE								/* Motor current filter debug-buffers */


#if MCU_ASSP_MODE
/* ASSP-mode: IO[0], IO[1] and IO[2] can be used for debugging */
#define DEBUG_INI_IO_ABC()			{ ANA_OUTM = (IO2_OUTCFG_SOFT | IO1_OUTCFG_SOFT | IO0_OUTCFG_SOFT); \
									ANA_OUTF = (IO2_ENA | IO1_ENA | IO0_ENA); }
#define DEBUG_CLR_IO_A()			{ANA_OUTN &= ~(1u << 1);}					/* Clear  IO[1] */
#define DEBUG_SET_IO_A()			{ANA_OUTN |= (1u << 1);}					/* Set    IO[1] */
#define DEBUG_TOG_IO_A()			{ANA_OUTN ^= (1u << 1);}					/* Toggle IO[1] */
#define DEBUG_CLR_IO_B()			{ANA_OUTN &= ~(1u << 2);}					/* Clear  IO[2] */
#define DEBUG_SET_IO_B()			{ANA_OUTN |= (1u << 2);}					/* Set    IO[2] */
#define DEBUG_TOG_IO_B()			{ANA_OUTN ^= (1u << 2);}					/* Toggle IO[2] */
#define DEBUG_SET_IO_AB_00()		{ANA_OUTN &= ~(3u << 1);}					/* Clear  IO[2:1] = 00 */
#define DEBUG_SET_IO_AB_01()	{ANA_OUTN = (ANA_OUTN & ~(3u << 1)) | (1u << 1);}	/* Set    IO[2:1] = 01 */
#define DEBUG_SET_IO_AB_10()	{ANA_OUTN = (ANA_OUTN & ~(3u << 1)) | (1u << 2);}	/* Set    IO[2:1] = 10 */
#define DEBUG_SET_IO_AB_11()		{ANA_OUTN |= (3u << 1);}					/* Set    IO[2:1] = 11 */
#define DEBUG_CLR_IO_C()			{ANA_OUTN &= ~(1u << 0);}					/* Clear  IO[0] */
#define DEBUG_SET_IO_C()			{ANA_OUTN |= (1u << 0);}					/* Set    IO[0] */
#define DEBUG_TOG_IO_C()			{ANA_OUTN ^= (1u << 0);}					/* Toggle IO[0] */
#else  /* MCU_ASSP_MODE */	
#define DEBUG_CLR_IO_A()			{IO_EXTIO &= ~(1u << 1);}					/* Clear  IO[4] */
#define DEBUG_SET_IO_A()			{IO_EXTIO |= (3u << 0);}					/* Set    IO[4] */
#define DEBUG_TOG_IO_A()			{IO_EXTIO ^= (1u << 1);}					/* Toggle IO[4] */
#define DEBUG_CLR_IO_B()			{IO_EXTIO &= ~(1u << 5);}					/* Clear  IO[5] */
#define DEBUG_SET_IO_B()			{IO_EXTIO |= (3u << 4);}					/* Set    IO[5] */
#define DEBUG_TOG_IO_B()			{IO_EXTIO ^= (1u << 5);}					/* Toggle IO[5] */
#define DEBUG_CLR_IO_AB()			{IO_EXTIO &= ~((1u << 5) | (1u << 1));}		/* Clear  IO[5:4] */
#define DEBUG_SET_IO_AB()			{IO_EXTIO |= ((1u << 5) | (1u << 1));}		/* Set    IO[5:4] */
#define DEBUG_SET_IO_AB_00()		{IO_EXTIO = ((1u << 4) | (1u << 0));}		/* Set    IO[5:4] = 00 */
#define DEBUG_SET_IO_AB_01()		{IO_EXTIO = ((1u << 4) | (3u << 0));}		/* Set    IO[5:4] = 01 */
#define DEBUG_SET_IO_AB_10()		{IO_EXTIO = ((3u << 4) | (1u << 0));}		/* Set    IO[5:4] = 10 */
#define DEBUG_SET_IO_AB_11()		{IO_EXTIO = ((3u << 4) | (3u << 0));}		/* Set    IO[5:4] = 11 */
#endif /* MCU_ASSP_MODE */

#endif /*_BUILD_*/
