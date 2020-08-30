/*! \file		NVRAM_UserPage.c
 *  \brief		MLX81310 NVRAM User Page
 *
 * \note		project MLX81310
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-16
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	NVRAM_CRC8()
 *				NVRAM_CountCRC8()
 *				NVRAM_PageVerify()
 *				NVRAM_Store()
 *				NVRAM_LoadUserPage()
 *				PlaceError()
 *				NVRAM_LogError()
 *				NVRAM_GetLastError()
 *				NVRAM_ClearErrorLog()
 *				NVRAM_EmergencyStore()
 *				NVRAM_StorePatch()
 *				NVRAM_MlxCalibrationAreaCheck()
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
 * ****************************************************************************	*/

#include "Build.h"
#include "NVRAM_UserPage.h"
#include "ADC.h"																/* ADC support */
#include "ErrorCodes.h"															/* Error-logging support */
#include "MotorDriver.h"														/* Motor-driver support */
#include "Timer.h"																/* Timer support */
#include <awdg.h>																/* Analogue Watchdog support */
#ifndef NVRAM_H_
#include <nvram.h>																/* NVRAM support */
#endif /* NVRAM_H_ */

#if ((LINPROT & LINXX) == LIN2X)												/* LIN 2.x */
#include "LIN_2x.h"
#endif /* ((LINPROT & LINXX) == LIN2X) */

#if ((LINPROT & LINXX) == LIN2J)
#include "LIN_SAE_J2602.h"
#endif /* ((LINPROT & LINXX) == LIN2J) */

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION (@TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp
NVRAM_USER g_NvramUser;
#pragma space none

const NVRAM_USER defNvramUser =
{
	0,													/* 0x00: CRC-8 */
	C_NVRAM_USER_REV,									/* 0x01: NVRAM Structure revision */
#if LIN_COMM
#if ((LINPROT & LINXX) == LIN2X)						/* LIN 2.x */
	/* Generic */
	C_DEFAULT_NAD,										/* 0x02: NAD */
	C_VARIANT,											/* 0x03: Variant-ID */
	0x81,												/* 0x04: HW-Revision (programmed by production) */
	C_SW_REF,											/* 0x05: (Reserved for) SW-Revision */
	mlxCONTROL,											/* 0x06: Control-message Frame-ID */
	mlxSTATUS,											/* 0x07: Status-message Frame-ID */
	CONFIGURATION_ID,									/* 0x08: Configuration ID */
	0,													/* 0x0A: LIN-AutoAaddressing Gain DifferentialMode-CommonMode delta */
	0xFFFF,												/* 0x0C: (Optional) Serial-number (LSW) (programmed by production) */
	0x2DC0,												/* 0x0E: (Optional) Serial-number (MSW) */
#if (LINPROT == LIN2X_ACT44)
	/* Actuator */
	0,													/* 0x10: Motor rotational direction: 0=CW, 1=CCW */
	1,													/* 0x10: Emergency-run: 0=Disabled, 1=Enabled */
	1,													/* 0x10: Emergency-run end-stop: 0=Low, 1=High */
	1,													/* 0x10: Stall-detector: 0=Disabled, 1=Enabled */
	1,													/* 0x10: Motor holding-current: 0=Off, 1=On */
	0,													/* 0x10: Motor speed */
	C_ENDSTOP_PAUSETIME,								/* 0x11: End-stop pause time */
	C_DEF_TRAVEL,										/* 0x12: Default Travel */
	C_DEF_TRAVEL_TOLERANCE,								/* 0x14: Default Travel Tolerance (Lower) */
	C_DEF_TRAVEL_TOLERANCE,								/* 0x15: Default Travel Tolerance (Upper) */
#endif /* (LINPROT == LIN2X_ACT44) */
#endif /* ((LINPROT & LINXX) == LIN2X) */
#if ((LINPROT & LINXX) == LIN2J)
	/* Generic */
	0x6C,												/* 0x02: NAD *///NAD for GM
	C_VARIANT,											/* 0x03: Variant-ID */
	0x81,												/* 0x04: HW-Revision (programmed by production) */
	C_SW_REF,											/* 0x05: (Reserved for) SW-Revision */
	0x00,												/* 0x06: Reserved */
	0x00,												/* 0x07: Reserved */
	CONFIGURATION_ID,									/* 0x08: Configuration ID */
	0,													/* 0x0A: LIN-AutoAaddressing Gain DifferentialMode-CommonMode delta */
	0xFFFF,												/* 0x0C: (Optional) Serial-number (LSW) (programmed by production) */
	0x2DC0,												/* 0x0E: (Optional) Serial-number (MSW) */
#if (LINPROT == LIN2J_VALVE_GM)
	/* Actuator */
	0,													/* 0x10: Motor rotational direction: 0=CW, 1=CCW */
	1,													/* 0x10: Emergency-run: 0=Disabled, 1=Enabled */
	1,													/* 0x10: Emergency-run end-stop: 0=Low, 1=High */
	1,													/* 0x10: Stall-detector: 0=Disabled, 1=Enabled */
	1,													/* 0x10: Motor holding-current: 0=Off, 1=On */
	0,													/* 0x10: Reserved */
	C_ENDSTOP_PAUSETIME,								/* 0x11: End-stop pause time */
	C_DEF_TRAVEL,										/* 0x12: Default Travel */
	C_DEF_TRAVEL_TOLERANCE,								/* 0x14: Default Travel Tolerance (Lower) */
	C_DEF_TRAVEL_TOLERANCE,								/* 0x15: Default Travel Tolerance (Upper) */
#endif /* (LINPROT == LIN2J_VALVE_GM) */
#endif /* ((LINPROT & LINXX) == LIN2J) */
#endif /* LIN_COMM */
#if ((LINPROT & LINX) == LIN20 )
	C_FUNCTION_ID,										/* 0x16: (Programmable) Function ID */
#else  /* ((LINPROT & LINX) == LIN20 ) */
	0xFFFF,												/* 0x16: Reserved (No LIN 2.x) */
#endif /* ((LINPROT & LINX) == LIN20 ) */
	C_SUPPLIER_ID,										/* 0x18: (Programmable) Supplier ID */
	0xFFFF,												/* 0x1A: Production Date */
#if (LINPROT == LIN2J_VALVE_GM)
	0,													/* 0x1C: Current Position (CPOS) */
	0,													/* 0x1E: Application Status */
#else  /* (LINPROT == LIN2J_VALVE_GM) */
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)										/* MMP150125-1 - Begin */
	C_INVALD_GAD,										/* 0x1C: Group-address */
	mlxGROUP_CONTROL,									/* 0x1D: Group-Control-message Frame-ID */
#else  /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
	0,													/* 0x1C: Reserved */
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */								/* MMP150125-1 - End */
	0,													/* 0x1E: Reserved */
#endif /* (LINPROT == LIN2J_VALVE_GM) */
	MOTOR_FAMILY,										/* 0x1F: Motor Family */

	0,													/* 0x20: Reserved */
	0,													/* 0x22: Reserved */
	0,													/* 0x24: Reserved */
	0,													/* 0x26: Reserved */
	0,													/* 0x28: Reserved */
	0,													/* 0x2A: Reserved */
	0,													/* 0x2C: Reserved */
	0,													/* 0x2E: Reserved */
	0			,										/* 0x30.[5:0]: Reserved */
	C_BROWN_OUT_LEVEL,									/* 0x30.[7:6]: Brown-out level */
	C_VSUP_REF,											/* 0x31: Vsupply reference [1/8V] */
	MOTOR_GEAR_BOX_RATIO,								/* 0x32.[12: 0]: Gear-box ratio, e.g. 600:1 */
	(MOTOR_POLE_PAIRS - 1),								/* 0x32.[15:13]: Number of pole-pairs + 1 */
#if _SUPPORT_STALLDET_B
	0,													/* 0x34.[4:0]: Stall-detector "B" delay */
#elif _SUPPORT_STALLDET_O
	C_STALL_O_WIDTH,									/* 0x34.[2:0]: Stall-detector "O" Width */
	0,													/* 0x34.[4:3]: Reserved */
#else
	0,													/* 0x34.[4:0]: Reserved */
#endif /* _SUPPORT_STALLDET_B */
	MOTOR_MICROSTEPS,									/* 0x34.[6:5]: Number of micro-steps: 2^n (or 1 << n) */
	(4 - MOTOR_PHASES),									/* 0x34.[7]: Number of motor-phases: 1 = 3-phase, 0 = Bi-polar */
#if _SUPPORT_STALLDET_B
	C_STALL_B_THRESHOLD,								/* 0x35: Stall-detector "B" threshold (BEMF) */
#elif _SUPPORT_STALLDET_O
	C_STALL_O_THRESHOLD,								/* 0x35: Stall-detector "O" threshold (Oscillation) */
#else
	0x00,												/* 0x35: Reserved */
#endif /* _SUPPORT_STALLDET_O */
	C_SPEED_0,											/* 0x36: Speed_1 */
	C_SPEED_1,											/* 0x38: Speed_2 */
	C_SPEED_2,											/* 0x3A: Speed_3 */
	C_SPEED_3,											/* 0x3C: Speed_3 */
#if _SUPPORT_SPEED_AUTO
	(C_AUTOSPEED_TEMP_1 + C_TEMPOFF),					/* 0x3E: Auto-speed temperature #1 */
	(C_AUTOSPEED_TEMP_2 + C_TEMPOFF),					/* 0x3F: Auto-speed temperature #2 */
	(C_AUTOSPEED_TEMP_3 + C_TEMPOFF),					/* 0x40: Auto-speed temperature #3 */
	C_AUTOSPEED_TEMPZONE_1,								/* 0x41.[1:0]: Zone I speed control vs. temperature */
	C_AUTOSPEED_TEMPZONE_2,								/* 0x41.[3:2]: Zone II speed control vs. temperature */
	C_AUTOSPEED_TEMPZONE_3,								/* 0x41.[5:4]: Zone III speed control vs. temperature */
	C_AUTOSPEED_TEMPZONE_4,								/* 0x41.[7:6]: Zone IV speed control vs. temperature */
	C_AUTOSPEED_VOLT_1,									/* 0x42: Auto-speed voltage #1 */
	C_AUTOSPEED_VOLT_2,									/* 0x43: Auto-speed voltage #2 */
	C_AUTOSPEED_VOLT_3,									/* 0x44: Auto-speed voltage #3 */
	C_AUTOSPEED_VOLTZONE_1,								/* 0x45.[1:0]: Zone I speed control vs. voltage */
	C_AUTOSPEED_VOLTZONE_2,								/* 0x45.[3:2]: Zone II speed control vs. voltage */
	C_AUTOSPEED_VOLTZONE_3,								/* 0x45.[5:4]: Zone III speed control vs. voltage */
	C_AUTOSPEED_VOLTZONE_4,								/* 0x45.[7:6]: Zone IV speed control vs. voltage */
#else  /* _SUPPORT_SPEED_AUTO */
	0,													/* 0x3E: Reserved */
	0,													/* 0x40: Reserved */
	0,													/* 0x42: Reserved */
	0,													/* 0x44: Reserved */
#endif /* _SUPPORT_SPEED_AUTO */
	C_SPEED_MIN,										/* 0x46: Minimum speed */
	C_ACCELERATION_CONST,								/* 0x48: Acceleration-constant */
	C_ACCELERATION_POINTS,								/* 0x4A.[5:0]: Acceleration-points */
	C_TACHO_MODE,										/* 0x4A.[7:6]: Tacho-mode */
	C_STALL_A_THRESHOLD,								/* 0x4B.[6:0]: Stall current threshold ratio */
	C_RESTALL_POR,										/* 0x4B.[7]: Restall POR flag */
	C_MOTOR_CONST_10MV_PER_RPS,							/* 0x4C: Motor Constant [10mV/rps] */
	C_COILS_RTOT,										/* 0x4D: Motor coil resistance (total) */
	C_PID_HOLDING_CURR_LEVEL,							/* 0x4E: Holding Torque current threshold */
#if (_SUPPORT_DOUBLE_MOTOR_CURRENT != FALSE)
	(C_PID_RUNNING_CURR_LEVEL / 2),						/* 0x4F: Running Torque current threshold */
#else  /* (_SUPPORT_DOUBLE_MOTOR_CURRENT != FALSE) */
	C_PID_RUNNING_CURR_LEVEL,							/* 0x4F: Running Torque current threshold */
#endif /* (_SUPPORT_DOUBLE_MOTOR_CURRENT != FALSE) */
	(C_APPL_UTEMP + C_TEMPOFF),							/* 0x50: Application under-temperature */
	(C_APPL_OTEMP + C_TEMPOFF),							/* 0x51: Application over-temperature */
	C_APPL_UVOLT,										/* 0x52: Application under-voltage */
	C_APPL_OVOLT,										/* 0x53: Application over-voltage */
	(C_CURRTHRSHLD_TEMP_1 + C_TEMPOFF),					/* 0x54: Current threshold temperature #1 */
	C_CURRTHRSHLD_RATIO_1,								/* 0x55: Current threshold ratio #1 */
	(C_CURRTHRSHLD_TEMP_2 + C_TEMPOFF),					/* 0x56: Current threshold temperature #2 */
	C_CURRTHRSHLD_RATIO_2,								/* 0x57: Current threshold ratio #2 */
	(C_CURRTHRSHLD_TEMP_3 + C_TEMPOFF),					/* 0x58: Current threshold temperature #3 */
	C_CURRTHRSHLD_RATIO_3,								/* 0x59: Current threshold ratio #3 */
	(C_CURRTHRSHLD_TEMP_4 + C_TEMPOFF),					/* 0x5A: Current threshold temperature #4 */
	C_CURRTHRSHLD_RATIO_4,								/* 0x5B: Current threshold ratio #4 */
	C_CURRTHRSHLD_AREA_1,								/* 0x5C.[0]: Zone I */ 
	C_CURRTHRSHLD_AREA_2,								/* 0x5C.[2:1]: Zone II */ 
	C_CURRTHRSHLD_AREA_3,								/* 0x5C.[4:3]: Zone III */ 
	C_CURRTHRSHLD_AREA_4,								/* 0x5C.[6:5]: Zone IV */ 
	C_CURRTHRSHLD_AREA_5,								/* 0x5C.[7]: Zone V */ 
	C_PID_RUNNINGCTRL_PERIOD,							/* 0x5D: PID running control-period */
	C_PID_HOLDINGCTRL_PERIOD,							/* 0x5E: PID holding control-period */
	C_PID_COEF_P,										/* 0x5F: PID-Coefficient P */
	C_PID_COEF_I,										/* 0x60: PID-Coefficient I */
	C_PID_COEF_D,										/* 0x61: PID-Coefficient D */
	C_PID_THRSHLDCTRL_PERIOD,							/* 0x62: PID-period for threshold control */
	C_MIN_HOLDCORR_RATIO,								/* 0x63: PID Lower-limit Holding (output) */
	C_MIN_CORR_RATIO,									/* 0x64: PID Lower-limit Running (output) */
	C_MAX_CORR_RATIO,									/* 0x65: PID Upper-limit (output) */
	0,													/* 0x66: PID Ramp-down limitation */
	0,													/* 0x67: PID Ramp-up limitation */
	C_REWIND_STEPS,										/* 0x68: Rewind-steps */
#if (_SUPPORT_DOUBLE_MOTOR_CURRENT != FALSE)
	(C_PID_BOOST_CURR_LEVEL / 2),						/* 0x69: Running Torque-boost current threshold */
#else  /* (_SUPPORT_DOUBLE_MOTOR_CURRENT != FALSE) */
	C_PID_BOOST_CURR_LEVEL,								/* 0x69: Running Torque-boost current threshold */
#endif /* (_SUPPORT_DOUBLE_MOTOR_CURRENT != FALSE) */
	C_SPEED_TORQUE_BOOST,								/* 0x6A: Speed Torque-boost */
	((C_DETECTOR_DELAY + 4) >> 3),						/* 0x6C: Stall detector delay */
#if _SUPPORT_LIN_UV
	C_LIN_UV,											/* 0x6D.[2:0]: LIN UV threshold (MMP131216-1) */
#else  /* _SUPPORT_LIN_UV */
	0,													/* 0x6D.[2:0]: Reserved */
#endif /* _SUPPORT_LIN_UV */
	0,													/* 0x6D.[7:3]: Reserved */
	C_HALL_LATCHES,										/* 0x6E.[1:0]: Hall Latches */
	(C_DECELERATION_STEPS - 1),							/* 0x6E.[5:2]: Deceleration-uSteps */
	C_AUTO_RECALIBRATE,									/* 0x6E.[6]: Auto re-calibrate */
#if (_SUPPORT_BUSTIMEOUT_SLEEP)
	C_BUSTIMEOUT_SLEEP,									/* 0x6E.[7]: Bus-Timeout Sleep */
#else  /* (_SUPPORT_BUSTIMEOUT_SLEEP) */
	0,
#endif /* (_SUPPORT_BUSTIMEOUT_SLEEP) */
	C_VDS_THRESHOLD,									/* 0x6F.[5:0]: VDS Threshold */
	C_STALL_O,											/* 0x6F.[6]: Stall "O" disabled/enabled */
	C_STALL_SPEED_DEPENDED								/* 0x6F.[7]: Stall threshold speed depended */
};

void PlaceError( uint16 *pu16ErrorElement, uint16 u16OddEven, uint8 u8ErrorCode);

/* ****************************************************************************	*
 *  NVRAM_CRC8
 *
 *	Pre:	byReplaceCRC = FALSE: Check CRC
 *							TRUE: Replace CRC (calculate)
 *	Post:	Result of the CRC8-calculation.
 *				byReplaceCRC = FALSE: 0xFF = CRC8 is correct, otherwise not.
 *								TRUE: Calculated CRC8
 *
 *	Calculate CRC8 on User-NVRAM
 * ****************************************************************************	*/
uint8 NVRAM_CRC8( uint8 byReplaceCRC)
{
	uint16 u16CRC;

	if ( byReplaceCRC != FALSE )
	{
		g_NvramUser.CRC8 = 0x00;
	}

	u16CRC = nvram_CalcCRC( (uint16 *) &g_NvramUser, (sizeof(g_NvramUser)/sizeof(uint16)));	/* MMP151202-1 */

	if ( byReplaceCRC != FALSE )
	{
		g_NvramUser.CRC8 = (0xFF - u16CRC);
	}

	return ( (uint8) u16CRC );
} /* End of NVRAM_CRC8() */

/* ****************************************************************************	*
 *  NVRAM_CountCRC8
 *
 *	Pre:	Pointer to NVRAM-structure of Write-cycle counter
 *			byReplaceCRC = FALSE: Check CRC
 *							TRUE: Replace CRC (calculate)
 *	Post:	(uint8) XOR result of calculated CRC8 and stored CRC8.
 *				byReplaceCRC = FALSE: When correct, the result = 0xFF, otherwise != 0xFF
 *								TRUE: Calculated CRC8.
 *
 *	Calculate CRC8 on Write-cycle counter.
 * ****************************************************************************	*/
uint8 NVRAM_CountCRC8( PNVRAM_ERRORLOG pNVERRLOG, uint8 byReplaceCRC)
{
	uint16 u16CRC = nvram_CalcCRC( (uint16 *) &pNVERRLOG->NvramProgramCycleCount, 1);	/* MMP151202-1 */

	if ( byReplaceCRC != FALSE )
	{
		pNVERRLOG->ErrorLogIndex_CRC = (pNVERRLOG->ErrorLogIndex_CRC & 0x00FF) | ((0xFF - u16CRC) << 8);
	}
	return ( (uint8) ((pNVERRLOG->ErrorLogIndex_CRC >> 8) ^ u16CRC) );
} /* End of NVRAM_CountCRC8() */

/* ****************************************************************************	*
 * void NVRAM_PageVerify
 *
 *	Pre:	Pointer to NVRAM-page shadow-RAM Memory
 *	Post:	FALSE: NVRAM shadow-RAM page structure and RAM-structure are not the same
 *			TRUE : NVRAM shadow-RAM page structure and RAM-structure are the same
 *
 * ****************************************************************************	*/
uint16 NVRAM_PageVerify( const uint16 *pMRAM)
{
	uint16 *pURAM = (uint16 *) &g_NvramUser;
	do
	{
		if ( *pMRAM++ != *pURAM++ )
		{
			/* Error */
			return ( FALSE );
		}
	} while ( pURAM < ((uint16 *) &g_NvramUser + (sizeof(g_NvramUser)/sizeof(uint16))) );
	return ( TRUE );
} /* End of NVRAM_PageVerify() */

/* ****************************************************************************	*
 * NVRAM_Store
 *
 *	Pre:	u16Page: C_NVRAM_USER_PAGE_1 : Write only first User Page
 *					 C_NVRAM_USER_PAGE_2 : Write only second User Page
 *					 C_NVRAM_USER_PAGE_ALL = (C_NVRAM_USER_PAGE_1 | C_NVRAM_USER_PAGE_2)
 *					 C_NVRAM_USER_PAGE_FORCE : Force Write without pre-verification if same data
 *	Post:	(uint16) C_NVRAM_STORE_OKAY : Stored successfully
 *					 C_NVRAM_STORE_MAX_WRITE_CYCLE : NVRAM Write-cycle reached maximum
 *
 * Store user NVRAM page (shadow RAM to NVRAM) and load all NVRAM pages.
 *
 * C_ADDR_USERPAGE1+0x00:0x7F:	User page #1 (64x 16-bits words)
 * C_ADDR_USERPAGE2+0x00:0x7F:	User page #2 (64x 16-bits words) (Backup)
 * ****************************************************************************	*/
uint16 NVRAM_Store( uint16 u16Page)
{
	uint16 *pURAM;
	uint16 *pMRAM;
	uint16 u16Result = C_NVRAM_STORE_OKAY;										/* MMP150219-1 */

	/* Store NVRAM */
	if ( (u16Page & C_MVRAM_USER_PAGE_NoCRC) == 0 )
	{
		(void) NVRAM_CRC8( TRUE);												/* Update CRC8 */
	}

	/* Copy NVRAM UserRAM-copy into NVRAM MirrorRAM */
	if ( (u16Page & C_NVRAM_USER_PAGE_1) &&
		 ((u16Page & C_NVRAM_USER_PAGE_FORCE) || (NVRAM_PageVerify( (uint16 *) C_ADDR_USERPAGE1) == FALSE)) ) /*lint !e845 */
	{
		/* Forced Write, or RAM differs from NVRAM; Copy RAM to NVRAM */
		do
		{
			/* Update NVRAM program cycle counter */							/* MMP150219-1 - Begin */
			PNVRAM_ERRORLOG pNVERRLOG = (PNVRAM_ERRORLOG) (C_ADDR_USERPAGE1 + sizeof(NVRAM_USER));
			if ( NVRAM_CountCRC8( pNVERRLOG, FALSE) != (uint8) 0xFFU )
			{
				/* Invalid NVRAM Program-counter */
				pNVERRLOG->NvramProgramCycleCount = 0U;
				u16Result = C_NVRAM_STORE_INVALID_COUNTER;
			}
#if _SUPPORT_UNLIMITED_NVRAM_WRITE_CYCLES
			if ( pNVERRLOG->NvramProgramCycleCount < 0xFFFEU )
			{
				pNVERRLOG->NvramProgramCycleCount++;
			}
#else  /* _SUPPORT_UNLIMITED_NVRAM_WRITE_CYCLES */
			if ( ++pNVERRLOG->NvramProgramCycleCount >= C_MAX_NVRAM_PROGRAM_COUNT )
			{
				u16Result = C_NVRAM_STORE_MAX_WRITE_CYCLE;
				break;															/* Skip NVRAM update (reached max-write cycle count) */
			}
#endif /* _SUPPORT_UNLIMITED_NVRAM_WRITE_CYCLES */
			(void) NVRAM_CountCRC8( pNVERRLOG, TRUE);							/* Calculate Cycle-count CRC8 */

			/* Copy System RAM version to NVRAM User-page 1 shadow-RAM */
			{
				pURAM = (uint16 *) &g_NvramUser;
				pMRAM = (uint16 *) C_ADDR_USERPAGE1;
				do
				{
					*pMRAM++ = *pURAM++;
				} while ( pURAM < ((uint16 *) &g_NvramUser + (sizeof(g_NvramUser)/sizeof(uint16))) );
			}																	/* MMP150219-1 - End */

			/* Save (NV)RAM to NV(RAM) */
			NVRAM_SavePage( NVRAM1_PAGE1);

#if _SUPPORT_NVRAM_RECOVER_CYCLE_ONCE
			/* Check (NV)RAM page */
			NVRAM_LoadAll();
			if ( NVRAM_PageVerify( (uint16 *) C_ADDR_USERPAGE1) == FALSE )
			{
				/* Error */
				SetLastError( (uint8) C_ERR_NVRAM_PG11);
				continue;
			}
#endif /* _SUPPORT_NVRAM_RECOVER_CYCLE_ONCE */
		} while ( FALSE );
	}

#if _SUPPORT_NVRAM_BACKUP
	/* Duplicate NVRAM user page 1 into page 2 */
	if ( (u16Page & C_NVRAM_USER_PAGE_2) &&
		 ((u16Page & C_NVRAM_USER_PAGE_FORCE) || (NVRAM_PageVerify( (uint16 *) C_ADDR_USERPAGE2) == FALSE)) ) /*lint !e845 */
	{
		/* Forced Write, or RAM differs from NVRAM; Copy RAM to NVRAM */
		do
		{
			/* Update NVRAM program cycle counter */							/* MMP150219-1 - Begin */
			PNVRAM_ERRORLOG pNVERRLOG = (PNVRAM_ERRORLOG) (C_ADDR_USERPAGE2 + sizeof(NVRAM_USER));
			if ( NVRAM_CountCRC8( pNVERRLOG, FALSE) != (uint8) 0xFFU )
			{
				/* Invalid NVRAM Program-counter */
				pNVERRLOG->NvramProgramCycleCount = 0U;
				u16Result = C_NVRAM_STORE_INVALID_COUNTER;
			}
#if _SUPPORT_UNLIMITED_NVRAM_WRITE_CYCLES
			if ( pNVERRLOG->NvramProgramCycleCount < 0xFFFEU )
			{
				pNVERRLOG->NvramProgramCycleCount++;
			}
#else  /* _SUPPORT_UNLIMITED_NVRAM_WRITE_CYCLES */
			if ( ++pNVERRLOG->NvramProgramCycleCount >= C_MAX_NVRAM_PROGRAM_COUNT )
			{
				u16Result = C_NVRAM_STORE_MAX_WRITE_CYCLE;
				break;															/* Skip NVRAM update (reached max-write cycle count) */
			}
#endif /* _SUPPORT_UNLIMITED_NVRAM_WRITE_CYCLES */
			(void) NVRAM_CountCRC8( pNVERRLOG, TRUE);

			/* Copy System RAM version to NVRAM User-page 2 shadow-RAM */
			{
				pURAM = (uint16 *) &g_NvramUser;
				pMRAM = (uint16 *) C_ADDR_USERPAGE2;
				do
				{
					*pMRAM++ = *pURAM++;
				} while ( pURAM < ((uint16 *) &g_NvramUser + (sizeof(g_NvramUser)/sizeof(uint16))) );
			}																	/* MMP150219-1 - End */

			/* Save (NV)RAM to NV(RAM) */
			NVRAM_SavePage( NVRAM2_PAGE1);

#if _SUPPORT_NVRAM_RECOVER_CYCLE_ONCE
			/* Check (NV)RAM page */
			NVRAM_LoadAll();
			if ( NVRAM_PageVerify( (uint16 *) C_ADDR_USERPAGE2) == FALSE )
			{
				/* Error */
				SetLastError( (uint8) C_ERR_NVRAM_PG21);
				continue;
			}
#endif /* _SUPPORT_NVRAM_RECOVER_CYCLE_ONCE */
		} while ( FALSE );
	}
#endif /* _SUPPORT_NVRAM_BACKUP */
	return ( u16Result );														/* MMP150219-1 */
} /* NVRAM_Store() */

/* ****************************************************************************	*
 * void NVRAM_LoadUserPage
 *
 * Load user NVRAM page (NVRAM to User-RAM).
 * ****************************************************************************	*/
void NVRAM_LoadUserPage( void)
{
	uint16 u16ErrorFlag;
	uint16 *pURAM = (uint16 *) &g_NvramUser;
#if (LINPROT == LIN2J_VALVE_GM)
	uint16 *pMRAM;
	if ( (((NVRAM_USER *) C_ADDR_USERPAGE1)->AppStatus & 0x80) ^ (((NVRAM_USER *) C_ADDR_USERPAGE2)->AppStatus & 0x80) )
		pMRAM = (uint16 *) C_ADDR_USERPAGE2;
	else
		pMRAM = (uint16 *) C_ADDR_USERPAGE1;
#else  /* (LINPROT == LIN2J_VALVE_GM) */
	uint16 *pMRAM = (uint16 *) C_ADDR_USERPAGE1;
#endif /* (LINPROT == LIN2J_VALVE_GM) */

	NVRAM_LoadAll();
	u16ErrorFlag = (VARIOUS_L & EENV_DED);										/* Double-bit error state */
	/* Copy NVRAM MirrorRAM-copy into NVRAM UserRAM (to allow byte/bit access) */
	do
	{
		*pURAM++ = *pMRAM++;
	} while ( pURAM < (uint16 *) &g_NvramUser + (sizeof(g_NvramUser)/sizeof(uint16)) );
	/* Check Double-bit NVRAM set, User-NVRAM structure-revision and User-NVRAM Checksum */
	if ( ((u16ErrorFlag == FALSE) && (VARIOUS_L & EENV_DED)) || (g_NvramUser.Revision != C_NVRAM_USER_REV) || (NVRAM_CRC8( FALSE) != 0xFF) || (g_NvramUser.ConfigurationID != CONFIGURATION_ID) )
	{
		u16ErrorFlag = TRUE;													/* User page #1 is corrupt */
	}
	else
	{
		u16ErrorFlag = FALSE;													/* User page #1 is valid, and will be used */
	}

#if _SUPPORT_NVRAM_BACKUP
	/* Check Double-Bit Error NVRAM, Wrong revision or Invalid Checksum */
	if ( u16ErrorFlag )
	{
		/* Double-bit error or incorrect revision or Invalid CRC; Copy NVRAM user page 2 into UserRAM */
		/* Note: EENV_DED can't be cleared !! In case of first user-page DBE, a second-page DBE can't be detected */
		u16ErrorFlag = (VARIOUS_L & EENV_DED);

#if (LINPROT == LIN2J_VALVE_GM)
		if ( (((NVRAM_USER *) C_ADDR_USERPAGE1)->AppStatus & 0x80) ^ (((NVRAM_USER *) C_ADDR_USERPAGE2)->AppStatus & 0x80) )
			pMRAM = (uint16 *) C_ADDR_USERPAGE1;
		else
			pMRAM = (uint16 *) C_ADDR_USERPAGE2;
#else  /* (LINPROT == LIN2J_VALVE_GM) */
		pMRAM = (uint16 *) C_ADDR_USERPAGE2;
#endif /* (LINPROT == LIN2J_VALVE_GM) */
		pURAM = (uint16 *) &g_NvramUser;
		do
		{
			*pURAM++ = *pMRAM++;
		} while ( pURAM < (uint16 *) &g_NvramUser + (sizeof(g_NvramUser)/sizeof(uint16)) );
		if ( ((u16ErrorFlag != FALSE) || ((VARIOUS_L & EENV_DED) == 0)) && (g_NvramUser.Revision == C_NVRAM_USER_REV) && (NVRAM_CRC8( FALSE) == 0xFF) && (g_NvramUser.ConfigurationID == CONFIGURATION_ID) )
		{
			/* Second User Page is correct; Rewrite 1st page */
			SetLastError( (uint8) C_ERR_INV_USERPAGE_1);
			(void) NVRAM_Store( (uint16) (C_NVRAM_USER_PAGE_1 | C_NVRAM_USER_PAGE_FORCE));
			u16ErrorFlag = FALSE;
		}
		else
		{
			u16ErrorFlag = TRUE;
		}
	}
#if (LINPROT != LIN2J_VALVE_GM)
	else
	{
		/* First user page is correct; Check 2nd page against first page (bit-compare) */
		pMRAM = (uint16 *) C_ADDR_USERPAGE2;
		pURAM = (uint16 *) &g_NvramUser;
		do
		{
			if ( *pURAM++ != *pMRAM++ )
			{
				/* Not equal! Rewrite 2nd page */
				SetLastError( (uint8) C_ERR_INV_USERPAGE_2);
				(void) NVRAM_Store( (uint16) (C_NVRAM_USER_PAGE_2 | C_NVRAM_USER_PAGE_FORCE));
				break;
			}
		} while ( pURAM < (uint16 *) &g_NvramUser + (sizeof(g_NvramUser)/sizeof(uint16)) );
		u16ErrorFlag = FALSE;
	}
#endif /* (LINPROT != LIN2J_VALVE_GM) */
#endif /* _SUPPORT_NVRAM_BACKUP */

#if (MOTOR_PARAMS == MP_NVRAM)
#if (_SUPPORT_CODE_PARAMS == FALSE)
	if ( u16ErrorFlag )
	{
		/* First and/or second page is corrupt; Log error and use default data */
		SetLastError( (uint8) C_ERR_INV_USERPAGE_BOTH);
#else  /* (_SUPPORT_CODE_PARAMS == FALSE) */
		/* Always use hard-coded NVRAM value to be re-written in RAM-structure */
#endif /* (_SUPPORT_CODE_PARAMS == FALSE) */

		/* UniROM */
		pMRAM = (uint16 *) &defNvramUser;
		pURAM = (uint16 *) &g_NvramUser;
		do
		{
			*pURAM++ = *pMRAM++;
		} while ( pURAM < (uint16 *) &g_NvramUser + (sizeof(g_NvramUser)/sizeof(uint16)) );

#if (_SUPPORT_CODE_PARAMS == FALSE)
		{
			PNVRAM_ERRORLOG pNVERRLOG_UPG = (PNVRAM_ERRORLOG) (C_ADDR_USERPAGE1 + sizeof(NVRAM_USER));
			if ( NVRAM_CountCRC8( pNVERRLOG_UPG, FALSE) != 0xFF )				/* Check NVRAM User Page #1 Write-cycle counter CRC */
			{
				pNVERRLOG_UPG->NvramProgramCycleCount = 0;						/* Clear program cycle-counter */
				pNVERRLOG_UPG->ErrorLogIndex_CRC = 0;							/* Clear Error-log index & CRC */
			}
			pNVERRLOG_UPG = (PNVRAM_ERRORLOG) (C_ADDR_USERPAGE2 + sizeof(NVRAM_USER));
			if ( NVRAM_CountCRC8( pNVERRLOG_UPG, FALSE) != 0xFF )				/* Check NVRAM User Page #2 Write-cycle counter CRC */
			{
				pNVERRLOG_UPG->NvramProgramCycleCount = 0;						/* Clear program cycle-counter */
				pNVERRLOG_UPG->ErrorLogIndex_CRC = 0;							/* Clear Error-log index & CRC */
			}
		}
		(void) NVRAM_Store( (uint16) (C_NVRAM_USER_PAGE_ALL | C_NVRAM_USER_PAGE_FORCE));	/* Write (both) user page(s) with default data */
	}
#else  /* (_SUPPORT_CODE_PARAMS == FALSE) */
	/* Only update RAM-CRC8, not write it to NVRAM */
	(void) NVRAM_CRC8( TRUE);													/* Update CRC8 */
#endif /* (_SUPPORT_CODE_PARAMS == FALSE) */
#endif /* (MOTOR_PARAMS == MP_NVRAM) */

#if ((LINPROT & LINXX) == LIN2J)
	/* Generic */
	(void)g_NvramUser.HwRef;													/* 0x04: HW-Revision */
	(void)g_NvramUser.SwRef;													/* 0x05: (Reserved for) SW-Revision */
	(void)g_NvramUser.ControlFrameID;											/* 0x06: Control-message Frame-ID */
	(void)g_NvramUser.StatusFrameID;											/* 0x07: Status-message Frame-ID */
	(void)g_NvramUser.ConfigurationID;											/* 0x08: Configuration ID */
	(void)g_NvramUser.AASDMCM_delta;											/* 0x0A: LIN-AutoAaddressing Gain DifferentialMode-CommonMode delta */
#endif /* ((LINPROT & LINXX) == LIN2J) */
#if (LINPROT == LIN2X_ACT44)
	(void)g_NvramUser.NAD;														/* 0x02: NAD */
	(void)g_NvramUser.Variant;													/* 0x03: Variant-ID */
	(void)g_NvramUser.ControlFrameID;											/* 0x06: Control-message Frame-ID */
	(void)g_NvramUser.StatusFrameID;											/* 0x07: Status-message Frame-ID */
	(void)g_NvramUser.ConfigurationID;											/* 0x08: Configuration ID */
#endif /* (LINPROT == LIN2X_ACT44) */
	(void)g_NvramUser.FunctionID;												/* 0x16: Function ID */
	(void)g_NvramUser.CustomerID;												/* 0x18: Customer ID */
	(void)g_NvramUser.ProductionDate;											/* 0x1A: Production Date: [15:9] Year (00-99), [8:5] Month (1-12), [4:0] Day (1-31) */
#if (LINPROT != LIN2J_VALVE_GM)
#if (_SUPPORT_HVAC_GROUP_ADDRESS == FALSE)										/* MMP150125-1 - Begin */
//	(void)g_NvramUser.Reserved1C;												/* 0x1C: Reserved */
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS == FALSE) */								/* MMP150125-1 - End */
//	(void)g_NvramUser.Reserved1E;
#endif /* (LINPROT != LIN2J_VALVE_GM) */
	(void)g_NvramUser.Reserved20;												/* 0x20: Reserved */
	(void)g_NvramUser.Reserved22;												/* 0x22: Reserved */
	(void)g_NvramUser.Reserved24;												/* 0x24: Reserved */
	(void)g_NvramUser.Reserved26;												/* 0x26: Reserved */
	(void)g_NvramUser.Reserved28;												/* 0x28: Reserved */
	(void)g_NvramUser.Reserved2A;												/* 0x2A: Reserved */
	(void)g_NvramUser.Reserved2C;												/* 0x2C: Reserved */
	(void)g_NvramUser.Reserved2E;												/* 0x2E: Reserved */
	(void)g_NvramUser.MotorFamily;
	(void)g_NvramUser.LeadAngle;
#if _SUPPORT_SPEED_AUTO
	(void)g_NvramUser.AutoSpeedZone1;
	(void)g_NvramUser.AutoSpeedZone2;
	(void)g_NvramUser.AutoSpeedZone3;
	(void)g_NvramUser.AutoSpeedZone4;
	(void)g_NvramUser.AutoSpeedVoltage3;
	(void)g_NvramUser.AutoSpeedVoltageZone1;
	(void)g_NvramUser.AutoSpeedVoltageZone2;
	(void)g_NvramUser.AutoSpeedVoltageZone3;
	(void)g_NvramUser.AutoSpeedVoltageZone4;
#else  /* _SUPPORT_SPEED_AUTO */
	(void)g_NvramUser.Reserved3E;												/* 0x3E: Reserved */
	(void)g_NvramUser.Reserved40;												/* 0x40: Reserved */
	(void)g_NvramUser.Reserved42;												/* 0x42: Reserved */
	(void)g_NvramUser.Reserved44;												/* 0x44: Reserved */
#endif /* _SUPPORT_SPEED_AUTO */
	(void)g_NvramUser.TachoMode;
	(void)g_NvramUser.ApplUnderTemperature;
	(void)g_NvramUser.PidRampDown;
	(void)g_NvramUser.PidRampUp;
	(void)g_NvramUser.BoostTorqueCurrent;
	(void)g_NvramUser.VdsThreshold;
	(void)g_NvramUser.StallO;

} /* NVRAM_LoadUserPage() */

/* ****************************************************************************	*
 * void PlaceError
 *
 *	Pre:	*pu16ErrorElement: Pointer to NVRAM 16-bit Word address
 *			u16OddEven: FALSE: LSB of 16-bits Word, TRUE: MSB of 16-bits Word
 *			u8ErrorCode: Error-code
 *	Post:	-
 *	Comments: Write error-code into NVRAM (16-bits words based)
 * ****************************************************************************	*/
void PlaceError( uint16 *pu16ErrorElement, uint16 u16OddEven, uint8 u8ErrorCode)
{
	if ( u16OddEven )
	{
		/* Odd index: MSB of uint16 */
		*pu16ErrorElement = (uint8)(*pu16ErrorElement) | (((uint16) u8ErrorCode) << 8);
	}
	else
	{
		/* Even index: LSB of uint16 */
		*pu16ErrorElement = (*pu16ErrorElement & 0xFF00) | ((uint16) u8ErrorCode);
	}
} /* End of PlaceError() */

/* ****************************************************************************	*
 * NVRAM_LogError
 *
 *	Pre:	8-bit error-code
 *	Post:	(uint16) C_NVRAM_STORE_OKAY : Stored successfully
 *					 C_NVRAM_STORE_MAX_WRITE_CYCLE : NVRAM Write-cycle reached maximum
 *
 * Store severe error-code. Circular buffer. Write error-code at 'empty/over-write'
 *	index and update index. In case index beyond array-size, set to 0xFF. In case both 
 *	arrays on both pages full, set index to 0xFF or 0xFE. The XOR-of both page indexes
 *	result is either 0 or 1, indicating which page should be written next-time.
 * Index at user-page #1/#2: 0-11: Empty/over-write index, FF: Full
 * If index user-page #1 is not 0xFF (Full), write error at page #1, otherwise page #2
 * ****************************************************************************	*/
uint16 NVRAM_LogError( uint8 u8ErrorCode)
{
	uint16 u16Result = C_NVRAM_STORE_OKAY;										/* MMP150219-1 */
	PNVRAM_ERRORLOG pNVERRLOG_UPG1 = (PNVRAM_ERRORLOG) (C_ADDR_USERPAGE1 + sizeof(NVRAM_USER));
	PNVRAM_ERRORLOG pNVERRLOG_UPG2 = (PNVRAM_ERRORLOG) (C_ADDR_USERPAGE2 + sizeof(NVRAM_USER));
	uint16 u16ErrorLogIdx1 = (uint8) (pNVERRLOG_UPG1->ErrorLogIndex_CRC);
	uint16 u16ErrorLogIdx2 = (uint8) (pNVERRLOG_UPG2->ErrorLogIndex_CRC);
	
	if ( (u16ErrorLogIdx1 ^ u16ErrorLogIdx2) == 0x00 )
	{
		u16ErrorLogIdx1 = 0x00;													/* Second UserPage last time written full; Start from index 0 at User Page #1 */
	}
	else if ( (u16ErrorLogIdx1 ^ u16ErrorLogIdx2) == 0x01 )
	{
		u16ErrorLogIdx2 = 0x00;
	}
	
	if ( (u16ErrorLogIdx1 & 0x80) == 0x00 )
	{
		/* Store error on User-Page #1 */										/* MMP150219-1 - Begin */
		if ( u16ErrorLogIdx1 >= C_MAX_ERRORS_PER_PAGE )							/* Check against array overflow */
		{
			u16ErrorLogIdx1 = 0;
		}
		if ( NVRAM_CountCRC8( pNVERRLOG_UPG1, FALSE) == 0x00 )
		{
			pNVERRLOG_UPG1->NvramProgramCycleCount++;
		}
		else
		{
			pNVERRLOG_UPG1->NvramProgramCycleCount = 1U;
		}																		/* MMP150219-1 - End */
		if ( pNVERRLOG_UPG1->NvramProgramCycleCount < (C_MAX_NVRAM_PROGRAM_COUNT - 1000) )
		{
			(void) NVRAM_CountCRC8( pNVERRLOG_UPG1, TRUE);
			PlaceError( (uint16 *) &(pNVERRLOG_UPG1->ErrorLog[u16ErrorLogIdx1 >> 1]), u16ErrorLogIdx1 & 0x01, u8ErrorCode);
			/* Update Error-log Index */
			u16ErrorLogIdx1++;
			if ( u16ErrorLogIdx1 >= C_MAX_ERRORS_PER_PAGE )
			{
				/* Array full; Switch to page #2 */
				if ( u16ErrorLogIdx2 & 0x80 )
				{
					u16ErrorLogIdx1 = (u16ErrorLogIdx2 ^ 0x01);
				}
				else
				{
					u16ErrorLogIdx1 = 0xFF;
				}
			}
			pNVERRLOG_UPG1->ErrorLogIndex_CRC = ((pNVERRLOG_UPG1->ErrorLogIndex_CRC) & 0xFF00) | u16ErrorLogIdx1;
			/* Save (NV)RAM to NV(RAM) */
			NVRAM_SavePage( NVRAM1_PAGE1 | NVRAM_PAGE_WR_SKIP_WAIT);
		}
		else
		{
			u16Result = C_NVRAM_STORE_MAX_WRITE_CYCLE;
		}
	}
	else 
	{
		/* Store error on User-Page #2 */										/* MMP150219-1 - Begin */
		if ( u16ErrorLogIdx2 >= C_MAX_ERRORS_PER_PAGE )							/* Check against array overflow */
		{
			u16ErrorLogIdx2 = 0;
		}
		if ( NVRAM_CountCRC8( pNVERRLOG_UPG2, FALSE) == 0x00 )
		{
			pNVERRLOG_UPG2->NvramProgramCycleCount++;
		}
		else
		{
			pNVERRLOG_UPG2->NvramProgramCycleCount = 1U;
		}																		/* MMP150219-1 - End  */
		if ( pNVERRLOG_UPG2->NvramProgramCycleCount < (C_MAX_NVRAM_PROGRAM_COUNT - 1000) )
		{
			(void) NVRAM_CountCRC8( pNVERRLOG_UPG2, TRUE);
			PlaceError( (uint16 *) &(pNVERRLOG_UPG2->ErrorLog[u16ErrorLogIdx2 >> 1]), u16ErrorLogIdx2 & 0x01, u8ErrorCode);
			/* Update Error-log Index */
			u16ErrorLogIdx2++;
			if ( u16ErrorLogIdx2 >= C_MAX_ERRORS_PER_PAGE )
			{
				/* Array full; Switch to page #1  */
				u16ErrorLogIdx2 = u16ErrorLogIdx1;
			}
			pNVERRLOG_UPG2->ErrorLogIndex_CRC = ((pNVERRLOG_UPG2->ErrorLogIndex_CRC) & 0xFF00) | u16ErrorLogIdx2;
			/* Save (NV)RAM to NV(RAM) */
			NVRAM_SavePage( NVRAM2_PAGE1 | NVRAM_PAGE_WR_SKIP_WAIT);
		}
		else
		{
			u16Result = C_NVRAM_STORE_MAX_WRITE_CYCLE;
		}
	}
	return ( u16Result );														/* MMP150219-1 */
} /* End of NVRAM_LogError() */

/* ****************************************************************************	*
 * uint8 NVRAM_GetLastError
 *
 *	Pre:	-
 *	Post:	(uint8) Last logged error-code
 * ****************************************************************************	*/
uint8 NVRAM_GetLastError( void)
{
	uint16 u16ErrorLogIdx1, u16ErrorLogIdx2;
	uint8 u8Result = 0x00;														/* No error's */
	PNVRAM_ERRORLOG pNVERRLOG_UPG1 = (PNVRAM_ERRORLOG) (C_ADDR_USERPAGE1 + sizeof(NVRAM_USER));
	PNVRAM_ERRORLOG pNVERRLOG_UPG2 = (PNVRAM_ERRORLOG) (C_ADDR_USERPAGE2 + sizeof(NVRAM_USER));
	u16ErrorLogIdx1 = (uint8) (pNVERRLOG_UPG1->ErrorLogIndex_CRC);
	u16ErrorLogIdx2 = (uint8) (pNVERRLOG_UPG2->ErrorLogIndex_CRC);

	if ( (u16ErrorLogIdx1 != 0x00) || (u16ErrorLogIdx2 != 0x00) )
	{
		if ( (u16ErrorLogIdx1 ^ u16ErrorLogIdx2) == 0x00 )
		{
			u16ErrorLogIdx2 = C_MAX_ERRORS_PER_PAGE;								/* Second UserPage last time written full */
		}
		else if ( (u16ErrorLogIdx1 ^ u16ErrorLogIdx2) == 0x01 )
		{
			u16ErrorLogIdx1 = C_MAX_ERRORS_PER_PAGE;								/* First UserPage last time written full, and Second UserPage is full */
		}
		else if ( (u16ErrorLogIdx1 == 0xFF) && (u16ErrorLogIdx2 == 0) )				/* MMP140218-1 */
		{
			u16ErrorLogIdx1 = C_MAX_ERRORS_PER_PAGE;								/* First UserPage last time written full, and Second UserPage is empty */
		}																			/* MMP140218-1 */

		if ( (u16ErrorLogIdx1 & 0x80) == 0x00 )
		{
			/* Get last error from User-Page #1 */
			u16ErrorLogIdx1--;
			{
				uint16 u16ErrorCodes = pNVERRLOG_UPG1->ErrorLog[u16ErrorLogIdx1 >> 1];
				if ( u16ErrorLogIdx1 & 0x01 )
				{
					u8Result = (uint8) (u16ErrorCodes >> 8);
				}
				else
				{
					u8Result = (uint8) u16ErrorCodes;
				}
			}
		}
		else
		{
			/* Get last error from User-Page #2 */
			u16ErrorLogIdx2--;
			{
				uint16 u16ErrorCodes = pNVERRLOG_UPG2->ErrorLog[u16ErrorLogIdx2 >> 1];
				if ( u16ErrorLogIdx2 & 0x01 )
				{
					u8Result = (uint8) (u16ErrorCodes >> 8);
				}
				else
				{
					u8Result = (uint8) u16ErrorCodes;
				}
			}
		}
	}
	return( u8Result );

} /* End of NVRAM_GetLastError() */

/* ****************************************************************************	*
 * void NVRAM_ClearErrorLog
 *
 *	Pre:	-
 *	Post:	-
 *	Comments: Clear Application Error logging (in both User-NVRAM pages)
 * ****************************************************************************	*/
void NVRAM_ClearErrorLog( void)
{
	uint16 i;
	PNVRAM_ERRORLOG pNVERRLOG_UPG = (PNVRAM_ERRORLOG) (C_ADDR_USERPAGE1 + sizeof(NVRAM_USER));
	pNVERRLOG_UPG->ErrorLogIndex_CRC = ((pNVERRLOG_UPG->ErrorLogIndex_CRC) & 0xFF00) | 0x00;	/* Set index at 0x00 */
	for ( i = 0; i < (C_MAX_ERRORS_PER_PAGE/2); i++ )
	{
		pNVERRLOG_UPG->ErrorLog[i] = 0x0000;
	}
	(void) NVRAM_CountCRC8( pNVERRLOG_UPG, TRUE);
	NVRAM_SavePage( NVRAM1_PAGE1);

	pNVERRLOG_UPG = (PNVRAM_ERRORLOG) (C_ADDR_USERPAGE2 + sizeof(NVRAM_USER));
	pNVERRLOG_UPG->ErrorLogIndex_CRC = ((pNVERRLOG_UPG->ErrorLogIndex_CRC) & 0xFF00) | 0x00;	/* Set index at 0x00 */
	for ( i = 0; i < (C_MAX_ERRORS_PER_PAGE/2); i++ )
	{
		pNVERRLOG_UPG->ErrorLog[i] = 0x0000;
	}
	(void) NVRAM_CountCRC8( pNVERRLOG_UPG, TRUE);
	NVRAM_SavePage( NVRAM2_PAGE1);
} /* End of NVRAM_ClearErrorLog() */

/* ****************************************************************************	*
 * void NVRAM_StorePatch
 *
 * Store Patch NVRAM page (shadow RAM to NVRAM) and load all NVRAM pages.
 *
 * C_ADDR_PATCHPAGE+0x00:0x6B:	Patch-code
 * C_ADDR_PATCHPAGE+0x6C:0x6D:	PATCH0_I
 * C_ADDR_PATCHPAGE+0x6E:0x6F:	PATCH1_I
 * C_ADDR_PATCHPAGE+0x70:0x71:	PATCH2_I
 * C_ADDR_PATCHPAGE+0x72:0x73:	PATCH3_I
 * C_ADDR_PATCHPAGE+0x74:0x75:	PATCH0_A
 * C_ADDR_PATCHPAGE+0x76:0x77:	PATCH1_A
 * C_ADDR_PATCHPAGE+0x78:0x79:	PATCH2_A
 * C_ADDR_PATCHPAGE+0x7A:0x7B:	PATCH3_A
 *
 * ****************************************************************************	*/
void NVRAM_StorePatch( void)
{
	NVRAM_SavePage( NVRAM1_PAGE2);
	NVRAM_LoadUserPage();
} /* End of NVRAM_StorePatch() */


/* ****************************************************************************	*
 * void NVRAM_MlxCalibrationAreaCheck()
 *
 *	Pre:	-
 *	Post:	-
 * Comments: Check Melexis NVRAM-page gain-factors (non-zero)
 * ****************************************************************************	*/
void NVRAM_MlxCalibrationAreaCheck( void)
{
	if ( (EE_GLAA == 0) ||														/* Auto addressing gain calibration */
		 (EE_GDMCMAA == 0) ||													/* Common Mode Rejection factor */
		 (EE_GMCURR == 0) ||													/* CGAINCAL_FLT (Current sensor filter gain) */
		 (EE_GVOLTAGE == 0) ||													/* VGAINCAL (supply sensor filter gain) */
		 (EE_GTEMP == 0) )														/* Temperature Gain */
	{
		SetLastError( (uint8) C_ERR_NVRAM_MLX_CAL_GN);
	}

} /* End of NVRAM_MlxCalibrationAreaCheck() */

/* EOF */
