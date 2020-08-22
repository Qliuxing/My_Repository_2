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

#include "Build.h"

#if LIN_COMM

#include "LIN_Communication.h"
#if ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 3)) || (__MLX_PLTF_VERSION_MAJOR__ >= 4)
#include "lin_internal.h"
#endif /* ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 3)) || (__MLX_PLTF_VERSION_MAJOR__ >= 4) */
#include "lin.h"
#include "main.h"
#include "ADC.h"																/* ADC support */
#include "ErrorCodes.h"															/* Error-logging support */
#include "NVRAM_UserPage.h"														/* NVRAM Functions & Layout */

#if (__MLX_PLTF_VERSION_MAJOR__ == 3)
extern ml_Status ml_ReleaseBuffer( void);
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 3) */

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION (@TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp
#if _SUPPORT_LIN_AA
/************************ AUTO ADDRESSING ***********************/
#if (LINAA_BSM_SNPD_R1p0 == FALSE)
volatile uint8 l_e8AutoAddressingState;
volatile uint8 l_u8AutoAddressingPulse = 0;										/* Local copy of pulse variable set by MLX4 LIN API */
#endif /* (LINAA_BSM_SNPD_R1p0 == FALSE) */
volatile uint8 g_u8AutoAddressingFlags = 0x00;									/* Reset all auto addressing flags */
#endif /* _SUPPORT_LIN_AA */

uint8 g_u8BufferOutID = QR_INVALID;												/* LIN output buffer is invalid */
uint8 l_u8LinInFrameMsgID;														/* LIN input frame message-ID */
LININBUF g_LinCmdFrameBuffer;
#if ((LINPROT & LINX) == LIN2)
RFR_DIAG g_DiagResponse;
#endif /* ((LINPROT & LINX) == LIN2) */
#pragma space none

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	(@NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp
uint8 g_u8LinInFrameBufState = C_LIN_IN_FREE;									/* LIN input frame-buffer status is FREE */
uint8 l_u8LastMsgIndex;															/* Last received message ID */
volatile uint8 g_u8ErrorCommunication = FALSE;									/* Flag indicate of LIN communication errors occurred */
volatile uint8 g_u8ErrorCommBusTimeout = FALSE;									/* Flag indicate of LIN bus time-out occurred */

#if _SUPPORT_LIN_AA
/************************ AUTO ADDRESSING ***********************/
#if (LINAA_BSM_SNPD_R1p0 != FALSE)
uint16 l_u16SlowBaudrateAdjustment = 0;											/* Minimum time */
uint16 l_u16LinAATimerPeriod;
PSNPD_DATA pSNPD_Data;
#endif /* (LINAA_BSM_SNPD_R1p0 != FALSE) */
int16 l_i16Ishunt;
uint16 l_u16AutoAddressingCM = 0;												/* Running sum of up to 2 Common mode ADC results */
int16 l_i16AutoAddressingDM = 0;												/* Running sum of up to 8 Differential mode ADC results */
int16 l_i16Ishunt1, l_i16Ishunt2, l_i16Ishunt3;									/* Shunt resistor currents */
#if (LINAA_BSM_SNPD_R1p0 == FALSE)
uint8 l_u8Step;
#endif /* (LINAA_BSM_SNPD_R1p0 == FALSE) */
#if LIN_AA_INFO
#if (USE_MULTI_PURPOSE_BUFFER == FALSE)
SNPD_DATA l_aSNPD_Data[LIN_AA_INFO_SZ];
#endif /* (USE_MULTI_PURPOSE_BUFFER == FALSE) */
uint8 l_u8SNPD_CycleCount;
#endif /* LIN_AA_INFO */
volatile ADC_LINAA LinAutoAddressing; /*lint !e552 */							/* LIN Auto Addressing measurement results */
#endif /* _SUPPORT_LIN_AA */

#pragma space none

/* ****************************************************************************	*
 *	Internal function prototypes												*
 * ****************************************************************************	*/
#if _SUPPORT_LIN_AA
void ml_AutoAddressingCfgADC( void);
#endif /* _SUPPORT_LIN_AA */

/* ****************************************************************************	*
 * LIN_Init()
 *
 * Initialise LIN communication interface.
 * Default start-up, at 9600 Baud
 * ****************************************************************************	*/
void LIN_Init( uint16 u16WarmStart)
{
	/* Initialise LIN Communication */
	LIN_XCFG &= ~DISTERM;														/* Enable LIN pull-up resistor */
	(void) ml_InitLinModule();													/* Initialise the LIN module */

	/* Setup LIN baudrate */
#if (__MLX_PLTF_VERSION_MAJOR__ == 4)
#if (_SUPPORT_AUTO_BAUDRATE != FALSE)
	/* Auto baudrate only on first LIN frame */
	(void) ml_SetAutoBaudRateMode( ML_ABR_ON_FIRST_FRAME);
#else  /* (_SUPPORT_AUTO_BAUDRATE != FALSE) */
	/* Fixed baudrate */
	(void) ml_SetBaudRate( (ml_uint8) LIN_BR_PRESCALER, (ml_uint8)LIN_BR_DIV);	/* Program the baudrate : default startup : 9600baud @ 20.0 Mhz	*/
#endif /* (_SUPPORT_AUTO_BAUDRATE != FALSE) */
#else  /*((__MLX_PLTF_VERSION_MAJOR__ == 4) */
	(void) ml_SetBaudRate( (ml_uint8) LIN_BR_PRESCALER, (ml_uint8)LIN_BR_DIV);	/* Program the baudrate : default startup : 9600baud @ 20.0 Mhz	*/
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 4) */

	/* Setup LIN options, including slew-rate */
#if (__MLX_PLTF_VERSION_MAJOR__ == 3)
#if ((LIN_BR < 10000) && (_SUPPORT_AUTO_BAUDRATE == FALSE))
	(void) ml_SetOptions( 1,													/* IDStopBitLength */
						  0,													/* TXStopBitLength */
						  ML_ENABLED,											/* StateChangeSignal */
						  1,													/* Light-sleep mode with timeout */
						  ML_SLEWLOW);											/* SlewRate: ML_SLEWHIGH=20kbps / ML_SLEWLOW=10kbps */
#else /* ((LIN_BR < 10000) && (_SUPPORT_AUTO_BAUDRATE == FALSE)) */
	(void) ml_SetOptions( 1,													/* IDStopBitLength */
						  0,													/* TXStopBitLength */
						  ML_ENABLED,											/* StateChangeSignal */
						  1,													/* Light-sleep mode with timeout */
						  ML_SLEWHIGH);											/* SlewRate: ML_SLEWHIGH=20kbps / ML_SLEWLOW=10kbps */
#endif /* ((LIN_BR < 10000) && (_SUPPORT_AUTO_BAUDRATE == FALSE)) */
#elif (__MLX_PLTF_VERSION_MAJOR__ == 4)
	(void) ml_SetOptions( 1U,													/* IDStopBitLength = 1.5 Bit (Melexis LIN Master has 1.5 Tbit stop bit */
						  0U,													/* TXStopBitLength = 1 Bit */
						  ML_ENABLED,											/* StateChangeSignal */
						  ML_LIGHTSLEEP);										/* SleepMode: light-sleep mode */

#if ((LIN_BR < 12000) && (_SUPPORT_AUTO_BAUDRATE == FALSE))
	(void) ml_SetSlewRate( ML_SLEWLOW);
#else /* ((LIN_BR < 10000) && (_SUPPORT_AUTO_BAUDRATE == FALSE)) */
	(void) ml_SetSlewRate( ML_SLEWHIGH);
#endif /* ((LIN_BR < 10000) && (_SUPPORT_AUTO_BAUDRATE == FALSE)) */
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 4) */

#if (__MLX_PLTF_VERSION_MAJOR__ == 3) && (_SUPPORT_AUTO_BAUDRATE == FALSE)
	LinBaudCtrl |= ML_LIN_BAUDRATE_DETECTED;									/* Fixed baudrate */
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 3) && (_SUPPORT_AUTO_BAUDRATE == FALSE) */

#if defined (HAS_LIN_NEXT_FRAME_ID)
	(void) ml_EnableFilter( 0);													/* Enable filter */
#endif

	/* Initialise communication layer */
#if ((LINPROT & LINXX) == LIN2X)
	LIN_2x_Init( u16WarmStart);
#endif /* ((LINPROT & LINXX) == LIN2X) */

#if ((LINPROT & LINXX) == LIN2J)
	LIN_SAE_J2602_Init( u16WarmStart);
#endif /* ((LINPROT & LINXX) == LIN2J) */

	(void) ml_Connect();
	g_u8ErrorCommBusTimeout = FALSE;
} /* End of LIN_Init() */

/* ****************************************************************************	*
 *  LIN API event: mlu_ApplicationStop
 * ****************************************************************************	*/
ml_Status mlu_ApplicationStop(void)
{
	/* Stop motor (e.g. disconnect drivers) */
	MotorDriverStop( (uint16) C_STOP_IMMEDIATE);								/* Application stop */
	SetLastError( (uint8) C_ERR_APPL_STOP);

	/* Disable all IRQ's, except LIN */
	XI0_MASK = 0;
	XI1_MASK = 0;
	XI2_MASK = 0;
	XI3_MASK = 0;
	XI4_MASK = 0;
	MASK = EN_M4_SHE_IT;														/* Disable all interrupts, except LIN M4_SHE_IT */
	g_e8MotorStatusMode |= (uint8) C_MOTOR_STATUS_APPL_STOP;					/* Don't perform periodic MLX4 Status checks */

	return ( ML_SUCCESS );														/* Return that the application has stopped */
} /* End of mlu_ApplicationStop */

/* ****************************************************************************	*
 *  LIN API event: Data Request (slave TX)
 * ****************************************************************************	*/
void mlu_DataRequest( ml_MessageID MessageID) 
{
	g_u8ErrorCommBusTimeout = FALSE;											/* Data requested; No longer Bus time-out */

#if ((LINPROT & LINXX) == LIN2X)
	if ( MessageID == (uint8) mlxRFR_DIAG )
	{
		/* Diagnostic */
		if ( g_u8BufferOutID == (uint8) QR_RFR_DIAG )
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
#if (LINPROT == LIN2X_ACT44)
	else if ( MessageID == MSG_STATUS )
	{
		HandleActStatus();														/* Handle HVAC or AGS Status */
		/* (void) ml_DataReady( ML_END_OF_TX_DISABLED); */						/* Handled by HandleActStatus() */
	}
#endif /* (LINPROT == LIN2X_ACT44) */
#if (__MLX_PLTF_VERSION_MAJOR__ == 3)
	(void) ml_ReleaseBuffer();													/* See MELEXIS doc */
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 3) */
#endif /* ((LINPROT & LINXX) == LIN2X) */

#if ((LINPROT & LINXX) == LIN2J)												/* LIN 2.x_J2602 */
	if ( MessageID == (uint8) mlxRFR_DIAG )
	{
		/* Diagnostic */
		if ( g_u8BufferOutID == (uint8) QR_RFR_DIAG )
		{
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
#if (LINPROT == LIN2J_VALVE_VW)
	else if ( (g_u8NAD >= (uint8) C_MIN_J2602_NAD) && (g_u8NAD <= (uint8) C_MAX_J2602_NAD) )
	{
		if ( MessageID == (uint8) mlxACT_STATUS )
		{
			/* Status AGS */
			HandleActRfrSta();
			(void) ml_DataReady( ML_END_OF_TX_ENABLED);
		}
	}
#endif /* (LINPROT == LIN2J_VALVE_VW) */
#if (__MLX_PLTF_VERSION_MAJOR__ == 3)
	(void) ml_ReleaseBuffer();													/* See MELEXIS doc */
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 3) */
#endif /* ((LINPROT & LINXX) == LIN2J) */

#if ((LINPROT & LINXX) == LIN2X)
	if ( g_u8LinAAMode != 0 )
	{
		(void)ml_GetAutoaddressingStatus();
	}
#endif /* ((LINPROT & LINXX) == LIN2X) */
} /* End of mlu_DataRequest */

/* ****************************************************************************	*
 *  LIN API event: mlu_DataTransmitted
 * ****************************************************************************	*/
void mlu_DataTransmitted(void) 
{
#if ((LINPROT & LINXX) == LIN2J)
#if (LINPROT != LIN2J_VALVE_VW)
	if ( g_u8SAE_SendErrorState )
	{
		g_u8SAE_ErrorFlags &= ~(uint8)(1 << g_u8SAE_SendErrorState);			/* Clear error-flags which have been transmitted */
	}
#else  /* (LINPROT != LIN2J_VALVE_VW) */
	if ( g_u8SAE_SendErrorState )
	{
		g_u8ErrorCommunication = FALSE;
	}
#endif /* (LINPROT != LIN2J_VALVE_VW) */
#endif /* ((LINPROT & LINXX) == LIN2J) */

#if (__MLX_PLTF_VERSION_MAJOR__ == 3)
	(void) ml_ReleaseBuffer();
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 3) */
} /* End of mlu_DataTransmitted() */

/* ****************************************************************************	*
 *  LIN API event: mlu_ErrorDetected
 * ****************************************************************************	*/
void mlu_ErrorDetected( ml_LinError Error)
{
	SetLastError( (uint8) C_ERR_LIN_COMM | ((uint8)Error & 0x1F));
#if (LINPROT == LIN2X_ACT44)
	LIN2x_ErrorHandling( Error);
#if (__MLX_PLTF_VERSION_MAJOR__ == 3)
	(void) ml_ReleaseBuffer(); 													/* Release the buffer in case there was a reception overflow */
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 3) */
#elif (LINPROT == LIN2J_VALVE_VW)
	LIN2J_ErrorHandling( Error);
#else  /* (LINPROT == LIN2X_ACT44) */

#if (__MLX_PLTF_VERSION_MAJOR__ == 3)
	(void) ml_ReleaseBuffer(); 													/* Release the buffer in case there was a reception overflow */
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 3) */
	switch ( Error )
	{
		case ml_erLinModuleReset :												/* erCRASH : LIN task crashed, trying to reboot              */
		case ml_erTimeOutResponse : 											/* erTORESP: Response Timeout                                */
		case ml_erBreakDetected :												/* erBRFRM : Break in frame                           	     */
			break;

		case ml_erIdParity :													/* erIDPAR : Parity error in ID field received               */
#if ((LINPROT & LINXX) == LIN2J)
			g_u8SAE_ErrorFlags |= (1 << C_SAE_ID_PARITY_ERROR);
#else  /* ((LINPROT & LINXX) == LIN2J) */
			g_u8ErrorCommunication = TRUE;										/* Set the communication error flag */
#endif /* ((LINPROT & LINXX) == LIN2J) */
			break;

		case ml_erCheckSum : 													/* erCKSUM : Checksum error in message received              */
#if ((LINPROT & LINXX) == LIN2J)
			g_u8SAE_ErrorFlags |= (1 << C_SAE_CHECKSUM_ERROR);
#else  /* ((LINPROT & LINXX) == LIN2J) */
			g_u8ErrorCommunication = TRUE;										/* Set the communication error flag */
#endif /* ((LINPROT & LINXX) == LIN2J) */
			break;

		case ml_erStopBitTX :
		case ml_erDataFraming : 												/* erSTOP : Stop bit error (regular data byte or checksum)   */
#if ((LINPROT & LINXX) == LIN2J)
			g_u8SAE_ErrorFlags |= (1 << C_SAE_BYTE_FRAMING_ERROR);
			break;
#endif /* ((LINPROT & LINXX) == LIN2J) */

		case ml_erBit : 														/* erTXCOL : Data collision during the transmit cycle        */
#if ((LINPROT & LINXX) == LIN2J)
			g_u8SAE_ErrorFlags |= (1 << C_SAE_DATA_ERROR);
			break;
#endif /* ((LINPROT & LINXX) == LIN2J) */

		case ml_erIdFraming : 													/* erIDSTOP : Stop bit error of the ID field                 */
		case ml_erShort : 														/* erSHORT : Short detected on the LIN bus                   */
#if ((LINPROT & LINXX) == LIN2X)
		case ml_erNextFrameIDCode : 											/* erQRCODE : QR Code error                                  */
#endif /* ((LINPROT & LINXX) == LIN2X) */
#if ((LINPROT & LINXX) != LIN2J)
			g_u8ErrorCommunication = TRUE;										/* Set the communication error flag */
#endif /* ((LINPROT & LINXX) != LIN2J) */
			break;

		case ml_erSynchField :													/* erSYNC : Sync field timing error                          */
#if ((LINPROT & LINXX) == LIN2J)
			g_u8SAE_ErrorFlags |= (1 << C_SAE_DATA_ERROR);
#endif /* ((LINPROT & LINXX) == LIN2J) */
			break;

		case ml_erBufferLocked : 												/* erRXOVR : Message received but buffer was full            */
#if (__MLX_PLTF_VERSION_MAJOR__ == 3)
			(void) ml_ReleaseBuffer();
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 3) */
			break;

		case ml_erShortDone :													/* erSHORTDONE : A short state has been seen on the bus, but now recovered */
		case ml_erWakeUpInit :													/* erWKUPINIT : A valid wake-up pulse has been seen while waiting to enter in sleep state */
		default :																/* Unrecognised error                                        */
			break;
	}
#endif /* (LINPROT == LIN2X_ACT44) */
#if ((LINPROT & LINXX) == LIN2X)
	if ( g_u8LinAAMode != 0 )
	{
		(void)ml_GetAutoaddressingStatus();
	}
#endif /* ((LINPROT & LINXX) == LIN2X) */
} /* End of mlu_ErrorDetected() */

#if defined (USE_LIN_FRAME_MODE)
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
void mlu_LinSleepMode(ml_StateReason Reason)
{
	/*
	 * MLX4 FW handles Goto Sleep frame (0x3C, 0x00 ...) automatically
	 * and does not report it via mlu_MessageReceived event.
	 */
	if ( (Reason == ml_reasonMaster) || (Reason == ml_reasonCommand) )
	{
		{
			g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_SLEEP;
		}
	}

	if ( (Reason == ml_reasonTimeOut) || (Reason == ml_reasonTimeOutDominant) )
	{
		/*
		 * LIN bus was inactive for 4 seconds without receiving an explicit
		 * "Go-to-Sleep frame". This can be considered as a failure of the Master or
		 * PHY layer. Slave can enter limp-home mode.
		 */
		HandleBusTimeout();
	}

} /* End of mlu_LinSleepMode() */

/* ****************************************************************************	*
 *  LIN API event: MessageReceived (slave RX)
 * ****************************************************************************	*/
void mlu_MessageReceived( ml_MessageID byMessageID)
{
	if ( g_u8LinInFrameBufState != (uint8) C_LIN_IN_FULL )
	{
		/* Buffer is either empty or message is postpone (overwrite allowed) */
		l_u8LinInFrameMsgID = byMessageID;

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
#if (__MLX_PLTF_VERSION_MAJOR__ == 3)
		(void) ml_ReleaseBuffer();												/* See MELEXIS doc */
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 3) */
		g_u8LinInFrameBufState = (uint8) C_LIN_IN_FULL;
		g_u8ErrorCommBusTimeout = FALSE;										/* Frame received; No longer Bus time-out */
		LinFrame[0] = 0x00;														/* Clear NAD address */
	}
} /* End of mlu_MessageReceived() */

void HandleLinInMsg( void)
{
	if ( g_u8LinInFrameBufState == (uint8) C_LIN_IN_POSTPONE )
	{
		/* Last message postponed; Try again (without overwritten by LIN message ISR */
		g_u8LinInFrameBufState = (uint8) C_LIN_IN_FULL;
	}

#if ((LINPROT & LINXX) == LIN2X)												/* LIN 2.x */
	if ( l_u8LinInFrameMsgID == (uint8) mlxDFR_DIAG )
	{
		/* Diagnostic */
		HandleDfrDiag();
	}
#if (LINPROT == LIN2X_ACT44)													/* LIN 2.x - HVAC Actuator 4.4 or (Valeo) AGS 13 */
	else if ( l_u8LinInFrameMsgID == (uint8) MSG_CONTROL )
	{
		/* Control */
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
		HandleActCtrl( FALSE);
#else  /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
		HandleActCtrl();
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
	}
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
	else if ( l_u8LinInFrameMsgID == (uint8) MSG_GROUP_CONTROL )
	{
		/* Control */
		HandleActCtrl( TRUE);
	}
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
#endif /* (LINPROT == LIN2X_ACT44) */
#endif /* ((LINPROT & LINXX) == LIN2X) */

#if ((LINPROT & LINXX) == LIN2J)												/* LIN 2.x_J2602 */
	if ( l_u8LinInFrameMsgID == (uint8) mlxDFR_DIAG )
	{
		/* Diagnostic frame */
		HandleDfrDiag();
	}
#if (LINPROT == LIN2J_VALVE_VW)
	else if ( (g_u8NAD >= (uint8) C_MIN_J2602_NAD) && (g_u8NAD <= (uint8) C_MAX_J2602_NAD) && (l_u8LinInFrameMsgID == (uint8) mlxACT_CTRL) )
	{
		/* Control */
		HandleActCfrCtrl();
	}
#endif /* (LINPROT == LIN2J_VALVE_VW) */
#endif /* ((LINPROT & LINXX) == LIN2J) */

	if ( g_u8LinInFrameBufState != (uint8) C_LIN_IN_POSTPONE )
	{
		/* LIN Message is handled; Release LIN message buffer */
		g_u8LinInFrameBufState = (uint8) C_LIN_IN_FREE;
	}

#if ((LINPROT & LINXX) == LIN2X)
	if ( g_u8LinAAMode != 0 )
	{
		(void)ml_GetAutoaddressingStatus();
	}
#endif /* ((LINPROT & LINXX) == LIN2X) */
} /* End of HandleLinInMsg() */

#if _SUPPORT_LIN_AA

/************************ AUTO ADDRESSING ***********************/
uint16 const SBASE_LIN[19] = { 
	(ADC_CH15 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 15 : CM/2, Vref = 2.5V, #1 */
	(ADC_CH15 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 15 : CM/2, Vref = 2.5V, #2 */
#if (LINAA_NON_CHOPPER_MODE == FALSE)
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #1 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #1 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #2 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #2 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #3 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #3 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #4 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #4 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #5 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #5 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #6 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #6 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #7 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #7 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #8 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #8 */
#else /* LINAA_NON_CHOPPER_MODE == FALSE */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #1 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #2 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #3 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #4 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #5 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #6 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #7 */
	(ADC_CH12 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 12 : LIN-Shunt (Mode 1), Vref = 2.5V, #8 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #1 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #2 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #3 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #4 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #5 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #6 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #7 */
	(ADC_CH28 | ADC_HW_TRIGGER_TMR2_CMPB | ADC_REF_2_50_V),						/* Channel 28 : LIN-Shunt (Mode 2), Vref = 2.5V, #8 */
#endif /* LINAA_NON_CHOPPER_MODE == FALSE */
	0xFFFF };																	/* End-of-table */

/* ****************************************************************************	*
 *  LIN API event: mlu_AutoAddressingStep
 * ****************************************************************************	*/
#define DELAY_40US (uint16)((40U * PLL_freq)/(1000000U*2*CYCLES_PER_INSTR))		/* 40us * PLL / (cy/inst) * us * 2 inst/loop */

/* Timer2 is used for LIN Auto-Addressing ADC trigger; Timer1 could also be used, as the motor is not running */
#define LINAA_TIMER_RESET()			{TMR2_CTRL = 0;}
#define LINAA_TIMER_SETUP()			{TMR2_CTRL = (0 * TMRx_DIV0) | (0 * TMRx_MODE0) | TMRx_T_EBLK;}
#define LINAA_TIMER_START()			{TMR2_CTRL = (0 * TMRx_DIV0) | (0 * TMRx_MODE0) | TMRx_T_EBLK | TMRx_START;}
#define LINAA_TIMER_STOP()			{TMR2_CTRL = (0 * TMRx_DIV0) | (0 * TMRx_MODE0) | TMRx_T_EBLK;}
#define LINAA_TIMER_PERIOD(p)		{TMR2_REGB = p;}
#define LINAA_TIMER_IRQ_DIS()		{BEGIN_CRITICAL_SECTION(); MASK &= ~EN_EXT1_IT; END_CRITICAL_SECTION();}

void mlu_AutoAddressingStep( ml_uint8 StepNumber)
{
#if (LINAA_BSM_SNPD_R1p0 == FALSE)
	l_u8AutoAddressingPulse = StepNumber;
#else  /* (LINAA_BSM_SNPD_R1p0 == FALSE) */
	if ( StepNumber == 1 )
	{
		/* *** Step 0: Setup LIN-AA for current-offset measurement *** */
		/* configure Auto addressing hardware cell on all slaves*/
		LIN_XCFG |= DISTERM;													/* Disable LIN pull-up resistor */
		ANA_OUTH = 0x0000;														/* Reset at start */
		ANA_OUTH = EN_LIN_AGC | ((0 << 3) & TRIM_LIN_CS) | (0 & SEL_LIN_CS);	/* Enable LIN shunt measurement, zero-current measurement */
		{
			uint8 u8AutoAddressingFlags = g_u8AutoAddressingFlags;
			u8AutoAddressingFlags &= (uint8) ~(SLAVEFINALSTEP | LASTSLAVE);		/* LASTSLAVE flag is cleared */
			g_u8AutoAddressingFlags = u8AutoAddressingFlags;

			if ( (u8AutoAddressingFlags & SLAVEADDRESSED) == 0 )
			{
				/* Configure LIN-AA Timer to be used for ADC LIN shunt current measurement. */
				LINAA_TIMER_SETUP();											/* Setup LIN-AA Timer: Divider = 1, Mode = 0 (Timer Mode) */
				LINAA_TIMER_PERIOD( l_u16LinAATimerPeriod);

				ml_AutoAddressingCfgADC();										/* Setup LIN-shunt current measurement */

#if ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE))
				pSNPD_Data = LIN_AA_DATA + l_u8SNPD_CycleCount;
#endif /* ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE)) */
			}
		}
	}
	else if ( (g_u8AutoAddressingFlags & SLAVEADDRESSED) == 0 )
	{
		if ( StepNumber == 2 )
		{
			/* *** Step 1: Perform current-offset measurement *** */
			NopDelay( l_u16SlowBaudrateAdjustment); /*lint !e522 */

			/* Test-module: No ADC-measurement, only current source */
			ADC_CTRL = (ADC_TRIG_SRC | ADC_SYNC_SOC | ADC_START);
			LINAA_TIMER_START();												/* Start LIN-shunt current measurement (LIN-AA Timer) */
		}
		else if ( StepNumber == 3 )
		{
			/* *** Step 2: Setup for pull-up LIN-shunt current measurement *** */
			if ( l_u16SlowBaudrateAdjustment )
			{
				NopDelay( DELAY_40US / 2); /*lint !e522 */
			}

			/* Setup 2nd current measurement; Either enable pull-up or enable current-source of 1.12mA */
#if (FORCE_LINAA_OLD != FALSE)
			/* switch on LIN Pull up resistor */
			LIN_XCFG &= ~DISTERM;												/* Enable LIN pull-up resistor (LIN 1.3/Cooling) */
#else  /* (FORCE_LINAA_OLD != FALSE) */
			if ( g_u8LinAAMode & C_SNPD_METHOD_2 )
			{
				LIN_XCFG &= ~DISTERM;											/* Enable LIN pull-up resistor (LIN 2.x/BSM2) */
			}
			else
			{
				/* Enable LIN shunt measurement, LIN high-current measurement (1.12mA) (LIN 2.x/BSM) */
				ANA_OUTH = EN_LIN_AGC | (CalibrationParams.EE_ANA_OUTH_112_205 & (TRIM_LIN_CS | SEL_LIN_CS));
			}
#endif /* (FORCE_LINAA_OLD != FALSE) */

			/* Normal slave module */
			l_i16Ishunt1 = l_i16Ishunt;

			ml_AutoAddressingCfgADC();											/* Start LIN-shunt current measurement */
		}
		else if ( StepNumber == 4 )
		{
			/* *** Step 3: Perform pull-up LIN-shunt measurement *** */

			/* Test-module: No ADC-measurement, only current source */
			ADC_CTRL = (ADC_TRIG_SRC | ADC_SYNC_SOC | ADC_START);
			LINAA_TIMER_START();												/* Start LIN-shunt current measurement (LIN-AA Timer) */

#if ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE))
			pSNPD_Data->u16CM_1 = l_u16AutoAddressingCM;
			pSNPD_Data->u16DM_1 = (uint16) l_i16AutoAddressingDM;
#endif /* ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE)) */
			/* Clear variables */
			l_u16AutoAddressingCM = 0;
			l_i16AutoAddressingDM = 0;
		}
		else if ( StepNumber == 5 )
		{
			/* *** Step 4: Pre-select: Check I2-I1 < threshold1 then Setup for current-source measurement, otherwise "quit" (wait) *** */
			NopDelay( l_u16SlowBaudrateAdjustment); /*lint !e522 */

			l_i16Ishunt2 = l_i16Ishunt;
#if (FORCE_LINAA_OLD != FALSE)
			if ( (l_i16Ishunt2 - l_i16Ishunt1) > C_LIN13AA_dI_1 )				/* Calculate difference Ishunt2 - Ishunt1 */
#else  /* (FORCE_LINAA_OLD != FALSE) */
			if ( ((g_u8LinAAMode & C_SNPD_METHOD_2) && ((l_i16Ishunt2 - l_i16Ishunt1) > C_LIN2xAA_dI_1_BSM2)) ||		/* BSM 2: Calculate difference Ishunt2 - Ishunt1 > 0.5mA */
				 (((g_u8LinAAMode & C_SNPD_METHOD_2) == 0) && ((l_i16Ishunt2 - l_i16Ishunt1) > C_LIN2xAA_dI_1_BSM)) )	/* BSM 1: Calculate difference Ishunt2 - Ishunt1 > 2.0mA */
#endif /* (FORCE_LINAA_OLD != FALSE) */
			{
				/* If Ishunt2 - Ishunt1 > It_1, disable standard pull up and goto wait state */
				ANA_OUTH = EN_LIN_AGC | ((0 << 3) & TRIM_LIN_CS) | (0 & SEL_LIN_CS);/* Enable LIN shunt measurement, zero-current measurement */
				LIN_XCFG |= DISTERM;											/* Disable LIN pull-up resistor */

#if ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE))
				pSNPD_Data->u16CM_2 = l_u16AutoAddressingCM;
				pSNPD_Data->u16DM_2 = (uint16) l_i16AutoAddressingDM;
#endif /* ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE)) */
			}
			else
			{
				/* Setup 3th current measurement; Either enable pull-up + current-source of 2.05mA or enable current-source of 4.62mA */
#if (FORCE_LINAA_OLD != FALSE)
				/* Turn on current source value of 2.05mA */
				ANA_OUTH = EN_LIN_AGC | ((CalibrationParams.EE_ANA_OUTH_112_205 >> 8) & (TRIM_LIN_CS | SEL_LIN_CS)); /* Enable LIN shunt measurement, LIN high-current measurement (2.05mA) */
#else  /* (FORCE_LINAA_OLD != FALSE) */
				if ( g_u8LinAAMode & C_SNPD_METHOD_2 )
				{
					/* Turn on current source value of 2.05mA */
					ANA_OUTH = EN_LIN_AGC | ((CalibrationParams.EE_ANA_OUTH_112_205 >> 8) & (TRIM_LIN_CS | SEL_LIN_CS)); /* Enable LIN shunt measurement, LIN high-current measurement (2.05mA) */
				}
				else
				{
					/* Enable LIN shunt measurement, LIN high-current measurement (4.62mA) (LIN 2.x/BSM); Use 3.5mA current-source only */
					ANA_OUTH = EN_LIN_AGC | (CalibrationParams.EE_ANA_OUTH_350_800 & (TRIM_LIN_CS | SEL_LIN_CS));
				}
#endif /* (FORCE_LINAA_OLD != FALSE) */
				g_u8AutoAddressingFlags |= SLAVEFINALSTEP;

				ml_AutoAddressingCfgADC();										/* Start LIN-shunt current measurement */
			}
		}
		else if ( (StepNumber == 6) && ((g_u8AutoAddressingFlags & SLAVEFINALSTEP) != 0) ) /*lint !e845 */
		{
			/* *** Step 5: Perform current source measurement *** */
			NopDelay( l_u16SlowBaudrateAdjustment); /*lint !e522 */

			/* Test-module: No ADC-measurement, only current source */
			ADC_CTRL = (ADC_TRIG_SRC | ADC_SYNC_SOC | ADC_START);
			LINAA_TIMER_START();												/* Start LIN-shunt current measurement (LIN-AA Timer) */

#if ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE))
			pSNPD_Data->u16CM_2 = l_u16AutoAddressingCM;
			pSNPD_Data->u16DM_2 = (uint16) l_i16AutoAddressingDM;
#endif /* ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE)) */
			/* Clear variables */
			l_u16AutoAddressingCM = 0;
			l_i16AutoAddressingDM = 0;
		}
	}
	if (StepNumber >= 7)
	{
		/* All slaves switch off current source and switch on pull up */
		ANA_OUTH &= ~EN_LIN_AGC;												/* Turn off LIN Shunt Current measurement */
		LIN_XCFG &= ~DISTERM;													/* Enable LIN pull-up resistor */
		LINAA_TIMER_RESET();													/* Reset LIN-AA timer */

		if ( (StepNumber == 7) && ((g_u8AutoAddressingFlags & SLAVEFINALSTEP) != 0) ) /*lint !e845 */
		{
			/* *** Step 6 is only updated for slaves that did not skip any step before;
				 Last-slave?: Check I3-I1 < threshold2, then last-slave otherwise not  *** */
			l_i16Ishunt3 = l_i16Ishunt;
#if ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE))
			pSNPD_Data->u16CM_3 = l_u16AutoAddressingCM;
			pSNPD_Data->u16DM_3 = (uint16) l_i16AutoAddressingDM;
#endif /* ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE)) */

#if (FORCE_LINAA_OLD != FALSE)
			if ( (l_i16Ishunt3 - l_i16Ishunt1) < C_LIN13AA_dI_2 )				/* Check current difference between Ishunt3 and Ishunt1 */
#else  /* (FORCE_LINAA_OLD != FALSE) */
			if ( ((g_u8LinAAMode & C_SNPD_METHOD_2) && ((l_i16Ishunt3 - l_i16Ishunt1) < C_LIN2xAA_dI_2_BSM2)) ||		/* BSM 2: Calculate difference Ishunt2 - Ishunt1 < 1.2mA */
				(((g_u8LinAAMode & C_SNPD_METHOD_2) == 0) && ((l_i16Ishunt2 - l_i16Ishunt1) < C_LIN2xAA_dI_2_BSM)) )	/* BSM 1: Calculate difference Ishunt2 - Ishunt1 < 2.0mA */
#endif /* (FORCE_LINAA_OLD != FALSE) */
			{
				g_u8AutoAddressingFlags |= (uint8) LASTSLAVE;					/* This LIN-slave is the last slave: take the address and set SLAVEADDRESSED flag */
			}
		}
		if ( StepNumber == 7 )
		{
#if LIN_AA_INFO
			pSNPD_Data->byStepAndFlags = (7 << 3) | (g_u8AutoAddressingFlags & 0x87);
			pSNPD_Data->byIshunt1 = (uint8) l_i16Ishunt1;
			pSNPD_Data->byIshunt2 = (uint8) l_i16Ishunt2;
			pSNPD_Data->byIshunt3 = (uint8) l_i16Ishunt3;
			if ( l_u8SNPD_CycleCount < (LIN_AA_INFO_SZ - 1) )					/* Don't increase index in case last AA-structure index */
			{
				l_u8SNPD_CycleCount++;
			}
#endif /* LIN_AA_INFO */
		}
	}

#endif /* (LINAA_BSM_SNPD_R1p0 == FALSE) */
} /* End of mlu_AutoAddressingStep() */

/* ****************************************************************************	*
 *  ml_SetSlaveNotAddressed
 *
 * Resets the SLAVEADDRESSED flag. The actuator will try to acquire a new  
 * address next sync break.
 * ****************************************************************************	*/
void ml_SetSlaveNotAddressed( void)
{
	g_u8AutoAddressingFlags &= (uint8) ~SLAVEADDRESSED;
} /* End of ml_SetSlaveNotAddressed() */

/* ****************************************************************************	*
 * void ml_SetSlaveNotAddressed(void)
 * 
 * Sets the SLAVEADDRESSED flag. The actuator will assume it is already addressed
 * and not execute all auto-addressing steps.
 * ****************************************************************************	*/
void ml_SetSlaveAddressed( void)
{
	g_u8AutoAddressingFlags |= (uint8) SLAVEADDRESSED;
} /* End of ml_SetSlaveAddressed */

/* ****************************************************************************	*
 * void ml_GetAutoaddressingstatus(void)
 * 
 * Initialise the hardware and software of the auto addressing module
 *
 *
 * The first addressing-pulse from the MLX4 is "delayed" by approx. 35us. This is 
 * the MLX4 reference for generating future/next addressing-pulses. To get the AA 
 * steps on time, optional an extra delay is added. Therefore the second 
 * addressing-pulse comes after 0.5Tbit. At 19200Baud, the 1Tbit period (52us) is 
 * at 61us (35us+26us). For 9600baud, the 1Tbit period (104us) is at 117us
 * (35us+52us+30us delay)
 *
 * Hardware resources: Timer2 (can also use Timer1, as motor is stopped anyway)
 *
 * IO5:	L = Executing code
 *		H = Waiting for pulse from MLX4
 * IO4:	Toggles each "step"
 *	Steps:										 IO[5:4]	l_u8AutoAddressingPulse
 *	-: Pre-setup									0:1			0
 *	-: Waiting for next LIN-break (pulse #1)		1:1			0
 *	0: Pulse #1 received							0:1			1
 *	0: Switch off pull-up, setup timer2				0:0			1
 *	0: Waiting for pulse #2 @ 1T					1:0			1
 *	1: Pulse #2 received							0:0			2
 *	1: Setup ADC									0:1			2
 *	1: ADC & Timer started (measurement on going)	0:0			2
 *	1: Wait for pulse #3 @ 4T						1:0			2
 *	1: ADC ISR (Measurement done)					1:1			2
 *	1: Ishunt calculated from measurements			1:0			2
 *	2: Pulse #3 received							0:1			3
 *	2: Switch on pull-up							0:0			3
 *	2: Wait for pulse #4 @ 5T						1:0			3
 *	3: Pulse #4 received							0:0			4
 *	3: Setup ADC									0:1			4
 *	3: ADC & Timer started (measurement on going)	0:0			4
 *	3: Wait for pulse #5 @ 8T						1:0			4
 *	3: ADC ISR (Measurement done)					1:1			4
 *	3: Ishunt calculated from measurements			1:0			4
 *	4: Pulse #5 received							0:1			5
 *	4: Switch on pull-up & current-source			0:0			5
 *	4: Wait for pulse #6 @ 9T						1:0			5
 *	5: Pulse #6 received							0:0			6
 *	5: Setup ADC									0:1			6
 *	5: ADC & Timer started (measurement on going)	0:0			6
 *	5: Wait for pulse #7 @ 12T						1:0			6
 *	5: ADC ISR (Measurement done)					1:1			6
 *	5: Ishunt calculated from measurements			1:0			6
 *	6: Pulse #7 received							0:1			7
 *	6: Switch off current-source en ADC-timer		0:0			7
 *	6: Last device check							0:1			7
 *	6: Finished (< 13T)								0:0			7
 * ****************************************************************************	*/
#if (LINAA_BSM_SNPD_R1p0 != FALSE)
void ml_InitAutoAddressing( void)
{
	/* Calculate the detected MLX4 LIN Baudrate, based on baudrate divider and pre-scaler */
#if (__MLX_PLTF_VERSION_MAJOR__ == 4)
	if ( ml_GetBaudRate() < 12000 )
#elif ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 1))
	if ( divU16_U32byU16( (PLL_freq/2), (((uint16) u8NominalBaudRateDiv) << (u8BaudRatePreScaler & 0x0F))) < 12000 )
#else  /* ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 1)) */
	if ( divU16_U32byU16( (PLL_freq/2), (((uint16) u8NominalBaudRateDiv) << (u8BaudRatePreScaler >> 4))) < 12000 )
#endif /* ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 1)) */
	{
		l_u16SlowBaudrateAdjustment = DELAY_40US;								/* Time at 9600 Baud */
	}

	/* LIN-AA will claim the ADC and a Timer */
	/* Disable the ADC & IRQ */
	ADC_Stop();
	g_u8AdcIsrMode = (uint8) C_ADC_ISR_LIN_AA;									/* Set switch ADC to Auto-addressing-mode */
	ADC_SBASE = (uint16) SBASE_LIN;												/* Switch ADC input source to LIN Shunt */
#if (_LINAA_ASM == FALSE)
	ADC_DBASE = (uint16) &LinAutoAddressing.Result_LinShunt1_CommonMode;
#else  /* (_LINAA_ASM == FALSE) */
	ADC_DBASE = (uint16) &LinAutoAddressing.Result_LinAA[0];
#endif /* (_LINAA_ASM == FALSE) */

	/* Disable Timer2 & IRQ */
	LINAA_TIMER_RESET();														/* Reset timer */
	LINAA_TIMER_IRQ_DIS();														/* Disable LIN-AA Timer interrupt */

#if ((LINPROT & LINXX) == LIN13)
	/* LIN 1.3: 18T @ 9600 Baud or 36T @ 19200T, Measurement = 3.5-4T @ 9600 => 350us --> max 18us */
	l_u16LinAATimerPeriod = (PLL_freq/71428);									/* Set to 71.4kHz rate, 14us (266-280us) */
#else /* ((LINPROT & LINXX) == LIN13) */
	/* LIN 2.x: 13T, Measurement = 3T; At 9600Baud: 312.5us --> max 16us, At 19200 Baud: 156us --> max 8us */
	if ( l_u16SlowBaudrateAdjustment == DELAY_40US )
	{
		l_u16LinAATimerPeriod = (PLL_freq/83333);								/* Set to 83.3kHz rate, 12us (228-240us) */
	}
	else
	{
#if (LINAA_NON_CHOPPER_MODE == FALSE)
		l_u16LinAATimerPeriod = (PLL_freq/142857);								/* Set to 143kHz rate, 7us (133-150us) */
#else /* LINAA_NON_CHOPPER_MODE == FALSE */
		l_u16LinAATimerPeriod = (PLL_freq/166667);								/* Set to 167kHz rate, 6us (114-120us)*/
#endif /* LINAA_NON_CHOPPER_MODE == FALSE */
	}
#endif /* ((LINPROT & LINXX) == LIN13) */

#if defined (HAS_LIN_AUTOADDRESSING)
	/* enable the MLX4 auto addressing pulses */
	(void) ml_AutoAddressingConfig( TRUE);
#endif
} /* End of ml_InitAutoAddressing() */

void ml_StopAutoAddressing( void)
{
#if defined (HAS_LIN_AUTOADDRESSING)
	/* enable the MLX4 auto addressing pulses */
	(void) ml_AutoAddressingConfig( FALSE);
#endif

	g_u8AdcIsrMode = (uint8) C_ADC_ISR_NONE;									/* Set switch ADC off */

#if (LIN_AA_INFO != FALSE) && (USE_MULTI_PURPOSE_BUFFER != FALSE)
	g_MPBuf.u8Usage = (uint8) C_MP_BUF_FREE;									/* Multi-purpose buffer is free */
#endif /* (LIN_AA_INFO != FALSE) && (USE_MULTI_PURPOSE_BUFFER != FALSE) */
} /* End of ml_StopAutoAddressing() */
#endif /* (LINAA_BSM_SNPD_R1p0 != FALSE) */

ml_Status ml_GetAutoaddressingStatus( void)
{
#if (LINAA_BSM_SNPD_R1p0 == FALSE)
	uint16 u16SlowBaudrateAdjustment = 0;										/* Minimum time */
#if ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE))
	PSNPD_DATA pSNPD_Data = LIN_AA_DATA + l_u8SNPD_CycleCount;
#endif /* ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE)) */

	/* Disable the ADC & IRQ */
	ADC_Stop();
	g_u8AdcIsrMode = (uint8) C_ADC_ISR_LIN_AA;									/* Set switch ADC to Autoaddressingmode */
	ADC_SBASE = (uint16) SBASE_LIN;												/* Switch ADC input source to LIN Shunt */
#if (_LINAA_ASM == FALSE)
	ADC_DBASE = (uint16) &LinAutoAddressing.Result_LinShunt1_CommonMode;
#else  /* (_LINAA_ASM == FALSE) */
	ADC_DBASE = (uint16) &LinAutoAddressing.Result_LinAA[0];
#endif /* (_LINAA_ASM == FALSE) */

	/* Calculate the detected MLX4 LIN Baudrate, based on baudrate divider and pre-scaler */
#if (__MLX_PLTF_VERSION_MAJOR__ == 4)
	if ( ml_GetBaudRate() < 12000 )
#elif ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 1))
	if ( divU16_U32byU16( (PLL_freq/2), (((uint16) u8NominalBaudRateDiv) << (u8BaudRatePreScaler & 0x0F))) < 12000 )
#else  /* ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 1)) */
	if ( divU16_U32byU16( (PLL_freq/2), (((uint16) u8NominalBaudRateDiv) << (u8BaudRatePreScaler >> 4))) < 12000 )
#endif /* ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 1)) */
	{
		u16SlowBaudrateAdjustment = DELAY_40US;									/* Time at 9600 Baud */
	}
	/* initialise variables */
	l_u8AutoAddressingPulse = 0;
	l_e8AutoAddressingState = (uint8) AUTOADDRESSING_IDLE;

	/* slave not addressed, action not completed, slave not waiting,
	   not the first sample, AcqMode = CM, ADCIRQ is for auto-addressing mode */
	g_u8AutoAddressingFlags = (g_u8AutoAddressingFlags & SLAVEADDRESSED) | WAITINGFORBREAK; /* Remember the previous state & start background Watchdog ACKnowledges */

	/* clear the Current measurement variables */
	l_i16Ishunt1 = 0;
	l_i16Ishunt2 = 0;
	l_i16Ishunt3 = 0;

	/* Clear variables */
	l_u16AutoAddressingCM = 0;
	l_i16AutoAddressingDM = 0;

	l_u8Step = 0;

#if defined (HAS_LIN_AUTOADDRESSING)
	/* enable the MLX4 auto addressing pulses */
	(void) ml_AutoAddressingConfig( ML_AUTORESET);
#endif

	/* Disable Timer2 & IRQ */
	TMR2_CTRL = 0;																/* Reset timer */
	BEGIN_CRITICAL_SECTION();
	MASK &= ~EN_EXT1_IT;														/* Disable Timer 2 interrupt */
	END_CRITICAL_SECTION();

	/* run the state machine until it has reached the end of step6.
	   The state machine starts in IDLE mode and will stay there until the next
	   sync break */

	/* *** Idle step *** */
	while ( l_u8AutoAddressingPulse < 1 ) {};									/* Wait for Pulse #1 */

	/* *** Step 0: Setup LIN-AA for current-offset measurement *** */
	/* configure Auto addressing hardware cell on all slaves*/
	LIN_XCFG |= DISTERM;														/* Disable LIN pullup resistor */
	ANA_OUTH = 0x0000;															/* Reset at start */
	ANA_OUTH = EN_LIN_AGC | ((0 << 3) & TRIM_LIN_CS) | (0 & SEL_LIN_CS);		/* Enable LIN shunt measurement, zero-current measurement */
	g_u8AutoAddressingFlags &= (uint8) ~WAITINGFORBREAK;						/* Break-signal happend; Stop (background) Watchdog acknowledges */

	/* Test-module: No ADC-measurement, only current source */
	/* Configure Timer2 to be used for ADC LIN shunt current measurement. Note: For LIN-AA also Timer1 could be used, as the motor is not running */
	TMR2_CTRL = (0 * TMRx_DIV0) | (0 * TMRx_MODE0) | TMRx_T_EBLK;				/* Enable Timer Module 2, Div = 1, Mode = 0 (Timer Mode) */
#if ((LINPROT & LINXX) == LIN13)
	/* LIN 1.3: 18T @ 9600 Baud or 36T @ 19200T, Measurement = 3.5-4T @ 9600 => 350us --> max 18us */
	TMR2_REGB = (PLL_freq/71428);												/* Set to 71.4kHz rate, 14us (266-280us) */
#else /* ((LINPROT & LINXX) == LIN13) */
	/* LIN 2.x: 13T, Measurement = 3T; At 9600Baud: 312.5us --> max 16us, At 19200 Baud: 156us --> max 8us */
	if ( u16SlowBaudrateAdjustment == DELAY_40US )
	{
		TMR2_REGB = (PLL_freq/83333);											/* Set to 83.3kHz rate, 12us (228-240us) */
	}
	else
	{
#if (LINAA_NON_CHOPPER_MODE == FALSE)
		LINAA_TIMER_PERIOD( PLL_freq/142857);									/* Set to 143kHz rate, 7us (133-150us) */
#else /* LINAA_NON_CHOPPER_MODE == FALSE */
		LINAA_TIMER_PERIOD( PLL_freq/166667);									/* Set to 167kHz rate, 6us (114-120us)*/
#endif /* LINAA_NON_CHOPPER_MODE == FALSE */
	}
#endif /* ((LINPROT & LINXX) == LIN13) */

	/* Check if slave is already addressed, or incorrect Pulse (sequence/time-out) */
	if ( ((g_u8AutoAddressingFlags & SLAVEADDRESSED) != 0) || (l_u8AutoAddressingPulse > 2) )
	{
		/* Put slave in waiting state */
		l_e8AutoAddressingState = (uint8) AUTOADDRESSING_WAIT;
	}
	else
	{
		g_u8AutoAddressingFlags &= (uint8) ~LASTSLAVE;
		ml_AutoAddressingCfgADC();												/* Setup LIN-shunt current measurement */
		while ( l_u8AutoAddressingPulse < 2 ) ;									/* Wait for Pulse #2 */
		NopDelay( u16SlowBaudrateAdjustment); /*lint !e522 */
		/* l_u8Step = 1; */

		/* Test-module: No ADC-measurement, only current source */
		ADC_CTRL = (ADC_TRIG_SRC | ADC_SYNC_SOC | ADC_START);
		LINAA_TIMER_START();													/* Start LIN-shunt current measurement (Timer2) */
	
		while ( l_u8AutoAddressingPulse < 3 ) ;									/* Wait for Pulse #3 */
		if ( u16SlowBaudrateAdjustment )
			NopDelay( DELAY_40US / 2); /*lint !e522 */

		/* *** Step 2: Setup for pull-up LIN-shunt current measurement *** */
		/* l_u8Step = 2; */

		/* Normal slave module */
		l_i16Ishunt1 = l_i16Ishunt; 

		/* NopDelay( 20); */

		/* Setup 2nd current measurement; Either enable pull-up or enable current-source of 1.12mA */
#if ((LINPROT == LIN13_COOLING23) || (FORCE_LINAA_OLD != FALSE))
		/* switch on LIN Pull up resistor */
		LIN_XCFG &= ~DISTERM;													/* Enable LIN pullup resistor (LIN 1.3/Cooling) */
#else  /* ((LINPROT == LIN13_COOLING23) || (FORCE_LINAA_OLD != FALSE)) */
		if ( g_u8LinAAMode & C_SNPD_METHOD_2 )
		{
			LIN_XCFG &= ~DISTERM;												/* Enable LIN pullup resistor (LIN 2.x/BSM2) */
		}
		else
		{
			/* Enable LIN shunt measurement, LIN high-current measurement (1.12mA) (LIN 2.x/BSM) */
			ANA_OUTH = EN_LIN_AGC | (CalibrationParams.EE_ANA_OUTH_112_205 & (TRIM_LIN_CS | SEL_LIN_CS));
		}
#endif /* ((LINPROT == LIN13_COOLING23) || (FORCE_LINAA_OLD != FALSE)) */

		ml_AutoAddressingCfgADC();												/* Start LIN-shunt current measurement */

		while ( l_u8AutoAddressingPulse < 4 ) ;									/* Wait for Pulse #4 */
		NopDelay( u16SlowBaudrateAdjustment); /*lint !e522 */

		/* *** Step 3: Perform pull-up LIN-shunt measurement *** */
		/* l_u8Step = 3; */

		/* Test-module: No ADC-measurement, only current source */
		ADC_CTRL = (ADC_TRIG_SRC | ADC_SYNC_SOC | ADC_START);
		LINAA_TIMER_START();													/* Start LIN-shunt current measurement (Timer2) */

#if ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE))
		pSNPD_Data->u16CM_1 = l_u16AutoAddressingCM;
		pSNPD_Data->u16DM_1 = (uint16) l_i16AutoAddressingDM;
#endif /* ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE)) */
		/* Clear variables */
		l_u16AutoAddressingCM = 0;
		l_i16AutoAddressingDM = 0;

		while ( l_u8AutoAddressingPulse < 5 ) ;									/* Wait for Pulse #5 */
		NopDelay( u16SlowBaudrateAdjustment); /*lint !e522 */

		/* *** Step 4: Pre-select: Check I2-I1 < threshold1 then Setup for current-source measurement, otherwise "quit" (wait) *** */
		/* l_u8Step = 4; */

		l_i16Ishunt2 = l_i16Ishunt; 
#if ((LINPROT == LIN13_COOLING23) || (FORCE_LINAA_OLD != FALSE))
		if ( (l_i16Ishunt2 - l_i16Ishunt1) > C_LIN13AA_dI_1 )					/* Calculate difference Ishunt2 - Ishunt1 */
#else  /* ((LINPROT == LIN13_COOLING23) || (FORCE_LINAA_OLD != FALSE)) */
		if ( ((g_u8LinAAMode & C_SNPD_METHOD_2) && ((l_i16Ishunt2 - l_i16Ishunt1) > C_LIN2xAA_dI_1_BSM2)) ||		/* BSM 2: Calculate difference Ishunt2 - Ishunt1 > 0.5mA */
			 (((g_u8LinAAMode & C_SNPD_METHOD_2) == 0) && ((l_i16Ishunt2 - l_i16Ishunt1) > C_LIN2xAA_dI_1_BSM)) )	/* BSM 1: Calculate difference Ishunt2 - Ishunt1 > 2.0mA */
#endif /* ((LINPROT == LIN13_COOLING23) || (FORCE_LINAA_OLD != FALSE)) */
		{
			/* If Ishunt2 - Ishunt1 > It_1, disable standard pull up and goto wait state */
			ANA_OUTH = EN_LIN_AGC | ((0 << 3) & TRIM_LIN_CS) | (0 & SEL_LIN_CS);/* Enable LIN shunt measurement, zero-current measurement */
			LIN_XCFG |= DISTERM;												/* Disable LIN pullup resistor */

#if ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE))
			pSNPD_Data->u16CM_2 = l_u16AutoAddressingCM;
			pSNPD_Data->u16DM_2 = (uint16) l_i16AutoAddressingDM;
#endif /* ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE)) */

			l_e8AutoAddressingState = (uint8) AUTOADDRESSING_WAIT;
		}
		else
		{
			/* Setup 3th current measurement; Either enable pull-up + current-source of 2.05mA or enable current-source of 4.62mA */
#if ((LINPROT == LIN13_COOLING23) || (FORCE_LINAA_OLD != FALSE))
			/* Turn on current source value of 2.05mA */
			ANA_OUTH = EN_LIN_AGC | ((CalibrationParams.EE_ANA_OUTH_112_205 >> 8) & (TRIM_LIN_CS | SEL_LIN_CS)); /* Enable LIN shunt measurement, LIN high-current measurement (2.05mA) */
#else  /* ((LINPROT == LIN13_COOLING23) || (FORCE_LINAA_OLD != FALSE)) */
			if ( g_u8LinAAMode & C_SNPD_METHOD_2 )
			{
				/* Turn on current source value of 2.05mA */
				ANA_OUTH = EN_LIN_AGC | ((CalibrationParams.EE_ANA_OUTH_112_205 >> 8) & (TRIM_LIN_CS | SEL_LIN_CS)); /* Enable LIN shunt measurement, LIN high-current measurement (2.05mA) */
			}
			else
			{
				/* Enable LIN shunt measurement, LIN high-current measurement (4.62mA) (LIN 2.x/BSM); Use 3.5mA current-source only */
				ANA_OUTH = EN_LIN_AGC | (CalibrationParams.EE_ANA_OUTH_350_800 & (TRIM_LIN_CS | SEL_LIN_CS));
			}
#endif /* ((LINPROT == LIN13_COOLING23) || (FORCE_LINAA_OLD != FALSE)) */

			ml_AutoAddressingCfgADC();											/* Start LIN-shunt current measurement */

			while ( l_u8AutoAddressingPulse < 6 ) ;								/* Wait for Pulse #6 */
			NopDelay( u16SlowBaudrateAdjustment); /*lint !e522 */
	
			/* *** Step 5: Perform current source measurement *** */
			l_u8Step = 5;

			/* Test-module: No ADC-measurement, only current source */
			ADC_CTRL = (ADC_TRIG_SRC | ADC_SYNC_SOC | ADC_START);
			LINAA_TIMER_START();												/* Start LIN-shunt current measurement (Timer2) */

#if ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE))
			pSNPD_Data->u16CM_2 = l_u16AutoAddressingCM;
			pSNPD_Data->u16DM_2 = (uint16) l_i16AutoAddressingDM;
#endif /* ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE)) */
			/* Clear variables */
			l_u16AutoAddressingCM = 0;
			l_i16AutoAddressingDM = 0;

			while ( l_u8AutoAddressingPulse < 7 ) ;								/* Wait for Pulse #7: All measurements are done */
			NopDelay( u16SlowBaudrateAdjustment); /*lint !e522 */
			l_e8AutoAddressingState = (uint8) AUTOADDRESSING_STEP6;				/* Make judgement on LIN-slave position */
		}
	}

	/* *** Wait Step *** */
	if ( l_e8AutoAddressingState == (uint8) AUTOADDRESSING_WAIT )
	{
		/* Wait for other devices to be finished (this device doesn't participate) */
		while ( l_u8AutoAddressingPulse < 7 ) ;									/* Wait for Pulse #7: All measurements are done */
		NopDelay( u16SlowBaudrateAdjustment); /*lint !e522 */
		l_e8AutoAddressingState = (uint8) AUTOADDRESSING_DONE;
	}

	/* All slaves switch off current source and switch on pull up */
	ANA_OUTH &= ~EN_LIN_AGC;													/* Turn off LIN Shunt Current measurement */
	LIN_XCFG &= ~DISTERM;														/* Enable LIN pull-up resistor */
	TMR2_CTRL = 0;																/* Stop Timer2 */

	/* *** Step 6 is only updated for slaves that did not skip any step before; 
		 Last-slave?: Check I3-I1 < threshold2, then last-slave otherwise not  *** */
	if ( l_e8AutoAddressingState == (uint8) AUTOADDRESSING_STEP6 )
	{
		l_u8Step = 6;

		l_i16Ishunt3 = l_i16Ishunt; 
#if ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE))
		pSNPD_Data->u16CM_3 = l_u16AutoAddressingCM;
		pSNPD_Data->u16DM_3 = (uint16) l_i16AutoAddressingDM;
#endif /* ((LIN_AA_INFO != FALSE) && (LIN_AA_SCREENTEST != FALSE)) */

#if ((LINPROT == LIN13_COOLING23) || (FORCE_LINAA_OLD != FALSE))
		if ( (l_i16Ishunt3 - l_i16Ishunt1) < C_LIN13AA_dI_2 )					/* Check current difference between Ishunt3 and Ishunt1 */
#else  /* ((LINPROT == LIN13_COOLING23) || (FORCE_LINAA_OLD != FALSE)) */
		if ( ((g_u8LinAAMode & C_SNPD_METHOD_2) && ((l_i16Ishunt3 - l_i16Ishunt1) < C_LIN2xAA_dI_2_BSM2)) ||		/* BSM 2: Calculate difference Ishunt2 - Ishunt1 < 1.2mA */
			 (((g_u8LinAAMode & C_SNPD_METHOD_2) == 0) && ((l_i16Ishunt2 - l_i16Ishunt1) < C_LIN2xAA_dI_2_BSM)) )	/* BSM 1: Calculate difference Ishunt2 - Ishunt1 < 2.0mA */
#endif /* ((LINPROT == LIN13_COOLING23) || (FORCE_LINAA_OLD != FALSE)) */
		{
			/* g_u8AutoAddressingFlags |= (uint8) (LASTSLAVE | SLAVEADDRESSED); */ /* This LIN-slave is the last slave: take the address and set SLAVEADDRESSED flag */
			g_u8AutoAddressingFlags |= (uint8) LASTSLAVE;						/* This LIN-slave is the last slave: take the address and set SLAVEADDRESSED flag */
		} 
																				/* Else we are not the last slave: nothing to do. */
		/* l_e8AutoAddressingState = (uint8) AUTOADDRESSING_DONE; */
	}

#if LIN_AA_INFO
	{
		pSNPD_Data->byStepAndFlags = ((l_u8Step & 0x0F) << 3) | (g_u8AutoAddressingFlags & 0x87);
		pSNPD_Data->byIshunt1 = (uint8) l_i16Ishunt1;
		pSNPD_Data->byIshunt2 = (uint8) l_i16Ishunt2;
		pSNPD_Data->byIshunt3 = (uint8) l_i16Ishunt3;
		if ( l_u8SNPD_CycleCount < (LIN_AA_INFO_SZ - 1) )						/* Don't increase index in case last AA-structure index */
		{
			l_u8SNPD_CycleCount++;
		}
#if USE_MULTI_PURPOSE_BUFFER
		g_MPBuf.u8Usage = (uint8) C_MP_BUF_FREE;								/* Multi-purpose buffer is free */
#endif /* USE_MULTI_PURPOSE_BUFFER */
	}
#endif /* LIN_AA_INFO */

	g_u8AdcIsrMode = (uint8) C_ADC_ISR_NONE;									/* Set switch ADC off */
#endif /* (LINAA_BSM_SNPD_R1p0 == FALSE) */

	/* the slave is addressed in this cylce if the flag in step 6 is set */
	if ( g_u8AutoAddressingFlags & LASTSLAVE )
	{
		return ( 1 );
	}
	else
	{
		return ( 0 );
	}
} /* End of ml_GetAutoaddressingstatus() */

/* ****************************************************************************	*
 * void AutoAddressingReadADCResult( void)
 *
 * function called when an ADC result has to be fetched in the
 * auto addressing mode
 * ****************************************************************************	*/
void AutoAddressingReadADCResult( void)
{
	LINAA_TIMER_STOP();															/* Stop LIN-AA Timer */

	/*	l_u16AutoAddressingCM =	((LinAutoAddressing.Result_LinShunt1_CommonMode & 0x03FF) + (LinAutoAddressing.Result_LinShunt2_CommonMode & 0x03FF)); */
#if (_LINAA_ASM == FALSE)
	int16 i16Sum;
	i16Sum =  *(int16 *) &LinAutoAddressing.Result_LinShunt1_CommonMode;
	i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt2_CommonMode;
	l_u16AutoAddressingCM = (i16Sum >> 1);
#else  /* (_LINAA_ASM == FALSE) */
	asm("mov x, #_LinAutoAddressing");
	asm("mov a, [x++]");		/* i16Sum =  *(int16 *) &LinAutoAddressing.Result_LinShunt1_CommonMode */
	asm("add a, [x++]");		/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt2_CommonMode */
	asm("lsr a, #1");			/* l_u16AutoAddressingCM = (i16Sum >> 1) */
	asm("mov _l_u16AutoAddressingCM, a");
#endif /* (_LINAA_ASM == FALSE) */
#if (LINAA_NON_CHOPPER_MODE == FALSE)
/*	l_i16AutoAddressingDM =	((int)(LinAutoAddressing.Result_LinShunt1_mode1 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt1_mode2 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt2_mode1 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt2_mode2 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt3_mode1 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt3_mode2 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt4_mode1 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt4_mode2 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt5_mode1 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt5_mode2 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt6_mode1 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt6_mode2 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt7_mode1 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt7_mode2 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt8_mode1 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt8_mode2 & 0x03FF)); */
	/* Check listing for usage X++/Y++ */
#if (_LINAA_ASM == FALSE)
	i16Sum =  *(int16 *) &LinAutoAddressing.Result_LinShunt1_mode1;
	i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt1_mode2;
	i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt2_mode1;
	i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt2_mode2;
	i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt3_mode1;
	i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt3_mode2;
	i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt4_mode1;
	i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt4_mode2;
	i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt5_mode1;
	i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt5_mode2;
	i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt6_mode1;
	i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt6_mode2;
	i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt7_mode1;
	i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt7_mode2;
	i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt8_mode1;
	i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt8_mode2;
	l_i16AutoAddressingDM = (i16Sum >> 4);
#else  /* (_LINAA_ASM == FALSE) */
	asm("mov a, [x++]");	/* i16Sum =  *(int16 *) &LinAutoAddressing.Result_LinShunt1_mode1 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt1_mode2 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt2_mode1 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt2_mode2 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt3_mode1 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt3_mode2 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt4_mode1 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt4_mode2 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt5_mode1 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt5_mode2 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt6_mode1 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt6_mode2 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt7_mode1 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt7_mode2 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt8_mode1 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt8_mode2 */
	asm("asr a, #2");
	asm("asr a, #2");
	asm("mov _l_i16AutoAddressingDM, a");
#endif /* (_LINAA_ASM == FALSE) */
#else /* LINAA_NON_CHOPPER_MODE == FALSE */
/*	AutoAddressingDM =	((int)(LinAutoAddressing.Result_LinShunt1_mode1 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt5_mode1 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt1_mode2 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt5_mode2 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt2_mode1 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt6_mode1 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt2_mode2 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt6_mode2 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt3_mode1 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt7_mode1 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt3_mode2 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt7_mode2 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt4_mode1 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt8_mode1 & 0x03FF)) +
 *						((int)(LinAutoAddressing.Result_LinShunt4_mode2 & 0x03FF) - (int)(LinAutoAddressing.Result_LinShunt8_mode2 & 0x03FF)); */
#if (_LINAA_ASM == FALSE)
	i16Sum =  *pi16Result;														/* LinAutoAddressing.Result_LinShunt1_mode1 */
	pi16Result++;
	i16Sum += *pi16Result;														/* LinAutoAddressing.Result_LinShunt1_mode2 */
	pi16Result++;
	i16Sum += *pi16Result;														/* LinAutoAddressing.Result_LinShunt2_mode1 */
	pi16Result++;
	i16Sum += *pi16Result;														/* LinAutoAddressing.Result_LinShunt2_mode2 */
	pi16Result++;
	i16Sum += *pi16Result;														/* LinAutoAddressing.Result_LinShunt3_mode1 */
	pi16Result++;
	i16Sum += *pi16Result;														/* LinAutoAddressing.Result_LinShunt3_mode2 */
	pi16Result++;
	i16Sum += *pi16Result;														/* LinAutoAddressing.Result_LinShunt4_mode1 */
	pi16Result++;
	i16Sum += *pi16Result;														/* LinAutoAddressing.Result_LinShunt4_mode2 */
	pi16Result++;
	i16Sum -= *pi16Result;														/* LinAutoAddressing.Result_LinShunt5_mode1 */
	pi16Result++;
	i16Sum -= *pi16Result;														/* LinAutoAddressing.Result_LinShunt5_mode2 */
	pi16Result++;
	i16Sum -= *pi16Result;														/* LinAutoAddressing.Result_LinShunt6_mode1 */
	pi16Result++;
	i16Sum -= *pi16Result;														/* LinAutoAddressing.Result_LinShunt6_mode2 */
	pi16Result++;
	i16Sum -= *pi16Result;														/* LinAutoAddressing.Result_LinShunt7_mode1 */
	pi16Result++;
	i16Sum -= *pi16Result;														/* LinAutoAddressing.Result_LinShunt7_mode2 */
	pi16Result++;
	i16Sum -= *pi16Result;														/* LinAutoAddressing.Result_LinShunt8_mode1 */
	pi16Result++;
	i16Sum -= *pi16Result;														/* LinAutoAddressing.Result_LinShunt8_mode2 */
	l_i16AutoAddressingDM = (i16Sum >> 4);
#else  /* (_LINAA_ASM == FALSE) */
	asm("mov a, [x++]");	/* i16Sum =  *(int16 *) &LinAutoAddressing.Result_LinShunt1_mode1 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt2_mode1 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt3_mode1 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt4_mode1 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt5_mode1 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt6_mode1 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt7_mode1 */
	asm("add a, [x++]");	/* i16Sum += *(int16 *) &LinAutoAddressing.Result_LinShunt8_mode1 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt1_mode2 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt2_mode2 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt3_mode2 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt4_mode2 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt5_mode2 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt6_mode2 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt7_mode2 */
	asm("sub a, [x++]");	/* i16Sum -= *(int16 *) &LinAutoAddressing.Result_LinShunt8_mode2 */
	asm("asr a, #2");
	asm("asr a, #2");
	asm("mov _AutoAddressingDM, a");
#endif /* (_LINAA_ASM == FALSE) */
#endif /*  LINAA_NON_CHOPPER_MODE == FALSE */

	{
		int16 i16AA_CM = (int16) l_u16AutoAddressingCM - EE_OCMAA;
#if (LIN_AA_INFO && LIN_AA_SCREENTEST)
		int16 AASDMCM = EE_GDMCMAA;
		AASDMCM -= (int16)g_NvramUser.AASDMCM_delta;
		/* l_i16Ishunt = (int16) (mulI32_I16byI16( l_i16Ishunt, AASDMCM) >> 15); */
		l_i16Ishunt = (int16) (((int32) i16AA_CM * AASDMCM) >> 15);
#else  /* (LIN_AA_INFO && LIN_AA_SCREENTEST) */
	/* l_i16Ishunt = (int16) (mulI32_I16byI16( l_i16Ishunt, EE_GDMCMAA) >> 15); */
		l_i16Ishunt = (int16) (((int32) i16AA_CM * EE_GDMCMAA) >> 15);
#endif /* (LIN_AA_INFO && LIN_AA_SCREENTEST) */
	}

	/* Get CM corrected DM data minus DM offset */
	l_i16Ishunt = (l_i16AutoAddressingDM - l_i16Ishunt) - EE_ODMAA;

	/* Correct DM gain to target gain (scale correction) */
	/* l_i16Ishunt = (int16) (mulI32_I16byI16( l_i16Ishunt, EE_GLAA) >> 12); */
	l_i16Ishunt = (int16) (((int32) l_i16Ishunt * EE_GLAA) >> 12);

	PEND = CLR_ADC_IT;
	MASK &= ~EN_ADC_IT;
} /* End of AutoAddressingReadADCResult() */

/* ****************************************************************************	*
 *  ml_AutoAddressingCfgADC
 *
 * Setup ADC for LIN Bust Shunt current measurement
 * 1. Common-mode voltage measurement (2x)
 * 2. Differential-mode current measurement (8x)
 * ****************************************************************************	*/
void ml_AutoAddressingCfgADC( void)
{
	/* switch off the ADC */
	ADC_CTRL = 0x00;

	/* Enable ADC IRQ */
	PEND = CLR_ADC_IT;
	MASK |= EN_ADC_IT;

	/* Set AdcCtrl to start CM, continuous */
	ADC_CTRL = ADC_TRIG_SRC | ADC_SYNC_SOC;										/* Sync ADC with Timer, skip first pulse */
	/* ADC_CTRL = (ADC_TRIG_SRC | ADC_SYNC_SOC | ADC_START); */

} /* End of ml_AutoAddressingCfgADC() */

/* ****************************************************************************	*
 *  ClearAAData()
 * ****************************************************************************	*/
#if LIN_AA_INFO
void ClearAAData(void)
{
	uint16 i;
	uint16 *pLinAAInfo = (uint16*) LIN_AA_DATA;
	for ( i = (sizeof(LIN_AA_DATA)/sizeof(uint16)); i > 0; i-- )
	{
		*pLinAAInfo = 0;
		pLinAAInfo++;
	}
	l_u8SNPD_CycleCount = 0;
#if USE_MULTI_PURPOSE_BUFFER
	g_MPBuf.u8Type = (uint8) C_MP_BUF_LIN_AA;
	g_MPBuf.u8Usage = (uint8) C_MP_BUF_INUSE;									/* Multi-purpose buffer is in use */
#endif /* USE_MULTI_PURPOSE_BUFFER */

} /* End of ClearAAData() */
#endif /* LIN_AA_INFO */

#else  /* _SUPPORT_LIN_AA */

/* ****************************************************************************	*
 *  LIN API event: mlu_AutoAddressingStep
 * ****************************************************************************	*/
void mlu_AutoAddressingStep( ml_uint8 StepNumber)
{
	(void) StepNumber; 
} /* End of mlu_AutoAddressingStep() */

#endif /* _SUPPORT_LIN_AA */
#endif /* LIN_COMM */

/* EOF */
