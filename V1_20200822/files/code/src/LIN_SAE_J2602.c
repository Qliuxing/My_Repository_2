/*! ----------------------------------------------------------------------------
 * \file		LIN_SAE_J2602.c
 * \brief		MLX81300 LIN J2602 communication handling
 *				Based on SAE J2602-1 Revised September-2005
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	LIN_SAE_J2602_Init()
 * 				LIN_SAE_J2602_Store()
 *				HandleActCfrCtrl()
 *				HandleActRfrSta()
 *				HandleBusTimeout()
 *				LIN2J_ErrorHandling()
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

#if ((LINPROT & LINXX) == LIN2J)

#include "LIN_Communication.h"
#include <lin.h>
#include "lin_internal.h"														/* LinFrame (MMP160613-1) */
#include "main.h"
#include "ADC.h"																/* ADC support */
#include "ErrorCodes.h"															/* Error-logging support */
#include "NVRAM_UserPage.h"														/* NVRAM Functions & Layout */
#include "PID_Control.h"
#include "private_mathlib.h"
#include <plib.h>																/* Use Melexis MLX81300 library (WDG_Manager) */
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION (@TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp
uint8 g_u8NAD = C_DEFAULT_J2602_NAD;											/* Actual NAD */
#if (LINPROT != LIN2J_VALVE_VW)
uint8 g_u8SAE_ErrorFlags = 0;
uint8 g_u8SAE_SendErrorState = 0;												/* Copy of g_u8SAE_ErrorFlags */
#else  /* (LINPROT != LIN2J_VALVE_VW) */
uint8 g_u8SAE_SendErrorState = FALSE;											/* Copy of g_u8ErrorCommunication */
#endif /* (LINPROT != LIN2J_VALVE_VW) */
uint8 l_u8ActDirection = 0;
#pragma space none

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	(@NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp																/* __NEAR_SECTION__ */
uint8 l_u8ActualPositionValid = FALSE;											/* Actual position is not valid */
uint8 l_e8PositionType = C_POSTYPE_INIT;										/* Status-flag position-type */
uint8 l_u8StaCounter = 0;
#pragma space none																/* __NEAR_SECTION__ */


/* ****************************************************************************	*
 * CalcProtectionBits
 *
 * ****************************************************************************	*/
uint8 CalcProtectionBits( uint8 byFrameID)
{
	byFrameID |= ((byFrameID & 0x01) ^ ((byFrameID & 0x02) >> 1) ^ ((byFrameID & 0x04) >> 2) ^ ((byFrameID & 0x10) >> 4)) ? 0x40 : 0x00;
	byFrameID |= (((byFrameID & 0x02) >> 1) ^ ((byFrameID & 0x08) >> 3) ^ ((byFrameID & 0x10) >> 4) ^ ((byFrameID & 0x20) >> 5)) ? 0x00 : 0x80;
	return ( byFrameID );
} /* End of CalcProtectionBits() */

/* ****************************************************************************	*
 * LIN SAE J2602 initialisation
 *
 * This routine initialise the SAE-J2602 interface.
 * 	Based on the NAD, the LIN frame-ID are defined for CONTROL and STATUS.
 * ****************************************************************************	*/
void LIN_SAE_J2602_Init( uint16 u16WarmStart)
{
	(void) u16WarmStart;

#if (LINPROT == LIN2J_VALVE_VW)
	/* Check wake-up from SLEEP (MMP160613-2) */
	if ( ANA_INB & WAKEUP_LIN )
	{
		/* Restore: NAD, Motor-direction, CPOS and Status */
		g_u8NAD = g_NvramUser.NAD;
		l_u8ActDirection = g_NvramUser.MotorDirectionCCW;

		/* The valve shall attempt to move and hit the Link/Bypass end stop if
		 * it was parked at 0%/100% position before sleep */
		{
			uint16 u16Pos = g_NvramUser.CPOS;
			if ( l_u8ActDirection != FALSE )
			{
				u16Pos = C_MAX_POS - u16Pos;
			}
			if ( u16Pos == C_MIN_POS )
			{
				g_u16TargetPosition = 0;
				g_u16ActualPosition = (2 * C_PERC_OFFSET);
				g_e8CalibrationStep = (uint8) C_CALIB_START;
				if ( g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED )
				{
					g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_START;
				}
				else
				{
					g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_START;
				}
			}
			else if ( u16Pos == C_MAX_POS )
			{
				g_u16TargetPosition = g_u16CalibTravel + (2 * C_PERC_OFFSET);
				g_u16ActualPosition = g_u16CalibTravel;
				g_e8CalibrationStep = (uint8) C_CALIB_START;
				if ( g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED )
				{
					g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_START;
				}
				else
				{
					g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_START;
				}
			}
			else
			{
				/* Not Link/Bypass end stop */
				g_u16ActualPosition = muldivU16_U16byU16byU16( u16Pos, g_u16CalibTravel, (C_MAX_POS - C_MIN_POS)) + C_PERC_OFFSET;
				g_u16TargetPosition = g_u16ActualPosition;
			}
		}

		{
			uint8 u8AppStatus = g_NvramUser.AppStatus;
			if ( u8AppStatus & 0x03 )
				g_e8ErrorElectric = (uint8) (u8AppStatus & 0x03U);
			if ( u8AppStatus & 0x08 )
				g_u8EmergencyRunOcc = TRUE;
		}
		g_u8ChipResetOcc = FALSE;
	}
	else if ( (g_NvramUser.NAD >= C_MIN_J2602_NAD) && (g_NvramUser.NAD <= C_MAX_J2602_NAD) && ((g_NvramUser.NAD & (C_STEP_J2602_NAD - 1)) == 0) ) /*lint !e587 */
	{
		g_u8NAD = g_NvramUser.NAD;
		l_u8ActDirection = g_NvramUser.MotorDirectionCCW;
#if (LINPROT != LIN2J_VALVE_VW)
		g_u8SAE_ErrorFlags = (1 << C_SAE_RESET_ERROR);
#endif /* (LINPROT != LIN2J_VALVE_VW) */
	}

	if ( (g_u8NAD & 0x0F) != 0x0F )
	{
		uint8 byFrameID = ((g_u8NAD & 0x0F) << 2) + 0x00;
		byFrameID = CalcProtectionBits( byFrameID);
		(void) ml_AssignFrameToMessageID( mlxACT_CTRL, byFrameID);
		byFrameID = ((g_u8NAD & 0x0F) << 2) + 0x01;
		byFrameID = CalcProtectionBits( byFrameID);
		(void) ml_AssignFrameToMessageID( mlxACT_STATUS, byFrameID);
	}
#else  /* (LINPROT == LIN2J_HVAC_VW) */
	if ( (g_NvramUser.NAD >= C_MIN_J2602_NAD) && (g_NvramUser.NAD <= C_MAX_J2602_NAD) )
	{
		g_u8NAD = g_NvramUser.NAD;
	}
	else
	{
		g_u8NAD = C_DEFAULT_J2602_NAD;
		SetLastError( (uint8) C_ERR_INV_NAD);
	}
	l_u8ActDirection = g_NvramUser.MotorDirectionCCW;
#endif /* (LINPROT == LIN2J_HVAC_VW) */

	(void) ml_SetLoaderNAD( g_u8NAD);											/* Setup NAD at power-up */
	
} /* End of LIN_SAE_J2602_Init() */

#if (LINPROT == LIN2J_VALVE_VW)
/* ****************************************************************************	*
 * ConvertMicroStepPosToPct
 *
 * ****************************************************************************	*/
uint8 ConvertMicroStepPosToPct( uint16 u16Position)
{
	if ( u16Position < (C_PERC_OFFSET + C_HALFPERC_OFFSET) )
	{
		u16Position = 0U;
	}
	else if ( g_u16ActualPosition > ((g_u16CalibTravel + C_PERC_OFFSET) - C_HALFPERC_OFFSET) )
	{
		u16Position = g_u16CalibTravel;
	}
	else
	{
		u16Position -= (C_PERC_OFFSET - (g_u16CalibTravel >> 9));
	}
	if ( l_u8ActDirection )
	{
		u16Position = g_u16CalibTravel - u16Position;
	}
	return ( (uint8) muldivU16_U16byU16byU16( u16Position, (C_MAX_POS - C_MIN_POS), g_u16CalibTravel) );
} /* End of ConvertMicroStepPosToPct() */

/* ****************************************************************************	*
 * LIN_SAE_J2602_Store
 *
 * Before the actuator enters in Sleep, it saves in EEPROM/NVRAM the CPOS,
 * the Status, and the NAD only if the value of cells is different as the RAM value.
 * MMP160613-2
 * ****************************************************************************	*/
void LIN_SAE_J2602_Store( void)
{
	uint16 u16Store = FALSE;

	uint8 u8Value = ConvertMicroStepPosToPct( g_u16ActualPosition);
	/* Round actual position */
	if ( g_NvramUser.CPOS != u8Value )
	{
		g_NvramUser.CPOS = u8Value;
		u16Store = TRUE;
	}

	u8Value = g_NvramUser.AppStatus & 0x80U;
	if ( g_e8ErrorElectric != (uint8) C_ERR_ELECTRIC_NO )
	{
		u8Value |= (g_e8ErrorElectric & 0x03);
	}
	if ( g_u8EmergencyRunOcc != FALSE )
	{
		u8Value |= 0x08U;
	}
	if ( u8Value != g_NvramUser.AppStatus )
	{
		g_NvramUser.AppStatus = u8Value;
		u16Store = TRUE;
	}

	if ( g_u8NAD != g_NvramUser.NAD )
	{
		g_NvramUser.NAD = g_u8NAD;
		u16Store = TRUE;
	}

	if ( u16Store != FALSE )
	{
		/* AppStatus.bit7 NVRAM Page1	Page 2
		 * 						  0		  0		Write C_NVRAM_USER_PAGE_1
		 * 						  0		  1		Write C_NVRAM_USER_PAGE_2
		 * 						  1		  0		Write C_NVRAM_USER_PAGE_2
		 * 						  1		  1		Write C_NVRAM_USER_PAGE_1
		 */
		if ( (((NVRAM_USER *) C_ADDR_USERPAGE1)->AppStatus & 0x80) ^ (((NVRAM_USER *) C_ADDR_USERPAGE2)->AppStatus & 0x80) )
		{
			g_NvramUser.AppStatus = (g_NvramUser.AppStatus & 0x7F) | (((NVRAM_USER *) C_ADDR_USERPAGE2)->AppStatus & 0x80);
			(void) NVRAM_Store( C_NVRAM_USER_PAGE_1);
		}
		else
		{
			g_NvramUser.AppStatus = (g_NvramUser.AppStatus & 0x7F) | ((((NVRAM_USER *) C_ADDR_USERPAGE2)->AppStatus & 0x80) ^ 0x80);
			(void) NVRAM_Store( C_NVRAM_USER_PAGE_2);
		}
	}

} /* End of LIN_SAE_J2602_Store() */

/* ****************************************************************************	*
 * HandleActCfrCtrl
 *
 *			| Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
 *			+---------------------------------------------------------------+
 *	Byte 1	|						 Target Position						|
 *			+-----------------------+-------------------------------+-------+
 *	Byte 2	|		Reserved		|		TorqueLevel				| MovEn |
 *			+-----------------------+-------------------------------+-------+
 *
 * MMP160614-2 & MMP160614-3
 * ****************************************************************************	*/
void HandleActCfrCtrl( void)
{
	ACT_CFR_CTRL *pCfrCtrl = &g_LinCmdFrameBuffer.cfrCtrl;

	if ( (pCfrCtrl->byMovEn == C_CTRL_MOVE_ENA) && (pCfrCtrl->byTorqueLevel != C_CTRL_TORQUE_UNCHANGED) )
	{
		uint16 u16Pos = pCfrCtrl->byPosition;
		if ( l_u8ActDirection != FALSE )
		{
			u16Pos = C_MAX_POS - u16Pos;
		}
		if ( g_e8CalibrationStep != (uint8) C_CALIB_DONE )
		{
			/* Not initialised; Only allow to move towards the end-stops */
			if ( u16Pos == C_MIN_POS )
			{
				g_u16TargetPosition = 0;
				g_u16ActualPosition = g_u16CalibTravel + (2 * C_PERC_OFFSET);
				g_e8CalibrationStep = (uint8) C_CALIB_START;
				if ( g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED )
				{
					g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_START;
				}
				else
				{
					g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_START;
				}
			}
			else if ( u16Pos == C_MAX_POS )
			{
				g_u16TargetPosition = g_u16CalibTravel + (2 * C_PERC_OFFSET);
				g_u16ActualPosition = 0;
				g_e8CalibrationStep = (uint8) C_CALIB_START;
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
		else
		{
			/* Initialised; Any position allowed */
			g_u16TargetPosition = muldivU16_U16byU16byU16( u16Pos, g_u16CalibTravel, (C_MAX_POS - C_MIN_POS));
			if ( u16Pos == C_MAX_POS )										/* Upper end-position: Stall-detection */
			{
				g_u16TargetPosition += C_PERC_OFFSET;
			}
			if ( u16Pos != C_MIN_POS )										/* Lower end-position: Stall-detection */
			{
				g_u16TargetPosition += C_PERC_OFFSET;
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
		if ( (pCfrCtrl->byTorqueLevel >= C_CTRL_TORQUE_NOMINAL) && (pCfrCtrl->byTorqueLevel <= C_CTRL_TORQUE_BOOST_100PCT) )
		{
			g_u8TorqueBoostRequest = (pCfrCtrl->byTorqueLevel - C_CTRL_TORQUE_NOMINAL) * 10U;	/* Percentage */
			g_u16PidRunningThreshold = NVRAM_RUNNING_CURR_LEVEL + muldivU16_U16byU16byU16( NVRAM_RUNNING_CURR_LEVEL, g_u8TorqueBoostRequest, 100U);
			g_u16PidRunningThresholdADC = muldivU16_U16byU16byU16( g_u16PidRunningThreshold, C_GMCURR_DIV, EE_GMCURR);	/* Convert [mA] to [ADC-lsb] */
		}
	}
	else
	{
		g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_STOP;
	}

	g_u8ChipResetOcc = FALSE;											/* Clear 'reset'-flag only after CFR_INI (4.2.6.3) */

} /* End of HandleActCfrCtrl() */

/* ****************************************************************************	*
 * HandleActRfrSta
 *
 *			| Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
 *			+-------------------------------+-------+---------------+-------+
 *	Byte 1	|			Torque-level		|  Move	|	  Fault		|LinErr	|
 *			+-------------------------------+-------+---------------+-------+
 *	Byte 2	|					Actual position Actuator					|
 *			+-------------------------------+---------------+---------------+
 *	Byte 3	|			Reserved			|	ArcState	|	InitState	|
 *			+-------------------------------+---------------+---------------+
 *
 * ****************************************************************************	*/
void HandleActRfrSta( void)
{
	ACT_RFR_STA *pRfrSta = (ACT_RFR_STA *)LinFrameDataBuffer;

	/* Byte 2 */
	pRfrSta->byActPosition = ConvertMicroStepPosToPct( g_u16ActualPosition);

	/* Byte 1 */
	if ( g_u8ErrorCommunication != FALSE )
	{
		pRfrSta->byLinErr = C_STATUS_LIN_ERR;
	}
	else
	{
		pRfrSta->byLinErr = C_STATUS_LIN_OK;
	}
	g_u8SAE_SendErrorState = g_u8ErrorCommunication;
	if ( g_e8ErrorElectric == C_ERR_ELECTRIC_PERM )
	{
		pRfrSta->byFaultState = C_STATUS_FAULT;
	}
	else if ( (g_e8ErrorElectric != (uint8) C_ERR_ELECTRIC_NO) || (g_e8ErrorVoltage != (uint8) C_ERR_VOLTAGE_IN_RANGE) ||
			  /* (g_u8ChipResetOcc != FALSE) || */ (g_e8ErrorOverTemperature != (uint8) C_ERR_OTEMP_NO) ||
			  (g_u8MechError != FALSE) || (g_u8StallOcc != FALSE))
	{
		pRfrSta->byFaultState = C_STATUS_FAULT;
	}
	else
	{
		pRfrSta->byFaultState = C_STATUS_NO_FAULT;
	}
	if ( (g_e8MotorStatusMode & C_MOTOR_STATUS_RUNNING) == 0 )
	{
		pRfrSta->byMoveState = C_STATUS_MOVE_IDLE;
		pRfrSta->byTorqueLevel = 0U;
	}
	else
	{
		pRfrSta->byMoveState = C_STATUS_MOVE_ACTIVE;
		pRfrSta->byTorqueLevel = (uint8) divU16_U32byU16( (uint32) g_u8TorqueBoostRequest, 10U) + C_CTRL_TORQUE_NOMINAL;
	}

	/* Byte #3 */
	if ( g_e8CalibrationStep == (uint8) C_CALIB_DONE )
	{
		pRfrSta->byInitState = C_STATUS_INIT_DONE;
	}
	else if ( (g_e8CalibrationStep == (uint8) C_CALIB_NONE) || (g_e8CalibrationStep >= (uint8) C_CALIB_FAILED) )
	{
		pRfrSta->byInitState = C_STATUS_NOT_INIT;
	}
	else
	{
		pRfrSta->byInitState = C_STATUS_INIT_BUSY;
	}
	pRfrSta->byArcState = l_u8StaCounter;
	l_u8StaCounter++;
	pRfrSta->byReserved2_4 = 0;													/* Reserved field filled with '0' */
} /* End of HandleActRfrSta() */
#endif /* (LINPROT == LIN2J_VALVE_VW) */

/* ****************************************************************************	*
 *  Bus-timeout
 * ****************************************************************************	*/
void HandleBusTimeout( void)
{
#if _SUPPORT_BUSTIMEOUT
	if ( g_u8ErrorCommBusTimeout == FALSE )
	{
		/* Emergency run is enabled */
		g_u8ErrorCommBusTimeout = TRUE;
		SetLastError( (uint8) C_ERR_LIN_BUS_TIMEOUT);

		if ( g_NvramUser.EmergencyRunEna != FALSE )
		{
			if ( g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED )
			{
				/* Module is in degraded-mode; Postpone emergency-run till after degraded-mode have been obsoleted */
				g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_EMRUN;
				g_e8DegradedMotorRequest = g_e8MotorRequest;
			}
			else
			{
				/* Perform emergency-run immediately */
				g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_EMRUN;
			}
		}
#if (_SUPPORT_BUSTIMEOUT_SLEEP != FALSE)
		else
		{
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_SLEEP;
		}
#endif /* (_SUPPORT_BUSTIMEOUT_SLEEP != FALSE) */
	}
#endif /* _SUPPORT_BUSTIMEOUT */
} /* End of HandleBusTimeout() */

/* ****************************************************************************	*
 * LIN2J_ErrorHandling
 *
 *	LIN2.0/SAE-J2602 VW communication error handling.
 *	Only report an communication error, in case the LIN slave is addressed.
 *	To see if this slave is addressed, check the NAD in the LinFrame. The
 *	position within the LinFrame depends on the Frame-ID.
 *
 * ****************************************************************************	*/
void LIN2J_ErrorHandling( ml_LinError Error)
{
	uint8 u8FrameID = (uint8) (LinProtectedID & 0x3F);							/* Get Frame-ID without parity bits */
	if ( (u8FrameID == (uint8) ML_MRF_ID) && ((Error == ml_erDataFraming) || (Error == ml_erCheckSum)) )
	{
		/*
		 * Abort Diagnostic communication with corrupted Diagnostic request
		 * Checked by LIN2.1 CT test case 13.2.2
		 */
		g_u8BufferOutID = (uint8) QR_INVALID;
	}

	/* ---- ml_erLinModuleReset -------------------------------------------- */
	if ( Error == ml_erLinModuleReset )
	{
		/* Non-recoverable failure has occurred in the LIN Module */
		/* switch to System Mode and reinitialise LIN module */
	}
	/* ---- ml_erIdParity -------------------------------------------------- */
	else if ( (Error == ml_erIdParity) || (Error == ml_erIdFraming ) )
	{
		/* Parity error in ID field received -or- Stop bit error of the ID field (SAE_J2602-2: 5.4.1.1) */
//		if ( (u8FrameID == (((g_u8NAD & 0x0F) << 2) + 0x00)) ||					/* CONTROL_MSG */
//			 (u8FrameID == (((g_u8NAD & 0x0F) << 2) + 0x01)) ||					/* STATUS_MSG */
//			 ((u8FrameID == ML_MRF_ID) && (LinFrame[0] == g_u8NAD)) ||			/* DIAG_3C_MSG */
//			 ((u8FrameID == ML_SRF_ID) && (g_u8BufferOutID == (uint8) QR_RFR_DIAG) && (g_DiagResponse.byNAD == g_u8NAD)) )	/* DIAG_3D_MSG */
		{
			g_u8ErrorCommunication = TRUE;
		}
	}
	else if ( (Error == ml_erCheckSum) || (Error == ml_erDataFraming) )
	{
		/* Checksum error in message received -OR- Stop or Start bit error while receiving data (SAE_J2602-2: 5.4.1.2 & 5.4.1.3) */
		if ( (u8FrameID == (((g_u8NAD & 0x0F) << 2) + 0x00)) ||					/* CONTROL_MSG */
			 ((u8FrameID == ML_MRF_ID) && (LinFrame[0] == g_u8NAD)) )			/* DIAG_3C_MSG */
		{
			g_u8ErrorCommunication = TRUE;
		}
	}
	else if ( Error == ml_erSynchField )
	{
		/* Sync field timing error;
		 * In case: BufferOutID is QR_RFR_DIAG, and NAD is real-NAD (SAE_J2602-2: 5.4.1.4) */
		if ( (g_u8BufferOutID == (uint8) QR_RFR_DIAG) && (g_DiagResponse.byNAD == g_u8NAD) )
		{
			g_u8ErrorCommunication = TRUE;
		}
	}
	else if ( Error == ml_erBit )
	{
		/* Data collision during the transmit cycle (SAE_J2602-2: 5.4.1.5) */
		g_u8ErrorCommunication = TRUE;
	}
	return;
} /* End of LIN2J_ErrorHandling() */

#endif /* ((LINPROT & LINXX) == LIN2J) */

/* EOF */


