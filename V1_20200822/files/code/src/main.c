/*! \file		Main.c
 *  \brief		MLX81310 Main
 *
 * \note		project MLX81310
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2011-04-17
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	main()
 *				RamBackgroundTest()
 *				FlashBackgroundTest()
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
 * **************************************************************************** *
 *	Hardware resources:
 *		Timer1: Motor-driver (commutation), HALT-resume timer.
 *		Timer2: LIN-AA, Windmilling
 *		CoreTimer: Administrative timer (500us periods)
 *		PWM1-5: Motor-driver
 *		ADC:	Environment measurement (Supply-voltage, Motor-current, Chip temperature, BEMF, IO's)
 *		SPI:	(Optional)
 *		IO's:	(Optional)
 *				Motor Fam.	MLX81310A
 *		  TC1				-
 *		  TC2				-
 *			0				Debugging (C)
 *			1				Debugging (A)
 *			2				Debugging (B)
 *			3				PWM-IN
 *			4				-
 *			5				-
 *		LIN:	LIN-Communication or PWM-OUT
 *
 *	IRQ priorities:
 *		3:		Diagnostic IRQ (OC, OVT, OV, UV)
 *		4:		Timer1 (Motor-driver commutation timer)
 *		5:		LIN Communication
 *				ADC (Environment measurement)
 *		6:		Administrative timer
 *				(Optional) SPI
 *
 *	Variable name conversion:
 *		c_		Constant
 *		g_		Globals (access through-out the project)
 *		l_		Locals (accessed only in file)
 *		..		No prefix are local function variable (on stack)
 *		  a		Array of
 *		  e8	enumerated, 8-bit	(ANSI-C: enum)
 *		  i8	signed, 8-bit		(ANSI-C: char)
 *		  i16	signed, 16-bit		(ANSI-C: short)
 *		  i32	signed, 32-bit		(ANSI-C: long)
 *		  u8	unsigned, 8-bit 	(ANSI-C: unsigned char)
 *		  u16	unsigned, 16-bit 	(ANSI-C: unsigned short)
 *		  u32	unsigned, 32-bit	(ANSI-C: unsigned long)
 * ****************************************************************************	*/

#include "Build.h"
#include "main.h"
#include <awdg.h>																/* Analogue Watchdog support */
#include "ADC.h"																/* ADC support */
#include "Diagnostic.h"															/* Diagnostics support */
#include "ErrorCodes.h"															/* Error-logging support */
#include "MotorStall.h"															/* Motor stall detector support */
#include "NVRAM_UserPage.h"														/* NVRAM support */
#include "PID_Control.h"														/* PID Controller support */
#include "Timer.h"																/* Timer support */
#include <plib.h>																/* Use Melexis MLX813xx library (WDG_Manager) */
#include <lin.h>																/* Use Melexis LIN library */
#include "private_mathlib.h"
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */
#if _DEBUG_SPI
#include "SPI_Debug.h"
#endif /* _DEBUG_SPI */

#if ((__MLX16_GCC_MAJOR__ == 1) && (__MLX16_GCC_MINOR__ >= 10))
/* OK */
#else
#warning "Code compilation require GCC version 1.10 or newer."
#endif
/* ****************************************************************************	*
 *	Internal function prototypes												*
 * ****************************************************************************	*/
uint16 RamBackgroundTest( uint16 u16Page);
uint16 FlashBackgroundTest( uint16 u16Size);

/* ****************************************************************************	*
 *	Definitions																	*
 * ****************************************************************************	*/


/* ****************************************************************************	*
 *	Variables																	*
 * ****************************************************************************	*/


/* ****************************************************************************	*
 *	SPECIFIC IMPLEMENTATION OF THE EEPROM MEMORY								*
 * ****************************************************************************	*/
#pragma space nodp 																/* __NEAR_SECTION__ */
#pragma space none																/* __NEAR_SECTION__ */

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION (@TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp 																/* __TINY_SECTION__ */
uint8 g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
volatile uint8 g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_NO;					/* Status-flags electric error */
volatile uint8 g_e8ErrorCoil = 0;
volatile uint8 g_e8ErrorVoltage = (uint8) C_ERR_VOLTAGE_IN_RANGE;				/* Status-flags voltage */
uint8 g_e8MotorDirectionCCW = (uint8) C_MOTOR_DIR_UNKNOWN;						/* Control/Status-flag motor rotational direction Counter Clock-wise */
volatile uint8 g_u8ChipResetOcc = TRUE;											/* Status-flag indicate chip-reset occurred (POR) */
volatile uint8 g_u8StallOcc = FALSE;											/* Status-flag indicate stall occurred */
volatile uint8 g_u8EmergencyRunOcc = FALSE;										/* Status-flag indicate Emergency-run occurred */
volatile uint8 g_e8ErrorOverTemperature = (uint8) C_ERR_OTEMP_NO;				/* Status-flag over-temperature */
uint8 g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;					/* Degraded Motor Request */
volatile uint8 g_u8RewindFlags = 0;
volatile int16 g_i16ChipTemperature = 25;										/* Chip internal temperature */
#if _SUPPORT_AMBIENT_TEMP
volatile int16 g_i16AmbjTemperature = 25;										/* Ambient Temperature */
#endif /* _SUPPORT_AMBIENT_TEMP */
volatile int16 g_i16MotorVoltage = 1200;										/* Motor driver voltage */
uint8 g_u8StallTypeComm = (uint8) C_STALL_NOT_FOUND;
uint8 g_u8MotorStatusSpeed = (uint8) C_MOTOR_SPEED_STOP;						/* (Status) Actual motor-speed */
uint8 g_e8CalibrationStep = (uint8) C_CALIB_NONE;								/* Calibration step */
/* MMP151118-2 */
uint8 g_e8MotorCtrlMode __attribute__ ((section(".dp.noinit")));				/* Control-flags motor mode (from Master) [WD] */
volatile uint8 g_e8MotorStatusMode __attribute__ ((section(".dp.noinit")));		/* Status-flags motor mode (to Master) */
uint8 g_e8StallDetectorEna __attribute__ ((section(".dp.noinit")));				/* Control-flag Stall-detector enabled [WD] */
uint8 g_u8MotorHoldingCurrEna __attribute__ ((section(".dp.noinit")));			/* Control-flag motor Holding-current enabled [WD] */
uint16 g_u16ActualPosition __attribute__ ((section(".dp.noinit")));				/* (Control/Status) Actual motor-rotor position [WD] */
uint16 g_u16TargetPosition __attribute__ ((section(".dp.noinit")));				/* (Control) Target motor-rotor position (invalid) [WD] */
uint8 g_u8MotorCtrlSpeed __attribute__ ((section(".dp.noinit")));				/* (Control) Selected motor-speed */
uint16 g_u16CalibTravel __attribute__ ((section(".dp.noinit")));				/* Number of steps between two end-stops */

uint8 g_e8EXVMoveEnableRequestFlag = (uint8)C_EXV_MOVE_DISABLE;						//EXV Move enable flag, Ban
uint8 g_e8EXVInitDirection = (uint8)C_EXV_INIT_DIR_NO_SELECT;						//EXV Init direction, Ban
uint8 g_e8WarningOverTemperature = (uint8)C_WARNING_OTEMP_NO;
uint8 g_e8EXVFault = (uint8)C_STATUS_NO_FAULT;
uint8 g_e8EXVErrorBlock = FALSE;

#pragma space none																/* __TINY_SECTION__ */

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	(@NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp																/* __NEAR_SECTION__ */
volatile int16 g_i16SupplyVoltage = 1200;										/* Supply Voltage */
#if _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE)
volatile int16 g_i16PhaseVoltage = 1200U;										/* Phase Voltage */
#endif /* _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE) */
volatile int16 g_i16Current = 0;												/* Supply Current */
uint16 g_u16TargetPositionRewind;												/* Memorised Target Position after Rewind */

#if _SUPPORT_PHASE_SHORT_DET
uint16 l_u16AdcHoldMode = 0;													/* ADC-mode during holding-mode */
#endif /* _SUPPORT_PHASE_SHORT_DET */

#if _SUPPORT_CHIP_TEMP_PROFILE
uint16 g_u16TemperatureStabilityCounter = 0;									/* Temperature stability counter */
int16 l_i16ChipTemperaturePrev = 0;												/* Previous time-stamp chip temperature */
int16 l_i16ChipTemperatureInt = 0;												/* Chip temperature profile filter */
#endif /* _SUPPORT_CHIP_TEMP_PROFILE */

#if (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44)				/* MMP150128-1 - Begin */
uint16 u16DegradeDelay = 0xFFFF;												/* Disable degrade delay timer */
#endif /* (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44) */		/* MMP150128-1 - End */

#if _SUPPORT_LIN_BUS_ACTIVITY_CHECK
uint16 g_u16Mlx4StateCheckCounter = 0;											/* State check counter */
uint8 g_u8Mlx4ErrorState = 0;													/* Number of MLX4 Error states occurred */
#endif /* _SUPPORT_LIN_BUS_ACTIVITY_CHECK */
uint8 g_u8OverTemperatureCount = 0;												/* Number of over-temperature events */
uint8 g_u8MotorStartDelay = 0;													/* Motor start delay (500us) */
uint8 g_e8ErrorVoltageComm = (uint8) C_ERR_VOLTAGE_IN_RANGE;					/* Status-flags voltage (Communication) */
uint8 g_u8TorqueBoostRequest = 0;												/* No Torque Boost (0%) */

uint8 l_e8ErrorDebounceFilter = C_DEBFLT_ERR_NONE;								/* Debounce filter */
uint8 l_u8RamPreError = FALSE;													/* RAM vs. NVRAM test first-failure (MMP150925-1) */
uint8 g_u8MechError = FALSE;													/* No mechanical error */

uint8 g_e8CalibPostMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;					/* Post calibration Motor Request */
uint16 g_u16CalibPauseCounter = 0;

uint16 g_u16EXVTargetPositionTemp = 0;
//uint8 g_e8EXVStatusCommErr = C_STATUS_NO_COMM_ERR;
uint8 g_e8EXVStatusFaultState = C_STATUS_NO_FAULT;
uint8 g_e8EXVStatusMovInProcs = C_STATUS_MOVE_IDLE;
uint16 g_u16EXVStatusCurrentPositon = 0;
uint8 g_e8EXVStatusInitStat = C_STATUS_NOT_INIT;
uint8 l_e8GmcvInitDirection = C_GMCV_INIT_DIR_NO_SELECT;

#if USE_MULTI_PURPOSE_BUFFER
MP_BUF g_MPBuf;
#endif /* USE_MULTI_PURPOSE_BUFFER */

#pragma space none																/* __NEAR_SECTION__ */

const uint16 au16HaltZero[2] __attribute__((aligned(4))) =
{
		0x0000, 0x0000																/* MMP150224-2 */
};

/* ****************************************************************************	*
 * RamBackgroundTest
 *
 *	Pre:		u16Page: 0 = Check against User-page #1
 *						<>0 = Check against User-page #2
 *	Post:		FALSE : User-page against RAM compare failed
 *				TRUE  : User-page against RAM is correct
 *
 *	Comments:	Calculate a Melexis standard 8-bit CRC as used in NVRAM, for the
 *				Actuator important RAM data block (copied from NVRAM). This data
 *				should be stable during the actuator operation.
 * ****************************************************************************	*/
uint16 RamBackgroundTest( uint16 u16Page)
{
	/* Use compare instead of CRC-check !! */
	uint16 *pu16Page;
#if (LINPROT == LIN2J_VALVE_GM)
	(void) u16Page;
	if ( (((NVRAM_USER *) C_ADDR_USERPAGE1)->AppStatus & 0x80) ^ (((NVRAM_USER *) C_ADDR_USERPAGE2)->AppStatus & 0x80) )
		pu16Page = (uint16 *) C_ADDR_USERPAGE2;
	else
		pu16Page = (uint16 *) C_ADDR_USERPAGE1;
#else  /* (LINPROT == LIN2J_VALVE_GM) */
	if ( u16Page == 0 )
	{
		pu16Page = (uint16 *) C_ADDR_USERPAGE1;									/* Compare NVRAM User-page #1 against RAM */
	}
	else 
	{
		pu16Page = (uint16 *) C_ADDR_USERPAGE2;									/* Compare NVRAM User-page #2 against RAM */
	}
#endif /* (LINPROT == LIN2J_VALVE_GM) */

	return ( NVRAM_PageVerify( pu16Page) );

} /* End of RamBackgroundTest() */

/* ****************************************************************************	*
 * FlashBackgroundTest
 *
 *	Pre:		uint16 u16Size = Amount of Flash/ROM (in 16-bits words) to add to 
 *				CRC-calculation
 *	Post:		C_FLASH_CRC_FAILED		: CRC calculation failed
 *				C_FLASH_CRC_OK			: CRC calculation is correct
 *				C_FLASH_CRC_CALCULATING	: CRC calculation is ongoing (busy)
 *
 *	Comments:	The CCITT CRC-16 polynomial is X^16 + X^12 + X^5 + 1. The detection 
 *				rate is 99.9984%, worse case. In hex the pattern is 0x11021. A 17-bit 
 *				register is simulated by testing the MSB before shifting the data, 
 *				which affords us the luxury of specifying the polynomial as a 16-bit
 *				value, 0x1021. Because the CRC is process in reverse order, the 
 *				polynomial is reverse too: 0x8408.
 *				To avoid huge delay, the calculation is split into segments of u16Size
 *				16-bits words. When reaching the Flash-end, the checksum is compared
 *				against first calculated Flash CRC.
 * ****************************************************************************	*/
#define POLY	0x1021
#define FLASH_START_ADDR			0x4000
#define FLASH_CRC_ADDR				0xBF4E
#define FLASH_END_ADDR				0xC000
#define C_FLASH_SEGMENT_SZ			4											/* Max 250us (196us), Halt-mode: Full-check is once per 8:40s; Running-mode: 1.5s */

#define C_FLASH_CRC_FAILED			0
#define C_FLASH_CRC_OK				1
#define C_FLASH_CRC_CALCULATING		2

uint16 FlashBackgroundTest( uint16 u16Size)
{
	uint16 u16Result = C_FLASH_CRC_CALCULATING;
	static uint16 *pu16Segment = (uint16 *) FLASH_START_ADDR;
	static uint16 u16FlashCRC = 0;

	if ( u16Size == 0U )
	{
		pu16Segment = (uint16 *) FLASH_START_ADDR;
		return ( u16Result );
	}
	if ( pu16Segment == (uint16 *) FLASH_START_ADDR )
	{
		u16FlashCRC = 0xFFFF;													/* Initialise the CRC preset with 0xFFFF */
	}
	if ( ((uint16) pu16Segment + u16Size) > FLASH_END_ADDR )
	{
		u16Size = (FLASH_END_ADDR - (uint16) pu16Segment);
	}
	for ( ; u16Size > 0; u16Size-- )
	{
		if ( pu16Segment != (uint16 *) FLASH_CRC_ADDR )
		{
			uint8 u8Count;
			uint16 u16Data = *pu16Segment;
			for ( u8Count = 16; u8Count > 0; u8Count-- )
			{
				uint16 u16XorFlag = !!(u16FlashCRC & 0x8000);
				u16FlashCRC = (u16FlashCRC << 1);
				if ( u16Data & 0x8000 )
				{
					u16FlashCRC++;
				}
				if ( u16XorFlag )
				{
					u16FlashCRC ^= POLY;
				}
				u16Data <<= 1;
			}
		}
		pu16Segment++;
	}

	if ( (uint16) pu16Segment >= FLASH_END_ADDR )
	{
		/* CRC fully calculated, check values */
		pu16Segment = (uint16 *) FLASH_START_ADDR;
		if ( *(uint16 *) FLASH_CRC_ADDR != 0 )									/* Flash/ROM Checksum programmed? */
		{
			if ( *(uint16 *) FLASH_CRC_ADDR != u16FlashCRC )
			{
				u16Result = C_FLASH_CRC_FAILED;
			}
			else
			{
				u16Result = C_FLASH_CRC_OK;
			}
		}
	}

	return ( u16Result );

} /* End of FlashBackgroundTest() */

/* ****************************************************************************	*
 * noinit_section_init
 * MMP151118-2 *
 * ****************************************************************************	*/
void noinit_section_init(void)
{
	g_e8MotorCtrlMode = (uint8) C_MOTOR_CTRL_STOP;
	//g_e8StallDetectorEna = (uint8) C_STALLDET_ALL;								/* Control-flag Stall-detector enabled [WD] */;
	g_u8MotorHoldingCurrEna = FALSE;
	g_u16ActualPosition = 32767U;
	g_u16TargetPosition = 65535U;
	g_u8MotorCtrlSpeed = (uint8) C_DEFAULT_MOTOR_SPEED;
	g_e8MotorStatusMode = (uint8) C_MOTOR_STATUS_STOP;
	g_u16CalibTravel = C_EXV_DEF_TRAVEL;									/* Number of steps between two end-stops */

	/* MotorDriver.c variables */
	g_u16ActuatorActPos = 0;
	g_u16ActuatorTgtPos = 0;
} /* End of noinit_section_init() */

void RteExv2Lin(void)
{

//	g_e8CalibrationStep = (uint8)C_CALIB_DONE;

//	g_e8EXVStatusInitStat = C_STATUS_INIT_DONE;

	if (g_e8CalibrationStep == (uint8)C_CALIB_NONE)
	{
		g_e8EXVStatusInitStat = C_STATUS_NOT_INIT;
	}
	else if(g_e8CalibrationStep == (uint8)C_CALIB_DONE)
	{
		g_e8EXVStatusInitStat = C_STATUS_INIT_DONE;

	}
	else
	{
		g_e8EXVStatusInitStat = C_STATUS_INIT_BUSY;
	}

	if(g_e8CalibrationStep == (uint8)C_CALIB_DONE)
	{
		if(g_u8StallOcc == TRUE)
		{
			g_e8EXVErrorBlock = TRUE;
			g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;// Ban,
		}
	}

	//TODO need to add the indetermint, ban debug
	if(g_e8ErrorVoltage == (uint8) C_ERR_VOLTAGE_OVER)
	{
		g_e8EXVStatusFaultState = C_STATUS_FAULT_OVER_VOLTAGE;
	}
	else if(g_e8ErrorVoltage == (uint8) C_ERR_VOLTAGE_UNDER)
	{
		g_e8EXVStatusFaultState = C_STATUS_FAULT_UNDER_VLOTAGE;
	}
	else if((g_e8ErrorCoil == (uint8) C_ERR_SELFTEST_A) || (g_e8ErrorCoil == (uint8) C_ERR_SELFTEST_B))
	{
		g_e8EXVStatusFaultState = C_STATUS_FAULT_COIL_SHORT;
	}
	else if(g_e8ErrorCoil == (uint8) C_ERR_SELFTEST_C)
	{
		g_e8EXVStatusFaultState = C_STATUS_FAULT_COIL_OPEN;
	}
	else if(g_e8ErrorOverTemperature == (uint8)C_ERR_OTEMP_YES)
	{
		g_e8EXVStatusFaultState = C_STATUS_FAULT_OVER_TEMP_SHUTDOWN;
	}
	else if(g_e8EXVErrorBlock == TRUE)
	{
		g_e8EXVStatusFaultState = C_STATUS_FAULT_UNEXPECT_STALL;
	}
	else if(g_e8WarningOverTemperature == C_WARNING_OTEMP_YES)
	{
		g_e8EXVStatusFaultState = C_STATUS_FAULT_OVER_TEMP_WARNING;
	}
	else
	{
		g_e8EXVStatusFaultState = C_STATUS_NO_FAULT;
	}

	//if(((g_e8ErrorElectric == (uint8) C_ERR_ELECTRIC_PERM) && (g_e8EXVStatusFaultState != C_STATUS_NO_FAULT)) || ((g_e8CalibrationStep > C_CALIB_NONE) && (g_e8CalibrationStep < C_CALIB_DONE) && ((g_e8ErrorVoltage != C_ERR_VOLTAGE_IN_RANGE) || (g_e8ErrorOverTemperature == (uint8)C_ERR_OTEMP_YES))))//coil open/short/over temp/stall,ov/uv, changed to un-initialized
	if((g_e8ErrorElectric == (uint8) C_ERR_ELECTRIC_PERM) && (g_e8EXVStatusFaultState != C_STATUS_NO_FAULT))//coil open/short/stall, changed to un-initialized
	{
		g_e8CalibrationStep = C_CALIB_NONE;
		g_e8MotorRequest = C_MOTOR_REQUEST_NONE;
		g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
		g_e8CalibPostMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
		g_u16TargetPosition = g_u16ActualPosition;
		g_u16ActuatorActPos = g_u16ActualPosition;
		g_u16ActuatorTgtPos = g_u16TargetPosition;
	}

	if((g_e8EXVStatusInitStat == (uint8)C_STATUS_INIT_BUSY) || (g_e8EXVStatusInitStat == (uint8)C_STATUS_INIT_DONE))
	{
		if(g_u16ActualPosition <= C_EXV_ZERO_POS)
		{
			g_u16EXVStatusCurrentPositon = 0;
		}
		else if(g_u16ActualPosition >= C_EXV_POSITION_STD + C_EXV_ZERO_POS)
		{
			g_u16EXVStatusCurrentPositon = 0x3FF;
		}
		else
		{
			g_u16EXVStatusCurrentPositon = ((uint32)(g_u16ActualPosition - C_EXV_ZERO_POS)*1023+C_EXV_POSITION_STD/2)/C_EXV_POSITION_STD;
		}
	}
	else if(g_e8EXVStatusInitStat == (uint8)C_STATUS_NOT_INIT)
	{
		g_u16EXVStatusCurrentPositon = 0;
	}

}

//In default MLX, update speed during motor start or receive motor speed change command(will not excute other command)
//In updated MLX by Ban, update speed during motor start and receive motor speed change command(doesn't impact other command)
void UpdateMotorSpeed(void)
{
	uint8 u8MotorSpeedIdx;
	if(g_u8TorqueBoostRequest == 0)//normal torque
	{
		g_u8MotorCtrlSpeed = (uint8) C_DEFAULT_MOTOR_SPEED;//speed 1
		u8MotorSpeedIdx = (g_u8MotorCtrlSpeed & 0x07);
		g_u8MotorStatusSpeed = u8MotorSpeedIdx;
		g_u16MotorSpeedRPS = g_au16MotorSpeedRPS[u8MotorSpeedIdx];
		g_u16TargetCommutTimerPeriod = g_au16MotorSpeedCommutTimerPeriod[u8MotorSpeedIdx];
	}
	else//boost torque, reduce the speed
	{
		g_u8MotorCtrlSpeed = (uint8) C_MOTOR_SPEED_MID_LOW;//Speed 0
		u8MotorSpeedIdx = (g_u8MotorCtrlSpeed & 0x07);
		g_u8MotorStatusSpeed = u8MotorSpeedIdx;
		uint16 u16RPM = NVRAM_SPEED1 - muldivU16_U16byU16byU16((NVRAM_SPEED1 - NVRAM_SPEED0),g_u8TorqueBoostRequest,100);
		uint32 u32Temp = divU32_U32byU16( (TIMER_CLOCK * 60), g_u16MotorMicroStepsPerMechRotation);
		g_u16MotorSpeedRPS = divU16_U32byU16( (uint32)(uint16)(u16RPM + 30U), 60);
		g_u16TargetCommutTimerPeriod = divU16_U32byU16( u32Temp, u16RPM) - 1;
	}
}

/* ****************************************************************************	*
 * main()
 *
 * Application sequence:
 *	A. Initialise background schedule (Task-ID)
 *	B. (Optional) Check crash-recovery (fatal-error handler)
 *	Not crash-recovery (so e.g. COLD_START, WATCHDOG_RESET, LIN_CMD_RESET)
 *	C. Initialise watch-dogs, both analogue and digital
 *	D. (Optional) Chip test-mode disable
 *	E. Load user NVRAM page
 *	F. Initialise Error-logging management
 *	G. Initialise chip (H/W) Diagnostic
 *	H. Initialise ADC
 *	I. Initialise Motor-driver
 *	J. Initialise rewind management
 *	K. Motor-driver self-test
 *	L. Initialise PID-Control
 *	M. Initialise (Task) Core Timer
 *	N. (Optional) Initialise SPI communication
 *	O. Initialise LIN-communication
 *	P. (Optional) Initialise PWM-communication
 *	Q. Perform an initial supply and temperature measurement
 *	R. Stop any motor activity immediately
 *	S. (Optional) Determine actuator position
 *	T. (Watch-dog Reset) Start actuator (if needed)
 *	U. Main application loop
 * ****************************************************************************	*/
int16 main( void)
{
	/* *************************************************** */
	/* *** A. Initialise background schedule (Task-ID) *** */
	/* *************************************************** */
	uint8 u8BackgroundSchedulerTaskID = 0;
	uint16 u16CalibrationTravelStartPos = 0;
#if _SUPPORT_DIG_LIN
	uint8 u8LinState = LOW;
#endif /* _SUPPORT_DIG_LIN */

#if _SUPPORT_CRASH_RECOVERY
	/* **************************************************************** */
	/* *** B. (Optional) Check crash-recovery (fatal-error handler) *** */
	/* **************************************************************** */
	if ( bistResetInfo == C_CHIP_STATE_FATAL_CRASH_RECOVERY )
	{
#if LIN_COMM
		g_u8LinInFrameBufState = C_LIN_IN_FREE;
#endif /* LIN_COMM */
		if ( (g_e8StallDetectorEna != (uint8) C_STALLDET_NONE) && ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) != 0)) /*lint !e845 */
		{
			/* Actuator is active; clear motor-raw-current moving-average buffer */
			MotorDriverCurrentMeasureInit();
#if 0
#if (C_MOVAVG_SSZ < 6)
			g_u16MotorCurrentLPFx64 = (g_u16MotorCurrentMovAvgxN << (6 - C_MOVAVG_SSZ));
#endif /* (C_MOVAVG_SSZ < 6 ) */
#if (C_MOVAVG_SSZ == 6 )
			g_u16MotorCurrentLPFx64 = g_u16MotorCurrentMovAvgxN;
#endif /* (C_MOVAVG_SSZ == 6 ) */
#endif /* 0 */
		}
	}
	else
#endif /* _SUPPORT_CRASH_RECOVERY */
	{
		/* *********************************************************** */
		/* *** C. Initialise watch-dogs, both analogue and digital *** */
		/* *********************************************************** */
#if WATCHDOG == DISABLED
		WD_CTRL = 0;															/* Disable digital Watch-dog */
		AWD_CTRL = 0x8000;														/* Disable analogue Watch-dog */
#endif /* WATCHDOG == DISABLED */
#if WATCHDOG == ENABLED
		WD_T = WatchDog_PeriodOf100ms;											/* Initialise the (Digital) watch-dog comparator to 100ms */
		WD_CTRL = WatchDog_ModeTimer;											/* Define the mode and start the watchdog */
		awdg_init( AWDG_DIV_16, C_AWD_PERIOD_250MS);
		/* Maximum Watch-dog period */
#endif /* WATCHDOG == ENABLED */
#if MCU_ASSP_MODE
		ANA_OUTL |= ASSP;														/* ASSP-mode */
#endif /* MCU_ASSP_MODE */

#if _SUPPORT_TESTMODE_OFF
		/* ******************************************** */
		/* *** D. (Optional) Chip test-mode disable *** */
		/* ******************************************** */
		CONTROL |= OUTA_WE;														/* Grant access to ANA_OUTx registers */
		ANA_OUTA |= TEST_MODE_DIS;												/* Disable test-mode */
		CONTROL &= ~OUTA_WE;
#endif /* _SUPPORT_TESTMODE_OFF */

#if _DEBUG
#if _DEBUG_SPI
		ANA_OUTM = (IO3_OUTCFG_SPI | IO2_OUTCFG_SPI | IO1_OUTCFG_SPI | IO0_OUTCFG_SPI);
		ANA_OUTF = (IO3_ENA | IO2_ENA | IO0_ENA);								/* IO[0] = MOSI, IO[2] = CLK, IO[3] = -CS */
		SpiDebugInit();
#else  /* _DEBUG_SPI */
#if MCU_ASSP_MODE
		DEBUG_INI_IO_ABC();
#else  /* MCU_ASSP_MODE */
		/* None-ASSP-mode: IO[4] and IO[5] can be used for debugging */
		DEBUG_SET_IO_AB_00();
#endif /* MCU_ASSP_MODE */
#endif /* _DEBUG_SPI */
#endif /* _DEBUG */

		/* Application mode */
		SET_PRIORITY(0);
		g_e8MotorStatusMode = (uint8) C_MOTOR_STATUS_INIT;
		/* ******************************* */
		/* *** E. Load user NVRAM page *** */
		/* ******************************* */
		NVRAM_LoadUserPage();													/* Load User NVRAM storage parameters */

#if USE_MULTI_PURPOSE_BUFFER
		g_MPBuf.u8Type = (uint8) C_MP_BUF_NONE;
		g_MPBuf.u8Usage = (uint8) C_MP_BUF_FREE;
#endif /* USE_MULTI_PURPOSE_BUFFER */

		if ( bistResetInfo != C_CHIP_STATE_WATCHDOG_RESET )						/* MMP151118 - Begin */
		{
			noinit_section_init();
		}																		/* MMP151118 - End */
		g_e8StallDetectorEna = C_STALLDET_NONE;
		if ( g_NvramUser.StallDetectorEna )
		{
#if _SUPPORT_STALLDET_H
			g_e8StallDetectorEna |= C_STALLDET_H;								/* Control-flag Stall-detector "H" and "A" enabled */
#else  /* _SUPPORT_STALLDET_H */
			g_e8StallDetectorEna |= C_STALLDET_A;								/* Control-flag Stall-detector "A" enabled */
#endif /* _SUPPORT_STALLDET_H */
		}
		if ( NVRAM_STALL_O )
		{
			g_e8StallDetectorEna |= C_STALLDET_O;								/* Control-flag Stall-detector "O" enabled */
		}

		//g_e8StallDetectorEna = C_STALLDET_NONE;//Ban for GM CV, stall O is enabled and not programmable. TODO
		g_u8MotorHoldingCurrEna = g_NvramUser.MotorHoldingCurrentEna;			/* Control-flag motor Holding-current enabled */

		/* ********************************************** */
		/* *** F. Initialise Error-logging management *** */
		/* ********************************************** */
		ErrorLogInit();

		/* Log Watch-dog reset */
		if ( bistResetInfo != C_CHIP_STATE_LIN_CMD_RESET )						/* LIN-command chip reset use WD; No need to log */
		{
			if ( (CONTROL & WD_BOOT) || (AWD_CTRL & AWD_RST) )
			{
				if ( CONTROL & WD_BOOT )
				{
					if ( AWD_CTRL & AWD_RST )
					{
						SetLastError( (uint8) C_ERR_WD_AWD_RST);				/* Both Analogue & Digital Watch-dog reset */
					}
					else
					{
						SetLastError( (uint8) C_ERR_WD_RST);					/* Digital Watch-dog reset */
					}
				}
				else
				{
					SetLastError( (uint8) C_ERR_AWD_RST);						/* Analogue Watch-dog reset */
				}
			}
		}

		/* ******************************************* */
		/* *** G. Initialise chip (H/W) Diagnostic *** */
		/* ******************************************* */
		DiagnosticsInit();														/* Initialise Diagnostic */

		/* ************************* */
		/* *** H. Initialise ADC *** */
		/* ************************* */
		ADC_Init();																/* Initialise ADC */

		/* ********************************** */
		/* *** I. Initialise Motor-driver *** */
		/* ********************************** */
		MotorDriverInit();														/* Initialise Motor-Driver */

		/* *************************************** */
		/* *** J. Initialise rewind management *** */
		/* *************************************** */
#if _SUPPORT_WD_RST_RECOVERY
		if ( (bistResetInfo != C_CHIP_STATE_WATCHDOG_RESET) && (NVRAM_RESTALL_POR != FALSE) && (NVRAM_REWIND_STEPS != 0) )
#else  /* _SUPPORT_WD_RST_RECOVERY */
			if ( (NVRAM_RESTALL_POR != FALSE) && (NVRAM_REWIND_STEPS != 0) )
#endif /* _SUPPORT_WD_RST_RECOVERY */
			{
				g_u8RewindFlags = (uint8) (C_REWIND_STALL_DETECT | C_REWIND_DIRECTION_AUTO);/* After POR: Enable Rewind and set auto-direction */
			}

		SET_PRIORITY(7);

		/* ********************************* */
		/* *** K. Motor-driver self-test *** */
		/* ********************************* */
#if _SUPPORT_WD_RST_RECOVERY
		if ( (bistResetInfo != C_CHIP_STATE_WATCHDOG_RESET) && (bistResetInfo != C_CHIP_STATE_LIN_CMD_RESET) )
#else  /* _SUPPORT_WD_RST_RECOVERY */
			if ( ((CONTROL & WD_BOOT) == 0) && ((AWD_CTRL & AWD_RST) == 0) && (bistResetInfo != C_CHIP_STATE_LIN_CMD_RESET) )
#endif /* _SUPPORT_WD_RST_RECOVERY */
			{
#if (_SUPPORT_MOTOR_SELFTEST != FALSE)
				g_e8MotorStatusMode = (uint8) C_MOTOR_STATUS_SELFTEST;
				MotorDriverSelfTest();												/* Self-test Motor-Driver */
#endif /* (_SUPPORT_MOTOR_SELFTEST != FALSE) */
			}
#if _SUPPORT_WD_RST_RECOVERY
			else if ( bistResetInfo == C_CHIP_STATE_WATCHDOG_RESET )				/* MMP130626-11 */
			{
				g_u8ChipResetOcc = FALSE;											/* Clear chip-reset flag before LIN initialisation */
			}
#endif /* _SUPPORT_WD_RST_RECOVERY */

		g_e8MotorStatusMode = (uint8) C_MOTOR_STATUS_INIT;

		/* ********************************* */
		/* *** L. Initialise PID-Control *** */
		/* ********************************* */
		PID_Init();																/* PID Control initialisation */

		/* *************************************** */
		/* *** M. Initialise (Task) Core Timer *** */
		/* *************************************** */
		Timer_Init();															/* Initialise (Core) Timer */

		g_e8MotorStatusMode = (uint8) C_MOTOR_STATUS_STOP;

#if LIN_COMM
		/* *************************************** */
		/* *** O. Initialise LIN-communication *** */
		/* *************************************** */
#if _SUPPORT_WD_RST_RECOVERY
		if ( bistResetInfo == C_CHIP_STATE_WATCHDOG_RESET )
		{
			/* Do not change actual and target position */
			LIN_Init( TRUE);													/* Initialise LIN communication interface */
		}
		else
#endif /* _SUPPORT_WD_RST_RECOVERY */
		{
			LIN_Init( FALSE);													/* Initialise LIN communication interface */
		}

		/* Check chip-state for LIN-command RESET, to setup diagnostic-response */
		if ( bistResetInfo == C_CHIP_STATE_LIN_CMD_RESET )
		{
#if (((LINPROT & LINXX) == LIN2X) && _SUPPORT_MLX_DEBUG_MODE)
			RfrDiagReset();														/* Prepare a diagnostics response reply */
#endif /* (((LINPROT & LINXX) == LIN2X) && _SUPPORT_MLX_DEBUG_MODE) */
#if ((LINPROT & LINXX) == LIN2J)
			RfrDiagReset();														/* Prepare a diagnostics response reply */
#endif /* ((LINPROT & LINXX) == LIN2J) */
			bistResetInfo = C_CHIP_STATE_COLD_START;
		}
#endif /* LIN_COMM */

#if _SUPPORT_WD_RST_RECOVERY
		if ( bistResetInfo != C_CHIP_STATE_WATCHDOG_RESET )
#endif /* _SUPPORT_WD_RST_RECOVERY */
		{
			g_u16ActuatorActPos = g_u16ActualPosition;							/* Initialise the Actuator positions too */
			g_u16ActuatorTgtPos = g_u16TargetPosition;
		}

		/* **************************************************************** */
		/* *** Q. Perform an initial supply and temperature measurement *** */
		/* **************************************************************** */
		MeasureVsupplyAndTemperature();
		GetChipTemperature( TRUE);												/* MMP131020-1 */;

		/* ********************************************** */
		/* *** R. Stop any motor activity immediately *** */
		/* ********************************************** */
		MotorDriverStop( (uint16) C_STOP_IMMEDIATE);							/* Start-up: Energyce coils if needed */

		/* ************************************************* */
		/* *** S. (Optional) Determine actuator position *** */
		/* ************************************************* */
#if _SUPPORT_AUTO_CALIBRATION
		if ( (g_u8ChipResetOcc != FALSE) && (NVRAM_AUTO_RECALIBRATE != 0) )
		{
			g_e8CalibrationStep = (uint8) C_CALIB_START;
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_CALIBRATION;
		}
#endif /* _SUPPORT_AUTO_CALIBRATION */

#if _SUPPORT_WD_RST_RECOVERY
		/* ****************************************************** */
		/* *** T. (Watch-dog Reset) Start actuator (if needed) *** */
		/* ****************************************************** */
		if ( bistResetInfo == C_CHIP_STATE_WATCHDOG_RESET )
		{
			if ( g_u16ActuatorActPos != g_u16ActuatorTgtPos )
			{
				g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_START;
			}
		}
#endif /* _SUPPORT_WD_RST_RECOVERY */
	}

#if _SUPPORT_CRASH_RECOVERY
	bistResetInfo = C_CHIP_STATE_FATAL_RECOVER_ENA;							/* Enable Fatal crash recovery */
#endif /* _SUPPORT_CRASH_RECOVERY */

#if _SUPPORT_DIG_LIN
	if ( ml_GetState( ML_NOT_CLEAR) == stSHORT )
		u8LinState = LOW;
	else
		u8LinState = HIGH;
#endif /* _SUPPORT_DIG_LIN */

	/* ******************************** */
	/* *** U. Main application loop *** */
	/* ******************************** */
	for(;;)
	{
		/* ************************************ */
		/* *** a. Watch-dog acknowledgement *** */
		/* ************************************ */
#if WATCHDOG == ENABLED
		WDG_Manager();
#endif /* WATCHDOG == ENABLED */

#if _SUPPORT_DIG_LIN
		uint8 u8NewLinState;
		if ( ml_GetState( ML_NOT_CLEAR) == stSHORT )
			u8NewLinState = LOW;
		else
			u8NewLinState = HIGH;

		if ( u8NewLinState != u8LinState )
		{
			u8LinState = u8NewLinState;
			if ( u8LinState == FALSE )
			{
				/* Close valve */
				g_u16TargetPosition = C_MIN_POS;
			}
			else
			{
				/* Open valve */
				g_u16TargetPosition = C_MAX_POS;
			}
			g_u8MotorCtrlSpeed = C_MOTOR_SPEED_MID;
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_START;
		}
#endif /* _SUPPORT_DIG_LIN */



		/* ********************************* */
		/* *** c. LIN(-IN) communication *** */
		/* ********************************* */
#if LIN_COMM
		if ( g_u8LinInFrameBufState != C_LIN_IN_FREE )
		{
			/* LIN message buffer filled */
			HandleLinInMsg();
		}
#if ((LINPROT & LINXX) == LIN2X)
		if ( g_u8LinAAMode != 0 )												/* MMP140417-2: Don't allow any motor control */
		{
			continue;
		}																		/* MMP140417-2 - End */
#endif /* ((LINPROT & LINXX) == LIN2X) */



		/* ******************************* */
		/* *** d. Motor Driver current *** */
		/* ******************************* */
		/* Calculate Current (1000LSB/A) [mA] */
		if ( ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) != 0) || (g_u8MotorHoldingCurrState != FALSE) )
		{
			/* Average current = unfiltered_current * Motor_PWM_DutyCycle */
			g_i16Current = (int16) muldivU16_U16byU16byU16( (uint16) GetMotorDriverCurrent(), g_u16CorrectionRatio, (PWM_REG_PERIOD << (4 + PWM_PRESCALER_N)));

			if ( (g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) == 0 )
			{
				/* Motor holding mode: Update motor-current LPF for PID-control */
				g_u16MotorCurrentLPFx64 = (g_u16MotorCurrentLPFx64 - ((g_u16MotorCurrentLPFx64 + 16) >> 4)) + (g_i16Current << 2);
#if (C_MOVAVG_SSZ < 6)
				g_u16MotorCurrentMovAvgxN = ((g_u16MotorCurrentLPFx64 + 2) >> (6 - C_MOVAVG_SSZ));	/* -=#=- Just for debug feedback */
#endif /* (C_MOVAVG_SSZ < 6 ) */
#if (C_MOVAVG_SSZ == 6 )
				g_u16MotorCurrentMovAvgxN = g_u16MotorCurrentLPFx64;			/* -=#=- Just for debug feedback */
#endif /* (C_MOVAVG_SSZ == 6 ) */
			}
		}
		else if ( (g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) == 0 )
#else  /* LIN_COMM */
			if ( (g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) == 0 )
#endif /* LIN_COMM */
			{
				g_i16Current = 0;
				g_u16MotorCurrentLPFx64 = 0;
				g_u16MotorCurrentMovAvgxN = 0;
				MeasureVsupplyAndTemperature();										/* Perform Vbat/Temperature measurement incase motor is stopped */
			}



		/* ************************************************************** */
		/* *** e. Chip and Motor Driver voltage (degraded-mode check) *** */
		/* ************************************************************** */
		/* Calculate Voltage (100LSB/V) [10mV] */
		GetVsupply();
		GetVsupplyMotor();
		if ( g_i16MotorVoltage < (NVRAM_APPL_UVOLT - C_VOLTAGE_HYS) )//under voltage is 9-0.5=8.5V accroding to FMEA, set in NVRAM
		{
			/* First time application under-voltage error */					/* MMP150128-1 - Begin */
			if ( (l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_UV) == 0x00 )
			{
				/* Need twice a under-voltage detection, to avoid ESD-pulses disturbance will cause degraded mode entering */
				l_e8ErrorDebounceFilter |= (uint8) C_DEBFLT_ERR_UV;
			}
			else
			{
				if ( g_e8ErrorVoltage != (uint8) C_ERR_VOLTAGE_UNDER )
				{
					g_e8ErrorVoltage = (uint8) C_ERR_VOLTAGE_UNDER;				/* 9.5.3.4 */
					g_e8ErrorVoltageComm = g_e8ErrorVoltage;
					SetLastError( (uint8) C_ERR_APPL_UNDER_VOLT);
				}
#if (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44)
				u16DegradeDelay = 0xFFFF;										/* Disable degrade delay timer */
#endif /* (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44) */
			}
		}
		else if ( g_i16MotorVoltage > (NVRAM_APPL_OVOLT + C_VOLTAGE_HYS) )//Over voltage is 19+0.5=19.5V, according to FMEA, set in NVRAM
		{
			/* First time application over-voltage error */
			if ( (l_e8ErrorDebounceFilter & C_DEBFLT_ERR_OV) == 0x00 )
			{
				/* Need twice a over-voltage detection, to avoid ESD-pulses disturbance will cause degraded mode entering */
				l_e8ErrorDebounceFilter |= (uint8) C_DEBFLT_ERR_OV;
			}
			else
			{
				if ( g_e8ErrorVoltage != (uint8) C_ERR_VOLTAGE_OVER )
				{
					g_e8ErrorVoltage = (uint8) C_ERR_VOLTAGE_OVER;				/* 9.5.3.4 */
					g_e8ErrorVoltageComm = g_e8ErrorVoltage;
					SetLastError( (uint8) C_ERR_APPL_OVER_VOLT);
				}
#if (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44)
				u16DegradeDelay = 0xFFFF;										/* Disable degrade delay timer */
#endif /* (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44) */
			}
		}
		//else if ( (g_i16MotorVoltage >= (NVRAM_APPL_UVOLT + C_VOLTAGE_HYS)) && (g_i16SupplyVoltage <= (NVRAM_APPL_OVOLT - C_VOLTAGE_HYS)) )
		else if ( (g_i16MotorVoltage >= (NVRAM_APPL_UVOLT - 10)) && (g_i16SupplyVoltage <= (NVRAM_APPL_OVOLT + 10)) )//Accroding to FMEA, set the voltage to 8.9V & 19.1V
		{
#if (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44)
			if ( g_e8ErrorVoltage != (uint8) C_ERR_VOLTAGE_IN_RANGE )
			{
				/* Voltage error before */
				if ( u16DegradeDelay == 0xFFFF )
				{
					/* Degrade delay timer is disabled */
					if ( g_e8DegradedMotorRequest != (uint8) C_MOTOR_REQUEST_NONE )
					{
						/* Degrade Request is pending; Set Degrade delay time */
						extern uint8 g_u8NAD;
						/* Delay is 100 - 420ms = 100 + (NAD & 0x1F)*10 [ms] */
						u16DegradeDelay = (100 + ((g_u8NAD & 0x1F) * 10)) * PI_TICKS_PER_MILLISECOND;	/* Set Degrade delay timer,depended on NAD */
					}
					else
					{
						/* No Degrade Request pending; Immediate leave Degrade-mode */
						g_e8ErrorVoltage = (uint8) C_ERR_VOLTAGE_IN_RANGE;
					}
				}
				else if ( u16DegradeDelay == 0 )
				{
					/* Degrade delay time-out */
					u16DegradeDelay = 0xFFFF;									/* Disable Degrade Delay timer */

					g_e8ErrorVoltage = (uint8) C_ERR_VOLTAGE_IN_RANGE;			/* Signal voltage-in range */
				}
			}
			l_e8ErrorDebounceFilter &= (uint8) ~(C_DEBFLT_ERR_UV | C_DEBFLT_ERR_OV);
#else  /* (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44) */
			g_e8ErrorVoltage = (uint8) C_ERR_VOLTAGE_IN_RANGE;
			l_e8ErrorDebounceFilter &= (uint8) ~(C_DEBFLT_ERR_UV | C_DEBFLT_ERR_OV);
#endif /* (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44) */		/* MMP150128-1 - End */
		}

#if _SUPPORT_LIN_UV
		if ( NVRAM_LIN_UV != 0 )												/* MMP131216-1 - Begin */
		{
			if ( g_i16MotorVoltage >= (int16) (600 + (NVRAM_LIN_UV * 50)) )
			{
				g_u16LinUVTimeCounter = 0;										/* Stop LIN UV time-counter */
			}
			else if ( g_u16LinUVTimeCounter > PI_TICKS_PER_SECOND )
			{
				/* Restart MLX4 Bus-timeout */
				(void) ml_Disconnect();
				(void) ml_Connect();
				g_u16LinUVTimeCounter = 1;										/* Re-start LIN UV time-counter */
			}
			else if ( g_u16LinUVTimeCounter == 0 )
			{
				g_u16LinUVTimeCounter = 1;										/* Start LIN UV time-counter */
			}
		}																		/* MMP131216-1 - End */
#endif /* _SUPPORT_LIN_UV */



		/* ************************************************************* */
		/* *** f. Chip and ambient temperature (degraded-mode check) *** */
		/* ************************************************************* */
		/* Calculate Chip internal temperature (1LSB/C) [C] */
		GetChipTemperature( FALSE)												/* MMP131020-1 */;
#if _SUPPORT_AMBIENT_TEMP
		SelfHeatCompensation();
		if ( ((g_i16AmbjTemperature > (int16) (NVRAM_APPL_OTEMP + C_TEMPERATURE_HYS)) && ((g_e8MotorStatusMode & C_MOTOR_STATUS_RUNNING) == 0)) ||
#if (_SUPPORT_DIAG_OVT != FALSE)
				(g_i16ChipTemperature > (int16) (C_CHIP_OVERTEMP_LEVEL + C_TEMPERATURE_HYS)) )
#else  /* (_SUPPORT_DIAG_OVT != FALSE) */
			(g_i16ChipTemperature > (int16) (NVRAM_APPL_OTEMP + C_TEMPERATURE_HYS)) )
#endif /* (_SUPPORT_DIAG_OVT != FALSE) */
#else  /* _SUPPORT_AMBIENT_TEMP */
		//if ( ((g_i16ChipTemperature > (int16) (NVRAM_APPL_OTEMP + C_TEMPERATURE_HYS)) && ((g_e8MotorStatusMode & C_MOTOR_STATUS_RUNNING) == 0)) ||
		//	 (g_i16ChipTemperature > C_CHIP_OVERTEMP_LEVEL) )
		//Ban, the over temperate is set to 150 degree, also go to degrade mode
		if ( ((g_i16ChipTemperature > (int16) (NVRAM_APPL_OTEMP + 10)) && ((g_e8MotorStatusMode & C_MOTOR_STATUS_RUNNING) == 0)) || (g_i16ChipTemperature > (int16) C_CHIP_OVERTEMP_LEVEL) )
#endif /* _SUPPORT_AMBIENT_TEMP */
		{
			if ( (l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_OVT) == 0x00 )
			{
				/* Need twice a over-temperature detection, to avoid ESD-pulses disturbance will cause degraded mode entering */
				l_e8ErrorDebounceFilter |= (uint8) C_DEBFLT_ERR_OVT;
			}
			else if ( g_e8ErrorOverTemperature != (uint8) C_ERR_OTEMP_YES )
			{
				g_u8OverTemperatureCount++;
				if ( g_u8OverTemperatureCount >= (uint8) C_OVERTEMP_TO_PERMDEFECT_THRSHLD )
				{
					g_e8ErrorOverTemperature = (uint8) C_ERR_OTEMP_YES;
					SetLastError( (uint8) C_ERR_APPL_OVER_TEMP);
				}
			}
		}
#if _SUPPORT_AMBIENT_TEMP
		else if ( g_i16AmbjTemperature < (int16) (NVRAM_APPL_OTEMP - C_TEMPERATURE_HYS) )
#else  /* _SUPPORT_AMBIENT_TEMP */
			//else if ( g_i16ChipTemperature < (int16) (NVRAM_APPL_OTEMP - C_TEMPERATURE_HYS) )
			else if ( g_i16ChipTemperature < (int16) (NVRAM_APPL_OTEMP) )//Over temp pass criteria is 140 degree
#endif /* _SUPPORT_AMBIENT_TEMP */
			{
				g_e8ErrorOverTemperature = (uint8) C_ERR_OTEMP_NO;
				l_e8ErrorDebounceFilter &= (uint8) ~C_DEBFLT_ERR_OVT;
				g_u8OverTemperatureCount = 0;
			}

		if (g_i16ChipTemperature > (int16) (C_CHIP_WARNING_OVERTEMP_LEVEL + C_TEMPERATURE_HYS))//over temperature waning process
		{
			if ( g_e8WarningOverTemperature != (uint8) C_WARNING_OTEMP_YES )
			{
				g_e8WarningOverTemperature = (uint8) C_WARNING_OTEMP_YES;
			}
		}
		else if(g_i16ChipTemperature < (int16) (C_CHIP_WARNING_OVERTEMP_LEVEL - C_TEMPERATURE_HYS))
		{
			g_e8WarningOverTemperature = (uint8) C_WARNING_OTEMP_NO;
		}



		/* ****************************** */
		/* *** g. Degraded-mode check *** */
		/* ****************************** */
		/* Degraded check */
		if ( ((g_e8ErrorVoltage != (uint8) C_ERR_VOLTAGE_IN_RANGE) || (g_e8ErrorOverTemperature == (uint8) C_ERR_OTEMP_YES)) && ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED) == 0))
		{
			/* Not in degradation state; Stop motor, remember last "request" and enter degradation state */
			if ( g_e8MotorRequest != (uint8) C_MOTOR_REQUEST_NONE )				/* MMP150313-3 - Begin */
			{
				g_e8DegradedMotorRequest = g_e8MotorRequest;
				g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
				MotorDriverStop( (uint16) C_STOP_RAMPDOWN);						/* Degraded-mode (Running) */
			}
			else if ( (g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) != 0 )	/* MMP150313-3 - End */
			{
				if ( g_e8MotorStatusMode == (uint8) C_MOTOR_STATUS_RUNNING )
				{
					g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_START;	/* Set before Diagnostics UV/OV kicks-in */
					MotorDriverStop( (uint16) C_STOP_RAMPDOWN);					/* Degraded-mode (Running) */
				}
				else
				{
					/* "Stopping" */
				}
			}
			else if ( g_e8DegradedMotorRequest == (uint8) C_MOTOR_REQUEST_NONE )
			{
				/* Actuator is stopped, but maybe still powered (holding-mode);
				 * Request to stop without ramp-down */
				g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_STOP;		/* In case non-degraded mode, (optionally) turn on holding-mode */
				MotorDriverStop( (uint16) C_STOP_EMERGENCY);					/* Degraded-mode (MMP150313-2) */
			}
			g_e8MotorStatusMode |= (uint8) C_MOTOR_STATUS_DEGRADED;
		}
		else if ( (g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED) && 
				(g_e8ErrorVoltage == (uint8) C_ERR_VOLTAGE_IN_RANGE) && (g_e8ErrorOverTemperature == (uint8) C_ERR_OTEMP_NO))
		{
			/* No longer degraded mode */
			if ( g_e8DegradedMotorRequest != (uint8) C_MOTOR_REQUEST_NONE ) 
			{
				g_e8MotorRequest = (uint8) g_e8DegradedMotorRequest;
				g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
				if(g_e8CalibrationStep == (uint8) C_CALIB_CHECK_HI_ENDPOS)//go back to continue calibration
				{
					g_e8CalibrationStep = (uint8) C_CALIB_SETUP_HI_ENDPOS;
				}
				else if(g_e8CalibrationStep == (uint8) C_CALIB_CHECK_LO_ENDPOS)
				{
					g_e8CalibrationStep = (uint8) C_CALIB_SETUP_LO_ENDPOS;
				}
			}
			g_e8MotorStatusMode &= (uint8) ~C_MOTOR_STATUS_DEGRADED;
		}

//----------------------------------------------------------------------------------------------------------------

		/* ************************************************* */
		/* *** i. Handling Motor Request (Emergency Run) *** */
		/* ************************************************* */
#if (_SUPPORT_BUSTIMEOUT)
		/* Bus-timeout occurred */
		if ( g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_EMRUN )
		{
			//TODO, where to clear the emergency run flag, emergency run only occurred in initialized stage
			/* Actuator move's towards Emergency Run position */
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_NONE;


#if (((LINPROT & LINXX) == LIN2X) || ((LINPROT & LINXX) == LIN2J))
			if(g_e8CalibrationStep == (uint8) C_CALIB_DONE)
			{
				if ( g_NvramUser.EmergencyRunEndStopHi == 0 )
				{
					/* Move to Low EndStop */
					g_u16TargetPosition = C_EXV_ZERO_POS;
				}
				else
				{
					/* Move to High EndStop */
					g_u16TargetPosition = g_u16CalibTravel;
				}
				if ( g_u16TargetPosition != g_u16ActualPosition)
				{
					/* Only move actuator when not already at position */
					g_u8EmergencyRunOcc = TRUE;
					//g_e8StallDetectorEna = (uint8) C_STALLDET_ALL;
					g_u8StallOcc = FALSE;
					g_u8StallTypeComm &= ~M_STALL_MODE;								/* MMP130916-1 */

					if ( g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED )
					{
						g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_START;
					}
					else
					{
						g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_START;
					}
				}
			}

#if (_SUPPORT_BUSTIMEOUT_SLEEP != FALSE)										/* MMP130815-1 - Begin */
			else
			{
				/* Actuator already at target-position; Just enter SLEEP */
				g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_SLEEP;	
			}
#endif /* (_SUPPORT_BUSTIMEOUT_SLEEP != FALSE) */								/* MMP130815-1 - End */
#endif /* (((LINPROT & LINXX) == LIN2X) || ((LINPROT & LINXX) == LIN2J)) */
		}
#endif /* (_SUPPORT_BUSTIMEOUT) */
		//Ban, Go to sleep after emergency run
		if((g_u8EmergencyRunOcc == TRUE) && (g_u16ActualPosition == g_u16CalibTravel))
		{
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_SLEEP;
		}

//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------

		/* ********************************************************************************************* */
		/* *** j. Handling Motor Request (resp. STOP, INIT, START, CALIBRATION, SLEEP, SPEED-CHANGE) *** */
		/* ********************************************************************************************* */

//		g_e8MotorRequest = C_MOTOR_REQUEST_START;
//		g_e8CalibrationStep = C_CALIB_DONE;

		if ( g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_STOP )
		{
			/* Stop Actuator */
			g_u8RewindFlags &= (uint8) ~(C_REWIND_ACTIVE | C_REWIND_REWIND);
			MotorDriverStop( (uint16) C_STOP_RAMPDOWN);							/* LIN-cmd request */
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
			g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;			/* MMP150313-1 */
			//g_e8CalibrationStep = (uint8) C_CALIB_NONE;
			//g_e8CalibPostMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
		}
#if LIN_COMM
		else if ( (g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_INIT) || (g_e8DegradedMotorRequest == (uint8) C_MOTOR_REQUEST_INIT) )	/* MMP150313-1 */
		{
			/* Actuator initialisation: Set new actual position */
			g_u16ActuatorActPos = g_u16ActualPosition;
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
			g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;			/* MMP150313-1 */
			g_e8CalibrationStep = (uint8) C_CALIB_DONE;
		}
#endif /* LIN_COMM */
		else if ( (g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_START) && (g_u8MotorStartDelay == 0))
		{
			//g_e8EXVMoveEnableRequestFlag = (uint8) C_EXV_MOVE_DISABLE;//make sure just run once
			if((g_u8EmergencyRunOcc == FALSE) && (g_e8EXVMoveEnableRequestFlag == (uint8) C_EXV_MOVE_ENABLE))
			{
#if _SUPPORT_ENDSTOP_DETECTION
				if(g_u16EXVTargetPositionTemp == 0x00)//move to end stop
				{
					g_u16TargetPosition = C_EXV_ZERO_POS + g_u16CalibTravel - C_EXV_RANGE_MAX;
				}
				else if(g_u16EXVTargetPositionTemp == 0xFF)//move to end stop
				{
					g_u16TargetPosition = C_EXV_ZERO_POS + C_EXV_RANGE_MAX;
				}
				else
#endif
				{
					g_u16TargetPosition = (((uint32)g_u16EXVTargetPositionTemp)*C_EXV_POSITION_STD+512)/1023 + C_EXV_ZERO_POS;//update the target position
				}
			}
			/* Start Actuator */
			if (( g_u16ActualPosition != g_u16TargetPosition ) && (g_e8CalibrationStep == C_CALIB_DONE))
			{
				uint8 u8NewMotorDirectionCCW;
#if LIN_COMM
				UpdateMotorSpeed();//TODO, when accelerate or Decelerate, may changed the speed
#else  /* LIN_COMM */
				g_u8MotorStatusSpeed = (uint8) C_MOTOR_SPEED_2;
				g_u16MotorSpeedRPS = g_au16MotorSpeedRPS[C_MOTOR_SPEED_2];		/* Use Speed #2 as default */
				g_u16TargetCommutTimerPeriod = g_au16MotorSpeedCommutTimerPeriod[C_MOTOR_SPEED_2];
#endif /* LIN_COMM */

				u8NewMotorDirectionCCW = (g_u16TargetPosition < g_u16ActualPosition) ? TRUE : FALSE;

#if FALSE
				if ( g_u8RewindFlags & (uint8) C_REWIND_STALL_DETECT )
				{
					if ( (NVRAM_REWIND_STEPS != 0) &&							/* MMP140331-1 */
							(((g_u8RewindFlags & (uint8) C_REWIND_DIRECTION_CCW) == u8NewMotorDirectionCCW) || (g_u8RewindFlags & C_REWIND_DIRECTION_AUTO)) )
					{
						/* Start rewind-function, with "rewinding" */
						g_u8RewindFlags = (uint8) (C_REWIND_ACTIVE | C_REWIND_REWIND);	/* Start rewind-process (MMP140331-1) */
						g_u16TargetPositionRewind = g_u16TargetPosition;
						if ( u8NewMotorDirectionCCW )
						{
							if ( g_u16ActualPosition < (uint16) (C_MAX_POS - NVRAM_REWIND_STEPS) )
							{
								g_u16TargetPosition = g_u16ActualPosition + NVRAM_REWIND_STEPS;
								u8NewMotorDirectionCCW = FALSE;					/* MMP140331-3 */
							}
							else
							{
								g_u8RewindFlags = 0;							/* No rewind possible */
							}
						}
						else
						{
							if ( g_u16ActualPosition > NVRAM_REWIND_STEPS )
							{
								g_u16TargetPosition = g_u16ActualPosition - NVRAM_REWIND_STEPS;
								u8NewMotorDirectionCCW = TRUE;					/* MMP140331-3 */
							}
							else
							{
								g_u8RewindFlags = 0;							/* No rewind possible */
							}
						}
					}
					else if ( (g_u8RewindFlags & (uint8) C_REWIND_DIRECTION_CCW) != u8NewMotorDirectionCCW ) /* MMP140331-1 - Begin */
					{
						g_u8RewindFlags = 0;									/* Clear previous detected stall flags */
					}															/* MMP140331-1 - End */
				}
#endif

				if ( g_e8MotorStatusMode == (uint8) C_MOTOR_STATUS_STOP )
				{
					g_e8MotorDirectionCCW = u8NewMotorDirectionCCW;
					MotorDriverStart();
				}
				else if ( u8NewMotorDirectionCCW != g_e8MotorDirectionCCW )
				{
					/* Changing direction; Stop motor first before starting in opposite direction */
					MotorDriverStop( (uint16) C_STOP_RAMPDOWN);					/* Change of direction */
					continue;
				}
				else
				{
					g_u16ActuatorTgtPos = g_u16TargetPosition;					/* Motor already started; Update target-position only */
				}
			}
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
		}
		else if ( (g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_CALIBRATION) && (g_e8EXVMoveEnableRequestFlag == (uint8) C_EXV_MOVE_ENABLE) )
		{
			//TODO, ban, how about emergency run?
			//g_e8EXVMoveEnableRequestFlag = (uint8) C_EXV_MOVE_DISABLE;//make sure just run once
			if ( g_e8CalibrationStep == (uint8) C_CALIB_START )//not initialized, case 00
			{
				g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_NO;
				g_e8ErrorCoil = 0;//Clean coil status because this is not updated until error happened, need to debug(ban debug)
				g_u8StallOcc = FALSE;
				g_e8EXVErrorBlock = FALSE;
				g_e8ErrorOverTemperature = (uint8) C_ERR_OTEMP_NO;

				g_e8CalibrationStep = (uint8) C_CALIB_SETUP_HI_ENDPOS;
				if(g_u16EXVTargetPositionTemp == (uint16)C_EXV_FULLY_OPEN_LIN)//Move to Full open, 100% first
				{
					g_u16ActualPosition = (uint16)C_EXV_ZERO_POS;
					g_u16TargetPosition = (uint16)C_EXV_RANGE_MAX + C_EXV_ZERO_POS;
					l_e8GmcvInitDirection = (uint8)C_GMCV_INIT_DIR_OPEN_FIRST;
				}

				if ( g_e8MotorStatusMode != (uint8) C_MOTOR_STATUS_STOP ) //stop the motor if it is running
				{
					MotorDriverStop( (uint16) C_STOP_IMMEDIATE);
					g_u16CalibPauseCounter = C_PI_TICKS_STABILISE_CALIB;
				}
			}

			if ( (g_e8CalibrationStep == (uint8) C_CALIB_SETUP_HI_ENDPOS) && (g_u16CalibPauseCounter == 0) )//moving to full open
			{
				UpdateMotorSpeed();
				g_e8MotorDirectionCCW = (g_u16TargetPosition < g_u16ActualPosition) ? (uint8) C_MOTOR_DIR_CLOSING : (uint8) C_MOTOR_DIR_OPENING;
				MotorDriverStart();
				g_e8CalibrationStep = (uint8) C_CALIB_CHECK_HI_ENDPOS;		/* Check for FIRST End-stop */
			}
			else if ( g_e8CalibrationStep == (uint8) C_CALIB_CHECK_HI_ENDPOS )//At the full open
			{
				if ( g_u8StallOcc || (g_u16TargetPosition == g_u16ActualPosition))//Ban
				{
					g_e8CalibrationStep = (uint8) C_CALIB_SETUP_LO_ENDPOS;
					g_u16CalibPauseCounter = C_PI_TICKS_STABILISE_CALIB;
					g_u8StallOcc = FALSE;
					g_u8StallTypeComm &= ~M_STALL_MODE;
				}
			}
			else if ( (g_e8CalibrationStep == (uint8) C_CALIB_SETUP_LO_ENDPOS) && (g_u16CalibPauseCounter == 0) )
			{
				UpdateMotorSpeed();
				g_u16ActualPosition = C_EXV_RANGE_MAX + C_EXV_ZERO_POS;
				g_u16TargetPosition = C_EXV_ZERO_POS;
				g_e8MotorDirectionCCW = (g_u16TargetPosition < g_u16ActualPosition) ? C_MOTOR_DIR_CLOSING : C_MOTOR_DIR_OPENING;
				MotorDriverStart();
				g_e8CalibrationStep = (uint8) C_CALIB_CHECK_LO_ENDPOS;		/* Check for SECOND End-stop */
			}
			else if ( g_e8CalibrationStep == (uint8) C_CALIB_CHECK_LO_ENDPOS )//////////////////////////////
			{
				if ( g_u8StallOcc ||  (g_u16TargetPosition == g_u16ActualPosition))
				{
					g_u16CalibTravel = (C_EXV_RANGE_MAX + C_EXV_ZERO_POS) - g_u16ActualPosition;
					if((g_u16CalibTravel < C_EXV_POSITION_STD) || (g_u16CalibTravel >= C_EXV_RANGE_MAX))//unexpect stall, under range or over range, TODO
					{
						g_e8CalibrationStep = C_CALIB_NONE;
						g_e8EXVErrorBlock = TRUE;
						g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;// Ban, stop the motor in case of unexpect stall
					}
					else
					{
						g_e8CalibrationStep = (uint8) C_CALIB_END;
					}
				}
			}
			if(g_e8CalibrationStep == (uint8) C_CALIB_END)
			{
				g_u8StallOcc = FALSE;
				g_u8StallTypeComm &= ~M_STALL_MODE;

				g_u16ActualPosition = C_EXV_ZERO_POS;
				g_u16TargetPosition = C_EXV_ZERO_POS;

				g_u16ActuatorActPos = g_u16ActualPosition;
				g_u16ActuatorTgtPos = g_u16TargetPosition;

				g_e8CalibrationStep = C_CALIB_DONE;
				g_u8MotorStartDelay = 255;	//C_PI_TICKS_STABILISE_CALIB;
				g_e8MotorRequest = C_MOTOR_REQUEST_NONE;
				g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
				//TODO, the post motor request, related to emergency run
				g_e8CalibPostMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
			}
		}
		else if ( (g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_SLEEP) || (g_e8DegradedMotorRequest == (uint8) C_MOTOR_REQUEST_SLEEP) )	/* MMP150313-1 */
		{
			/* Actuator enters (Deep-)SLEEP mode (lowest power mode; Only LIN/PWM msg can wake-up) */
			if ( ((g_e8MotorStatusMode & ~C_MOTOR_STATUS_DEGRADED) == (uint8) C_MOTOR_STATUS_STOP) && /* MMP130730-1 */
					(g_u8MotorStopDelay == 0) &&
					((NV_CTRL & NV_BUSY) == 0) ) /* MMP140812-3 */
			{
				if ( g_u8MotorHoldingCurrEna &&									/* Holding mode enabled */
						(g_e8ErrorElectric != (uint8) C_ERR_ELECTRIC_PERM) && (g_e8ErrorVoltage == (uint8) C_ERR_VOLTAGE_IN_RANGE) )
				{
					MotorDriverStop( (uint16) C_STOP_SLEEP);					/* Disable holding current */
				}

#if (LINPROT == LIN2J_VALVE_GM)
				/* Before the actuator enters in Sleep, it saves in EEPROM the CPOS,
				 * the Status and the NAD only if the value of cells is different as the RAM value.
				 */
				LIN_SAE_J2602_Store();											/* MMP160613-2 */
#endif /* (LINPROT == LIN2J_VALVE_GM) */
				/* MMP140812-4 - Begin */
				/* Put the chip in it's Sleep Mode by fulfilling following condition:
				 *                     _
				 * MLX16_HALTED ------| \
				 *  MLX4_HALTED ------|  \____ goto sleep
				 *     ADC_BUSY -----o|  /
				 *      NV_BUSY -----o|_/
				 *
				 * Internal power supplies will be switched off and chip can only be woken up by one
				 * of the following wake up sources:
				 * - LIN bus
				 * - IO3
				 * - Internal timer
				 * Wake up is done by resetting the chip. The source of the wake up can be found in ANA_INB.
				 * Note: In case between MLX4_HALT and MLX16_HALT a LIN-bus event happens, the chip will not
				 * enter SLEEP but HALT-mode. To allow a LIN-wakeup, the Analogue Watchdog is set to minimum
				 * period of 100us to allow a chip reset.
				 */
				IO_WU = 0;														/* Disable IO3 wake up */
				ANA_OUTG = ANA_OUTG & 0xFF9E;									/* Clear Internal WU delay and DIS_GTSM */
				MASK = 0;
				ADC_Stop();														/* MMP140812-4 - End */
				/* Go into sleep/halt */
				AWD_CTRL = (3u << 8) | 1;										/* Set 1:1 prescaler and minimal period; AWD timeout will be 100 us (MMP140813-2) */
				MLX4_RESET();
				MLX16_HALT();													/* See MELEXIS doc */
				/* Chip should reset upon LIN bus changes */
				/* We should never make it to here, as a backup we add a chip reset */
				MLX16_RESET();
			}
			else if ( g_e8MotorStatusMode != (uint8) C_MOTOR_STATUS_STOP )
			{
				MotorDriverStop( (uint16) C_STOP_IMMEDIATE);	
			}
		}
#if LIN_COMM
		else if ( (g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_SPEED_CHANGE) && (g_e8MotorStatusMode == (uint8) C_MOTOR_STATUS_RUNNING) )
		{
			UpdateMotorSpeed();
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
		}
#endif /* LIN_COMM */

//--------------------------------------------------------------------------------------------------------------------------------------------

		/* ************************ */
		/* *** k. Status update *** */
		/* ************************ */
#if _SUPPORT_SPEED_AUTO
		/* Speed adaption (Auto-speed); Only if not degraded and not stopping */
		if ( (g_e8MotorStatusMode == (uint8) C_MOTOR_STATUS_RUNNING) && (g_u8MotorCtrlSpeed == (uint8) C_MOTOR_SPEED_AUTO) )
		{
			uint8 u8MotorSpeedIdx = g_u8MotorStatusSpeed;
#if _SUPPORT_AMBIENT_TEMP
			if ( (g_i16AmbjTemperature < NVRAM_AUTOSPEED_TEMP_1) || (g_i16MotorVoltage < NVRAM_AUTOSPEED_VOLT_1) )
#else  /* _SUPPORT_AMBIENT_TEMP */
				if ( (g_i16ChipTemperature < NVRAM_AUTOSPEED_TEMP_1) || (g_i16MotorVoltage < NVRAM_AUTOSPEED_VOLT_1) )
#endif /* _SUPPORT_AMBIENT_TEMP */
				{
					/* Temperature below TEMP_1 and/or voltage below VOLT_1 */
					u8MotorSpeedIdx = (uint8) C_MOTOR_SPEED_1;						/* Speed #1 */
				}
#if _SUPPORT_AMBIENT_TEMP
				else if ( (g_i16AmbjTemperature < NVRAM_AUTOSPEED_TEMP_2) || (g_i16AmbjTemperature > NVRAM_AUTOSPEED_TEMP_3) || (g_i16MotorVoltage < NVRAM_AUTOSPEED_VOLT_2) )
#else  /* _SUPPORT_AMBIENT_TEMP */
					else if ( (g_i16ChipTemperature < NVRAM_AUTOSPEED_TEMP_2) || (g_i16ChipTemperature > NVRAM_AUTOSPEED_TEMP_3) || (g_i16MotorVoltage < NVRAM_AUTOSPEED_VOLT_2) )
#endif /* _SUPPORT_AMBIENT_TEMP */
					{
						/* Temperature below TEMP_2 or above TEMP_3 or voltage below VOLT_2 */
#if _SUPPORT_AMBIENT_TEMP
						if ( (g_u8MotorStatusSpeed > (uint8) C_MOTOR_SPEED_2) || ((g_u8MotorStatusSpeed == (uint8) C_MOTOR_SPEED_1) && (g_i16AmbjTemperature > (NVRAM_AUTOSPEED_TEMP_1 + 3)) && (g_i16MotorVoltage > (NVRAM_AUTOSPEED_VOLT_1 + 25))) )
#else  /* _SUPPORT_AMBIENT_TEMP */
							if ( (g_u8MotorStatusSpeed > (uint8) C_MOTOR_SPEED_2) || ((g_u8MotorStatusSpeed == (uint8) C_MOTOR_SPEED_1) && (g_i16ChipTemperature > (NVRAM_AUTOSPEED_TEMP_1 + 3)) && (g_i16MotorVoltage > (NVRAM_AUTOSPEED_VOLT_1 + 25))) )
#endif /* _SUPPORT_AMBIENT_TEMP */
							{
								/* Speed index above 2 or Speed index 1 and temperature above TEMP_1 + 3 degrees and voltage above VOLT_1 + 25mV */
								u8MotorSpeedIdx = (uint8) C_MOTOR_SPEED_2;					/* Speed #2 */
							}
					}
#if _SUPPORT_AMBIENT_TEMP
					else if ( (g_i16AmbjTemperature > (NVRAM_AUTOSPEED_TEMP_2 + 3)) && (g_i16AmbjTemperature < (NVRAM_AUTOSPEED_TEMP_3 - 3)) && (g_i16MotorVoltage > (NVRAM_AUTOSPEED_VOLT_2 + 25)) )
#else  /* _SUPPORT_AMBIENT_TEMP */
						else if ( (g_i16ChipTemperature > (NVRAM_AUTOSPEED_TEMP_2 + 3)) && (g_i16ChipTemperature < (NVRAM_AUTOSPEED_TEMP_3 - 3)) && (g_i16MotorVoltage > (NVRAM_AUTOSPEED_VOLT_2 + 25)) )
#endif /* _SUPPORT_AMBIENT_TEMP */
						{
							/* temperature above TEMP_2 + 3 degrees and below TEMP_3 - 3 degrees and voltage above VOLT_2 + 25mV */
							u8MotorSpeedIdx = (uint8) C_MOTOR_SPEED_4;						/* Speed #4 */
						}
			if ( g_u8MotorStatusSpeed != u8MotorSpeedIdx )
			{
				g_u8MotorStatusSpeed = u8MotorSpeedIdx;
				g_u16MotorSpeedRPS = g_au16MotorSpeedRPS[u8MotorSpeedIdx];
				g_u16TargetCommutTimerPeriod = g_au16MotorSpeedCommutTimerPeriod[u8MotorSpeedIdx];
			}
		}
#endif /* _SUPPORT_SPEED_AUTO */

		/* Update status actual-position (only incase not the initial position have been changed) */
		//if ( ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) != 0) && ((g_u8RewindFlags & (uint8) C_REWIND_ACTIVE) == 0) )				/* MMP130626-4 */
		if ((g_u8RewindFlags & (uint8) C_REWIND_ACTIVE) == 0)//Ban, fix the move to 0 bug, because it stopped,but g_u16ActuatorActPos = 0,g_u16ActualPosition is not zero
		{
			g_u16ActualPosition = g_u16ActuatorActPos;
		}

//--------------------------------------------------------------------------------------------------------------------

		/* ********************************************************** */
		/* *** l. Threshold control (Stepper: Current-threshold) *** */
		/* ********************************************************** */
		ThresholdControl();													

//------------------------------------------------------------------------------------------------------------------

		/* ************************************************* */
		/* *** m. PID control (Stepper: current-control) *** */
		/* ************************************************* */
		PID_Control();															/* PID-control (Current) */

//----------------------------------------------------------------------------------------------------------------------------

		/* ********************** */
		/* *** n. MLX4 status *** */
		/* ********************** */
		{
			uint16 u16Mlx4CounterThreshold = C_MLX4_STATE_TIMEOUT;				/* MMP130905-4 - Begin */
#if _SUPPORT_LIN_BUS_ACTIVITY_CHECK && (__MLX_PLTF_VERSION_MAJOR__ == 4)
			{
				/* MLX4 LIN-Bus activity check when not in LIN-AA mode (only __MLX_PLTF_VERSION_MAJOR__ == 4) */
				if ( (ml_GetState( ML_NOT_CLEAR) != ml_stINVALID) && ((LinStatus & ML_LIN_BUS_ACTIVITY) != 0) )
				{
					/* MLX4 has detected a SYNC field */
					g_u16Mlx4StateCheckCounter = 0;
					g_u8ErrorCommBusTimeout = FALSE;
					(void) ml_GetState( ML_CLR_LIN_BUS_ACTIVITY);
				}
				else
				{
					g_u16Mlx4StateCheckCounter++;								/* State check counter */
				}
			}
#endif /* _SUPPORT_LIN_BUS_ACTIVITY_CHECK && (__MLX_PLTF_VERSION_MAJOR__ == 4) */

			if ( (g_u16Mlx4StateCheckCounter >= u16Mlx4CounterThreshold) || ((g_u8Mlx4ErrorState & (uint8) C_MLX4_STATE_IMMEDIATE_RST) != 0) ) /* MMP130905-4 - End */
			{
				/* Didn't receive MLX4 LIN command and/or data-request in the last period, or need immediate reset */
				g_u16Mlx4StateCheckCounter = 0;										/* MLX4 State check counter reset; MLX4 still active */
				if ( ((g_u8Mlx4ErrorState & (uint8) C_MLX4_STATE_IMMEDIATE_RST) != 0) ||
						( ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_APPL_STOP) == 0x00)
								&& (g_e8MotorRequest != (uint8) C_MOTOR_REQUEST_SLEEP)
#if (__MLX_PLTF_VERSION_MAJOR__ == 4)
								&& (ml_GetState( ML_NOT_CLEAR) == ml_stINVALID) ) )		/* MMP130811-1 */
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 4) */
#if (__MLX_PLTF_VERSION_MAJOR__ == 3)
					&& (ml_GetState() == ml_stINVALID) ) )					/* MMP130811-1 */
#endif /*((__MLX_PLTF_VERSION_MAJOR__ == 3) */
		{
			g_u8Mlx4ErrorState++;
			if ( g_u8Mlx4ErrorState >= (uint8) C_MLX4_STATE_ERROR_THRSHLD )
			{
				/* Signal Error; Reset MLX4 */
				MLX4_RESET();
				NOP();
				NOP();
				NOP();
				MLX4_START();
				if ( (g_u8Mlx4ErrorState & C_MLX4_STATE_NOT_LOGGED) == 0 )	/* MMP131126-1 */
				{
					SetLastError( (uint8) C_ERR_MLX4_RESTART);
				}
				LIN_Init( TRUE);										/* Re-initialise LIN interface w/o changing position */
				g_u8Mlx4ErrorState = 0;
			}
		}
		else
		{
			g_u8Mlx4ErrorState = 0;
		}
			}
		}

//--------------------------------------------------------------------------------------------------------------------------------

		/* ********************************** */
		/* *** o. Background System check *** */
		/* ********************************** */
		if ( (u8BackgroundSchedulerTaskID == 0) || (u8BackgroundSchedulerTaskID == 128) )
		{
#if (LINPROT == LIN2J_VALVE_GM)
			if ( RamBackgroundTest( 0) == FALSE )								/* Check RAM against NVRAM User-page */
#else  /* (LINPROT == LIN2J_VALVE_GM) */
				if ( RamBackgroundTest( u8BackgroundSchedulerTaskID ? 1 : 0) == FALSE )	/* Check RAM against NVRAM User-page #1/#2 */
#endif /* (LINPROT == LIN2J_VALVE_GM) */
				{
					/* MMP150925-1: RAM g_NvramUser structure not same as NVRAM Page #1.1.
					 * Either System RAM is corrupted or the NVRAM. Allow one time NVRAM reload */
					if ( l_u8RamPreError == FALSE )
					{
						NVRAM_LoadAll();
						l_u8RamPreError = TRUE;
					}
					else
					{
						SetLastError( (uint8) C_ERR_RAM_BG);						/* Log RAM failure */
#if (LINPROT == LIN2J_VALVE_GM)
						MLX4_RESET();												/* Reset the Mlx4   */
						bistResetInfo = C_CHIP_STATE_LIN_CMD_RESET;
						MLX16_RESET();												/* Reset the Mlx16  */
#else  /* (LINPROT == LIN2J_VALVE_GM) */
						g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;			/* Permanent electric failure */
#endif /* (LINPROT == LIN2J_VALVE_GM) */
					}
				}
				else
				{
					l_u8RamPreError = FALSE;										/* Error is gone (caused by wrong NVRAM shadow-RAM) */
				}
		}
		else if ( (FL_CTRL0 & FL_DETECT) != 0 )									/* MMP150603-2 */
		{
			if ( FlashBackgroundTest( C_FLASH_SEGMENT_SZ) == C_FLASH_CRC_FAILED )	/* Check Flash/ROM Memory Checksum (max. 250us) */
			{
				SetLastError( (uint8) C_ERR_FLASH_BG);
#if (LINPROT == LIN2J_VALVE_GM)
				MLX4_RESET();													/* Reset the Mlx4   */
				bistResetInfo = C_CHIP_STATE_LIN_CMD_RESET;
				MLX16_RESET();													/* Reset the Mlx16  */
#endif /* (LINPROT == LIN2J_VALVE_GM) */
			}
		}
		u8BackgroundSchedulerTaskID++; 

//----------------------------------------------------------------------------------------------------------------------------------

		/* *********************************************** */
		/* *** p. Motor-phase shortage to ground check *** */
		/* *********************************************** */
#if _SUPPORT_PHASE_SHORT_DET
		if ( (g_e8ErrorElectric != (uint8) C_ERR_ELECTRIC_PERM) && (NVRAM_VDS_THRESHOLD != 0) &&
				(((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) && (g_u16StartupDelay == 0)) ||
						(g_u8MotorHoldingCurrState != FALSE)) )
		{
			GetPhaseMotor();
			if ( g_i16PhaseVoltage < (int16) (g_u16MotorVoltage - NVRAM_VDS_THRESHOLD) )
			{
				/* Phase-voltage is more then 2V below motor-driver voltage */
				if ( (l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_PHASE_SHORT) == 0x00 )
				{
					l_e8ErrorDebounceFilter |= (uint8) C_DEBFLT_ERR_PHASE_SHORT;
				}
				else
				{
					g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
					MotorDriverStop( (uint16) C_STOP_IMMEDIATE);				/* Short between motor-phase and ground */
					SetLastError( C_ERR_PHASE_SHORT_GND);
				}
			}
			else
			{
				l_e8ErrorDebounceFilter &= (uint8) ~C_DEBFLT_ERR_PHASE_SHORT;
			}
		}
#endif /* _SUPPORT_PHASE_SHORT_DET */

#if _SUPPORT_CHIP_TEMP_PROFILE
		/* ***************************************************** */
		/* *** q. Chip temperature stability (profile) check *** */
		/* ***************************************************** */
		if ( g_u16TemperatureStabilityCounter > C_TEMP_STABIL_TIMEOUT )
		{
			g_u16TemperatureStabilityCounter -= C_TEMP_STABIL_TIMEOUT;

			if ( l_i16ChipTemperaturePrev != 0 )
			{
				/* Chip temperature profile check: 
				 * Take the delta between latest and previous measured chip temperature (differentiate)
				 * Integrate the delta, with a leakage of 1/2^n
				 */
				int i16DeltaTemperature = g_i16ChipTemperature - l_i16ChipTemperaturePrev;
				l_i16ChipTemperatureInt = (l_i16ChipTemperatureInt - ((l_i16ChipTemperatureInt + (1 << (C_TEMP_STABIL_INT_FILTER_COEF - 1))) >> C_TEMP_STABIL_INT_FILTER_COEF)) + (i16DeltaTemperature << C_TEMP_STABIL_INT_FILTER_COEF);
				if ( l_i16ChipTemperatureInt < 0 )
				{
					l_i16ChipTemperatureInt = 0;
				}
				else if ( l_i16ChipTemperatureInt > (int16)(C_TEMP_STABIL_THRESHOLD << C_TEMP_STABIL_INT_FILTER_COEF) )
				{
					if ( (l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_TEMP_PROFILE) == 0x00 )
					{
						l_e8ErrorDebounceFilter |= (uint8) C_DEBFLT_ERR_TEMP_PROFILE;
					}
					else
					{
						g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
						MotorDriverStop( (uint16) C_STOP_IMMEDIATE);			/* Temperature increase dT/dt too much */
						SetLastError( C_ERR_CHIP_TEMP_PROFILE);					
					}
				}
				else
				{
					l_e8ErrorDebounceFilter &= (uint8) ~C_DEBFLT_ERR_TEMP_PROFILE;
				}
			}
			l_i16ChipTemperaturePrev = g_i16ChipTemperature;
		}
#endif /* _SUPPORT_CHIP_TEMP_PROFILE */

		/* ****************************** */
		/* *** (Optional) SPI support *** */
		/* ****************************** */

#if _SUPPORT_MLX16_HALT
		/* ************************************* */
		/* *** r. Power-saving (non-running) *** */
		/* ************************************* */
		/* In case MLX4 is inactive, don't enter HALT mode, because the chip enters SLEEP mode which will stop the Core-timer as well */
		if ( ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) == 0) && (g_u8MotorStopDelay == 0) && ((CONTROL & M4_RB) != 0) && (g_u8LinInFrameBufState == C_LIN_IN_FREE) )
		{
			uint16 u16XI0_Mask = XI0_MASK;
			uint16 u16IsrMask = MASK;
			uint16 u16Timer1Ctrl = TMR1_CTRL;

			ADC_PowerOff();														/* Stop ADC, including turning off reference voltage (Approx: 0.6mA) */
			DRVCFG |= DIS_SHOA;													/* Disable OpAmp for ADC measurement of shunt current (Approx: 0.6mA) */

			/* Setup wake-up timer event */
			TMR1_CTRL = (2 * TMRx_DIV0) | (0 * TMRx_MODE0) | TMRx_T_EBLK;		/* Timer mode 0, Divider 256 */
			TMR1_REGB = C_SLEEP_PERIOD;											/* Set sleep-period */
			XI0_PEND = CLR_T1_INT4;												/* Clear (potentially) Timer1 second level interrupts (T1_INT4) */
			XI0_MASK = EN_T1_INT4;												/* Disable Timer1 all 2nd level interrupts, except INT4 (CMP) */
			PEND = CLR_EXT0_IT;
			MASK = EN_EXT4_IT | EN_EXT0_IT | EN_M4_SHE_IT;						/* Enable Diagnostics, Timer1 and MLX4 IRQ's only */
			TMR1_CTRL = (2 * TMRx_DIV0) | (0 * TMRx_MODE0) | TMRx_T_EBLK | TMRx_START;	/* Start timer */

			/* Go into sleep/halt */
#if 0
			if ( g_u8LinInFrameBufState == C_LIN_IN_FREE )						/* Double check again for any received LIN commands */
			{
				MLX16_HALT();													/* See MELEXIS doc */
			}
#endif /* 0 */
			/* The above code fails as the LIN message is received just after
			 * the LinInFrameBufState check and before the actual entering of
			 * the HALT-state. The LIN message is not lost, but delayed by the
			 * HALT time-out period. Below code blocks the IRQ's (post-pone)
			 * until the MLX16 enters HALT-state.
			 */
			__asm__("psup #0");													/* Block IRQ's */
			__asm__("lod AL, _g_u8LinInFrameBufState");							/* Check for LIN message received */
			__asm__("jne _HALT_10");											/* Skip HALT in case LIN message received */
			__asm__("lod X, #_au16HaltZero");									/* X pointer to flash memory with 0x0000 (MMP150224-2) */
			__asm__("lod AL, 0x2000");											/* Get MLX16 Control-state */
			__asm__("or  AL, #0x02");											/* Set HALT-state */
			__asm__("mov R, #0");												/* Restore IRQ-state */
			__asm__("pop M");
			__asm__("mov 0x2000, AL");											/* Enter HALT-state */
			__asm__("mov A,[X]");												/* MMP150224-2 */
			__asm__("jmp _HALT_20");											/* Leave HALT-state */
			__asm__("_HALT_10:");
			__asm__("mov R, #0");
			__asm__("pop M");
			__asm__("_HALT_20:");

			DRVCFG &= ~DIS_SHOA;												/* Enable OpAmp for ADC measurement of shunt current (Approx: 0.6mA) */

			{
				uint16 u16TimerCnt = TMR1_CNT;									/* Take a copy of the Timer-count value */
				TMR1_CTRL = u16Timer1Ctrl & ~TMRx_START;						/* Stop Timer */
				XI0_PEND = u16XI0_Mask;											/* Clear (potentially) Timer1 second level interrupts (T1_INT4) */
				XI0_MASK = u16XI0_Mask;											/* Restore Timer1 interrupt mask */
				PEND = CLR_EXT0_IT;
				MASK = u16IsrMask;												/* Restore 1st level interrupt mask */

				if ( g_u8MotorHoldingCurrState != FALSE )
				{
					/* Start ADC I, V, T measurement, using Motor-PWM trigger source */
#if _SUPPORT_PHASE_SHORT_DET
					if ( l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_PHASE_SHORT )
					{
						/* Check next motor-phase; 1 (U), 2 (T), 3 (W) and 4 (V); 0 = motor-phase based on micro-step index */
						if (  l_u16AdcHoldMode >= 4 )
						{
							l_u16AdcHoldMode = 1;
						}
						else
						{
							l_u16AdcHoldMode = (l_u16AdcHoldMode + 1);
						}
					}
					ADC_Start( l_u16AdcHoldMode);
#else  /* _SUPPORT_PHASE_SHORT_DET */
					ADC_Start();
#endif /* _SUPPORT_PHASE_SHORT_DET */
				}
				Timer_SleepCompensation( u16TimerCnt);							/* Compensate Timer counters for sleep-period */
			}
		}
#endif /* _SUPPORT_MLX16_HALT */

#if _SUPPORT_IOREG_CHECK
		/* ************************************ */
		/* *** s. Critical peripheral check *** */
		/* ************************************ */
		if ( (g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_APPL_STOP) == 0x00 )	/* If application is NOT stopped ... */
		{
			/* Check: Motor commutation timer disabled */
			if ( (TMR1_CTRL & TMRx_T_EBLK) == 0 )
			{
				/* Communication timer is disabled; Motor is stopped too */
				if ( g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING )
				{
					TMR1_CTRL = C_TMRx_CTRL_MODE0 | TMRx_START;					/* Start timer mode 0 */
				}
				else
				{
					TMR1_CTRL = C_TMRx_CTRL_MODE0;								/* Timer mode 0 */
				}
				SetLastError( (uint8) C_ERR_IOREG);
			}
			/* Check: Administrative timer disabled */
			if ( (TIMER & TMR_EN) == 0 )							
			{
				TIMER = TMR_EN | CT_PERIODIC_RATE;
				SetLastError( (uint8) C_ERR_IOREG);
			}
			/* Check: IRQ-Mask (Respectively: Diagnostics, Timer1, CoreTimer and LIN-Communication */
			if ( (MASK & (EN_EXT4_IT | EN_EXT0_IT | EN_TIMER_IT | EN_M4_SHE_IT)) != (EN_EXT4_IT | EN_EXT0_IT | EN_TIMER_IT | EN_M4_SHE_IT) )
			{
				PEND = (EN_EXT4_IT | EN_EXT0_IT | EN_TIMER_IT | EN_M4_SHE_IT);
				MASK |= (EN_EXT4_IT | EN_EXT0_IT | EN_TIMER_IT | EN_M4_SHE_IT);
				SetLastError( (uint8) C_ERR_IOREG);
			}
			/* Check: IRQ-priority (Respectively: Diagnostics, Timer1, CoreTimer) */
			if ( (PRIO & ((3u << 14) | (3u << 6) | (3u << 0))) != (/*((3-3) << 14) |*/ ((4-3) << 6) | ((6-3) << 0)) )
			{
				PRIO = (PRIO & ~((3u << 14) | (3u << 6) | (3u << 0))) | (/*((3-3) << 14) |*/ ((4-3) << 6) | ((6-3) << 0));
				SetLastError( (uint8) C_ERR_IOREG);
			}
			/* Check: 2nd level IRQ Timer1 */
			if ( (XI0_MASK & EN_T1_INT4) == 0 )
			{
				XI0_PEND = EN_T1_INT4;
				XI0_MASK = EN_T1_INT4;
				SetLastError( (uint8) C_ERR_IOREG);
			}
			/* Check: 2nd level IRQ Diagnostics */
			if ( (XI4_MASK & (XI4_OVT | XI4_UV | XI4_OV | XI4_OC_DRV)) != C_DIAG_MASK )
			{
				XI4_PEND = C_DIAG_MASK;											/* MMP150409-1 */
				XI4_MASK = C_DIAG_MASK;											/* MMP150409-1 */
				SetLastError( (uint8) C_ERR_IOREG);
			}
#if 0
			if ( (ADC_DBASE < (uint16) &g_AdcMotorRunStepper4) && (ADC_DBASE >= (uint16) &g_AdcMotorRunStepper4 + 1) )
			{
				ADC_DBASE = (uint16) &g_AdcMotorRunStepper4.UnfilteredDriverCurrent;
				SetLastError( (uint8) C_ERR_IOREG);
			}
#endif /* 0 */
			if ( ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) != 0) && ((DRVCFG & (DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U)) == 0) )
			{
				/* Driver have been disabled */
				SetLastError( (uint8) C_ERR_IOREG);
			}
		}
#endif /* _SUPPORT_IOREG_CHECK */

		//DiagnosticCheck();//Ban
		RteExv2Lin();

	}																			/* Application loop */

} /* End of main() */

/* EOF */
