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

#if ((LINPROT & LINXX) == LIN2X)
#include "LIN_2x.h"
#include "LIN_Diagnostics.h"
#endif /* ((LINPROT & LINXX) == LIN2X) */

#if ((LINPROT & LINXX) == LIN2J)
#include "LIN_SAE_J2602.h"
#include "LIN_Diagnostics.h"
#endif /* ((LINPROT & LINXX) == LIN2J) */

#include <pltf_version.h>														/* MMP140408-1 */

/* LIN Communication */
#define LIN_BR					19200U
#if (LIN_BR < 12000)
#define LIN_BR_PRESCALER		3
#else /* (LIN_BR < 10000) */
#define LIN_BR_PRESCALER		2
#endif /* (LIN_BR < 10000) */
#define LIN_BR_DIV				((((FPLL/(1 << (1 + LIN_BR_PRESCALER)))*1000U)+(LIN_BR >> 1))/LIN_BR)	/* FPLL is given by command-line as N x 250kHz, eg 80 x 250kHz for 20MHz */
#if (LIN_BR_DIV < 99) || (LIN_BR_DIV > 200)
#error "ERROR: Wrong LinBaudrate pre-scaler; Please adapt pre-scaler value so the LinBaudrate is between 99 and 200."
#endif /* (LIN_BR_DIV < 99) || (LIN_BR_DIV > 200) */
#define C_DEF_DEVICE_ID			0x3F

#define C_LIN_IN_FREE			0x00											/* LIN input frame-buffer is free */
#define C_LIN_IN_FULL			0x01											/* LIN input frame-buffer is full; It's not allowed to be overwritten (could be processed) */
#define C_LIN_IN_POSTPONE		0x02											/* LIN input frame-buffer is full, but allowed to be overwritten by new LIN msg */

typedef union _LININBUF
{
#if ((LINPROT & LINXX) == LIN2X)
	DFR_DIAG Diag;
#if (LINPROT == LIN2X_ACT44)
	ACT_CTRL Ctrl;
#endif /* (LINPROT == LIN2X_ACT44) */
#endif /* ((LINPROT & LINXX) == LIN2X) */

#if ((LINPROT & LINXX) == LIN2J)
	DFR_DIAG Diag;
#if (LINPROT == LIN2J_VALVE_GM)
	ACT_CFR_CTRL cfrCtrl;														/* Control Service: Command-frame */
#endif /* (LINPROT == LIN2J_VALVE_GM) */
#endif /* ((LINPROT & LINXX) == LIN2J) */
} LININBUF, *PLININBUF;

typedef union _LINOUTBUF
{
#if ((LINPROT & LINXX) == LIN2X)
/*	RFR_DIAG DiagResponse; */	/* Not used */
#if (LINPROT == LIN2X_ACT44)
	ACT_STATUS Status;
#endif /* (LINPROT == LIN2X_ACT44) */
#endif /* ((LINPROT & LINXX) == LIN2X) */

#if ((LINPROT & LINXX) == LIN2J)
/*	RFR_DIAG DiagResponse; */	/* Not used */
#if (LINPROT == LIN2J_VALVE_GM)
	ACT_RFR_STA rfrSta;															/* Status Service: Response-frame */
#endif /* (LINPROT == LIN2J_VALVE_GM) */
#endif /* ((LINPROT & LINXX) == LIN2J) */
} LINOUTBUF, *PLINOUTBUF;
#define INVALID 0																/* BufferOut Status "INVALID" */
#define VALID	1																/* BufferOut Status "VALID" */

#if ((LINPROT & LINXX) != LIN2J)
/* flags in AutoAddressingFlags */
#define SLAVEADDRESSED					0x01									/* 0: slave still in the running; 1: slave is addressed */
#define WAITINGFORBREAK					0x02
#define SLAVEFINALSTEP					0x02
#define LINSHUNTCURRENT					0x08
#define SLAVEWAITING					0x04									/* 0: slave in the running for Ishunt3; 1: slave not in the running for this break */
#define LASTSLAVE						0x80								    /* 0: slave is not the  last in line; 1: slave is the last in line. */

#define _LINAA_ASM	TRUE														/* MMP140417-2 */
#if (_LINAA_ASM == FALSE)														/* MMP140417-2 - Begin */
typedef struct _ADC_LINAA
{
	uint16 Result_LinShunt1_CommonMode;											/* LIN Common-mode #1 */
	uint16 Result_LinShunt2_CommonMode;											/* LIN Common-mode #2 */
	uint16 Result_LinShunt1_mode1;												/* LIN Shunt current ADC chopper mode 1, #1 */
	uint16 Result_LinShunt1_mode2;												/* LIN Shunt current ADC chopper mode 2, #1 */
	uint16 Result_LinShunt2_mode1;												/* LIN Shunt current ADC chopper mode 1, #2 */
	uint16 Result_LinShunt2_mode2;												/* LIN Shunt current ADC chopper mode 2, #2 */
	uint16 Result_LinShunt3_mode1;												/* LIN Shunt current ADC chopper mode 1, #3 */
	uint16 Result_LinShunt3_mode2;												/* LIN Shunt current ADC chopper mode 2, #3 */
	uint16 Result_LinShunt4_mode1;												/* LIN Shunt current ADC chopper mode 1, #4 */
	uint16 Result_LinShunt4_mode2;												/* LIN Shunt current ADC chopper mode 2, #4 */
	uint16 Result_LinShunt5_mode1;												/* LIN Shunt current ADC chopper mode 1, #5 */
	uint16 Result_LinShunt5_mode2;												/* LIN Shunt current ADC chopper mode 2, #5 */
	uint16 Result_LinShunt6_mode1;												/* LIN Shunt current ADC chopper mode 1, #6 */
	uint16 Result_LinShunt6_mode2;												/* LIN Shunt current ADC chopper mode 2, #6 */
	uint16 Result_LinShunt7_mode1;												/* LIN Shunt current ADC chopper mode 1, #7 */
	uint16 Result_LinShunt7_mode2;												/* LIN Shunt current ADC chopper mode 2, #7 */
	uint16 Result_LinShunt8_mode1;												/* LIN Shunt current ADC chopper mode 1, #8 */
	uint16 Result_LinShunt8_mode2;												/* LIN Shunt current ADC chopper mode 2, #8 */
} ADC_LINAA, *PADC_LINAA;
#else  /* (_LINAA_ASM == FALSE) */
typedef struct _ADC_LINAA
{
	uint16 Result_LinAA[18];
} ADC_LINAA, *PADC_LINAA;
#endif /* (_LINAA_ASM == FALSE) */												/* MMP140417-2 - End */

/* threshold constants for meeting the protocol */
#define C_LIN13AA_dI_1					12										/* Set it back to ideal threshold (MMP131129-1) */
#define C_LIN13AA_dI_2					12										/* Set it back to ideal threshold */
#define C_LIN2xAA_dI_1_BSM2				12										/* MMP131129-1 */
#define C_LIN2xAA_dI_2_BSM2				12
#define C_LIN2xAA_dI_1_BSM				26										/* MMP140923-1 */
#define C_LIN2xAA_dI_2_BSM				26										/* MMP140923-1 */

/* definition of auto-addressing state names */
typedef enum 
{	
	AUTOADDRESSING_IDLE = 0,													/* Array with pointers to all the functions */
	AUTOADDRESSING_STEP0,														/* that have to be executed in the different auto addressing steps */
	AUTOADDRESSING_STEP1,
	AUTOADDRESSING_STEP2,
	AUTOADDRESSING_STEP3,
	AUTOADDRESSING_STEP4,
	AUTOADDRESSING_STEP5,
	AUTOADDRESSING_STEP6,
	AUTOADDRESSING_WAIT,
	AUTOADDRESSING_DONE 
} T_AUTOADDRESSING_STEP;
#endif /* ((LINPROT & LINXX) != LIN2J) */

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
extern void LIN_Init( uint16 u16WarmStart);										/* LIN communication initialisation */
extern void ml_SetSlaveNotAddressed( void);
extern void ml_SetSlaveAddressed( void);										/* MMP140414-1 */
extern ml_Status ml_GetAutoaddressingStatus( void);
extern void ClearAAData( void);
extern void AutoAddressingReadADCResult( void);									/* LIN Auto Addressing */
extern void HandleLinInMsg( void);												/* Handle LIN input frame buffer */
#if (LINAA_BSM_SNPD_R1p0 != FALSE)												/* MMP140417-2 - Begin */
extern void ml_InitAutoAddressing( void);
extern void ml_StopAutoAddressing( void);
#endif /* (LINAA_BSM_SNPD_R1p0 != FALSE) */										/* MMP140417-2 - End */

#pragma space dp
extern volatile uint8 g_u8AutoAddressingFlags;
extern uint8 g_u8BufferOutID;													/* LIN output buffer is invalid */
extern LININBUF g_LinCmdFrameBuffer;											/* (Copy of) LIN input frame-buffer */
#if ((LINPROT & LINX) == LIN2)
extern RFR_DIAG g_DiagResponse;
#endif /* ((LINPROT & LINX) == LIN2) */
#pragma space none

#pragma space nodp
extern uint8 g_u8LinInFrameBufState;											/* LIN input frame-buffer status */
#if ((LINPROT & LINXX) != LIN2J) || (LINPROT == LIN2J_VALVE_GM)
extern volatile uint8 g_u8ErrorCommunication;									/* Communication error */
#endif /* ((LINPROT & LINXX) != LIN2J) || (LINPROT == LIN2J_VALVE_GM) */
extern volatile uint8 g_u8ErrorCommBusTimeout;									/* Flag indicate of LIN bus time-out occurred */
#if LIN_AA_INFO
extern uint8 l_u8SNPD_CycleCount;												/* SNPD cycle count */
#endif /* LIN_AA_INFO */

#if (__MLX_PLTF_VERSION_MAJOR__ == 3)
/* MLX4 Data-area */															/* MMP130810-1 */
extern volatile uint8 u8ActualBaudRateDiv __attribute((nodp, addr(0x07B1)));	/* MLX4-Addr: 0x01 (Complete byte) */
extern volatile uint8 u8NominalBaudRateDiv __attribute((nodp, addr(0x07B2)));	/* MLX4-Addr: 0x02 (Complete byte) */
/* extern volatile uint8 u8BaudRatePreScaler __attribute((nodp, addr(0x07DF))); */	/* MLX4-Addr: 0x5F (Upper-nibble) */
#if ((__MLX_PLTF_VERSION_MINOR__ == 0) && (__MLX_PLTF_VERSION_REVISION__ == 15))
extern volatile uint8 u8BaudRatePreScaler __attribute((nodp, addr(0x07D7))); 	/* MLX4-Addr: 0x4F (Upper-nibble) (MMP131022-1: Clean-up MLX4 platform) */
#endif /* ((__MLX_PLTF_VERSION_MINOR__ == 0) && (__MLX_PLTF_VERSION_REVISION__ == 15)) */
#if (__MLX_PLTF_VERSION_MINOR__ >= 1)
extern volatile uint8 u8BaudRatePreScaler __attribute((nodp, addr(0x07B3)));	/* MLX4-Addr: 0x03 (Upper-nibble) (MMP140408-1: New MLX4 platform 3.1.0) */
#endif /* (__MLX_PLTF_VERSION_MINOR__ >= 1) */
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 3) */

#pragma space none

#endif /* LIN_COMMUNICATION_H_ */

/* EOF */
