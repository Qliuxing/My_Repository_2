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
volatile uint8 g_e8ErrorVoltage = (uint8) C_ERR_VOLTAGE_IN_RANGE;				/* Status-flags voltage */
uint8 g_e8MotorDirectionCCW = (uint8) C_MOTOR_DIR_UNKNOWN;						/* Control/Status-flag motor rotational direction Counter Clock-wise */
volatile uint8 g_u8ChipResetOcc = TRUE;											/* Status-flag indicate chip-reset occurred (POR) */
volatile uint8 g_u8StallOcc = FALSE;											/* Status-flag indicate stall occurred */
volatile uint8 g_u8EmergencyRunOcc = FALSE;										/* Status-flag indicate Emergency-run occurred */
volatile uint8 g_e8ErrorOverTemperature = (uint8) C_ERR_OTEMP_NO;				/* Status-flag over-temperature */
uint8 g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;					/* Degraded Motor Request */
volatile uint8 g_u8RewindFlags = 0U;
volatile int16 g_i16ChipTemperature = 25;										/* Chip internal temperature */
#if _SUPPORT_AMBIENT_TEMP
volatile int16 g_i16AmbjTemperature = 25;										/* Ambient Temperature */
#endif /* _SUPPORT_AMBIENT_TEMP */
volatile int16 g_i16MotorVoltage = 1200;										/* Motor driver voltage */
uint8 g_u8StallTypeComm = (uint8) C_STALL_NOT_FOUND;
uint8 g_u8MotorStatusSpeed = (uint8) C_MOTOR_SPEED_STOP;						/* (Status) Actual motor-speed */
uint8 g_e8CalibrationStep = (uint8) C_CALIB_NONE;								/* Calibration step */

#if (LINPROT == LIN2X_ACT44)
uint8 g_e8MotorCtrlMode __attribute__ ((section(".dp.noinit")));				/* Control-flags motor mode (from Master) [WD] */
#endif /* (LINPROT == LIN2X_ACT44) */
volatile uint8 g_e8MotorStatusMode __attribute__ ((section(".dp.noinit")));		/* Status-flags motor mode (to Master) */
uint8 g_e8StallDetectorEna __attribute__ ((section(".dp.noinit")));				/* Control-flag Stall-detector enabled [WD] */
uint8 g_u8MotorHoldingCurrEna __attribute__ ((section(".dp.noinit")));			/* Control-flag motor Holding-current enabled [WD] */
uint16 g_u16ActualPosition __attribute__ ((section(".dp.noinit")));				/* (Control/Status) Actual motor-rotor position [WD] */
uint16 g_u16TargetPosition __attribute__ ((section(".dp.noinit")));				/* (Control) Target motor-rotor position (invalid) [WD] */
uint8 g_u8MotorCtrlSpeed __attribute__ ((section(".dp.noinit")));				/* (Control) Selected motor-speed */
uint16 g_u16CalibTravel __attribute__ ((section(".dp.noinit")));				/* Number of steps between two end-stops */
#pragma space none																/* __TINY_SECTION__ */

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	(@NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp																/* __NEAR_SECTION__ */
volatile int16 g_i16SupplyVoltage = 1200;										/* Supply Voltage */
#if _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE)
volatile int16 g_i16PhaseVoltage = 1200;										/* Phase Voltage */
#endif /* _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE) */
volatile int16 g_i16Current = 0;												/* Supply Current */
uint16 g_u16TargetPositionRewind;												/* Memorised Target Position after Rewind */

#if _SUPPORT_PHASE_SHORT_DET
uint16 l_u16AdcHoldMode = 0U;													/* ADC-mode during holding-mode */
#endif /* _SUPPORT_PHASE_SHORT_DET */

#if _SUPPORT_CHIP_TEMP_PROFILE
uint16 g_u16TemperatureStabilityCounter = 0U;									/* Temperature stability counter */
int16 l_i16ChipTemperaturePrev = 0;												/* Previous time-stamp chip temperature */
int16 l_i16ChipTemperatureInt = 0;												/* Chip temperature profile filter */
#endif /* _SUPPORT_CHIP_TEMP_PROFILE */

#if (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44)
uint16 u16DegradeDelay = 0xFFFF;												/* Disable degrade delay timer */
#endif /* (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44) */

#if _SUPPORT_LIN_BUS_ACTIVITY_CHECK
uint16 g_u16Mlx4StateCheckCounter = 0U;											/* State check counter */
uint8 g_u8Mlx4ErrorState = 0U;													/* Number of MLX4 Error states occurred */
#endif /* _SUPPORT_LIN_BUS_ACTIVITY_CHECK */
#if _SUPPORT_OVT_PED
uint8 g_u8OverTemperatureCount = 0U;											/* Number of over-temperature events */
#endif /* _SUPPORT_OVT_PED */
uint8 g_u8MotorStartDelay = 0U;													/* Motor start delay (500us) */
#if (LINPROT == LIN2X_ACT44)
uint8 g_e8ErrorVoltageComm = (uint8) C_ERR_VOLTAGE_IN_RANGE;					/* Status-flags voltage (Communication) */
#endif /* (LINPROT == LIN2X_ACT44) */
uint8 g_u8TorqueBoostRequest = 0U;												/* No Torque Boost (0%) */

uint8 l_e8ErrorDebounceFilter = C_DEBFLT_ERR_NONE;								/* Debounce filter */
uint8 l_u8RamPreError = FALSE;													/* RAM vs. NVRAM test first-failure */
uint8 g_u8MechError = FALSE;													/* No mechanical error */

#if _SUPPORT_AUTO_CALIBRATION
uint8 g_e8CalibPostMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;					/* Post calibration Motor Request */
uint16 g_u16CalibPauseCounter = 0U;
#endif /* _SUPPORT_AUTO_CALIBRATION */

#if USE_MULTI_PURPOSE_BUFFER
MP_BUF g_MPBuf;
#endif /* USE_MULTI_PURPOSE_BUFFER */

#pragma space none																/* __NEAR_SECTION__ */

const uint16 au16HaltZero[2] __attribute__((aligned(4))) =
{
	0x0000U, 0x0000U
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
#if (LINPROT == LIN2J_VALVE_VW)
	(void) u16Page;
	if ( (((NVRAM_USER *) C_ADDR_USERPAGE1)->AppStatus & 0x80U) ^ (((NVRAM_USER *) C_ADDR_USERPAGE2)->AppStatus & 0x80U) )
	{
		pu16Page = (uint16 *) C_ADDR_USERPAGE2;
	}
	else
	{
		pu16Page = (uint16 *) C_ADDR_USERPAGE1;
	}
#else  /* (LINPROT == LIN2J_VALVE_VW) */
	if ( u16Page == 0 )
	{
		pu16Page = (uint16 *) C_ADDR_USERPAGE1;									/* Compare NVRAM User-page #1 against RAM */
	}
	else 
	{
		pu16Page = (uint16 *) C_ADDR_USERPAGE2;									/* Compare NVRAM User-page #2 against RAM */
	}
#endif /* (LINPROT == LIN2J_VALVE_VW) */

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
#define FLASH_START_ADDR			0x4000U
#define FLASH_CRC_ADDR				0xBF4EU
#define FLASH_END_ADDR				0xC000U
#define C_FLASH_SEGMENT_SZ			4U											/* Max 250us (196us), Halt-mode: Full-check is once per 8:40s; Running-mode: 1.5s */

#define C_FLASH_CRC_FAILED			0U
#define C_FLASH_CRC_OK				1U
#define C_FLASH_CRC_CALCULATING		2U

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
		u16FlashCRC = 0xFFFFU;													/* Initialise the CRC preset with 0xFFFF */
	}
	if ( ((uint16) pu16Segment + u16Size) > FLASH_END_ADDR )
	{
		u16Size = (FLASH_END_ADDR - (uint16) pu16Segment);
	}
	for ( ; u16Size > 0U; u16Size-- )
	{
		if ( pu16Segment != (uint16 *) FLASH_CRC_ADDR )
		{
			uint8 u8Count;
			uint16 u16Data = *pu16Segment;
			for ( u8Count = 16U; u8Count > 0U; u8Count-- )
			{
				uint16 u16XorFlag = !!(u16FlashCRC & 0x8000);
				u16FlashCRC = (u16FlashCRC << 1);
				if ( (u16Data & 0x8000U) != 0U )
				{
					u16FlashCRC++;
				}
				if ( u16XorFlag != 0U )
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
		if ( *(uint16 *) FLASH_CRC_ADDR != 0U )									/* Flash/ROM Checksum programmed? */
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
 * *
 * ****************************************************************************	*/
void noinit_section_init(void)
{
#if (LINPROT == LIN2X_ACT44)
	g_e8MotorCtrlMode = (uint8) C_MOTOR_CTRL_STOP;
#endif /* (LINPROT == LIN2X_ACT44) */
	g_e8StallDetectorEna = (uint8) C_STALLDET_ALL;								/* Control-flag Stall-detector enabled [WD] */;
	g_u8MotorHoldingCurrEna = FALSE;
#if (LINPROT == LIN2J_VALVE_VW)
	g_u16ActualPosition = (0U + C_PERC_OFFSET);
	g_u16TargetPosition = (0U + C_PERC_OFFSET);
#else  /* (LINPROT == LIN2J_VALVE_VW) */
	g_u16ActualPosition = 32767U;
	g_u16TargetPosition = 65535U;
#endif /* (LINPROT == LIN2J_VALVE_VW) */
	g_u8MotorCtrlSpeed = (uint8) C_DEFAULT_MOTOR_SPEED;
	g_e8MotorStatusMode = (uint8) C_MOTOR_STATUS_STOP;
	g_u16CalibTravel = g_NvramUser.DefTravel;									/* Number of steps between two end-stops */

	/* MotorDriver.c variables */
	g_u16ActuatorActPos = g_u16ActualPosition;
	g_u16ActuatorTgtPos = g_u16TargetPosition;
} /* End of noinit_section_init() */

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
 *	N. Perform an initial supply and temperature measurement and energies motor-coils (MotorStop)
 *	O. Initialise LIN-communication
 *	S. (Optional) Determine actuator position
 *	T. (Watch-dog Reset) Start actuator (if needed)
 *	U. Main application loop
 * ****************************************************************************	*/
int16 main( void)
{
	/* *************************************************** */
	/* *** A. Initialise background schedule (Task-ID) *** */
	/* *************************************************** */
	uint8 u8BackgroundSchedulerTaskID = 0U;

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
		WD_CTRL = 0U;															/* Disable digital Watch-dog */
		AWD_CTRL = 0x8000U;														/* Disable analogue Watch-dog */
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

		if ( bistResetInfo != C_CHIP_STATE_WATCHDOG_RESET )
		{
			noinit_section_init();
		}
		g_e8StallDetectorEna = C_STALLDET_NONE;
		if ( g_NvramUser.StallDetectorEna )
		{
#if _SUPPORT_STALLDET_H
			g_e8StallDetectorEna |= C_STALLDET_HnA;								/* Control-flag Stall-detector "H" and "A" enabled */
#else  /* _SUPPORT_STALLDET_H */
			g_e8StallDetectorEna |= C_STALLDET_A;								/* Control-flag Stall-detector "A" enabled */
#endif /* _SUPPORT_STALLDET_H */
		}
		if ( NVRAM_STALL_O )
		{
			g_e8StallDetectorEna |= C_STALLDET_O;								/* Control-flag Stall-detector "O" enabled */
		}
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
		if ( (NVRAM_RESTALL_POR != FALSE) && (NVRAM_REWIND_STEPS != 0U) )
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
		if ( ((CONTROL & WD_BOOT) == 0U) && ((AWD_CTRL & AWD_RST) == 0U) && (bistResetInfo != C_CHIP_STATE_LIN_CMD_RESET) )
#endif /* _SUPPORT_WD_RST_RECOVERY */
		{
#if (_SUPPORT_MOTOR_SELFTEST != FALSE)
			g_e8MotorStatusMode = (uint8) C_MOTOR_STATUS_SELFTEST;
			MotorDriverSelfTest();												/* Self-test Motor-Driver */
#endif /* (_SUPPORT_MOTOR_SELFTEST != FALSE) */
		}
#if _SUPPORT_WD_RST_RECOVERY
		else if ( bistResetInfo == C_CHIP_STATE_WATCHDOG_RESET )
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

		/* **************************************************************** */
		/* *** N. Perform an initial supply and temperature measurement *** */
		/* ***    and energies motor coils (MotorDriverStop()).			*** */
		/* **************************************************************** */
		MeasureVsupplyAndTemperature();
		GetChipTemperature( TRUE);
		MotorDriverStop( (uint16) C_STOP_IMMEDIATE);							/* Energies coils if needed */

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

		/* ************************************************* */
		/* *** S. (Optional) Determine actuator position *** */
		/* ************************************************* */
#if _SUPPORT_AUTO_CALIBRATION
		if ( (g_u8ChipResetOcc != FALSE) && (NVRAM_AUTO_RECALIBRATE != 0U) )
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
		if ( g_u8LinAAMode != 0U )												/* Don't allow any motor control */
		{
			continue;
		}
#endif /* ((LINPROT & LINXX) == LIN2X) */

		/* ******************************* */
		/* *** d. Motor Driver current *** */
		/* ******************************* */
		/* Calculate Current (1000LSB/A) [mA] */
		if ( ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) != 0U) || (g_u8MotorHoldingCurrState != FALSE) )
		{
			/* Average current = unfiltered_current * Motor_PWM_DutyCycle */
			g_i16Current = (int16) muldivU16_U16byU16byU16( (uint16) GetMotorDriverCurrent(), g_u16CorrectionRatio, (PWM_REG_PERIOD << (4 + PWM_PRESCALER_N)));

			if ( (g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) == 0U )
			{
				/* Motor holding mode: Update motor-current LPF for PID-control */
				MotorDriverCurrentMeasure( FALSE);
			}
		}
		else if ( (g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) == 0U )
#else  /* LIN_COMM */
		if ( (g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) == 0U )
#endif /* LIN_COMM */
		{
			g_i16Current = 0;
			g_u16MotorCurrentLPFx64 = 0U;
			g_u16MotorCurrentMovAvgxN = 0U;
			MeasureVsupplyAndTemperature();										/* Perform Vbat/Temperature measurement incase motor is stopped */
		}
		else
		{
			/* Nothing */
		}

		/* ************************************************************** */
		/* *** e. Chip and Motor Driver voltage (degraded-mode check) *** */
		/* ************************************************************** */
		/* Calculate Voltage (100LSB/V) [10mV] */
		GetVsupply();
		GetVsupplyMotor();
		if ( g_i16MotorVoltage < (NVRAM_APPL_UVOLT - C_VOLTAGE_HYS) )
		{
			/* First time application under-voltage error */
			if ( (l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_UV) == 0x00U )
			{
				/* Need twice a under-voltage detection, to avoid ESD-pulses disturbance will cause degraded mode entering */
				l_e8ErrorDebounceFilter |= (uint8) C_DEBFLT_ERR_UV;
			}
			else
			{
				if ( g_e8ErrorVoltage != (uint8) C_ERR_VOLTAGE_UNDER )
				{
					g_e8ErrorVoltage = (uint8) C_ERR_VOLTAGE_UNDER;
#if (LINPROT == LIN2X_ACT44)
					g_e8ErrorVoltageComm = g_e8ErrorVoltage;
#endif /* (LINPROT == LIN2X_ACT44) */
					SetLastError( (uint8) C_ERR_APPL_UNDER_VOLT);
				}
#if (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44)
				u16DegradeDelay = 0xFFFF;										/* Disable degrade delay timer */
#endif /* (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44) */
			}
		}
		else if ( g_i16MotorVoltage > (NVRAM_APPL_OVOLT + C_VOLTAGE_HYS) )
		{
			/* First time application over-voltage error */
			if ( (l_e8ErrorDebounceFilter & C_DEBFLT_ERR_OV) == 0x00U )
			{
				/* Need twice a over-voltage detection, to avoid ESD-pulses disturbance will cause degraded mode entering */
				l_e8ErrorDebounceFilter |= (uint8) C_DEBFLT_ERR_OV;
			}
			else
			{
				if ( g_e8ErrorVoltage != (uint8) C_ERR_VOLTAGE_OVER )
				{
					g_e8ErrorVoltage = (uint8) C_ERR_VOLTAGE_OVER;
#if (LINPROT == LIN2X_ACT44)
					g_e8ErrorVoltageComm = g_e8ErrorVoltage;
#endif /* (LINPROT == LIN2X_ACT44) */
					SetLastError( (uint8) C_ERR_APPL_OVER_VOLT);
				}
#if (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44)
				u16DegradeDelay = 0xFFFFU;										/* Disable degrade delay timer */
#endif /* (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44) */
			}
		}
		else if ( (g_i16MotorVoltage >= (NVRAM_APPL_UVOLT + C_VOLTAGE_HYS)) && (g_i16SupplyVoltage <= (NVRAM_APPL_OVOLT - C_VOLTAGE_HYS)) )
		{
#if (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44)
			if ( g_e8ErrorVoltage != (uint8) C_ERR_VOLTAGE_IN_RANGE )
			{
				/* Voltage error before */
				if ( u16DegradeDelay == 0xFFFFU )
				{
					/* Degrade delay timer is disabled */
					if ( g_e8DegradedMotorRequest != (uint8) C_MOTOR_REQUEST_NONE )
					{
						/* Degrade Request is pending; Set Degrade delay time */
						extern uint8 g_u8NAD;
						/* Delay is 100 - 420ms = 100 + (NAD & 0x1F)*10 [ms] */
						u16DegradeDelay = (100U + ((g_u8NAD & 0x1FU) * 10U)) * PI_TICKS_PER_MILLISECOND;	/* Set Degrade delay timer,depended on NAD */
					}
					else
					{
						/* No Degrade Request pending; Immediate leave Degrade-mode */
						g_e8ErrorVoltage = (uint8) C_ERR_VOLTAGE_IN_RANGE;
					}
				}
				else if ( u16DegradeDelay == 0U )
				{
					/* Degrade delay time-out */
					u16DegradeDelay = 0xFFFFU;									/* Disable Degrade Delay timer */

					g_e8ErrorVoltage = (uint8) C_ERR_VOLTAGE_IN_RANGE;			/* Signal voltage-in range */
				}
				else
				{
					/* Nothing */
				}
			}
			l_e8ErrorDebounceFilter &= (uint8) ~(C_DEBFLT_ERR_UV | C_DEBFLT_ERR_OV);
#else  /* (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44) */
			g_e8ErrorVoltage = (uint8) C_ERR_VOLTAGE_IN_RANGE;
			l_e8ErrorDebounceFilter &= (uint8) ~(C_DEBFLT_ERR_UV | C_DEBFLT_ERR_OV);
#endif /* (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44) */
		}

#if _SUPPORT_LIN_UV
		if ( NVRAM_LIN_UV != 0 )
		{
			if ( g_i16MotorVoltage >= (int16) (600 + (NVRAM_LIN_UV * 50)) )
			{
				g_u16LinUVTimeCounter = 0U;										/* Stop LIN UV time-counter */
			}
			else if ( g_u16LinUVTimeCounter > PI_TICKS_PER_SECOND )
			{
				/* Restart MLX4 Bus-timeout */
				(void) ml_Disconnect();
				(void) ml_Connect();
				g_u16LinUVTimeCounter = 1;										/* Re-start LIN UV time-counter */
			}
			else if ( g_u16LinUVTimeCounter == 0U )
			{
				g_u16LinUVTimeCounter = 1U;										/* Start LIN UV time-counter */
			}
		}
#endif /* _SUPPORT_LIN_UV */

		/* ************************************************************* */
		/* *** f. Chip and ambient temperature (degraded-mode check) *** */
		/* ************************************************************* */
		/* Calculate Chip internal temperature (1LSB/C) [C] */
		GetChipTemperature( FALSE);
#if _SUPPORT_AMBIENT_TEMP
		SelfHeatCompensation();
		if ( ((g_i16AmbjTemperature > (int16) (NVRAM_APPL_OTEMP + C_TEMPERATURE_HYS)) && ((g_e8MotorStatusMode & C_MOTOR_STATUS_RUNNING) == 0)) ||
#if (_SUPPORT_DIAG_OVT != FALSE)
			 (g_i16ChipTemperature > (int16) (C_CHIP_OVERTEMP_LEVEL + C_TEMPERATURE_HYS)) )
#else  /* (_SUPPORT_DIAG_OVT != FALSE) */
			 (g_i16ChipTemperature > (int16) (NVRAM_APPL_OTEMP + C_TEMPERATURE_HYS)) )
#endif /* (_SUPPORT_DIAG_OVT != FALSE) */
#else  /* _SUPPORT_AMBIENT_TEMP */
		if ( ((g_i16ChipTemperature > (int16) (NVRAM_APPL_OTEMP + C_TEMPERATURE_HYS)) && ((g_e8MotorStatusMode & C_MOTOR_STATUS_RUNNING) == 0)) ||
			 (g_i16ChipTemperature > C_CHIP_OVERTEMP_LEVEL) )
#endif /* _SUPPORT_AMBIENT_TEMP */
		{
			if ( g_e8ErrorOverTemperature != (uint8) C_ERR_OTEMP_YES )
			{
				if ( (l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_OVT) == 0x00U )
				{
					/* Need twice a over-temperature detection, to avoid ESD-pulses disturbance will cause degraded mode entering */
					l_e8ErrorDebounceFilter |= (uint8) C_DEBFLT_ERR_OVT;
				}
				else
				{
					g_e8ErrorOverTemperature = (uint8) C_ERR_OTEMP_YES;
					g_u16TargetPosition = g_u16ActualPosition;				/* If over-temperature, then FPOS = CPOS (9.5.3.2) */
#if _SUPPORT_OVT_PED
					if ( g_e8ErrorElectric != (uint8) C_ERR_ELECTRIC_PERM )
					{
						g_u8OverTemperatureCount++;
						if ( g_u8OverTemperatureCount >= (uint8) C_OVERTEMP_TO_PERMDEFECT_THRSHLD )
						{
							g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
						}
					}
#endif /* _SUPPORT_OVT_PED */
					SetLastError( (uint8) C_ERR_APPL_OVER_TEMP);
				}
			}
		}
#if _SUPPORT_AMBIENT_TEMP
		else if ( g_i16AmbjTemperature < (int16) (NVRAM_APPL_OTEMP - C_TEMPERATURE_HYS) )
#else  /* _SUPPORT_AMBIENT_TEMP */
		else if ( g_i16ChipTemperature < (int16) (NVRAM_APPL_OTEMP - C_TEMPERATURE_HYS) )
#endif /* _SUPPORT_AMBIENT_TEMP */
		{
			g_e8ErrorOverTemperature = (uint8) C_ERR_OTEMP_NO;
			l_e8ErrorDebounceFilter &= (uint8) ~C_DEBFLT_ERR_OVT;
		}
		else
		{
			/* Nothing */
		}

		/* ****************************** */
		/* *** g. Degraded-mode check *** */
		/* ****************************** */
		/* Degraded check */
		if ( ((g_e8ErrorVoltage != (uint8) C_ERR_VOLTAGE_IN_RANGE) ||
			  (g_e8ErrorOverTemperature == (uint8) C_ERR_OTEMP_YES)) &&
			 ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED) == 0U) )
		{
			/* Not in degradation state; Stop motor, remember last "request" and enter degradation state */
			if ( g_e8MotorRequest != (uint8) C_MOTOR_REQUEST_NONE )
			{
				g_e8DegradedMotorRequest = g_e8MotorRequest;
				g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
				MotorDriverStop( (uint16) C_STOP_RAMPDOWN);						/* Degraded-mode (Running) */
			}
			else if ( (g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) != 0U )
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
				MotorDriverStop( (uint16) C_STOP_EMERGENCY);					/* Degraded-mode */
			}
			else
			{
				/* Nothing */
			}
			g_e8MotorStatusMode |= (uint8) C_MOTOR_STATUS_DEGRADED;
		}
		else if ( (g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED) && 
			 (g_e8ErrorVoltage == (uint8) C_ERR_VOLTAGE_IN_RANGE) && 
			 (g_e8ErrorOverTemperature == (uint8) C_ERR_OTEMP_NO) )
		{
			/* No longer degraded mode */
			if ( g_e8DegradedMotorRequest != (uint8) C_MOTOR_REQUEST_NONE ) 
			{
				g_e8MotorRequest = (uint8) g_e8DegradedMotorRequest;
				g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
			}
			g_e8MotorStatusMode &= (uint8) ~C_MOTOR_STATUS_DEGRADED;
		}
		else
		{
			/* Nothing */
		}

		/* ************************************************* */
		/* *** i. Handling Motor Request (Emergency Run) *** */
		/* ************************************************* */
#if (_SUPPORT_BUSTIMEOUT)
		/* Bus-timeout occurred */
		if ( g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_EMRUN )
		{
			/* Actuator move's towards Emergency Run position */
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_NONE;

#if (((LINPROT & LINXX) == LIN2X) || ((LINPROT & LINXX) == LIN2J))
			if ( g_NvramUser.EmergencyRunEndStopHi == 0U )
			{
				/* Move to Low EndStop */
				g_u16TargetPosition = C_MIN_POS;
			}
			else
			{
				/* Move to High EndStop */
				g_u16TargetPosition = C_MAX_POS;
			}
			if ( g_NvramUser.MotorDirectionCCW )
			{
				g_u16TargetPosition = C_MAX_POS - g_u16TargetPosition;
			}
#if (LINPROT == LIN2J_VALVE_VW)
			/* Convert GM percentage position to micro-step position */
			if ( g_u16TargetPosition == C_MIN_POS )
			{
				g_u16TargetPosition = 0U;
			}
			else
			{
				g_u16TargetPosition = g_u16CalibTravel + (2U * C_PERC_OFFSET);
			}
#endif /* (LINPROT == LIN2J_VALVE_VW)*/

			if ( g_u16TargetPosition != g_u16ActualPosition )
			{
				/* Only move actuator when not already at position */
				g_u8EmergencyRunOcc = TRUE;
				g_e8StallDetectorEna = (uint8) C_STALLDET_ALL;
				g_u8StallOcc = FALSE;
				g_u8StallTypeComm &= ~M_STALL_MODE;
				if ( g_e8MotorStatusMode != (uint8) C_MOTOR_STATUS_RUNNING )
				{
					/* If motor not running, set speed; Otherwise leave speed as is */
					g_u8MotorCtrlSpeed = (uint8) C_MOTOR_SPEED_MID;
				}
				if ( g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED )
				{
					g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_START;
				}
				else
				{
					g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_START;
				}
			}
#if (_SUPPORT_BUSTIMEOUT_SLEEP != FALSE)
			else
			{
				/* Actuator already at target-position; Just enter SLEEP */
				g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_SLEEP;	
			}
#endif /* (_SUPPORT_BUSTIMEOUT_SLEEP != FALSE) */
#endif /* (((LINPROT & LINXX) == LIN2X) || ((LINPROT & LINXX) == LIN2J)) */
		}
#endif /* (_SUPPORT_BUSTIMEOUT) */

		/* ********************************************************************************************* */
		/* *** j. Handling Motor Request (resp. STOP, INIT, START, CALIBRATION, SLEEP, SPEED-CHANGE) *** */
		/* ********************************************************************************************* */
		if ( g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_STOP )
		{
			/* Stop Actuator */
			g_u8RewindFlags &= (uint8) ~(C_REWIND_ACTIVE | C_REWIND_REWIND);
			MotorDriverStop( (uint16) C_STOP_RAMPDOWN);							/* LIN-command request */
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
			g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
#if _SUPPORT_AUTO_CALIBRATION
			g_e8CalibPostMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
#endif /* _SUPPORT_AUTO_CALIBRATION */
		}
#if LIN_COMM
		else if ( (g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_INIT) || (g_e8DegradedMotorRequest == (uint8) C_MOTOR_REQUEST_INIT) )
		{
			/* Actuator initialisation: Set new actual position */
			g_u16ActuatorActPos = g_u16ActualPosition;
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
			g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
			g_e8CalibrationStep = (uint8) C_CALIB_DONE;
		}
#endif /* LIN_COMM */
		else if ( (g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_START) && (g_u8MotorStartDelay == 0) )
		{	
			/* Start Actuator */
#define C_MAX_VALID_MOTOR_CURR	600U		/* 600mA */
			uint16 u16MotorCurrent;												/* MMP170405-2 - Begin */
			if ( g_u16MotorCurrentMovAvgxN == 0U )
			{
				/* Sample once the motor current */
				MeasureMotorCurrent();
				u16MotorCurrent = (uint16) GetMotorDriverCurrent();
			}
			else
			{
				/* Use sampled (running or holding) motor current */
				u16MotorCurrent = (uint16) ((mulU32_U16byU16( g_u16MotorCurrentMovAvgxN, g_u16MCurrgain) + (C_GMCURR_DIV << (C_MOVAVG_SSZ - 1U))) / (C_GMCURR_DIV << C_MOVAVG_SSZ)); /* Moving average-motor current [mA] */
			}
			if ( (g_u16ActualPosition != g_u16TargetPosition) &&
				!((g_u16TargetPosition == 0) && (g_u16ActuatorActPos == (0U + C_PERC_OFFSET))) &&		/* Not position '0' */
				!((g_u16TargetPosition == (g_u16CalibTravel + (2*C_PERC_OFFSET))) && (g_u16ActuatorActPos == (g_u16CalibTravel + C_PERC_OFFSET))) && /* Not position '255' */
				(u16MotorCurrent < C_MAX_VALID_MOTOR_CURR) )					/* MMP170405-2 - End */
			{
				uint8 u8NewMotorDirectionCCW;
#if LIN_COMM
				uint8 u8MotorSpeedIdx = (uint8) (g_u8MotorCtrlSpeed & 0x07U);
				g_u8MotorStatusSpeed = u8MotorSpeedIdx;
				g_u16MotorSpeedRPS = g_au16MotorSpeedRPS[u8MotorSpeedIdx];
				g_u16TargetCommutTimerPeriod = g_au16MotorSpeedCommutTimerPeriod[u8MotorSpeedIdx];
#else  /* LIN_COMM */
				g_u8MotorStatusSpeed = (uint8) C_MOTOR_SPEED_2;
				g_u16MotorSpeedRPS = g_au16MotorSpeedRPS[C_MOTOR_SPEED_2];		/* Use Speed #2 as default */
				g_u16TargetCommutTimerPeriod = g_au16MotorSpeedCommutTimerPeriod[C_MOTOR_SPEED_2];
#endif /* LIN_COMM */

				u8NewMotorDirectionCCW = (g_u16TargetPosition < g_u16ActualPosition) ? TRUE : FALSE;

				if ( g_u8RewindFlags & (uint8) C_REWIND_STALL_DETECT )
				{
					if ( (NVRAM_REWIND_STEPS != 0U) &&
						(((g_u8RewindFlags & (uint8) C_REWIND_DIRECTION_CCW) == u8NewMotorDirectionCCW) || (g_u8RewindFlags & C_REWIND_DIRECTION_AUTO)) )
					{
						/* Start rewind-function, with "rewinding" */
						g_u8RewindFlags = (uint8) (C_REWIND_ACTIVE | C_REWIND_REWIND);	/* Start rewind-process */
						g_u16TargetPositionRewind = g_u16TargetPosition;
						if ( u8NewMotorDirectionCCW )
						{
#if (LINPROT == LIN2J_VALVE_VW)
							if ( g_u16ActualPosition < (uint16) ((g_u16CalibTravel + C_PERC_OFFSET) - NVRAM_REWIND_STEPS) )
#else  /* (LINPROT == LIN2J_VALVE_VW) */
							if ( g_u16ActualPosition < (uint16) (C_MAX_POS - NVRAM_REWIND_STEPS) )
#endif /* (LINPROT == LIN2J_VALVE_VW) */
							{
								g_u16TargetPosition = g_u16ActualPosition + NVRAM_REWIND_STEPS;
								u8NewMotorDirectionCCW = FALSE;
							}
							else
							{
								g_u8RewindFlags = 0U;							/* No rewind possible */
							}
						}
						else
						{
							if ( g_u16ActualPosition > NVRAM_REWIND_STEPS )
							{
								g_u16TargetPosition = g_u16ActualPosition - NVRAM_REWIND_STEPS;
								u8NewMotorDirectionCCW = TRUE;
							}
							else
							{
								g_u8RewindFlags = 0U;							/* No rewind possible */
							}
						}
					}
					else if ( (g_u8RewindFlags & (uint8) C_REWIND_DIRECTION_CCW) != u8NewMotorDirectionCCW )
					{
						g_u8RewindFlags = 0U;									/* Clear previous detected stall flags */
					}
				}

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
#if _SUPPORT_AUTO_CALIBRATION
		else if ( g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_CALIBRATION )
		{
			/* Calibrate Levelling Actuator */
			/* The actuator calibration is performed in the following steps:
			 * 1. Set current position as "MIN" (0)
			 * 2. Set target position as "MAX" (LIN 1.3: 0x7FFE or LIN 2.x: 0xFFFE)
			 * 3. Enable stall-detector and start actuator (C_CALIB_SETUP_HI_ENDPOS)
			 * 4. Wait for a stall detected (C_CALIB_CHECK_HI_ENDPOS); If not detected (no end-stop): C_CALIB_FAILED_NO_ENDSTOP
			 * 5. (Optional) Wait end-stop time (C_CALIB_PAUSE_HI_ENDSTOP)
			 * 6. Set current position as "MAX" (LIN 1.3: 0x7FFE or LIN 2.x: 0xFFFE), set target position as "MIN" (0x0000),
			 *	  Clear stall-detection flag and start actuator (C_CALIB_SETUP_LO_ENDPOS)
			 * 7. Wait for a stall detected (C_CALIB_CHECK_LO_ENDPOS); If not detected (no end-stop): C_CALIB_FAILED_NO_ENDSTOP
			 * 8. If stall detected, check travel-range with NVRAM stored range. If within range +/- tolerance: C_CALIB_DONE
			 *	  If travel-range < NVRAM stored range - tolerance: C_CALIB_FAILED_TOO_SHORT
			 *	  If travel-range > NVRAM stored range + tolerance: C_CALIB_FAILED_TOO_LONG
			 * 9. Set current position as "MIN" (0)
			 */
			if ( g_e8CalibrationStep == (uint8) C_CALIB_START )
			{
				/* Setup calibration (Low-position to High-position) */
				if ( g_NvramUser.MotorDirectionCCW )
				{
					g_u16ActualPosition = (g_NvramUser.DefTravel + C_PERC_OFFSET);
					g_u16TargetPosition = 0U;
				}
				else
				{
					g_u16ActualPosition = 0U;
					g_u16TargetPosition = (g_NvramUser.DefTravel + C_PERC_OFFSET);
				}
				g_u16CalibTravel = g_u16ActualPosition;							/* g_u16CalibTravel = Start-position */
				g_u8MechError = FALSE;
				g_e8CalibrationStep = (uint8) C_CALIB_SETUP_HI_ENDPOS;

				if ( g_e8MotorStatusMode != (uint8) C_MOTOR_STATUS_STOP )
				{
					MotorDriverStop( (uint16) C_STOP_IMMEDIATE);
					g_u16CalibPauseCounter = C_PI_TICKS_STABILISE_CALIB;
				}
			}

			if ( (g_e8CalibrationStep == (uint8) C_CALIB_SETUP_HI_ENDPOS) && (g_u16CalibPauseCounter == 0U) )
			{
				g_e8MotorDirectionCCW = (g_u16TargetPosition < g_u16ActualPosition) ? (uint8) C_MOTOR_DIR_CLOSING : (uint8) C_MOTOR_DIR_OPENING;
				g_u8MotorCtrlSpeed = (uint8) C_DEFAULT_MOTOR_SPEED;
				g_u16MotorSpeedRPS = g_au16MotorSpeedRPS[C_DEFAULT_MOTOR_SPEED];
				g_u16TargetCommutTimerPeriod = g_au16MotorSpeedCommutTimerPeriod[C_DEFAULT_MOTOR_SPEED];
				g_e8StallDetectorEna |= (uint8) C_STALLDET_CALIB;
				g_u8StallOcc = FALSE;
				MotorDriverStart();
				g_e8CalibrationStep = (uint8) C_CALIB_CHECK_HI_ENDPOS;		/* Check for FIRST End-stop */
			}
			else if ( g_e8CalibrationStep == (uint8) C_CALIB_CHECK_HI_ENDPOS )
			{
				if ( g_u8StallOcc )
				{
					g_e8CalibrationStep = (uint8) C_CALIB_SETUP_LO_ENDPOS;
					g_u16CalibPauseCounter = C_PI_TICKS_STABILISE_CALIB;
				}
				else if ( g_u16TargetPosition == g_u16ActualPosition )
				{
					g_e8CalibrationStep = (uint8) C_CALIB_FAILED_NO_ENDSTOP;
					g_e8MotorRequest = g_e8CalibPostMotorRequest;
					g_e8CalibPostMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
				}
			}
			else if ( (g_e8CalibrationStep == (uint8) C_CALIB_SETUP_LO_ENDPOS) && (g_u16CalibPauseCounter == 0) )
			{
				/* Setup calibration (High-position to Low-position) */
				g_u16ActualPosition = g_u16TargetPosition;						/* g_u16ActualPosition = End-position */
				g_u16TargetPosition = g_u16CalibTravel;							/* g_u16TargetPosition = Start-position */
				g_e8MotorDirectionCCW = (g_u16TargetPosition < g_u16ActualPosition) ? C_MOTOR_DIR_CLOSING : C_MOTOR_DIR_OPENING;
				g_u16CalibTravel = g_u16ActualPosition;							/* g_u16CalibTravel = End-position */
				g_u8MotorCtrlSpeed = (uint8) C_DEFAULT_MOTOR_SPEED;
				g_u16MotorSpeedRPS = g_au16MotorSpeedRPS[C_DEFAULT_MOTOR_SPEED];
				g_u16TargetCommutTimerPeriod = g_au16MotorSpeedCommutTimerPeriod[C_DEFAULT_MOTOR_SPEED];
				g_e8StallDetectorEna |= (uint8) C_STALLDET_CALIB;
				g_u8StallOcc = FALSE;
				MotorDriverStart();
				g_e8CalibrationStep = (uint8) C_CALIB_CHECK_LO_ENDPOS;		/* Check for SECOND End-stop */
			}
			else if ( g_e8CalibrationStep == (uint8) C_CALIB_CHECK_LO_ENDPOS )
			{
				if ( g_u8StallOcc )
				{
					if ( g_u16CalibTravel < g_u16ActualPosition )
					{
						g_u16CalibTravel = g_u16ActualPosition;
					}
					else
					{
						g_u16CalibTravel = g_u16CalibTravel - g_u16ActualPosition;
					}
					g_e8CalibrationStep = C_CALIB_DONE;
					g_u16ActualPosition = (g_NvramUser.MotorDirectionCCW ? g_u16CalibTravel : 0U);
					g_u16ActualPosition += C_PERC_OFFSET;
					g_u16ActuatorActPos = g_u16ActualPosition;
					g_u8StallOcc = FALSE;
					g_u8StallTypeComm &= ~M_STALL_MODE;
					g_e8StallDetectorEna &= (uint8) ~C_STALLDET_CALIB;
					g_u8MotorStartDelay = 255U;
					g_e8MotorRequest = g_e8CalibPostMotorRequest;
					g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
					g_e8CalibPostMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
				}
				else if ( g_u16TargetPosition == g_u16ActualPosition )
				{
					g_e8CalibrationStep = (uint8) C_CALIB_FAILED_NO_ENDSTOP;
					g_e8MotorRequest = g_e8CalibPostMotorRequest;
					g_e8CalibPostMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
					g_e8StallDetectorEna &= (uint8) ~C_STALLDET_CALIB;
				}
			}
		}
#endif /* _SUPPORT_AUTO_CALIBRATION */
		else if ( (g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_SLEEP) || (g_e8DegradedMotorRequest == (uint8) C_MOTOR_REQUEST_SLEEP) )
		{
			/* Actuator enters (Deep-)SLEEP mode (lowest power mode; Only LIN/PWM message can wake-up) */
			if ( ((g_e8MotorStatusMode & ~C_MOTOR_STATUS_DEGRADED) == (uint8) C_MOTOR_STATUS_STOP) &&
					(g_u8MotorStopDelay == 0U) &&
					((NV_CTRL & NV_BUSY) == 0U) )
			{
				if ( g_u8MotorHoldingCurrEna &&									/* Holding mode enabled */
					(g_e8ErrorElectric != (uint8) C_ERR_ELECTRIC_PERM) && (g_e8ErrorVoltage == (uint8) C_ERR_VOLTAGE_IN_RANGE) )
				{
					MotorDriverStop( (uint16) C_STOP_SLEEP);					/* Disable holding current */
				}

#if (LINPROT == LIN2J_VALVE_VW)
				/* Before the actuator enters in Sleep, it saves in EEPROM the CPOS,
				 * the Status and the NAD only if the value of cells is different as the RAM value.
				 */
				LIN_SAE_J2602_Store();											/* MMP160613-2 */
#endif /* (LINPROT == LIN2J_VALVE_VW) */
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
				 * enter SLEEP but HALT-mode. To allow a LIN wake-up, the Analogue Watchdog is set to minimum
				 * period of 100us to allow a chip reset.
				 */
				IO_WU = 0U;														/* Disable IO3 wake up */
				ANA_OUTG = ANA_OUTG & 0xFF9EU;									/* Clear Internal WU delay and DIS_GTSM */
				MASK = 0U;
				ADC_Stop();
				/* Go into sleep/halt */
				AWD_CTRL = (3U << 8) | 1U;										/* Set 1:1 prescaler and minimal period; AWD timeout will be 100 us */
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
			uint8 u8MotorSpeedIdx = (g_u8MotorCtrlSpeed & 0x07U);
			g_u8MotorStatusSpeed = u8MotorSpeedIdx;
			g_u16MotorSpeedRPS = g_au16MotorSpeedRPS[u8MotorSpeedIdx];
			g_u16TargetCommutTimerPeriod = g_au16MotorSpeedCommutTimerPeriod[u8MotorSpeedIdx];
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
		}
#endif /* LIN_COMM */
		else
		{
			/* Nothing */
		}

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
			else
			{
				/* Nothing */
			}
			if ( g_u8MotorStatusSpeed != u8MotorSpeedIdx )
			{
				g_u8MotorStatusSpeed = u8MotorSpeedIdx;
				g_u16MotorSpeedRPS = g_au16MotorSpeedRPS[u8MotorSpeedIdx];
				g_u16TargetCommutTimerPeriod = g_au16MotorSpeedCommutTimerPeriod[u8MotorSpeedIdx];
			}
		}
#endif /* _SUPPORT_SPEED_AUTO */

		/* Update status actual-position (only in case not the initial position have been changed) */
		if ( ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) != 0U) &&
			 ((g_u8RewindFlags & (uint8) C_REWIND_ACTIVE) == 0U) )
		{
			g_u16ActualPosition = g_u16ActuatorActPos;
		}

		/* ********************************************************** */
		/* *** l. Threshold control (Stepper: Current-threshold) *** */
		/* ********************************************************** */
		ThresholdControl();													

		/* ************************************************* */
		/* *** m. PID control (Stepper: current-control) *** */
		/* ************************************************* */
		PID_Control();															/* PID-control (Current) */

		/* ********************** */
		/* *** n. MLX4 status *** */
		/* ********************** */
		{
			uint16 u16Mlx4CounterThreshold = C_MLX4_STATE_TIMEOUT;
#if _SUPPORT_LIN_BUS_ACTIVITY_CHECK && (__MLX_PLTF_VERSION_MAJOR__ == 4)
			{
				/* MLX4 LIN-Bus activity check when not in LIN-AA mode (only __MLX_PLTF_VERSION_MAJOR__ == 4) */
				if ( (ml_GetState( ML_NOT_CLEAR) != ml_stINVALID) && ((LinStatus & ML_LIN_BUS_ACTIVITY) != 0U) )
				{
					/* MLX4 has detected a SYNC field */
					g_u16Mlx4StateCheckCounter = 0U;
					g_u8ErrorCommBusTimeout = FALSE;
					(void) ml_GetState( ML_CLR_LIN_BUS_ACTIVITY);
				}
				else
				{
					g_u16Mlx4StateCheckCounter++;								/* State check counter */
				}
			}
#endif /* _SUPPORT_LIN_BUS_ACTIVITY_CHECK && (__MLX_PLTF_VERSION_MAJOR__ == 4) */

			if ( (g_u16Mlx4StateCheckCounter >= u16Mlx4CounterThreshold) || ((g_u8Mlx4ErrorState & (uint8) C_MLX4_STATE_IMMEDIATE_RST) != 0) )
			{
				/* Didn't receive MLX4 LIN command and/or data-request in the last period, or need immediate reset */
				g_u16Mlx4StateCheckCounter = 0U;								/* MLX4 State check counter reset; MLX4 still active */
				if ( ((g_u8Mlx4ErrorState & (uint8) C_MLX4_STATE_IMMEDIATE_RST) != 0U) ||
					( ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_APPL_STOP) == 0x00U)
						&& (g_e8MotorRequest != (uint8) C_MOTOR_REQUEST_SLEEP)
#if (__MLX_PLTF_VERSION_MAJOR__ == 4)
						&& (ml_GetState( ML_NOT_CLEAR) == ml_stINVALID) ) )
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 4) */
#if (__MLX_PLTF_VERSION_MAJOR__ == 3)
						&& (ml_GetState() == ml_stINVALID) ) )
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
						if ( (g_u8Mlx4ErrorState & C_MLX4_STATE_NOT_LOGGED) == 0U )
						{
							SetLastError( (uint8) C_ERR_MLX4_RESTART);
						}
						LIN_Init( TRUE);										/* Re-initialise LIN interface w/o changing position */
						g_u8Mlx4ErrorState = 0U;
					}
				}
				else
				{
					g_u8Mlx4ErrorState = 0U;
				}
			}
		}

		/* ********************************** */
		/* *** o. Background System check *** */
		/* ********************************** */
		if ( (u8BackgroundSchedulerTaskID == 0U) || (u8BackgroundSchedulerTaskID == 128U) )
		{
#if (LINPROT == LIN2J_VALVE_VW)
			if ( RamBackgroundTest( 0U) == FALSE )								/* Check RAM against NVRAM User-page */
#else  /* (LINPROT == LIN2J_VALVE_VW) */
			if ( RamBackgroundTest( u8BackgroundSchedulerTaskID ? 1 : 0) == FALSE )	/* Check RAM against NVRAM User-page #1/#2 */
#endif /* (LINPROT == LIN2J_VALVE_VW) */
			{
				/* RAM g_NvramUser structure not same as NVRAM Page #1.1.
				 * Either System RAM is corrupted or the NVRAM. Allow one time NVRAM reload */
				if ( l_u8RamPreError == FALSE )
				{
					NVRAM_LoadAll();
					l_u8RamPreError = TRUE;
				}
				else
				{
					SetLastError( (uint8) C_ERR_RAM_BG);						/* Log RAM failure */
#if (LINPROT == LIN2J_VALVE_VW)
					MLX4_RESET();												/* Reset the Mlx4   */
					bistResetInfo = C_CHIP_STATE_LIN_CMD_RESET;
					MLX16_RESET();												/* Reset the Mlx16  */
#else  /* (LINPROT == LIN2J_VALVE_VW) */
					g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;			/* Permanent electric failure */
#endif /* (LINPROT == LIN2J_VALVE_VW) */
				}
			}
			else
			{
				l_u8RamPreError = FALSE;										/* Error is gone (caused by wrong NVRAM shadow-RAM) */
			}
		}
		else if ( (FL_CTRL0 & FL_DETECT) != 0U )
		{
			if ( FlashBackgroundTest( C_FLASH_SEGMENT_SZ) == C_FLASH_CRC_FAILED )	/* Check Flash/ROM Memory Checksum (max. 250us) */
			{
				SetLastError( (uint8) C_ERR_FLASH_BG);
#if (LINPROT == LIN2J_VALVE_VW)
				MLX4_RESET();													/* Reset the Mlx4   */
				bistResetInfo = C_CHIP_STATE_LIN_CMD_RESET;
				MLX16_RESET();													/* Reset the Mlx16  */
#endif /* (LINPROT == LIN2J_VALVE_VW) */
			}
		}
		else
		{
			/* Nothing */
		}
		u8BackgroundSchedulerTaskID++; 

		/* *********************************************** */
		/* *** p. Motor-phase shortage to ground check *** */
		/* *********************************************** */
#if _SUPPORT_PHASE_SHORT_DET
		if ( (g_e8ErrorElectric != (uint8) C_ERR_ELECTRIC_PERM) && (NVRAM_VDS_THRESHOLD != 0U) &&
			(((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) && (g_u16StartupDelay == 0U)) ||
			(g_u8MotorHoldingCurrState != FALSE)) )
		{
			GetPhaseMotor();
			if ( g_i16PhaseVoltage < (int16) (g_u16MotorVoltage - NVRAM_VDS_THRESHOLD) )
			{
				/* Phase-voltage is more then 2V below motor-driver voltage */
				if ( (l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_PHASE_SHORT) == 0x00U )
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
					if ( (l_e8ErrorDebounceFilter & (uint8) C_DEBFLT_ERR_TEMP_PROFILE) == 0x00U )
					{
						l_e8ErrorDebounceFilter |= (uint8) C_DEBFLT_ERR_TEMP_PROFILE;
					}
					else
					{
#if _SUPPORT_OVT_PED
						g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
#endif /* _SUPPORT_OVT_PED */
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
		if ( ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) == 0U) &&
			 (g_u8MotorStopDelay == 0U) &&
			 ((CONTROL & M4_RB) != 0U) &&
			 (g_u8LinInFrameBufState == C_LIN_IN_FREE) )
		{
			uint16 u16XI0_Mask = XI0_MASK;
			uint16 u16IsrMask = MASK;
			uint16 u16Timer1Ctrl = TMR1_CTRL;

			ADC_PowerOff();														/* Stop ADC, including turning off reference voltage (Approx: 0.6mA) */
			DRVCFG |= DIS_SHOA;													/* Disable OpAmp for ADC measurement of shunt current (Approx: 0.6mA) */

			/* Setup wake-up timer event */
			TMR1_CTRL = (2U * TMRx_DIV0) | (0U * TMRx_MODE0) | TMRx_T_EBLK;		/* Timer mode 0, Divider 256 */
			TMR1_REGB = C_SLEEP_PERIOD;											/* Set sleep-period */
			XI0_PEND = CLR_T1_INT4;												/* Clear (potentially) Timer1 second level interrupts (T1_INT4) */
			XI0_MASK = EN_T1_INT4;												/* Disable Timer1 all 2nd level interrupts, except INT4 (CMP) */
			PEND = CLR_EXT0_IT;
			MASK = EN_EXT4_IT | EN_EXT0_IT | EN_M4_SHE_IT;						/* Enable Diagnostics, Timer1 and MLX4 IRQ's only */
			TMR1_CTRL = (2U * TMRx_DIV0) | (0U * TMRx_MODE0) | TMRx_T_EBLK | TMRx_START;	/* Start timer */

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
			__asm__("lod X, #_au16HaltZero");									/* X pointer to flash memory with 0x0000 */
			__asm__("lod AL, 0x2000");											/* Get MLX16 Control-state */
			__asm__("or  AL, #0x02");											/* Set HALT-state */
			__asm__("mov R, #0");												/* Restore IRQ-state */
			__asm__("pop M");
			__asm__("mov 0x2000, AL");											/* Enter HALT-state */
			__asm__("mov A,[X]");
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
						if (  l_u16AdcHoldMode >= 4U )
						{
							l_u16AdcHoldMode = 1U;
						}
						else
						{
							l_u16AdcHoldMode = (l_u16AdcHoldMode + 1U);
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
			if ( (TMR1_CTRL & TMRx_T_EBLK) == 0U )
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
			if ( (TIMER & TMR_EN) == 0U )
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
			if ( (PRIO & ((3U << 14) | (3U << 6) | (3U << 0))) != (/*((3U-3U) << 14) |*/ ((4U-3U) << 6) | ((6U-3U) << 0)) )
			{
				PRIO = (PRIO & ~((3u << 14) | (3u << 6) | (3u << 0))) | (/*((3U-3U) << 14) |*/ ((4U-3U) << 6) | ((6U-3U) << 0));
				SetLastError( (uint8) C_ERR_IOREG);
			}
			/* Check: 2nd level IRQ Timer1 */
			if ( (XI0_MASK & EN_T1_INT4) == 0U )
			{
				XI0_PEND = EN_T1_INT4;
				XI0_MASK = EN_T1_INT4;
				SetLastError( (uint8) C_ERR_IOREG);
			}
			/* Check: 2nd level IRQ Diagnostics */
			if ( (XI4_MASK & (XI4_OVT | XI4_UV | XI4_OV | XI4_OC_DRV)) != C_DIAG_MASK )
			{
				XI4_PEND = C_DIAG_MASK;
				XI4_MASK = C_DIAG_MASK;
				SetLastError( (uint8) C_ERR_IOREG);
			}
#if 0
			if ( (ADC_DBASE < (uint16) &g_AdcMotorRunStepper4) && (ADC_DBASE >= (uint16) &g_AdcMotorRunStepper4 + 1) )
			{
				ADC_DBASE = (uint16) &g_AdcMotorRunStepper4.UnfilteredDriverCurrent;
				SetLastError( (uint8) C_ERR_IOREG);
			}
#endif /* 0 */
			if ( ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) != 0U) && ((DRVCFG & (DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U)) == 0U) )
			{
				/* Driver have been disabled */
				SetLastError( (uint8) C_ERR_IOREG);
			}
		}
#endif /* _SUPPORT_IOREG_CHECK */

	}																			/* Application loop */

} /* End of main() */

/* EOF */
