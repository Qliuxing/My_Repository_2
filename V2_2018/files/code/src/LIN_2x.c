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
#include "lin_internal.h"
#endif /* ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 3)) || (__MLX_PLTF_VERSION_MAJOR__ >= 4) */
#include "main.h"
#include "app_version.h"
#include "ErrorCodes.h"															/* Error-logging support */
#if _SUPPORT_MLX_DEBUG_MODE
#include "MotorStall.h"															/* Only for debugging purpose */
#include "PID_Control.h"														/* Only for debugging purpose */
#endif /* _SUPPORT_MLX_DEBUG_MODE */
#include "NVRAM_UserPage.h"														/* NVRAM Functions & Layout */
#include "Timer.h"																/* Periodic IRQ Timer support */
#include "private_mathlib.h"
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */

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
#if (LINPROT == LIN2X_ACT44)
uint8 l_u8PrevProgramMode = C_CTRL_PROGRAM_INV;									/* Previous Programming mode flags (initial: Invalid) */
#endif /* (LINPROT == LIN2X_ACT44) */
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
uint8 l_u8GAD = C_INVALD_GAD;
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
uint8 l_u8PctStatusFlagsSend = 0;
#pragma space none																/* __NEAR_SECTION__ */


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
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
	l_u8GAD = g_NvramUser.GAD;
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */

	l_u8ActDirection = g_NvramUser.MotorDirectionCCW;

	(void) ml_AssignFrameToMessageID( MSG_CONTROL, g_NvramUser.ControlFrameID);
	(void) ml_AssignFrameToMessageID( MSG_STATUS, g_NvramUser.StatusFrameID);
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
	(void) ml_AssignFrameToMessageID( MSG_GROUP_CONTROL, g_NvramUser.GroupControlFrameID);
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */

	(void) ml_SetLoaderNAD( g_u8NAD);											/* Setup NAD at power-up */

	if ( u16WarmStart == FALSE )
	{
		g_u16ActualPosition = ActPosition( 32767, g_NvramUser.MotorDirectionCCW);	/* Default: CPos = 0x7FFF */
		g_u16TargetPosition = ActPosition( 65535, g_NvramUser.MotorDirectionCCW);	/* Default: Invalid FPos */
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
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
void HandleActCtrl( uint16 u16Group)
#else  /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
void HandleActCtrl( void)
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
{
	if ( g_u8RewindFlags & (uint8) C_REWIND_REWIND )
	{
		g_u8LinInFrameBufState = (uint8) C_LIN_IN_POSTPONE;
	}
	else
	{
		ACT_CTRL *pCtrl = &g_LinCmdFrameBuffer.Ctrl;
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
		/* Check for correct NAD (Broadcast) and not LIN-AA mode */
		if ( (((u16Group == FALSE) && (pCtrl->byNAD == g_u8NAD)) ||
			  ((u16Group != FALSE) && (pCtrl->byNAD == l_u8GAD)) ||
			  (pCtrl->byNAD == (uint8) C_BROADCAST_NAD)) && (g_u8LinAAMode == 0) )
#else  /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
		/* Check for correct NAD (Broadcast) and not LIN-AA mode */
		if ( ((pCtrl->byNAD == g_u8NAD) || (pCtrl->byNAD == (uint8) C_BROADCAST_NAD)) && (g_u8LinAAMode == 0) )
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
			uint8 u8EventMode;
			/* Priority 1: Clear Event Flags */
			if ( (pCtrl->byClearEventFlags != (uint8) C_CTRL_CLREVENT_NONE) && (pCtrl->byClearEventFlags != (uint8) C_CTRL_CLREVENT_INV) )
			{
				/* Clear one or more event flags */
				if ( pCtrl->byClearEventFlags & (uint8) C_CTRL_CLREVENT_RESET )
				{
					/* Clear reset flag */
					g_u8ChipResetOcc = FALSE;
				}
				if ( pCtrl->byClearEventFlags & (uint8) C_CTRL_CLREVENT_STALL )
				{
					/* Clear stall detected flag */
					g_u8StallOcc = FALSE;
					if ( g_e8StallDetectorEna != C_STALLDET_NONE )
					{
						g_u8StallTypeComm &= ~M_STALL_MODE;
					}
					else if ( (pCtrl->byStallDetector == (uint8) C_CTRL_STALLDET_ENA) && ((g_u8StallTypeComm & M_STALL_MODE) != 0) )
					{
						g_u8StallOcc = TRUE;
						/* If actuator is still active, stop it */
						if ( g_e8MotorStatusMode == (uint8) C_MOTOR_STATUS_RUNNING )
						{
							g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_STOP;
						}
					}
				}
				if ( pCtrl->byClearEventFlags & (uint8) C_CTRL_CLREVENT_EMRUN )
				{
					/* Clear emergency-run occurred flag */
					if ( g_u8EmergencyRunOcc )
					{
						g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_STOP;			/* Stop actuator, in case of EmRun; See 4.6.2.1 */
					}
					g_u8EmergencyRunOcc = FALSE;
				}
				/* ( pCtrl->byClearEventFlags & C_CTRL_CLREVENT_RES ) is invalid */
			}
			u8EventMode = (g_u8ChipResetOcc | g_u8StallOcc | g_u8EmergencyRunOcc);

			/* Priority 2: Change of mode (STOP, NORMAL) */
			if ( u8EventMode == FALSE )
			{
				/* Only handled in case not in Event-mode */
				if ( pCtrl->byStopMode == (uint8) C_CTRL_STOPMODE_NORMAL )
				{
					g_e8MotorCtrlMode = (uint8) C_MOTOR_CTRL_NORMAL;
				}
				else if ( pCtrl->byStopMode == (uint8) C_CTRL_STOPMODE_STOP )
				{
					g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_STOP;
					g_e8MotorCtrlMode = (uint8) C_MOTOR_CTRL_STOP;
				}
			}
			
			/* Priority 3: Store data into NVRAM */
			if ( (pCtrl->byProgram == (uint8) C_CTRL_PROGRAM_ENA) && (g_e8MotorCtrlMode == (uint8) C_MOTOR_CTRL_STOP) && 
				 (u8EventMode == FALSE) && (l_u8PrevProgramMode == (uint8) C_CTRL_PROGRAM_DIS) )
			{
				/* Store info into NVRAM (Rotational-direction, Emergency-run info), only in Stop-mode, and not Event-mode */
				uint8 u8OrgActDirection = l_u8ActDirection;
				if ( (pCtrl->byRotationDirection == (uint8) C_CTRL_DIR_CW) || (pCtrl->byRotationDirection == (uint8) C_CTRL_DIR_CCW) )
				{
					l_u8ActDirection = (pCtrl->byRotationDirection == (uint8) C_CTRL_DIR_CCW) ? TRUE : FALSE;
					g_NvramUser.MotorDirectionCCW = l_u8ActDirection;
				}
				if ( (pCtrl->byEmergencyRun == (uint8) C_CTRL_EMRUN_DIS) || (pCtrl->byEmergencyRun == (uint8) C_CTRL_EMRUN_ENA) )
				{
					if ( pCtrl->byEmergencyRun == (uint8) C_CTRL_EMRUN_ENA )
						g_NvramUser.EmergencyRunEna = TRUE;
					else
						g_NvramUser.EmergencyRunEna = FALSE;
				}
				if ( (pCtrl->byEmergencyEndStop == (uint8) C_CTRL_ENRUN_ENDSTOP_LO) || (pCtrl->byEmergencyEndStop == (uint8) C_CTRL_ENRUN_ENDSTOP_HI) )
				{
					if ( pCtrl->byEmergencyEndStop == (uint8) C_CTRL_ENRUN_ENDSTOP_HI )
						g_NvramUser.EmergencyRunEndStopHi = TRUE;
					else
						g_NvramUser.EmergencyRunEndStopHi = FALSE;
				}
					
				(void) NVRAM_Store( (uint16) C_NVRAM_USER_PAGE_ALL);			/* Store NVRAM */
				g_e8MotorDirectionCCW = (uint8) C_MOTOR_DIR_UNKNOWN;			/* Direction is unknown (9.5.3.13) */
				
				if ( u8OrgActDirection != l_u8ActDirection )
				{
					g_u16ActualPosition = ActPosition( g_u16ActualPosition, 1);
					g_u16TargetPosition = ActPosition( g_u16TargetPosition, 1);
				}
			}
			if ( (pCtrl->byProgram == (uint8) C_CTRL_PROGRAM_DIS) || (pCtrl->byProgram == (uint8) C_CTRL_PROGRAM_ENA) )
			{
				l_u8PrevProgramMode = pCtrl->byProgram;
			}

			/* Priority 4: Stall detection enable/disable */
			if ( (g_u8EmergencyRunOcc == FALSE) && ((pCtrl->byStallDetector == (uint8) C_CTRL_STALLDET_DIS) || (pCtrl->byStallDetector == (uint8) C_CTRL_STALLDET_ENA)) )
			{
				if ( ( g_e8StallDetectorEna == C_STALLDET_NONE ) &&
					(pCtrl->byStallDetector == (uint8) C_CTRL_STALLDET_ENA) &&
					((g_u8StallTypeComm & M_STALL_MODE) != 0) )
				{
					g_u8StallOcc = TRUE;
					u8EventMode |= g_u8StallOcc;
					/* If actuator is still active, stop it */
					if ( g_e8MotorStatusMode == (uint8) C_MOTOR_STATUS_RUNNING )
					{
						g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_STOP;
					}
				}
				g_e8StallDetectorEna = (pCtrl->byStallDetector == (uint8) C_CTRL_STALLDET_ENA) ? C_STALLDET_ALL : C_STALLDET_NONE;
			}

			/* Priority 5: Speed change */
#if (LINPROT == LIN2X_ACT44)
			if ( (pCtrl->bySpeed >= (uint8) C_CTRL_SPEED_LOW) && (pCtrl->bySpeed <= (uint8) C_CTRL_SPEED_HIGH) )
#endif /* (LINPROT == LIN2X_ACT44) */
			{
				if ( g_u8MotorCtrlSpeed != pCtrl->bySpeed )
				{
					if ( g_e8MotorRequest != (uint8) C_MOTOR_REQUEST_STOP )
					{
						g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_SPEED_CHANGE;
					}
					g_u8MotorCtrlSpeed = pCtrl->bySpeed;
				}
			} 

			/* Priority 6: CPos/FPos update */
			if ( u8EventMode == FALSE )												/* See 9.4.3.4 */
			{
				/* Only allow change of CPos or FPos in case not in Event-mode */
				if ( pCtrl->byPositionType == (uint8) C_CTRL_POSITION_TARGET )
				{
					/* Accept (new) FPOS, in case not in Event-mode */
					uint16 u16Value = (((uint16) pCtrl->byTargetPositionMSB) << 8) | ((uint16) pCtrl->byTargetPositionLSB);
					if ( u16Value != 0xFFFFU )										/* Check for a valid FPos */
					{
						g_u16TargetPosition = ActPosition( u16Value, l_u8ActDirection);
						l_e8PositionType = (uint8) C_POSTYPE_TARGET;
						if ( g_e8MotorRequest != (uint8) C_MOTOR_REQUEST_STOP )
						{
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
				}
				else if ( (pCtrl->byPositionType == (uint8) C_CTRL_POSITION_INITIAL) && ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) == 0) )	/* See 9.4.3.6 */
				{
					/* Accept (new) CPOS, in case not normal-running and not in Event-mode */
					uint16 u16Value = (((uint16) pCtrl->byStartPositionMSB) << 8) | ((uint16) pCtrl->byStartPositionLSB);
					if ( u16Value != 0xFFFFU )											/* Check for a valid CPos */
					{
						g_u16ActualPosition = ActPosition( u16Value, l_u8ActDirection);
						g_u16TargetPosition = g_u16ActualPosition;
						l_e8PositionType = (uint8) C_POSTYPE_INIT;
						g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_INIT;
						if ( g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED )
						{
							g_e8DegradedMotorRequest = (uint8) C_MOTOR_REQUEST_NONE;
						}
					}
				}
			}

			/* Priority 7: Holding current enable/disable */
			if ( (pCtrl->byHoldingCurrent == (uint8) C_CTRL_MHOLDCUR_DIS) || (pCtrl->byHoldingCurrent == (uint8) C_CTRL_MHOLDCUR_ENA) )
			{
				uint8 u8HoldingCurrEna = (pCtrl->byHoldingCurrent == (uint8) C_CTRL_MHOLDCUR_ENA) ? TRUE : FALSE;
				if ( g_u8MotorHoldingCurrEna != u8HoldingCurrEna )
				{
					/* Change of motor holding current setting */
					if ( (g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_NONE) && ((g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING) == 0) )
					{
						g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_STOP;
					}
					g_u8MotorHoldingCurrEna = u8HoldingCurrEna;
				}
			}
		}
	}
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

#if (FUNC_ACT_STATUS == METHOD_STRUCT)
void HandleActStatus( void)
{
	if ( g_u8LinAAMode & C_SNPD_METHOD_2 )
	{
		(void) ml_DiscardFrame();												/* Do not respond on ACT_STATUS during LIN-AA */
	}
	else
	{
		volatile ACT_STATUS *pStatus = &((PLINOUTBUF) LinFrameDataBuffer)->Status;
		/* Byte 1 */
		if ( g_u8ErrorCommunication != FALSE )
		{
			pStatus->byResponseError = TRUE;
		}
		else
		{
			pStatus->byResponseError = FALSE;
		}
		g_u8ErrorCommunication = FALSE;											/* Data requested; No longer communication error */
		pStatus->byReserved1 = 0;
		if ( g_e8ErrorOverTemperature )
		{
			pStatus->byOverTemperature = (uint8) C_STATUS_OTEMP_YES;
		}
		else
		{
			pStatus->byOverTemperature = (uint8) C_STATUS_OTEMP_NO;
		}
		pStatus->byElectricDefect = (uint8) (g_e8ErrorElectric & 0x03);
		if ( g_e8ErrorElectric == (uint8) C_ERR_ELECTRIC_YES )
		{
			g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_NO;						/* Only "clear" Electric Error if not permanent */
		}
		pStatus->byVoltageError = (uint8) (g_e8ErrorVoltage & 0x03);
		if ( (pStatus->byVoltageError == 0) && (g_e8ErrorVoltageComm != 0) )
		{
			pStatus->byVoltageError = (uint8) (g_e8ErrorVoltageComm & 0x03);
		}
		g_e8ErrorVoltageComm = g_e8ErrorVoltage;								/* 9.5.3.4 */
		/* Byte 2 */
		if ( g_u8EmergencyRunOcc )
		{
			pStatus->byEmergencyOccurred = (uint8) C_STATUS_EMRUNOCC_YES;
		}
		else
		{
			pStatus->byEmergencyOccurred = (uint8) C_STATUS_EMRUNOCC_NO;
		}
		if ( g_e8StallDetectorEna != (uint8) C_STALLDET_NONE )
		{
			pStatus->byStallDetector = (uint8) C_STATUS_STALLDET_ENA;
		}
		else
		{
			pStatus->byStallDetector = (uint8) C_STATUS_STALLDET_DIS;
		}
		if ( g_u8StallOcc )
		{
			pStatus->byStallOccurred = (uint8) C_STATUS_STALLOCC_YES;
		}
		else
		{
			pStatus->byStallOccurred = (uint8) C_STATUS_STALLOCC_NO;
		}
		if ( g_u8ChipResetOcc )
		{
			pStatus->byReset = (uint8) C_STATUS_RESETOCC_YES;
		}
		else
		{
			pStatus->byReset = (uint8) C_STATUS_RESETOCC_NO;
		}
		/* Byte 3 */
		if ( g_u8MotorHoldingCurrEna )
		{
			pStatus->byHoldingCurrent = (uint8) C_STATUS_MHOLDCUR_ENA;
		}
		else
		{
			pStatus->byHoldingCurrent = (uint8) C_STATUS_MHOLDCUR_DIS;
		}
		if ( l_e8PositionType == (uint8) C_POSTYPE_INIT )
		{
			pStatus->byPositionTypeStatus = (uint8) C_STATUS_POSITION_INIT;
		}
		else
		{
			pStatus->byPositionTypeStatus = (uint8) C_STATUS_POSITION_ACTUAL;
		}
		if ( g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING )
		{
			pStatus->bySpeedStatus = g_u8MotorStatusSpeed;
		}
		else
		{
			pStatus->bySpeedStatus = 0;
		}
		/* Byte 4..5 */
		{
			uint16 u16CopyPosition = ActPosition( g_u16ActualPosition, l_u8ActDirection);
			pStatus->byActualPositionLSB = (uint8) (u16CopyPosition & 0xFF);
			pStatus->byActualPositionMSB = (uint8) (u16CopyPosition >> 8);
		}
		/* Byte 6 */
		if ( g_e8MotorDirectionCCW == (uint8) C_MOTOR_DIR_UNKNOWN )
		{
			pStatus->byActualRotationalDir = (uint8) C_STATUS_ACT_DIR_UNKNOWN;
		}
		else if ( (g_e8MotorDirectionCCW & 1) ^ l_u8ActDirection )
		{
			pStatus->byActualRotationalDir = (uint8) C_STATUS_ACT_DIR_CLOSING;
		}
		else
		{
			pStatus->byActualRotationalDir = (uint8) C_STATUS_ACT_DIR_OPENING;
		}
		pStatus->bySelfHoldingTorque = (uint8) C_STATUS_HOLDING_TORQUE_INV;
		if ( g_u8RewindFlags & (uint8) C_REWIND_ACTIVE )
		{
			pStatus->bySpecialFunctionActive = (uint8) C_STATUS_SFUNC1_ACTIVE_YES;
		}
		else
		{
			pStatus->bySpecialFunctionActive = (uint8) C_STATUS_SFUNC_ACTIVE_NO;
		}
		pStatus->byReserved = 3;
		/* Byte 7 */
		pStatus->byNAD = g_u8NAD;
		/* Byte 8 */
		if ( g_NvramUser.EmergencyRunEna )
		{
			pStatus->byEmergencyRun = (uint8) C_STATUS_EMRUN_ENA;
		}
		else
		{
			pStatus->byEmergencyRun = (uint8) C_STATUS_EMRUN_DIS;
		}
		if ( g_NvramUser.EmergencyRunEndStopHi )
		{
			pStatus->byEmergencyRunEndStop = (uint8) C_STATUS_EMRUN_ENDPOS_HI;
		}
		else
		{
			pStatus->byEmergencyRunEndStop = (uint8) C_STATUS_EMRUN_ENDPOS_LO;
		}
		if ( g_NvramUser.MotorDirectionCCW )
		{
			pStatus->byRotationDirection = (uint8) C_STATUS_DIRECTION_CCW;
		}
		else
		{
			pStatus->byRotationDirection = (uint8) C_STATUS_DIRECTION_CW;
		}
		{
			if ( g_e8MotorCtrlMode == (uint8) C_MOTOR_CTRL_STOP )
			{
				pStatus->byStopMode = (uint8) C_STATUS_STOPMODE_STOP;
			}
			else
			{
				pStatus->byStopMode = (uint8) C_STATUS_STOPMODE_NORMAL;
			}
		}

		(void) ml_DataReady( ML_END_OF_TX_DISABLED);
	}
} /* End of HandleActStatus() */
#endif /* (FUNC_ACT_STATUS == METHOD_STRUCT) */

#if (FUNC_ACT_STATUS == METHOD_BYTE)
void HandleActStatus( void)
{
	if ( g_u8LinAAMode & C_SNPD_METHOD_2 )
	{
		(void) ml_DiscardFrame();												/* Do not respond on ACT_STATUS during LIN-AA */
	}
	else
	{
		volatile ACT_STATUS *pStatus = &((PLINOUTBUF) LinFrameDataBuffer)->Status;
		uint8 u8Data = 0x00U;
		/* Byte 1 */
		if ( g_u8ErrorCommunication != FALSE )
		{
			u8Data |= 1;
		}
		else
		{
			u8Data |= 0;
		}
		g_u8ErrorCommunication = FALSE;											/* Data requested; No longer communication error */
		u8Data |= (0 << 1);
		if ( g_e8ErrorOverTemperature )
		{
			u8Data |= (uint8) (C_STATUS_OTEMP_YES << 2);
		}
		else
		{
			u8Data |= (uint8) (C_STATUS_OTEMP_NO << 2);
		}
		u8Data |= (uint8) ((g_e8ErrorElectric & 0x03) << 4);
		if ( g_e8ErrorElectric == (uint8) C_ERR_ELECTRIC_YES )
		{
			g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_NO;						/* Only "clear" Electric Error if not permanent */
		}
		u8Data |= (uint8) ((g_e8ErrorVoltage & 0x03) << 6);
		if ( ((u8Data & 0xC0) == 0) && (g_e8ErrorVoltageComm != 0) )
		{
			u8Data |= (uint8) ((g_e8ErrorVoltageComm & 0x03) << 6);
		}
		g_e8ErrorVoltageComm = g_e8ErrorVoltage;								/* 9.5.3.4 */
		((uint8 *)pStatus)[0] = u8Data;

		/* Byte 2 */
		u8Data = 0x00U;
		if ( g_u8EmergencyRunOcc )
		{
			u8Data |= (uint8) C_STATUS_EMRUNOCC_YES;
		}
		else
		{
			u8Data |= (uint8) C_STATUS_EMRUNOCC_NO;
		}
		if ( g_e8StallDetectorEna != (uint8) C_STALLDET_NONE )
		{
			u8Data |= (uint8) (C_STATUS_STALLDET_ENA << 2);
		}
		else
		{
			u8Data |= (uint8) (C_STATUS_STALLDET_DIS << 2);
		}
		if ( g_u8StallOcc )
		{
			u8Data |= (uint8) (C_STATUS_STALLOCC_YES << 4);
		}
		else
		{
			u8Data |= (uint8) (C_STATUS_STALLOCC_NO << 4);
		}
		if ( g_u8ChipResetOcc )
		{
			u8Data |= (uint8) (C_STATUS_RESETOCC_YES << 6);
		}
		else
		{
			u8Data |= (uint8) (C_STATUS_RESETOCC_NO << 6);
		}
		((uint8 *)pStatus)[1] = u8Data;

		/* Byte 3 */
		u8Data = 0x00U;
		if ( g_u8MotorHoldingCurrEna )
		{
			u8Data |= (uint8) C_STATUS_MHOLDCUR_ENA;
		}
		else
		{
			u8Data |= (uint8) C_STATUS_MHOLDCUR_DIS;
		}
		if ( l_e8PositionType == (uint8) C_POSTYPE_INIT )
		{
			u8Data |= (uint8) (C_STATUS_POSITION_INIT << 2);
		}
		else
		{
			u8Data |= (uint8) (C_STATUS_POSITION_ACTUAL << 2);
		}
		if ( g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_RUNNING )
		{
			u8Data |= ((g_u8MotorStatusSpeed & 0x0F) << 4);
		}
		else
		{
			u8Data |= (0 << 4);
		}
		((uint8 *)pStatus)[2] = u8Data;

		/* Byte 4..5 */
		{
			uint16 u16CopyPosition = ActPosition( g_u16ActualPosition, l_u8ActDirection);
			pStatus->byActualPositionLSB = (uint8) (u16CopyPosition & 0xFF);
			pStatus->byActualPositionMSB = (uint8) (u16CopyPosition >> 8);
		}
		/* Byte 6 */
		u8Data = (3U << 6);	/* Reserved */
		if ( g_e8MotorDirectionCCW == (uint8) C_MOTOR_DIR_UNKNOWN )
		{
			u8Data |= (uint8) C_STATUS_ACT_DIR_UNKNOWN;
		}
		else if ( (g_e8MotorDirectionCCW & 1) ^ l_u8ActDirection )
		{
			u8Data |= (uint8) C_STATUS_ACT_DIR_CLOSING;
		}
		else
		{
			u8Data |= (uint8) C_STATUS_ACT_DIR_OPENING;
		}
		u8Data |= (uint8) (C_STATUS_HOLDING_TORQUE_INV << 2);
		if ( g_u8RewindFlags & (uint8) C_REWIND_ACTIVE )
		{
			u8Data |= (uint8) (C_STATUS_SFUNC1_ACTIVE_YES << 4);
		}
		else
		{
			u8Data |= (uint8) (C_STATUS_SFUNC_ACTIVE_NO << 4);
		}
		((uint8 *)pStatus)[5] = u8Data;

		/* Byte 7 */
		pStatus->byNAD = g_u8NAD;

		/* Byte 8 */
		u8Data = 0x00U;
		if ( g_NvramUser.EmergencyRunEna )
		{
			u8Data |= (uint8) C_STATUS_EMRUN_ENA;
		}
		else
		{
			u8Data |= (uint8) C_STATUS_EMRUN_DIS;
		}
		if ( g_NvramUser.EmergencyRunEndStopHi )
		{
			u8Data |= (uint8) (C_STATUS_EMRUN_ENDPOS_HI << 2);
		}
		else
		{
			u8Data |= (uint8) (C_STATUS_EMRUN_ENDPOS_LO << 2);
		}
		if ( g_NvramUser.MotorDirectionCCW )
		{
			u8Data |= (uint8) (C_STATUS_DIRECTION_CCW << 4);
		}
		else
		{
			u8Data |= (uint8) (C_STATUS_DIRECTION_CW << 4);
		}
		{
			if ( g_e8MotorCtrlMode == (uint8) C_MOTOR_CTRL_STOP )
			{
				u8Data |= (uint8) (C_STATUS_STOPMODE_STOP << 6);
			}
			else
			{
				u8Data |= (uint8) (C_STATUS_STOPMODE_NORMAL << 6);
			}
		}
		((uint8 *)pStatus)[7] = u8Data;
		
		(void) ml_DataReady( ML_END_OF_TX_DISABLED);
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
#if _SUPPORT_AUTO_CALIBRATION
				if ( g_e8MotorRequest == (uint8) C_MOTOR_REQUEST_CALIBRATION )
				{
					/* Calibration is on-going; Postpone emergency-run till after calibration-process have been finished */
					g_e8CalibPostMotorRequest = (uint8) C_MOTOR_REQUEST_EMRUN;
				}
				else if ( g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED )
#else  /* _SUPPORT_AUTO_CALIBRATION */
				if ( g_e8MotorStatusMode & (uint8) C_MOTOR_STATUS_DEGRADED )
#endif /* _SUPPORT_AUTO_CALIBRATION */
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
#if (LINAA_BSM_SNPD_R1p0 != FALSE)
	ml_StopAutoAddressing();
#endif /* (LINAA_BSM_SNPD_R1p0 != FALSE) */
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
