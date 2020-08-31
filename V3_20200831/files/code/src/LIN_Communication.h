/*! \file		LIN_Communication.h
 *  \brief		MLX81300 LIN communication handling
 *
 * \note		project MLX81300
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
 * Copyright (C) 2008-2010 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * ****************************************************************************	*/

#ifndef LIN_COMMUNICATION_H_
#define LIN_COMMUNICATION_H_

#include <pltf_version.h>														/* MMP140408-1 */

#include "LIN_Diagnostics.h"
#include "LIN_Protocol.h"


/* LIN Communication */
#if (LIN_BR < 12000)
#define LIN_BR_PRESCALER		3
#else /* (LIN_BR < 10000) */
#define LIN_BR_PRESCALER		2
#endif /* (LIN_BR < 10000) */
#define LIN_BR_DIV				((((FPLL/(1 << (1 + LIN_BR_PRESCALER)))*1000UL)+(LIN_BR >> 1))/LIN_BR)	/* FPLL is given by command-line as N x 250kHz, eg 80 x 250kHz for 20MHz */
#if (LIN_BR_DIV < 99) || (LIN_BR_DIV > 200)
#error "ERROR: Wrong LinBaudrate pre-scaler; Please adapt pre-scaler value so the LinBaudrate is between 99 and 200."
#endif /* (LIN_BR_DIV < 99) || (LIN_BR_DIV > 200) */
#define C_DEF_DEVICE_ID			0x3F

/* LIN input frame buffer state */
#define C_LIN_IN_FREE			0x00											/* LIN input frame-buffer is free */
#define C_LIN_IN_FULL			0x01											/* LIN input frame-buffer is full; It's not allowed to be overwritten (could be processed) */
#define C_LIN_IN_POSTPONE		0x02											/* LIN input frame-buffer is full, but allowed to be overwritten by new LIN msg */
/* LIN output buffer status */
#define INVALID 0																/* BufferOut Status "INVALID" */
#define VALID	1																/* BufferOut Status "VALID" */

/* bus management:sleep request */
#define C_ML_REASON_NO          0x00u
#define C_ML_REASON_CMD         0x01u
#define C_ML_REASON_TIMEOUT     0x02u
#define C_ML_REASON_WAKEUP		0x04u

/* public type defines */
typedef union _LININBUF
{
	DFR_DIAG Diag;
	uint8    cfrBytes[8];
} LININBUF, *PLININBUF;

typedef union _LINOUTBUF
{
	RFR_DIAG DiagResponse; 	/* Not used */
	uint8    rfrBytes[8];
} LINOUTBUF, *PLINOUTBUF;

typedef struct
{
	uint8 NAD;
	uint8 ControlFrameID;
	uint8 StatusFrameID;
	uint8 Variant;
}LIN_GenericDataType;


/* public variables:only used in LIN diagnostic.c */
#pragma space dp
extern uint8 g_u8NAD;
extern uint8 g_u8ControlFrameID;
extern uint8 g_u8StatusFrameID;
extern uint8 g_u8BufferOutID;													/* LIN output buffer is invalid */
extern LININBUF g_LinCmdFrameBuffer;											/* (Copy of) LIN input frame-buffer */
extern RFR_DIAG g_DiagResponse;                                                 /* diagnostic response buffer */

#pragma space none


#pragma space nodp

#pragma space none


/* ****************************************************************************	*
 *	public functions                               												*
 * ****************************************************************************	*/
extern void LIN_Init( void);													/* LIN communication initialisation */
extern void LIN_MainFunction(void);

/* application implement function */
extern void HandleActCfrCtrl(const ACT_CFR_CTRL *pCfrCtrl);
extern void HandleActRfrSta(ACT_RFR_STA *pRfrSta); 
extern void HandleNMReq(uint8 reason);


#endif /* LIN_COMMUNICATION_H_ */

/* EOF */
