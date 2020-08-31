/*! ----------------------------------------------------------------------------
 * \file		LIN_Communication.c
 * \brief		MLX81300 LIN communication handling
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	LIN_Init()
 *				mlu_ApplicationStop()
 *				mlu_DataRequest()
 *				mlu_DataTransmitted()
 *				mlu_ErrorDetected()
 *				mlu_HeaderReceived()
 *				mlu_LinSleepMode()
 *				mlu_MessageReceived()
 *				NVRAM_CRC8()
 *				mlu_AutoAddressingStep()
 *				ml_SetSlaveNotAddressed()
 *				ml_SetSlaveAddressed()
 *				ml_GetAutoaddressingStatus()
 *				AutoAddressingReadADCResult()
 *				ml_AutoAddressingCfgADC()
 *				ClearAAData()
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
 * ****************************************************************************	*
 *
 *  Address	Name			Description
 *	0x0008	+-----------+	A LIN-frame buffer of 8-bytes, used to communicate between MLX4 and MLX16
 *			|  LinFrame |	Buffer used for IN and OUT frames. Buffer use semaphores.
 *	0x000F	+-----------+
 *			   /|\	|
 *				|	|
 *	ml_DataReady|	|ml_GetLinEventData
 *				|	|
 *				|  \|/
 *  0x00DP  +-----------+	The LIN-frame (IN) is copied into this LinFrameDataBuffer (pLinInFrameBuffer), in case of a LinCommand.
 *			|  LinFrame |	After copy, LinFrame buffer is released (free). The copy is taken place during the LIN-IRQ (ml_GetLinEventData).
 *			| DataBuffer|	The LIN-frame (OUT) is written in this buffer; When finished ml_DataReady() is called to copied to LinFrame
 *			+-----------+    
 *			   /|\	|
 *				|	|
 *	Application-+	|mlu_MessageReceived
 * e.g. ActStatus	|
 *				   \|/
 *	0x00DP	+-----------+
 *			| CopyLinIn |	MLX16 Application LIN-command frame buffer (IN). The LinFrameDataBuffer (pLinInFrameBuffer) 
 *			|FrameBuffer|   is copied to g_LinCmdFrameBuffer in case it is FREE (empty) (mlu_MessageReceived).
 *			+-----------+	The LIN-command is handled by calling HandleLinInMsg().
 *
 *							LIN 2.x/Actuator (4.4) Status requests are written directly in the 
 *							LinFrameDataBuffer (pLinOutFrameBuffer) after receiving the LIN-Header.
 *							LIN 1.3/Cooling (2.3) Request Frames are also directly written in the
 *							LinFrameDataBuffer (pLinOutFrameBuffer) after receiving the Demand frame.
 *	0x00DP	+------------+	LIN 2.x Diagnostics response frames (0x3D) can be requested multiple times, without 0x3C-frames. 
 *			|g_Diag		 |	Furthermore, between a 0x3C-frame (Request) and 0x3D-frame (Response) are other status 
 *			|	Response |	frames allowed. Therefore a separate 0x3D-Response-buffer is made.
 *			+------------+
 *
 *
 * ****************************************************************************	*/

#include "LIN_Communication.h"
#include "lin_internal.h"														/* LinFrame (MMP140417-1) */
#include <lin.h>
#include <syslib.h>
#include <nvram.h>																/* NVRAM support */
#include <plib.h>

#include "Timer.h"
#include "ErrorCodes.h"															/* Error-logging support */

#include "NVRAM_UserPage.h"


/*
 * Possible values for the LIN cell Slew Rate (Phymd)
 * Refer to the Mlx4 periphery documentation for the values of your chip
 * (see Phymd[1:0] in the FLAGS 1 dcom) 
 * SlewRate: ML_SLEWHIGH=20kbps/ML_SLEWLOW=10kbps/ML_SLEWFAST=max (fast protocol)
 *
 * Values for MLX81315 below
 */
#define    ML_SLEWHIGH     0
#define    ML_SLEWLOW      2
#define    ML_SLEWFAST     1

/******************** Private definitions ************************/
/* MLX4 status management */
#define C_MLX4_STATE_TIMEOUT	      (PI_TICKS_PER_MILLISECOND * 500U)			/* 500 ms */
#define C_MLX4_STATE_ERROR_THRSHLD	  4											/* After 'n' LIN MLX4 state error, re-initialise LIN interface */
#define C_MLX4_STATE_IMMEDIATE_RST	  0x80										/* Reset MLX4 immediate, without check */
#define C_MLX4_STATE_NOT_LOGGED		  0x40										/* Do not log MLX4 reset as error (e.g. switch between PWM to LIN) */

/******************** Private type definitions ************************/

#define COMM_STATE_PREOPERATIONAL		0x00U
#define COMM_STATE_OPERATIONAL			0x01U
#define COMM_STATE_STOPPED				0x02U

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION (@TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp

uint8 g_u8NAD = C_DEFAULT_NAD;											        /* Actual NAD */
uint8 g_u8ControlFrameID = mlxACT_CTRL;
uint8 g_u8StatusFrameID = mlxACT_STATUS;
RFR_DIAG g_DiagResponse;                                                        /* LIN diagnostic response buffer */
uint8 g_u8BufferOutID = QR_INVALID;												/* LIN output buffer is invalid */

LININBUF g_LinCmdFrameBuffer;

#pragma space none

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	(@NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp
uint8 l_u8LinInFrameMsgID;														/* LIN input frame message-ID */
uint8 l_u8LinInFrameBufState = C_LIN_IN_FREE;									/* LIN input frame-buffer status is FREE */
uint8 l_u8LastMsgIndex;															/* Last received message ID */

volatile uint8 l_u8ErrorCommunication = FALSE;

/* communication state */
uint8 l_e8CommState = COMM_STATE_PREOPERATIONAL;
uint8 l_u8NMEventPending = C_ML_REASON_NO;
/* mlx4 status management */
uint8 l_u8Mlx4ErrorStateOcc = 0;

#pragma space none

/* ****************************************************************************	*
 *	Internal function prototypes												*
 * ****************************************************************************	*/
void LIN2J_ErrorHandling( ml_LinError Error);
void LIN2x_ErrorHandling( ml_LinError Error);

void handleMLX4StatusSupervisor(void);
void handleLinInMsg(void);
void handleNetworkManagement(void);



/* public function implementation */

/* LIN communication software component */
void LIN_MainFunction(void)
{
	switch(l_e8CommState)
	{
	case COMM_STATE_PREOPERATIONAL:     /* Pre-operational */
		l_e8CommState = COMM_STATE_OPERATIONAL;
		break;
	case COMM_STATE_OPERATIONAL:    	/* operational */
		handleLinInMsg();
#if _SUPPORT_LIN_BUS_ACTIVITY_CHECK
		handleMLX4StatusSupervisor();
#endif /* _SUPPORT_LIN_BUS_ACTIVITY_CHECK */
		break;
	case COMM_STATE_STOPPED:    		/* sleep/timeout */
		
		break;
	default:
		l_e8CommState = COMM_STATE_PREOPERATIONAL;
		break;
	}

	handleNetworkManagement();
	
}

/* network management handler */
void handleNetworkManagement(void)
{
	if(l_e8CommState == COMM_STATE_OPERATIONAL)
	{
		if(l_u8NMEventPending != C_ML_REASON_NO)
		{
			HandleNMReq(l_u8NMEventPending);
			l_u8NMEventPending = C_ML_REASON_NO;
			l_e8CommState = COMM_STATE_STOPPED;
		}
	}
	else if(l_e8CommState == COMM_STATE_STOPPED)
	{
		
		if((ml_GetState( ML_NOT_CLEAR) != ml_stINVALID) && ((LinStatus & ML_LIN_BUS_ACTIVITY) != 0u))
		{
			/* MLX4 has detected a SYNC field */
			(void) ml_GetState( ML_CLR_LIN_BUS_ACTIVITY);

			l_e8CommState = COMM_STATE_OPERATIONAL;
			HandleNMReq(C_ML_REASON_WAKEUP);
		}
	}
	else
	{
	
	}
}

void handleMLX4StatusSupervisor(void)
{
	/* ********************** */
	/* *** n. MLX4 status *** */
	/* ********************** */
	/* MLX4 LIN-Bus activity check  */
	if ( (ml_GetState( ML_NOT_CLEAR) != ml_stINVALID) && ((LinStatus & ML_LIN_BUS_ACTIVITY) != 0u) )
	{
		/* MLX4 has detected a SYNC field */
		Timer_Start(MLX4_STATUS_CHECK_TIMER, C_MLX4_STATE_TIMEOUT);
		(void) ml_GetState( ML_CLR_LIN_BUS_ACTIVITY);
	}

	if ( Timer_IsExpired(MLX4_STATUS_CHECK_TIMER) == TRUE ) /* MMP130905-4 - End */
	{
		/* Didn't receive MLX4 LIN command and/or data-request in the last period, or need immediate reset */
		Timer_Start(MLX4_STATUS_CHECK_TIMER, C_MLX4_STATE_TIMEOUT);						/* MLX4 State check counter reset; MLX4 still active */
		if ( ml_GetState( ML_NOT_CLEAR) == ml_stINVALID )		/* MMP130811-1 */
		{
			l_u8Mlx4ErrorStateOcc++;
			if ( l_u8Mlx4ErrorStateOcc >= (uint8) C_MLX4_STATE_ERROR_THRSHLD )
			{
				/* Signal Error; Reset MLX4 */
				MLX4_RESET();
				NOP();
				NOP();
				NOP();
				MLX4_START();
				if ( (l_u8Mlx4ErrorStateOcc & (uint8)C_MLX4_STATE_NOT_LOGGED) == 0u )	/* MMP131126-1 */
				{
					SetLastError( (uint8) C_ERR_MLX4_RESTART);
				}
				LIN_Init();										/* Re-initialise LIN interface w/o changing position */
				l_u8Mlx4ErrorStateOcc = 0;
			}
		}
		else
		{
			l_u8Mlx4ErrorStateOcc = 0;
		}
	}
}


/* **************************************************************************** *
 * CalcProtectionBits
 *
 * **************************************************************************** */
uint8 CalcProtectionBits( uint8 byFrameID)
{
	uint8 u8Temp;
	
	u8Temp = ((byFrameID & 0x01u) ^ ((byFrameID & 0x02u) >> 1u) ^ ((byFrameID & 0x04u) >> 2u) ^ ((byFrameID & 0x10u) >> 4u));
	byFrameID |= (u8Temp > 0u) ? 0x40u : 0x00u;
	u8Temp = (((byFrameID & 0x02u) >> 1u) ^ ((byFrameID & 0x08u) >> 3u) ^ ((byFrameID & 0x10u) >> 4u) ^ ((byFrameID & 0x20u) >> 5u));
	byFrameID |= (u8Temp > 0u) ? 0x00u : 0x80u;
	
	return ( byFrameID );
} /* End of CalcProtectionBits() */


/* ****************************************************************************	*
 * LIN_Init()
 *
 * Initialise LIN communication interface.
 * Default start-up, at 9600 Baud
 * ****************************************************************************	*/
void LIN_Init(void)
{
	uint16 buf[2];
	uint8 temp;
	
	/* Initialise LIN Communication */
	LIN_XCFG &= ~DISTERM;														/* Enable LIN pull-up resistor (MMP150811-2) */
	(void) ml_InitLinModule();													/* Initialise the LIN module */

	/* Setup LIN baudrate */
#if _SUPPORT_AUTO_BAUDRATE == FALSE
	/* Fixed baudrate */
	(void) ml_SetBaudRate( (ml_uint8) LIN_BR_PRESCALER, (ml_uint8)LIN_BR_DIV);	/* Program the baudrate : default startup : 9600baud @ 20.0 Mhz	*/
#else
	/* Auto baudrate only on first LIN frame */
	(void) ml_SetAutoBaudRateMode( ML_ABR_ON_FIRST_FRAME);						/* MMP141215-1 */
#endif

	/* Setup LIN options, including slew-rate */
	(void) ml_SetOptions( 1U,													/* IDStopBitLength = 1.5 Bit (Melexis LIN Master has 1.5 Tbit stop bit */
						  0U,													/* TXStopBitLength = 1 Bit */
						  ML_ENABLED,											/* StateChangeSignal */
						  ML_LIGHTSLEEP);										/* SleepMode: light-sleep mode */

#if (LIN_BR < 12000)
	(void) ml_SetSlewRate( ML_SLEWLOW);
#else /* (LIN_BR < 10000) */
	(void) ml_SetSlewRate( ML_SLEWHIGH);
#endif /* (LIN_BR < 10000) */

#if defined (HAS_LIN_NEXT_FRAME_ID)
	(void) ml_EnableFilter( 0);													/* Enable filter */
#endif

	(void)NVRAM_Read(0x1000,buf,2);
	/* Initialise communication layer */
#if ((LINPROT & LINXX) == LIN2J)
	/* Check wake-up from SLEEP (MMP160613-2) */
	temp = (uint8)(buf[0] & 0x00FFu);
    if((temp >= C_MIN_J2602_NAD) && (temp <= C_MAX_J2602_NAD))
	{
		g_u8NAD = temp;
	}
	else
	{
	    g_u8NAD = C_DEFAULT_NAD;
		/* store to NVM */
		buf[0] = (uint8)C_DEFAULT_NAD;
		buf[0] |= ((uint16)mlxACT_CTRL << 8u);
		buf[1] = (uint8)mlxACT_STATUS;
		buf[1] |= ((uint16)C_VARIANT << 8u);
		(void)NVRAM_Write(0x1000, buf, 2);
	}
    /* J2602:Fixed DNN SNPD */
	if ( (g_u8NAD & 0x0Fu) != 0x0Fu )
	{
		uint8 byFrameID = ((g_u8NAD & 0x0Fu) << 2u) + 0x00u;
		byFrameID = CalcProtectionBits( byFrameID);
		(void) ml_AssignFrameToMessageID( MSG_CONTROL, byFrameID);
		byFrameID = ((g_u8NAD & 0x0Fu) << 2u) + 0x01u;
		byFrameID = CalcProtectionBits( byFrameID);
		(void) ml_AssignFrameToMessageID( MSG_STATUS, byFrameID);
	}
#endif

#if ((LINPROT & LINXX) == LIN20) || ((LINPROT & LINXX) == LIN21)
	temp = (uint8)(buf[0] & 0x00FFu);
    if ( (temp & 0x80u) == 0x00u )
	{
		g_u8NAD = temp;
		g_u8ControlFrameID = (uint8)(buf[0] >> 8u);
		g_u8StatusFrameID = (uint8)(buf[1] & 0x00FFu);
	}
	else
	{
        g_u8NAD = C_DEFAULT_NAD;
		g_u8ControlFrameID = mlxACT_CTRL;
		g_u8StatusFrameID  = mlxACT_STATUS;

		/* store to NVM */
		buf[0] = (uint8)C_DEFAULT_NAD;
		buf[0] |= ((uint16)mlxACT_CTRL << 8u);
		buf[1] = (uint8)mlxACT_STATUS;
		buf[1] |= ((uint16)C_VARIANT << 8u);
		(void)NVRAM_Write(0x1000, buf, 2);
	}
	/* dynamic bound message id to frame id */
	(void) ml_AssignFrameToMessageID( MSG_CONTROL, g_u8ControlFrameID);
	(void) ml_AssignFrameToMessageID( MSG_STATUS, g_u8StatusFrameID);
#endif

    (void) ml_SetLoaderNAD( g_u8NAD);											/* Setup NAD at power-up */

	(void) ml_Connect();

	/* Check chip-state for LIN-command RESET, to setup diagnostic-response */
	if ( bistResetInfo == C_CHIP_STATE_LIN_CMD_RESET )
	{
#if ((LINPROT & LINXX) == LIN2J)
		RfrDiagReset();														/* Prepare a diagnostics response reply */
#endif
		bistResetInfo = C_CHIP_STATE_COLD_START;
	}
} /* End of LIN_Init() */

/* ****************************************************************************	*
 *  LIN API event: mlu_ApplicationStop
 * ****************************************************************************	*/
ml_Status mlu_ApplicationStop(void)
{
	/* Disable all IRQ's, except LIN */
	XI0_MASK = 0;
	XI1_MASK = 0;
	XI2_MASK = 0;
	XI3_MASK = 0;
	XI4_MASK = 0;
	MASK = EN_M4_SHE_IT;														/* Disable all interrupts, except LIN M4_SHE_IT */

	return ( ML_SUCCESS );														/* Return that the application has stopped */
} /* End of mlu_ApplicationStop */

/* ****************************************************************************	*
 *  LIN API event: Data Request (slave TX)
 * ****************************************************************************	*/
void mlu_DataRequest( ml_MessageID MessageIndex) 
{
	/* Data requested; No longer Bus time-out */

	if ( MessageIndex == (uint8) mlxRFR_DIAG )
	{
#if ((LINPROT & LINXX) == LIN20) || ((LINPROT & LINXX) == LIN21)
		if(Timer_IsExpired(DIAG_RESPONSE_TIMER) == TRUE)
		{
			if ( g_u8BufferOutID == QR_RFR_DIAG )								/* Pending response type: Diagnostic */
			{
				g_u8BufferOutID = (uint8) QR_INVALID;							/* Invalidate Diagnostics response */
			}
		}
#endif	

		/* Diagnostic */
		if ( g_u8BufferOutID == (uint8) QR_RFR_DIAG ) /* diagnostic buffer ready? */
		{
			/* Copy g_DiagResponse to LinFrameDataBuffer */
			RFR_DIAG *pDiag = &g_DiagResponse;
			uint16 *src = (uint16 *) pDiag;
			uint16 *dst = (uint16 *) LinFrameDataBuffer;
			
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
			dst[3] = src[3];

			(void) ml_DataReady( ML_END_OF_TX_DISABLED);
			g_u8BufferOutID = (uint8) QR_INVALID;								/* Invalidate LIN output buffer */
		}
		else
		{
			(void) ml_DiscardFrame();											/* Output buffer response doesn't match requested response */
		}
	}
	else if ( MessageIndex == (uint8) MSG_STATUS )
	{
		ACT_RFR_STA *pRfrSta = (ACT_RFR_STA *)LinFrameDataBuffer;
		/* Status AGS */
		HandleActRfrSta(pRfrSta);
		/* LIN communication error */
		pRfrSta->byLinErr = l_u8ErrorCommunication;
		(void) ml_DataReady( ML_END_OF_TX_ENABLED);
	}
	else
	{
	
	}
} /* End of mlu_DataRequest */

/* ****************************************************************************	*
 *  LIN API event: mlu_DataTransmitted
 * ****************************************************************************	*/
void mlu_DataTransmitted(void) 
{
    l_u8ErrorCommunication = FALSE;

} /* End of mlu_DataTransmitted() */

/* ****************************************************************************	*
 *  LIN API event: mlu_ErrorDetected
 * ****************************************************************************	*/
void mlu_ErrorDetected( ml_LinError Error)
{
	SetLastError( (uint8) C_ERR_LIN_COMM | ((uint8)Error & 0x1Fu));
#if ((LINPROT & LINXX) == LIN2J)
     LIN2J_ErrorHandling(Error);
#endif
#if ((LINPROT & LINXX) == LIN20) || ((LINPROT & LINXX) == LIN21)
    LIN2x_ErrorHandling(Error);
#endif
} /* End of mlu_ErrorDetected() */

#ifdef USE_LIN_FRAME_MODE
/* ****************************************************************************	*
 *  LIN API event: mlu_HeaderReceived
 * ****************************************************************************	*/
void mlu_HeaderReceived( ml_MessageID MessageID, ml_bool ValidMessage)
{
	(void) MessageID;		/* unused parameter */
	(void) ValidMessage;	/* unused parameter */

	(void) ml_DiscardFrame();
} /* End of mlu_HeaderReceived() */
#endif /* defined (USE_LIN_FRAME_MODE) */

/* ****************************************************************************	*
 *  LIN API event: mlu_LinSleepMode
 * ****************************************************************************	*/
void mlu_LinSleepMode(ml_StateReason Reason)							    /* MMP130918-1 */
{
	/*
	 * MLX4 FW handles Goto Sleep frame (0x3C, 0x00 ...) automatically
	 * and does not report it via mlu_MessageReceived event.
	 */
	if ( (Reason == ml_reasonMaster) || (Reason == ml_reasonCommand) )			/* MMP130918-1 */
	{
		l_u8NMEventPending = C_ML_REASON_CMD;       
	}

	if ( (Reason == ml_reasonTimeOut) || (Reason == ml_reasonTimeOutDominant) )
	{
		/*
		 * LIN bus was inactive for 4 seconds without receiving an explicit
		 * "Go-to-Sleep frame". This can be considered as a failure of the Master or
		 * PHY layer. Slave can enter limp-home mode.
		 */
		l_u8NMEventPending = C_ML_REASON_TIMEOUT; 
        SetLastError( (uint8) C_ERR_LIN_BUS_TIMEOUT);
	}

} /* End of mlu_LinSleepMode() */

/* ****************************************************************************	*
 *  LIN API event: MessageReceived (slave RX)
 * ****************************************************************************	*/
void mlu_MessageReceived( ml_MessageID MessageIndex)
{
    if ( l_u8LinInFrameBufState != (uint8) C_LIN_IN_FULL )
	{
		/* Buffer is either empty or message is postpone (overwrite allowed) */
		l_u8LinInFrameMsgID = MessageIndex;

		/* LIN In-frame buffer to a Copy LIN In-frame buffer */
		{
			uint16 *pu16Source = (uint16 *) LinFrameDataBuffer;
			uint16 *pu16Target = (uint16 *) &g_LinCmdFrameBuffer;
			*pu16Target = *pu16Source;
			pu16Target++;
			pu16Source++;
			*pu16Target = *pu16Source;
			pu16Target++;
			pu16Source++;
			*pu16Target = *pu16Source;
			pu16Target++;
			pu16Source++;
			*pu16Target = *pu16Source;
		}
		l_u8LinInFrameBufState = (uint8) C_LIN_IN_FULL;
	
		LinFrame[0] = 0x00;														/* Clear NAD address */
	}
} /* End of mlu_MessageReceived() */

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
	uint8 u8FrameID = (uint8) LinProtectedID & 0x3Fu;							/* Get Frame-ID without parity bits */
	
	if ( (u8FrameID == (uint8) ML_MRF_ID) && 
		( (Error == ml_erDataFraming) || (Error == ml_erCheckSum) ) )
	{
		/*
		 * Abort Diagnostic communication with corrupted Diagnostic request
		 * Checked by LIN2.1 CT test case 13.2.2
		 */
		g_u8BufferOutID = (uint8) QR_INVALID;
	}

	/* ID parity error:parity error in PID field received */
	if( Error == ml_erIdParity )
	{
		/* Do NOT set response_error bit, because error occurred in a header */
	}
	
	/* Checksum error:checksum error in message received */
	if( Error == ml_erCheckSum )
	{
		if( ( u8FrameID == (uint8) ML_MRF_ID ) || ( u8FrameID == (g_u8ControlFrameID & 0x3Fu)) )
		{
			l_u8ErrorCommunication = TRUE;
		}
	}

	/* Transmit cycle data collision:data collision during transmit cycle */
	if( Error == ml_erBit )
	{
		l_u8ErrorCommunication = TRUE;
	}
	
	/* Error data framing:start or stop bit error while receiving data */
	if( Error == ml_erDataFraming )
	{
		if( (u8FrameID == (uint8) ML_MRF_ID) || (u8FrameID == (g_u8ControlFrameID & 0x3Fu)) )
		{
			l_u8ErrorCommunication = TRUE;
		}
	}

	/* Error during STOP bit transmission */
	if( Error == ml_erStopBitTX )
	{
		l_u8ErrorCommunication = TRUE;
	}
	
	/* ID framing:stop bit error of the PID field */
	if(Error == ml_erIdFraming)
	{
		/* Do NOT set response_error bit, because error occurred in a header */
	}
	
	/* Sync field:sync field timing error */
	if(Error == ml_erSynchField)
	{
		/* Do NOT set response_error bit, because error occurred in a header */
	}
	
	/* Message buffer locked:message received but buffer was full */
	if(Error == ml_erBufferLocked)
	{
	
	}
	
	/* Unexpectable break:A header has been detected inside a frame */
	if(Error == ml_erBreakDetected)
	{
		/* LCT3.8:INCOMPLETE FRAME RECEPTION */
		if( (u8FrameID == (uint8) ML_MRF_ID) || (u8FrameID == (g_u8ControlFrameID & 0x3Fu)) )
		{
			l_u8ErrorCommunication = TRUE;
		}
	}
	
	/* ---- ml_erLinModuleReset -------------------------------------------- */
	if ( Error == ml_erLinModuleReset )
	{
		/* Non-recoverable failure has occurred in the LIN Module */
		/* switch to System Mode and reinitialise LIN module */
	}
	
	return;
} /* End of LIN2x_ErrorHandling() */



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
	uint8 u8FrameID = (uint8) (LinProtectedID & 0x3Fu);							/* Get Frame-ID without parity bits */
	
	if ( (u8FrameID == (uint8) ML_MRF_ID) && ((Error == ml_erDataFraming) || (Error == ml_erCheckSum)) )
	{
		/*
		 * LCT13.2.2:Abort Diagnostic communication with corrupted Diagnostic request
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
		{
			l_u8ErrorCommunication = TRUE;
		}
	}
	else if ( (Error == ml_erCheckSum) || (Error == ml_erDataFraming) )
	{
		/* Checksum error in message received -OR- Stop or Start bit error while receiving data (SAE_J2602-2: 5.4.1.2 & 5.4.1.3) */
		if ( (u8FrameID == (((g_u8NAD & 0x0Fu) << 2u) + 0x00u)) ||				/* CONTROL_MSG */
			 ((u8FrameID == (uint8)ML_MRF_ID) && (LinFrame[0] == g_u8NAD)) )	/* DIAG_3C_MSG */
		{
			l_u8ErrorCommunication = TRUE;
		}
	}
	else if ( Error == ml_erSynchField )
	{
		/* Sync field timing error;
		 * In case: BufferOutID is QR_RFR_DIAG, and NAD is real-NAD (SAE_J2602-2: 5.4.1.4) 
		 * 5.4.1.4 if auto baudrate , status byte is ignored 
		 */
		l_u8ErrorCommunication = TRUE;
	}
	else if ( Error == ml_erBit )
	{
		/* Data collision during the transmit cycle (SAE_J2602-2: 5.4.1.5) */
		l_u8ErrorCommunication = TRUE;
	}
	else
	{
		
	}
	
	return;
} /* End of LIN2J_ErrorHandling() */



/* ****************************************************************************	*
 *  LIN API: HandleLinInMsg 
 * ****************************************************************************	*/
void handleLinInMsg( void)
{
	if(l_u8LinInFrameBufState != C_LIN_IN_FREE)
    {
		if ( l_u8LinInFrameBufState == (uint8) C_LIN_IN_POSTPONE )
		{
			/* Last message postponed; Try again (without overwritten by LIN message ISR */
			l_u8LinInFrameBufState = (uint8) C_LIN_IN_FULL;
		}

	    /* LIN 2.x,LIN 2.x_J2602 */
		if ( l_u8LinInFrameMsgID == (uint8) mlxDFR_DIAG )
		{
#if ((LINPROT & LINXX) == LIN20) || ((LINPROT & LINXX) == LIN21)
       		Timer_Start(DIAG_RESPONSE_TIMER, (uint16)PI_TICKS_PER_SECOND);
#endif
			/* Diagnostic request frame */
			HandleDfrDiag();
		}
		else if ( l_u8LinInFrameMsgID == (uint8) MSG_CONTROL )
		{
			/* Control */
			HandleActCfrCtrl((ACT_CFR_CTRL *)&g_LinCmdFrameBuffer);
		}
		else
		{
			
		}

		if ( l_u8LinInFrameBufState != (uint8) C_LIN_IN_POSTPONE )
		{
			/* LIN Message is handled; Release LIN message buffer */
			l_u8LinInFrameBufState = (uint8) C_LIN_IN_FREE;
		}
    }
} /* End of HandleLinInMsg() */

/* ****************************************************************************	*
 *  LIN API event: mlu_AutoAddressingStep
 * ****************************************************************************	*/
void mlu_AutoAddressingStep( ml_uint8 StepNumber)
{
	(void) StepNumber; 
} /* End of mlu_AutoAddressingStep() */


/* EOF */
