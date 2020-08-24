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
#if (LINPROT != LIN2J_VALVE_GM)
uint8 g_u8SAE_ErrorFlags = 0;
uint8 g_u8SAE_SendErrorState = 0;												/* Copy of g_u8SAE_ErrorFlags */
#else  /* (LINPROT != LIN2J_VALVE_GM) */
uint8 g_u8SAE_SendErrorState = FALSE;											/* Copy of g_u8ErrorCommunication */
#endif /* (LINPROT != LIN2J_VALVE_GM) */
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

#if (LINPROT == LIN2J_VALVE_GM)
	/* Check wake-up from SLEEP (MMP160613-2) */
	if ( ANA_INB & WAKEUP_LIN )
	{
		g_u8NAD = g_NvramUser.NAD;
		l_u8ActDirection = g_NvramUser.MotorDirectionCCW;
		g_u16ActualPosition = g_u16TargetPosition = g_NvramUser.CPOS;
		{
			uint8 u8AppStatus = g_NvramUser.AppStatus;
			if ( u8AppStatus & 0x03 )
				g_e8ErrorElectric = (uint8) (u8AppStatus & 0x03U);
			if ( u8AppStatus & 0x08 )
				g_u8EmergencyRunOcc = TRUE;
		}
		g_u8ChipResetOcc = FALSE;
	}
	else if ( (g_NvramUser.NAD >= C_MIN_J2602_NAD) && (g_NvramUser.NAD <= C_MAX_J2602_NAD) && ((g_NvramUser.NAD & (C_STEP_J2602_NAD - 1)) == 0) )
	{
		g_u8NAD = g_NvramUser.NAD;
		l_u8ActDirection = g_NvramUser.MotorDirectionCCW;
/*		g_u8SAE_ErrorFlags = (1 << C_SAE_RESET_ERROR);	MMP160613-1 */
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
#else  /* (LINPROT == LIN2J_HVAC_GM) */
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
#endif /* (LINPROT == LIN2J_HVAC_GM) */

	(void) ml_SetLoaderNAD( g_u8NAD);											/* Setup NAD at power-up */
	
} /* End of LIN_SAE_J2602_Init() */

#if (LINPROT == LIN2J_VALVE_GM)
/* ****************************************************************************	*
 * LIN_SAE_J2602_Store
 *
 * Before the actuator enters in Sleep, it saves in EEPROM the CPOS,
 * the Status, and the NAD only if the value of cells is different as the RAM value.
 * MMP160613-2
 * ****************************************************************************	*/
void LIN_SAE_J2602_Store( void)
{
	uint16 u16Store = FALSE;
	if ( g_u16ActualPosition != g_NvramUser.CPOS )
	{
		g_NvramUser.CPOS = g_u16ActualPosition;
		u16Store = TRUE;
	}
	{
		uint8 u8AppStatus = 0x00U;

		if ( g_e8ErrorElectric != (uint8) C_ERR_ELECTRIC_NO )
			u8AppStatus |= (g_e8ErrorElectric & 0x03);
		if ( g_u8EmergencyRunOcc != FALSE )
			u8AppStatus |= 0x08U;
		if ( u8AppStatus != g_NvramUser.AppStatus )
		{
			g_NvramUser.AppStatus = u8AppStatus;
			u16Store = TRUE;
		}
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
 * ****************************************************************************	*/
void HandleActCfrCtrl( void)
{
	ACT_CFR_CTRL *pCfrCtrl = &g_LinCmdFrameBuffer.cfrCtrl;

		g_u16EXVTargetPositionTemp = (((uint16)pCfrCtrl->byPositionMSB & 0x03) << 8) | pCfrCtrl->byPositionLSB;
		if((g_e8MotorStatusMode & ((uint8) C_MOTOR_STATUS_DEGRADED)) == 0) //not degrade mode, otherwise, just update the target position
		{
			if(pCfrCtrl->byMovEn == C_CTRL_MOVE_ENA){
				g_e8EXVMoveEnableRequestFlag = (uint8) C_EXV_MOVE_ENABLE;
				if(((g_e8CalibrationStep == (uint8) C_CALIB_NONE) || (g_e8CalibrationStep == (uint8) C_CALIB_DONE)) && (g_u16EXVTargetPositionTemp == 0x3FF))
				{
					g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_CALIBRATION;
					g_e8CalibrationStep = (uint8) C_CALIB_START;
				}
				else if(g_e8CalibrationStep == (uint8) C_CALIB_DONE)
				{
					g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_START;
				}
				else
				{
					//TODO,Ban,what to do if during initalization?
					g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_CALIBRATION;
				}
				if(pCfrCtrl->byStallEnable == C_CTRL_STALL_ENABLE)
				{
					g_e8StallDetectorEna = C_STALLDET_H;
				}
				else
				{
					g_e8StallDetectorEna = C_STALLDET_NONE;
				}
			}else{
				g_e8EXVMoveEnableRequestFlag = (uint8) C_EXV_MOVE_DISABLE;
				g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_STOP;
			}
		}
		else
		{
			g_u16TargetPosition = (((uint32)g_u16EXVTargetPositionTemp)*g_u16CalibTravel+128)/255 + C_EXV_ZERO_POS;//update the target position in degrade mode,
		}
		if ( (pCfrCtrl->byTorqueLevel >= C_CTRL_TORQUE_NOMINAL) && (pCfrCtrl->byTorqueLevel <= C_CTRL_TORQUE_BOOST_40PCT) )
		{
			g_u8TorqueBoostRequest = (pCfrCtrl->byTorqueLevel - C_CTRL_TORQUE_NOMINAL) * 10U;
		}
		else//undefined torque, just stop the motor
		{
			g_u8TorqueBoostRequest = C_CTRL_TORQUE_NO;
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_STOP;
		}

		g_u8ChipResetOcc = FALSE;											/* Clear 'reset'-flag only after CFR_INI (4.2.6.3) */

} /* End of HandleActCfrCtrl() */

/* ****************************************************************************	*
 * HandleActRfrSta
 *
 *			| Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
 *			+-----------------------+---------------------------------------+
 *	Byte 1	|		ERR State		|				APINFO status			| <-- Not used by GM; Removed (MMP160613-1)
 *			+-----------------------+-------+-------+---------------+-------+
 *	Byte 2	|			Torque-level		|  Move	|	  Fault		|LinErr	|
 *			+-------------------------------+-------+---------------+-------+
 *	Byte 3	|					Actual position Actuator					|
 *			+-------------------------------+---------------+---------------+
 *	Byte 4	|			Reserved			|	ArcState	|	InitState	|
 *			+-------------------------------+---------------+---------------+
 *
 * ****************************************************************************	*/
void HandleActRfrSta( void)
{
	ACT_RFR_STA *pRfrSta = (ACT_RFR_STA *)LinFrameDataBuffer;

	/*	if ( g_u8SAE_ErrorFlags != 0 )		(MMP160613-1) */
	if ( g_u8ErrorCommunication != FALSE )
	{
		pRfrSta->byLinErr = C_STATUS_LIN_ERR;
	}
	else
	{
		pRfrSta->byLinErr = C_STATUS_LIN_OK;
	}
	g_u8SAE_SendErrorState = g_u8ErrorCommunication;

	pRfrSta->byFaultState = g_e8EXVStatusFaultState;

	if ( (g_e8MotorStatusMode & C_MOTOR_STATUS_RUNNING) == 0 )
	{
		pRfrSta->byMoveState = C_STATUS_MOVE_IDLE;
	}
	else
	{
		pRfrSta->byMoveState = C_STATUS_MOVE_ACTIVE;
	}

	if ( (g_e8MotorStatusMode & C_MOTOR_STATUS_RUNNING) == 0 )
	{
		pRfrSta->byTorqueLevel = (uint8)C_CTRL_TORQUE_NO;
	}
	else
	{
		pRfrSta->byTorqueLevel = (uint8) divU16_U32byU16( (uint32) g_u8TorqueBoostRequest, 10U);
	}

	//Byte 2
	pRfrSta->byActPositionLSB = g_u16EXVStatusCurrentPositon&0xFF;
	pRfrSta->byActPositionMSB = (g_u16EXVStatusCurrentPositon>>8)&0x3;

	//Byte 3
	pRfrSta->byInitState = g_e8EXVStatusInitStat;

	pRfrSta->byArcState = l_u8StaCounter;
	l_u8StaCounter++;

	if(g_e8EXVErrorBlock == TRUE)
	{
		pRfrSta->byStallDetectStatus = TRUE;
	}
	else
	{
		pRfrSta->byStallDetectStatus = FALSE;
	}

	//pRfrSta->byReserved3_4 = g_e8ErrorCoil&0xF;
} /* End of HandleActRfrSta() */
#endif /* (LINPROT == LIN2J_VALVE_GM) */

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
#if (_SUPPORT_BUSTIMEOUT_SLEEP != FALSE)										/* MMP130626-8 */
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
 *	LIN2.0/SAE-J2602 GM communication error handling.
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
	else if ( Error == ml_erIdParity )
	{
		/* Do NOT set response_error bit, because error occurred in a header */
	}
	else if ( u8FrameID == (((g_u8NAD & 0x0F) << 2) + 0x01) )
	{
		/* Status Frame */
		if ( Error == ml_erIdFraming )
		{
			g_u8ErrorCommunication = TRUE;
		}
	}
	else if ( (u8FrameID == (((g_u8NAD & 0x0F) << 2) + 0x00)) ||			/* 1.3.4.2 */
			  ((u8FrameID == ML_MRF_ID) && (LinFrame[0] == g_u8NAD)) ||		/* 1.3.4.3 */
			  ((u8FrameID == ML_SRF_ID) && (LinFrame[0] == g_u8NAD)) )		/* 1.3.4.3 */
	{
		/* Control frame or Diagnostics frame */
		if ( (Error == ml_erCheckSum) || (Error == ml_erDataFraming) || (Error == ml_erIdFraming) )
		{
			g_u8ErrorCommunication = TRUE;
		}
	}
	return;
} /* End of LIN2J_ErrorHandling() */

#endif /* ((LINPROT & LINXX) == LIN2J) */

/* EOF */


