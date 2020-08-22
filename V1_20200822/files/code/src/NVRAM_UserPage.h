/*! \file		NVRAM_UserPage.h
 *  \brief		MLX81310 NVRAM USer Page handling
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
 * Copyright (C) 2008-2015 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * ****************************************************************************	*/

#ifndef NVRAM_USER_PAGE_H_
#define NVRAM_USER_PAGE_H_

#include "Build.h"
#include "nvram.h"																/* NVRAM support */
#include "MotorParams.h"
#include <plib.h>																/* Use Melexis MLX813xx library (WDG_Manager) */

#define C_NVRAM_USER_REV		0x01U

#define C_NVRAM_USER_PAGE_1		0x01U						/* NVRAM User Page #1 */
#define C_NVRAM_USER_PAGE_2		0x02U						/* NVRAM User Page #2 */
#define C_NVRAM_USER_PAGE_ALL	(C_NVRAM_USER_PAGE_1 | C_NVRAM_USER_PAGE_2)
#define C_NVRAM_USER_PAGE_AUTO	0x10U						/* Alternate save Page #1/#2 */
#define C_NVRAM_USER_PAGE_RESET	0x20U						/* Reset chip after NVRAM Write */
#define C_NVRAM_USER_PAGE_FORCE	0x40U						/* NVRAM Write w/o pre-verify */
#define C_MVRAM_USER_PAGE_NoCRC	0x80U						/* No CRC Update */

/* Note: NVRAM is currently LIN communication protocol depended !! */

#define C_ADDR_USERPAGE1		0x1000U
#define C_SIZE_USERPAGE1		0x0080U
#define C_ADDR_PATCHPAGE		0x1080U
#define C_SIZE_PATCHPAGE		0x007CU
#define C_ADDR_USERPAGE2		0x1100U
#define C_SIZE_USERPAGE2		0x0080U
#define C_ADDR_MLXF_PAGE		0x1180U
#define C_ADDR_MLX_HWSWID		0x1182U
#define C_ADDR_MLX_CHIPID		0x1188U
#define C_ADDR_MLX_TESTID		0x11A0U
#define C_SIZE_MLXF_PAGE		0x0080U

typedef struct _NVRAM_USER
{
	uint16 CRC8					: 8;						/* 0x00: CRC-8 */
	uint16 Revision				: 8;						/* 0x01: NVRAM Structure revision */
#if LIN_COMM
#if ((LINPROT & LINXX) == LIN2X)
	/* Generic */
	uint16 NAD					: 8;						/* 0x02: NAD */
	uint16 Variant				: 8;						/* 0x03: Variant-ID */
	uint16 HwRef				: 8;						/* 0x04: HW-Revision */
	uint16 SwRef				: 8;						/* 0x05: (Reserved for) SW-Revision */
	uint16 ControlFrameID		: 8;						/* 0x06: Control-message Frame-ID */
	uint16 StatusFrameID		: 8;						/* 0x07: Status-message Frame-ID */
	uint16 ConfigurationID;									/* 0x08: Configuration ID */
	uint16 AASDMCM_delta;									/* 0x0A: LIN-AutoAaddressing Gain DifferentialMode-CommonMode delta */
	uint16 SerialNumberLSW;									/* 0x0C: (Optional) Serial-number (LSW) */
	uint16 SerialNumberMSW;									/* 0x0E: (Optional) Serial-number (MSW) */
#if (LINPROT == LIN2X_ACT44)
	/* Actuator */
	uint16 MotorDirectionCCW	: 1;						/* 0x10: Motor rotational direction: 0=CW, 1=CCW */
	uint16 EmergencyRunEna		: 1;						/* 0x10: Emergency-run: 0=Disabled, 1=Enabled */
	uint16 EmergencyRunEndStopHi: 1;						/* 0x10: Emergency-run end-stop: 0=Low, 1=High */
	uint16 StallDetectorEna		: 1;						/* 0x10: Stall-detector: 0=Disabled, 1=Enabled */
	uint16 MotorHoldingCurrentEna:1;						/* 0x10: Motor holding-current: 0=Off, 1=On */
	uint16 Speed				: 3;						/* 0x10: Motor speed */
	uint16 ActReserved_1		: 8;						/* 0x11: Reserved*/
	uint16 DefTravel;										/* 0x12: Default Travel */
	uint16 Reserved14;										/* 0x14: Reserved */
#endif /* (LINPROT == LIN2X_ACT44) */
#endif /* ((LINPROT & LINXX) == LIN2X) */
#if ((LINPROT & LINXX) == LIN2J)
	/* Generic */
	uint16 NAD					: 8;						/* 0x02: NAD */
	uint16 Variant				: 8;						/* 0x03: Variant-ID */
	uint16 HwRef				: 8;						/* 0x04: HW-Revision */
	uint16 SwRef				: 8;						/* 0x05: (Reserved for) SW-Revision */
	uint16 ControlFrameID		: 8;						/* 0x06: Control-message Frame-ID */
	uint16 StatusFrameID		: 8;						/* 0x07: Status-message Frame-ID */
	uint16 ConfigurationID;									/* 0x08: Configuration ID */
	uint16 AASDMCM_delta;									/* 0x0A: LIN-AutoAaddressing Gain DifferentialMode-CommonMode delta */
	uint16 SerialNumberLSW;									/* 0x0C: (Optional) Serial-number (LSW) */
	uint16 SerialNumberMSW;									/* 0x0E: (Optional) Serial-number (MSW) */
#if (LINPROT == LIN2J_VALVE_VW)
	/* Actuator */
	uint16 MotorDirectionCCW	: 1;						/* 0x10: Motor rotational direction: 0=CW, 1=CCW */
	uint16 EmergencyRunEna		: 1;						/* 0x10: Emergency-run: 0=Disabled, 1=Enabled */
	uint16 EmergencyRunEndStopHi: 1;						/* 0x10: Emergency-run end-stop: 0=Low, 1=High */
	uint16 StallDetectorEna		: 1;						/* 0x10: Stall-detector: 0=Disabled, 1=Enabled */
	uint16 MotorHoldingCurrentEna:1;						/* 0x10: Motor holding-current: 0=Off, 1=On */
	uint16 Speed				: 3;						/* 0x10: Motor speed */
	uint16 EndStopTime			: 8;						/* 0x11: End-stop pause time */
	uint16 DefTravel;										/* 0x12: Default Travel */
	uint16 DefTravelToleranceLo	: 8;						/* 0x14: Default Travel Tolerance (Lower) */
	uint16 DefTravelToleranceUp	: 8;						/* 0x15: Default Travel Tolerance (Upper) */
#endif /* (LINPROT == LIN2J_VALVE_VW) */
#endif /* ((LINPROT & LINXX) == LIN2J) */
#endif /* LIN_COMM */
	uint16 FunctionID;										/* 0x16: Function ID */
	uint16 CustomerID;										/* 0x18: Customer ID */
	uint16 ProductionDate;									/* 0x1A: Production Date: [15:9] Year (00-99), [8:5] Month (1-12), [4:0] Day (1-31) */
/*	uint16 ProductionDay		: 5; */						/* 0x1A.[ 4: 0]: Production Day (1-31) */
/*	uint16 ProductionMonth		: 4; */						/* 0x1A.[ 8: 5]: Production Month (1-12) */
/*	uint16 ProductionYear		: 7; */						/* 0x1A.[15: 9]: Production Year (20nn, 00<nn<99) */
#if (LINPROT == LIN2J_VALVE_VW)
	uint16 CPOS					: 8;						/* 0x1C: Current Position (CPOS) */
	uint16 AppStatus			: 8;						/* 0x1D: Application Status */
#else  /* (LINPROT == LIN2J_VALVE_VW) */
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
	uint16 GAD					: 8;						/* 0x1C: Group-address */
	uint16 GroupControlFrameID	: 8;						/* 0x1D: Group-Control-message Frame-ID */
#else  /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
	uint16 Reserved1C;										/* 0x1C: Reserved */
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
#endif /* (LINPROT == LIN2J_VALVE_VW) */
	uint16 Reserved1E			: 8;						/* 0x1E: Reserved */
	uint16 MotorFamily			: 8;						/* 0x1F: Motor Family */

	uint16 Reserved20;										/* 0x20: Reserved */
	uint16 Reserved22;										/* 0x22: Reserved */
	uint16 Reserved24;										/* 0x24: Reserved */
	uint16 Reserved26;										/* 0x26: Reserved */
	uint16 Reserved28;										/* 0x28: Reserved */
	uint16 Reserved2A;										/* 0x2A: Reserved */
	uint16 Reserved2C;										/* 0x2C: Reserved */
	uint16 Reserved2E;										/* 0x2E: Reserved */
	uint16 LeadAngle			: 6;						/* 0x30.[5:0]: Lead Angle */
	uint16 BrownoutLevel		: 2;						/* 0x30.[7.6]: Brown-out level */
	uint16 VsupRef				: 8;						/* 0x31: Vsupply reference [1/8V] */
	uint16 GearBoxRatio			: 12;						/* 0x32 [ 9: 0]: Gear-box ratio, e.g. 600:1 */
	uint16 PolePairs			: 4;						/* 0x32 [12:10]: Number of pole-pairs + 1 */
#if _SUPPORT_STALLDET_B
	uint16 StallBemfDelay		: 5;						/* 0x34.[4:0]: Stall-detector "B" delay */
#elif _SUPPORT_STALLDET_O
	uint16 StallOscWidth		: 3;						/* 0x34.[2:0]: Stall-detector "O" Width */
	uint16 Reserved34			: 2;						/* 0x34.[4:3]: Reserved */
#else
	uint16 Reserved34			: 5;						/* 0x34.[4:0]: Reserved */
#endif /* _SUPPORT_STALLDET_B */
	uint16 MicroSteps			: 2;						/* 0x34 [6:5]: Number of micro-steps: 2^n (or 1 << n) */
	uint16 MotorPhases			: 1;						/* 0x34 [7]: Number of motor-phases: 1 = 3-phase, 0 = Bi-polar */
#if _SUPPORT_STALLDET_B
	uint16 StallBemfThreshold	: 8;						/* 0x35: Stall-detector "B" threshold (BEMF) */
#elif _SUPPORT_STALLDET_O
	uint16 StallOscThreshold	: 8;						/* 0x35: Stall-detector "O" threshold (Oscillation) */
#else
	uint16 Reserved35			: 8;						/* 0x35: Reserved */
#endif /* _SUPPORT_STALLDET_O */
	/* Four speed points */
	uint16 Speed_0;											/* 0x36: Speed_0 */
	uint16 Speed_1;											/* 0x38: Speed_1 */
	uint16 Speed_2;											/* 0x3A: Speed_2 */
	uint16 Speed_3;											/* 0x3C: Speed_3 */
#if _SUPPORT_SPEED_AUTO
	/* Auto-speed */
	uint16 AutoSpeedTemperature1: 8;						/* 0x3E: Auto-speed temperature #1 */
	uint16 AutoSpeedTemperature2: 8;						/* 0x3F: Auto-speed temperature #2 */
	uint16 AutoSpeedTemperature3: 8;						/* 0x40: Auto-speed temperature #3 */
	uint16 AutoSpeedZone1		: 2;						/* 0x41.[1:0]: Zone I speed control vs. temperature */
	uint16 AutoSpeedZone2		: 2;						/* 0x41.[3:2]: Zone II speed control vs. temperature */
	uint16 AutoSpeedZone3		: 2;						/* 0x41.[5:4]: Zone III speed control vs. temperature */
	uint16 AutoSpeedZone4		: 2;						/* 0x41.[7:6]: Zone IV speed control vs. temperature */
	uint16 AutoSpeedVoltage1	: 8;						/* 0x42: Auto-speed voltage #1 */
	uint16 AutoSpeedVoltage2	: 8;						/* 0x43: Auto-speed voltage #2 */
	uint16 AutoSpeedVoltage3	: 8;						/* 0x44: Auto-speed voltage #3 */
	uint16 AutoSpeedVoltageZone1: 2;						/* 0x45.[1:0]: Zone I speed control vs. voltage */
	uint16 AutoSpeedVoltageZone2: 2;						/* 0x45.[3:2]: Zone II speed control vs. voltage */
	uint16 AutoSpeedVoltageZone3: 2;						/* 0x45.[5:4]: Zone III speed control vs. voltage */
	uint16 AutoSpeedVoltageZone4: 2;						/* 0x45.[7:6]: Zone IV speed control vs. voltage */
#else  /* _SUPPORT_SPEED_AUTO */
	uint16 Reserved3E;										/* 0x3E: Reserved */
	uint16 Reserved40;										/* 0x40: Reserved */
	uint16 Reserved42;										/* 0x42: Reserved */
	uint16 Reserved44;										/* 0x44: Reserved */
#endif /* _SUPPORT_SPEED_AUTO */
	uint16 MinimumSpeed;									/* 0x46: Minimum speed */
	uint16 AccelerationConst;								/* 0x48: Acceleration-constant */
	uint16 AccelerationPoints	: 6;						/* 0x4A [5:0]: Acceleration-points */
	uint16 TachoMode			: 2;						/* 0x4A [7:6]: Tacho-mode */
	uint16 StallCurrentThreshold: 7;						/* 0x4B.[6:0]: Stall current threshold ratio */
	uint16 RestallPor			: 1;						/* 0x4B.[7]: Re-stall after POR dis/ena */
	/* Motor params */
	uint16 MotorConstant		: 8;						/* 0x4C: Motor Constant [10mV/rps] */
	uint16 MotorCoilRtot		: 8;						/* 0x4D: Motor coil resistance (total) */
	uint16 HoldingTorqueCurrent	: 8;						/* 0x4E: Holding Torque current threshold */
	uint16 RunningTorqueCurrent : 8;						/* 0x4F: Running Torque current threshold */
	/* Application diagnostic levels */
	uint16 ApplUnderTemperature	: 8;						/* 0x50: Application under-temperature */
	uint16 ApplOverTemperature	: 8;						/* 0x51: Application over-temperature */
	uint16 ApplUnderVoltage		: 8;						/* 0x52: Application under-voltage */
	uint16 ApplOverVoltage		: 8;						/* 0x53: Application over-voltage */
	/* Current vs. Temperature compensation */
	uint16 CurrThrshldTemp1		: 8;						/* 0x54: Current threshold temperature #1 */
	uint16 CurrThrshldRatio1	: 8;						/* 0x55: Current threshold ratio #1 */
	uint16 CurrThrshldTemp2		: 8;						/* 0x56: Current threshold temperature #2 */
	uint16 CurrThrshldRatio2	: 8;						/* 0x57: Current threshold ratio #2 */
	uint16 CurrThrshldTemp3		: 8;						/* 0x58: Current threshold temperature #3 */
	uint16 CurrThrshldRatio3	: 8;						/* 0x59: Current threshold ratio #3 */
	uint16 CurrThrshldTemp4		: 8;						/* 0x5A: Current threshold temperature #4 */
	uint16 CurrThrshldRatio4	: 8;						/* 0x5B: Current threshold ratio #4 */
	uint16 CurrThrshldZone1		: 1;						/* 0x5C.[0]: Zone I */ 
	uint16 CurrThrshldZone2		: 2;						/* 0x5C.[2:1]: Zone II */ 
	uint16 CurrThrshldZone3		: 2;						/* 0x5C.[4:3]: Zone III */ 
	uint16 CurrThrshldZone4		: 2;						/* 0x5C.[6:5]: Zone IV */ 
	uint16 CurrThrshldZone5		: 1;						/* 0x5C.[7]: Zone V */ 
	/* PID Control */
	uint16 PidRunningCtrlPeriod	: 8;						/* 0x5D: PID running control-period */
	uint16 PidHoldingCtrlPeriod	: 8;						/* 0x5E: PID holding control-period */
	uint16 PidCoefP				: 8;						/* 0x5F: PID-Coefficient P */
	uint16 PidCoefI				: 8;						/* 0x60: PID-Coefficient I */
	uint16 PidCoefD				: 8;						/* 0x61: PID-Coefficient D */
	uint16 PidThrshldCtrlPer	: 8;						/* 0x62: PID-period for threshold control */
	uint16 PidLowerHoldingLimit	: 8;						/* 0x63: PID Lower-limit Holding (output) */
	uint16 PidLowerLimit		: 8;						/* 0x64: PID Lower-limit Running (output) */
	uint16 PidUpperLimit		: 8;						/* 0x65: PID Upper-limit (output) */
	uint16 PidRampDown			: 8;						/* 0x66: PID Ramp-down limitation */
	uint16 PidRampUp			: 8;						/* 0x67: PID Ramp-up limitation */
	uint16 RewindSteps			: 8;						/* 0x68: Rewind steps */
	uint16 BoostTorqueCurrent	: 8;						/* 0x69: Running Torque-boost current threshold */
	uint16 SpeedTorqueBoost;								/* 0x6A: Speed (Torque-boost mode) */
	uint16 StallDetectorDelay	: 8;						/* 0x6C: Stall detector delay */
	uint16 LinUV				: 3;						/* 0x6D[2:0]: LIN UV threshold */
	uint16 Reserved6D			: 5;						/* 0x6D[7:3]: Reserved */
	uint16 HallLatches			: 2;						/* 0x6E[1:0]: Hall-Latches: 0b00: None, 0b01: One Hall-Latch, etc */
	uint16 DecelerationSteps	: 4;						/* 0x6E[5:2]: Deceleration-(u)Steps */
	uint16 AutoRecalibration	: 1;						/* 0x6E[6]: Auto re-calibration */
	uint16 BusTimeOutSleep		: 1;						/* 0x6E[7]: Bus-TimeOut Sleep */
	uint16 VdsThreshold			: 6;						/* 0x6F[5:0]: Vds Threshold */
	uint16 StallO				: 1;						/* 0x6F[6]: Stall "O" disabled/enabled */
	uint16 StallSpeedDepended	: 1;						/* 0x6F[7]: Stall speed depended */
} NVRAM_USER, *PNVRAM_USER;

typedef struct _NVRAM_ERRORLOG
{
	uint16	NvramProgramCycleCount;							/* 0x70: 16-bits Program cycle counter */
	uint16	ErrorLogIndex_CRC;								/* 0x72: Error-log index (0-11) & (Optional-CRC) */
#if _SUPPORT_LOG_NVRAM
	uint16	ErrorLog[6];									/* 0x74-0x7F: Error-log */
#endif /* _SUPPORT_LOG_NVRAM */
} NVRAM_ERRORLOG, *PNVRAM_ERRORLOG;
#define C_MAX_NVRAM_PROGRAM_COUNT		65000U				/* Maximum 65000 Write-cycles */
#define C_MAX_ERRORS_PER_PAGE			12U

#define C_NVRAM_STORE_OKAY				0x00U
#define C_NVRAM_STORE_MAX_WRITE_CYCLE	0x01U
#define C_NVRAM_STORE_INVALID_COUNTER	0x02U

#define EEP_FLASHTRIM					0x11FCU

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
#pragma space dp
extern NVRAM_USER g_NvramUser;
#pragma space none

extern void NVRAM_LoadUserPage( void);											/* Load user NVRAM page (NVRAM to User-RAM) */
extern uint16 NVRAM_PageVerify( const uint16 *pMRAM);							/* Verify NVRAM page versus RAM */
extern uint16 NVRAM_Store( uint16 u16Page);										/* Store user NVRAM page (shadow RAM to NVRAM) and load all NVRAM pages */
extern uint16 NVRAM_LogError( uint8 u8ErrorCode);								/* Store serious error code */
extern uint8 NVRAM_GetLastError( void);											/* Get last NVRAM error code */
extern void NVRAM_ClearErrorLog( void);											/* Clear NVRAM error log */
extern uint8 NVRAM_CRC8( uint8 byReplaceCRC);									/* Calculate CRC8 on User-NVRAM */
extern void NVRAM_StorePatch( void);											/* Store patch NVRAM page (shadow RAM to NVRAM) and load all NVRAM pages */
extern void NVRAM_MlxCalibrationAreaCheck( void);
#endif /* NVRAM_USER_PAGE_H_ */

/* EOF */
