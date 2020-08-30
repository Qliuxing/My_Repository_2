/*! ----------------------------------------------------------------------------
 * \file		LIN2x.c
 * \brief		MLX81300 LIN 2.x communication handling
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	LIN_2x_Init()
 *				HandleActCtrl()
 *				HandleActStatus()
 *				HandleBusTimeout()
 *				LinAATimeoutControl()
 *				LIN2x_ErrorHandling()
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

#if ((LINPROT & LINXX) == LIN2X)

#include "LIN_Communication.h"
#include "lin.h"
#if ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 3)) || (__MLX_PLTF_VERSION_MAJOR__ >= 4)
#include "lin_internal.h"														/* LinFrame (MMP140417-1) */
#endif /* ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 3)) || (__MLX_PLTF_VERSION_MAJOR__ >= 4) */
#include "main.h"
#include "app_version.h"														/* MMP140519-1 */
#include "ErrorCodes.h"															/* Error-logging support */
#if _SUPPORT_MLX_DEBUG_MODE
#include "MotorStall.h"															/* Only for debugging purpose */
#include "PID_Control.h"														/* Only for debugging purpose */
#endif /* _SUPPORT_MLX_DEBUG_MODE */
#include "NVRAM_UserPage.h"														/* NVRAM Functions & Layout */
#include "Timer.h"																/* Periodic IRQ Timer support */
#include "private_mathlib.h"
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */
#include "LIN_2x.h"

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION (@TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp
uint8 g_u8NAD = C_DEFAULT_NAD;													/* Actual NAD */
uint8 l_u8ActDirection = 0;
#pragma space none

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	(@NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp																/* __NEAR_SECTION__ */
volatile uint8 g_u8LinAAMode = 0;												/* LIN-AA mode (none) */
uint8 g_u8LinAATimeout = 0;														/* LIN-AA Timeout counter (seconds) */
uint16 g_u16LinAATicker = 0;													/* LIN-AA Ticker counter */
uint8 l_e8PositionType = C_POSTYPE_INIT;										/* Status-flag position-type */
uint8 l_u8StaCounter = 0;
#if (LINPROT == LIN2X_ACT44)
//uint8 l_u8PrevProgramMode = C_CTRL_PROGRAM_INV;									/* Previous Programming mode flags (initial: Invalid) */
#endif /* (LINPROT == LIN2X_ACT44) */
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)										/* MMP150125-1 - Begin */
uint8 l_u8GAD = C_INVALD_GAD;
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */								/* MMP150125-1 - End */
uint8 l_u8PctStatusFlagsSend = 0;												/* MMP130626-5 */
#pragma space none																/* __NEAR_SECTION__ */

uint8 g_u8SAE_ErrorFlags = 0;
uint8 g_u8SAE_SendErrorState = 0;												/* Copy of g_u8SAE_ErrorFlags */

/* ****************************************************************************	*
 * Convert actuator position into position based on direction
 * ****************************************************************************	*/
static INLINE uint16 ActPosition( uint16 u16Pos, uint8 u8Direction)
{
	if ( u8Direction )
	{
		u16Pos = (C_MAX_POS - u16Pos);
	}
	return ( u16Pos );
}

/* ****************************************************************************	* 
 * LIN 2.x initialisation
 * ****************************************************************************	*/
void LIN_2x_Init( uint16 u16WarmStart)
{
	if ( (g_NvramUser.NAD & 0x80) == 0x00 )
	{
		g_u8NAD = g_NvramUser.NAD;
	}
	else
	{
		/* Keep original default NAD address */
		SetLastError( (uint8) C_ERR_INV_NAD);
	}
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)										/* MMP150125-1 - Begin */
	l_u8GAD = g_NvramUser.GAD;
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */								/* MMP150125-1 - End */

	l_u8ActDirection = g_NvramUser.MotorDirectionCCW;

	(void) ml_AssignFrameToMessageID( MSG_CONTROL, g_NvramUser.ControlFrameID);
	(void) ml_AssignFrameToMessageID( MSG_STATUS, g_NvramUser.StatusFrameID);
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)										/* MMP150125-1 - Begin */
	(void) ml_AssignFrameToMessageID( MSG_GROUP_CONTROL, g_NvramUser.GroupControlFrameID);
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */								/* MMP150125-1 - End */

	(void) ml_SetLoaderNAD( g_u8NAD);											/* Setup NAD at power-up */

	if ( u16WarmStart == FALSE )
	{
//		g_u16ActualPosition = ActPosition( 32767, g_NvramUser.MotorDirectionCCW);	/* Default: CPos = 0x7FFF */
//		g_u16TargetPosition = ActPosition( 65535, g_NvramUser.MotorDirectionCCW);	/* Default: Invalid FPos */
	}

} /* End of LIN_2x_Init() */

#if (LINPROT == LIN2X_ACT44)
/* ****************************************************************************	*
 * Control
 *
 *	Message ID: 0x0001
 *	Message size: 8-bytes
 *	Repetitive time: min. 10ms (at 19200 Baud)
 *	
 *			| Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
 *			+---------------+---------------+---------------+---------------+
 *	Byte 8	|  Motor modus  | Rot.Direction*| EmRun EndStop*| Emergency Run*|
 *			+---------------+---------------+---------------+---------------+
 *	Byte 7	|				  Start position Actuator (MSB)					|
 *			+---------------------------------------------------------------+
 *	Byte 6	|				  Start position Actuator (LSB)					|
 *			+---------------------------------------------------------------+
 *	Byte 5	|				 Target position Actuator (MSB)					|
 *			+---------------------------------------------------------------+
 *	Byte 4	|				 Target position Actuator (LSB)					|
 *			+-------------------------------+---------------+---------------+
 *	Byte 3	|		Speed selection			| PositionType  |Holding current|
 *			+-------------------------------+---------------+---------------+
 *	Byte 2	|		Clear Event flags		| Stall Detector| Program Data* |
 *			+-------------------------------+---------------+---------------+
 *	Byte 1	|					Address Actuator (NAD)						|
 *			+---------------------------------------------------------------+
 *
 *	Program Data:	Rotational Direction CW/CCW
 *					Emergency Run enable/disable
 *					Emergency Run end-stop Low/High
 *	RAM data:	Stall detection
 *				Speed
 *				Motor holding current
 *				Motor mode
 * ****************************************************************************	*/
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)										/* MMP150125-1 - Begin */
void HandleActCtrl( uint16 u16Group)
#else  /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */


void HandleActCtrl( void)
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */								/* MMP150125-1 - End */
{

	ACT_CTRL *pCfrCtrl = &g_LinCmdFrameBuffer.Ctrl;

//		ACT_CTRL *pCtrl = &g_LinCmdFrameBuffer.Ctrl;

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


} /* End of HandleActCtrl() */

/* ****************************************************************************	*
 * Status
 *
 *	Message ID: 0x0002
 *	Message size: 8-bytes
 *	Repetitive time: min. 10ms (at 19200 Baud)
 *	
 *			| Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
 *			+---------------+---------------+---------------+---------------+
 *	Byte 8	|  Motor modus  | Rot.Direction*| EmRun EndStop*| Emergency Run*|
 *			+---------------+---------------+---------------+---------------+
 *	Byte 7	|						Node Address (NAD)						|
 *			+---------------+---------------+---------------+---------------+
 *	Byte 6	|	 Reserved	| Special Funct |  Holding Mode |   Direction	|
 *			+---------------+---------------+---------------+---------------+
 *	Byte 5	|				 Actual position Actuator (MSB)					|
 *			+---------------------------------------------------------------+
 *	Byte 4	|				 Actual position Actuator (LSB)					|
 *			+-------------------------------+---------------+---------------+
 *	Byte 3	|			Speed status		| PositionType  |Holding current|
 *			+---------------+---------------+---------------+---------------+
 *	Byte 2	|	Chip Reset	|Stall occurred | Stall Detector| Emerg.Run Occ |
 *			+---------------+---------------+---------------+-------+-------+
 *	Byte 1	| Voltage Error |Electric Defect|OverTemperature|  Res	| Error	|
 *			+---------------+---------------+-----------------------+-------+
 *
 * ****************************************************************************	*/
#define METHOD_STRUCT		0
#define METHOD_BYTE			1
#define FUNC_ACT_STATUS		METHOD_BYTE

#if (FUNC_ACT_STATUS == METHOD_BYTE)
void HandleActStatus( void)
{
//	if ( g_u8LinAAMode & C_SNPD_METHOD_2 )
//	{
//		(void) ml_DiscardFrame();												/* Do not respond on ACT_STATUS during LIN-AA */
//	}
//	else
//	{
//		volatile ACT_STATUS *pStatus = &((PLINOUTBUF) LinFrameDataBuffer)->Status;
//		uint8 u8Data = 0x00U;
//		/* Byte 1 */
//		((uint8 *)pStatus)[0] = u8Data;
//		/* Byte 2 */
//		((uint8 *)pStatus)[1] = u8Data;
//		/* Byte 3 */
//		((uint8 *)pStatus)[2] = u8Data;
//		/* Byte 4..5 */
//		/* Byte 6 */
//		((uint8 *)pStatus)[5] = u8Data;
//		/* Byte 7 */
//		pStatus->byNAD = g_u8NAD;
//		/* Byte 8 */
//		u8Data = 0x00U;
//		((uint8 *)pStatus)[7] = u8Data;
//		(void) ml_DataReady( ML_END_OF_TX_DISABLED);
//	}


	ACT_STATUS *pRfrSta = (ACT_STATUS *)LinFrameDataBuffer;

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




} /* End of HandleActStatus() */
#endif /* (FUNC_ACT_STATUS == METHOD_BYTE) */
#endif /* (LINPROT == LIN2X_ACT44) */

/* ****************************************************************************	*
 *  Bus-timeout
 *
 * ****************************************************************************	*/
void HandleBusTimeout( void)
{
#if _SUPPORT_BUSTIMEOUT
	if ( g_u8LinAAMode & (uint8) C_SNPD_METHOD_2 )
	{
		LinAATimeoutControl();
	}
	if ( g_u8ErrorCommBusTimeout == FALSE )
	{
		/* Emergency run is enabled */
		g_u8ErrorCommBusTimeout = TRUE;
		SetLastError( (uint8) C_ERR_LIN_BUS_TIMEOUT);
		{
			if ( g_NvramUser.EmergencyRunEna )
			{
				if ( g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_CALIBRATION )
				{
					/* Calibration is on-going; Postpone emergency-run till after calibration-process have been finished */
					g_e8CalibPostMotorRequest = (uint8) C_MOTOR_REQUEST_EMRUN;
				}
				else if ( g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED )
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
	}
#endif /* _SUPPORT_BUSTIMEOUT */
} /* End of HandleBusTimeout() */

/* ****************************************************************************	*
 * LinAATimeoutControl
 *
 * LIN-AutoAddressing time-out control
 *	If the LIN-AA mode is active for more than the time-out value (40s), the 
 *	original (NVRAM) saved NAD will be restored and the LIN-AA mode will be 
 *	cancelled.
 *
 * ****************************************************************************	*/
void LinAATimeoutControl( void)
{
	/* LIN-AA takes too long time */
	g_u8NAD = g_NvramUser.NAD;													/* Restore original NAD */
#if (LINAA_BSM_SNPD_R1p0 != FALSE)												/* MMP140417-2 - Begin */
	ml_StopAutoAddressing();
#endif /* (LINAA_BSM_SNPD_R1p0 != FALSE) */										/* MMP140417-2 - End */
	(void) ml_SetLoaderNAD( g_u8NAD);
	g_u16LinAATicker = 0;														/* Stop LIN-AA timeout counter */
	g_u8LinAAMode = 0;															/* Cancel LIN-AA mode */
	mlu_AutoAddressingStep( 8);													/* Cancel pending LIN-AA process too */
} /* End of LinAATimeoutControl() */

/* ****************************************************************************	*
 * LIN2x_ErrorHandling
 *
 *	LIN2x communication error handling.
 *	Only report an communication error, in case the LIN slave is addressed.
 *	To see if this slave is addressed, check the NAD in the LinFrame. The 
 *	position within the LinFrame depends on the Frame-ID.
 *
 * ****************************************************************************	*/
void LIN2x_ErrorHandling( ml_LinError Error)
{
	uint8 u8FrameID = (uint8) LinProtectedID & 0x3F;							/* Get Frame-ID without parity bits */
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
	else if ( u8FrameID == (g_NvramUser.StatusFrameID & 0x3F) )
	{
		uint8 u8CommNAD = LinFrame[6];											/* Seventh byte in LIN-frame is NAD */
		if ( u8CommNAD == g_u8NAD )
		{
			g_u8ErrorCommunication = TRUE;
		}
	}
	else if ( (u8FrameID == (g_NvramUser.ControlFrameID & 0x3F)) ||
			  (u8FrameID == ML_MRF_ID) ||
			  (u8FrameID == ML_SRF_ID) )
	{
		uint8 u8CommNAD = LinFrame[0];											/* First byte in LIN-frame is NAD */
		if ( u8CommNAD == g_u8NAD )
		{
			g_u8ErrorCommunication = TRUE;
		}
	}
	return;
} /* End of LIN2x_ErrorHandling() */

#endif /* ((LINPROT & LINXX) == LIN2X) */

/* EOF */
