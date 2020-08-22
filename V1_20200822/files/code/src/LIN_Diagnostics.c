/*! ----------------------------------------------------------------------------
 * \file		LIN_Diagnostics.c
 * \brief		MLX81300 LIN Diagnostics communication handling
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	SetupDiagResponse()
 *				CheckSupplier()
 *				ValidSupplierFunctionID()
 *				HandleDfrDiag()
 *
 * MELEXIS Microelectronic Integrated Systems
 * 
 * Copyright (C) 2012-2013 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * ****************************************************************************	*/

#include "Build.h"

#if ((LINPROT & LINXX) == LIN2X) || ((LINPROT & LINXX) == LIN2J)

#include "LIN_Communication.h"
#include "lin.h"
#if ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 3)) || (__MLX_PLTF_VERSION_MAJOR__ >= 4)
#include "lin_internal.h"														/* LinFrame */
#endif /* ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 3)) || (__MLX_PLTF_VERSION_MAJOR__ >= 4) */
#include "main.h"
#include "ADC.h"																/* ADC support */
#include "app_version.h"
#include "ErrorCodes.h"															/* Error-logging support */
#if _SUPPORT_MLX_DEBUG_MODE
#include "MotorStall.h"															/* Only for debugging purpose */
#include "PID_Control.h"														/* Only for debugging purpose */
#endif /* _SUPPORT_MLX_DEBUG_MODE */
#include "NVRAM_UserPage.h"														/* NVRAM Functions & Layout */
#include "Timer.h"
#include "private_mathlib.h"
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION (@TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	(@NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp																/* __NEAR_SECTION__ */
uint16 g_u16DiagResponseTimeoutCount = 0U;
#if _SUPPORT_LIN_AA && (LIN_AA_INFO != FALSE)
uint8 l_u8SNPD_CycleCountComm = 0U;												/* Communication Cycle counter LIN-AA info */
#endif /* _SUPPORT_LIN_AA && (LIN_AA_INFO != FALSE) */
#pragma space none																/* __NEAR_SECTION__ */

#if _SUPPORT_MLX_DEBUG_MODE
/*								 ANA_OUTA,ANA_OUTB,ANA_OUTC,ANA_OUTD,ANA_OUTE,ANA_OUTF,ANA_OUTG,ANA_OUTH */
const uint16 au16AnaOutRegs[] = { 0x201CU, 0x201EU, 0x2020U, 0x204AU, 0x204CU, 0x204EU, 0x28CCU, 0x28CEU};

const uint16 tMlxDbgSupport[] = {
	0xFFFEU, 0x0007U, 0x0000U, 0x0000U,
	0x0000U, 0x0000U, 0x0000U, 0x0000U,
	0x0000U, 0x0000U, C_DBG_SUBFUNC_SUPPORT_A, C_DBG_SUBFUNC_SUPPORT_B,
	C_DBG_SUBFUNC_SUPPORT_C, C_DBG_SUBFUNC_SUPPORT_D, C_DBG_SUBFUNC_SUPPORT_E, C_DBG_SUBFUNC_SUPPORT_F
};
#endif /* _SUPPORT_MLX_DEBUG_MODE */

void SetupDiagResponse( uint8 u8NAD, uint8 u8SID, uint8 u8ResponseCode);
uint16 CheckSupplier( uint16 const u16SupplierID);
uint16 ValidSupplierFunctionID( uint16 const u16SupplierID, uint16 const u16FunctionID );
#if _SUPPORT_MLX_DEBUG_MODE
extern uint16 GetRawChipSupply( void);
extern uint16 GetRawTemperature( void);
#endif /* _SUPPORT_MLX_DEBUG_MODE */

#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
extern uint8 l_u8GAD;
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */


/* ****************************************************************************	*
 * SetupDiagResponse
 * ****************************************************************************	*/
void SetupDiagResponse( uint8 u8NAD, uint8 u8SID, uint8 u8ResponseCode)
{
	g_DiagResponse.byNAD = u8NAD;
	if ( u8ResponseCode == (uint8) C_ERRCODE_POSITIVE_RESPONSE )
	{
		/* Positive feedback
		 *	+-------+-----+------+----------+----------+----------+----------+----------+
		 *	|  NAD  | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
		 *	+-------+-----+------+----------+----------+----------+----------+----------+
		 *	|Initial| 0x01|  SID | Reserved | Reserved | Reserved | Reserved | Reserved |
		 *	|  NAD  |     | |0x40|   0xFF   |   0xFF   |   0xFF   |   0xFF   |   0xFF   |
		 *	+-------+-----+------+----------+----------+----------+----------+----------+
		 */
		g_DiagResponse.byPCI = (uint8) C_RPCI_REASSIGN_NAD;
		g_DiagResponse.byRSID = (uint8) (u8SID | C_RSID_OK);
		g_DiagResponse.byD1 = (uint8) C_DIAG_RES;
		g_DiagResponse.byD2 = (uint8) C_DIAG_RES;
	}
	else
	{
		/* Pending or Negative feedback
		 *	+-------+-----+------+----------+----------+----------+----------+----------+
		 *	|  NAD  | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
		 *	+-------+-----+------+----------+----------+----------+----------+----------+
		 *	|Initial| 0x03| 0x7F | Requested| Response | Reserved | Reserved | Reserved |
		 *	|  NAD  |     |      |    SID   |   Code   |   0xFF   |   0xFF   |   0xFF   |
		 *	+-------+-----+------+----------+----------+----------+----------+----------+
		 */
		g_DiagResponse.byPCI = (uint8) C_RPCI_NOK;
		g_DiagResponse.byRSID = (uint8) C_RSID_NOK;
		g_DiagResponse.byD1 = u8SID;
		g_DiagResponse.byD2 = u8ResponseCode;
	}

	g_u8BufferOutID = (uint8) QR_RFR_DIAG;										/* LIN Output buffer is valid (RFR_DIAG) */
} /* End of SetupDiagResponse() */

/* ****************************************************************************	*
 * CheckSupplier
 * ****************************************************************************	*/
uint16 CheckSupplier( uint16 const u16SupplierID)
{
	uint16 u16Result = FALSE;
	if ( (u16SupplierID == (uint16) C_WILDCARD_SUPPLIER_ID)
		|| (u16SupplierID == (uint16)C_SUPPLIER_ID) )
	{
		u16Result = TRUE;
	}
	return ( u16Result );
} /* End of CheckSupplier() */

/* ****************************************************************************	*
 * ValidSupplierFunctionID
 *
 * Pre:		Pointer to address of supplier and function ID
 * Post:	FALSE: Incorrect supplier and/or function ID
 *			TRUE: Correct supplier and function ID
 * ****************************************************************************	*/
uint16 ValidSupplierFunctionID( uint16 const u16SupplierID, uint16 const u16FunctionID )
{
	uint16 u16Result = FALSE;
	if ( ((u16SupplierID == C_SUPPLIER_ID)
		|| (u16SupplierID == C_WILDCARD_SUPPLIER_ID)) &&
		((u16FunctionID == C_FUNCTION_ID)
		|| (u16FunctionID == C_WILDCARD_FUNCTION_ID)) )
	{
		u16Result = TRUE;
	}
	return ( u16Result );
} /* End of ValidSupplierFunctionID() */

static __inline__ void StoreD1to2( uint16 a)
{
	__asm__ __volatile__
	(
		"mov dp:_g_DiagResponse+3, AL\n\t"
		"mov dp:_g_DiagResponse+4, AH"
		:
		: "a" (a)
	);
	g_u8BufferOutID = (uint8) QR_RFR_DIAG;									/* LIN Output buffer is valid (RFR_DIAG) */
	return;
} /* End of StoreD1to2() */

static __inline__ void StoreD1to4( uint16 a, uint16 b)
{
	__asm__ __volatile__
	(
		"mov dp:_g_DiagResponse+3, AL\n\t"
		"mov dp:_g_DiagResponse+4, AH\n\t"
		"mov dp:_g_DiagResponse+5, YL\n\t"
		"mov dp:_g_DiagResponse+6, YH"
		:
		: "b" (a), "y" (b)
	);
	g_u8BufferOutID = (uint8) QR_RFR_DIAG;									/* LIN Output buffer is valid (RFR_DIAG) */
	return;
} /* End of StoreD1to4() */

static __inline__ void StoreD2to5( uint16 a, uint16 b)
{
	__asm__ __volatile__
	(
		"mov dp:_g_DiagResponse+4, AL\n\t"
		"mov dp:_g_DiagResponse+5, AH\n\t"
		"mov dp:_g_DiagResponse+6, YL\n\t"
		"mov dp:_g_DiagResponse+7, YH"
		:
		: "b" (a), "y" (b)
	);
	g_u8BufferOutID = (uint8) QR_RFR_DIAG;									/* LIN Output buffer is valid (RFR_DIAG) */
	return;
} /* End of StoreD2to5() */

/* ****************************************************************************	*
 * Diagnostic
 * ****************************************************************************	*/
void HandleDfrDiag( void)
{
	DFR_DIAG *pDiag = &g_LinCmdFrameBuffer.Diag;

#if ((LINPROT & LINXX) == LIN2X)
	g_u16DiagResponseTimeoutCount = PI_TICKS_PER_SECOND;						/* Set LIN Diagnostics Response time-out to 1 sec */

#if ((((LINPROT & LINXX) >= LIN21) && ((LINPROT & LINXX) <= LIN22)) || ((LINPROT & LINXX) == LIN2X))			/* LIN 2.1, LIN 2.2 */
	if ( pDiag->byNAD != 0x7E )
	{
		g_u8BufferOutID = (uint8) QR_INVALID;
	}
#endif /* ((((LINPROT & LINXX) >= LIN21) && ((LINPROT & LINXX) <= LIN22)) || ((LINPROT & LINXX) == LIN2X)) */	/* LIN 2.1, LIN 2.2 */

	if ( pDiag->byNAD == 0x00U )	/* Other bytes should be 0xFF, and are ignored */
	{
		/* ACT_DFR_DIAG_SLEEP: Sleep request (Optional) */
		g_e8MotorRequest = (uint8) C_MOTOR_REQUEST_SLEEP;
	}
	else if ( (pDiag->byNAD == g_u8NAD) || (pDiag->byNAD == (uint8) C_BROADCAST_NAD) )
#endif /* ((LINPROT & LINXX) == LIN2X) */
#if ((LINPROT & LINXX) == LIN2J)
	/* Targeted or Broadcast */
	if ( (pDiag->byNAD == g_u8NAD) || (pDiag->byNAD == C_BROADCAST_J2602_NAD) )
#endif /* ((LINPROT & LINXX) == LIN2J) */
	{
		uint16 u16DiagPCI_SID = (((uint16)pDiag->byPCI) << 8) | ((uint16)pDiag->bySID);

		g_DiagResponse.byD1 = (uint8) C_DIAG_RES;
		g_DiagResponse.byD2 = (uint8) C_DIAG_RES;
		g_DiagResponse.byD3 = (uint8) C_DIAG_RES;
		g_DiagResponse.byD4 = (uint8) C_DIAG_RES;
		g_DiagResponse.byD5 = (uint8) C_DIAG_RES;

#if (LINPROT == LIN2X_ACT44)
		if ( (u16DiagPCI_SID == C_PCI_SID_STOP_ACTUATOR) && (pDiag->byD5 == 0xFE) )
		{
			/* This is a broadcast LIN-command; Therefore no feedback is returned */
			/*
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | SID |    D1    |    D2    |    D3    |    D4    |    D5    |
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xB5| Supplier | Supplier | Function | Function |  "Stop"  |
			 *	|     |     |     | ID (LSB) | ID (MSB) | ID (LSB) | ID (MSB) |   0xFE   |
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 */
			if ( ValidSupplierFunctionID( (pDiag->byD1) | ((uint16)(pDiag->byD2) << 8), (pDiag->byD3) | ((uint16)(pDiag->byD4) << 8)) )
			{
				MotorDriverStop( (uint16) C_STOP_EMERGENCY);					/* Stop actuator NOW (LIN-AA) */
				g_u8ChipResetOcc = FALSE;										/* Clear all event flags too (Reset, ... */
				g_u8StallOcc = FALSE;											/* ... Stall detected, and ... */
				g_u8StallTypeComm &= ~M_STALL_MODE;
				g_u8EmergencyRunOcc = FALSE;									/* ... Emergency Run occurred  */
			}
		}
#if 1
		else if (  (g_e8MotorCtrlMode != (uint8) C_MOTOR_CTRL_STOP) &&
				  ((pDiag->bySID == (uint8) C_SID_REASSIGN_NAD) || (pDiag->bySID == (uint8) C_SID_ASSIGN_FRAME_ID) ||
				   (pDiag->bySID == (uint8) C_SID_DATA_DUMP) || (pDiag->bySID == (uint8) C_SID_ASSIGN_FRAME_ID_RNG)) )
		{
			/* Do not allow any "Write" except in STOP-mode. A new module has no Frame-ID, and can therefore not accept a control-frame to reset the events */
#else
		else if ( ((g_e8MotorCtrlMode != (uint8) C_MOTOR_CTRL_STOP) || ((g_u8ChipResetOcc | g_u8StallOcc | g_u8EmergencyRunOcc) != 0)) &&
				  ((pDiag->bySID == (uint8) C_SID_REASSIGN_NAD) || (pDiag->bySID == (uint8) C_SID_ASSIGN_FRAME_ID) ||
				   (pDiag->bySID == (uint8) C_SID_DATA_DUMP) || (pDiag->bySID == (uint8) C_SID_ASSIGN_FRAME_ID_RNG)) )
		{
			/* Do not allow any "Write" except in STOP-mode, without Event-mode */
#endif
			SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_COND_SEQ);	/* Status = Negative feedback */
			SetLastError( (uint8) C_ERR_LIN2X_WRITE);
		}
		else if ( (u16DiagPCI_SID == C_PCI_SID_REASSIGN_NAD) && (pDiag->byNAD != (uint8) C_DEFAULT_NAD) )
#else  /* (LINPROT == LIN2X_ACT44) */
		if ( u16DiagPCI_SID == C_PCI_SID_REASSIGN_NAD )
#endif /* (LINPROT == LIN2X_ACT44) */
		{
			/* Re-assign NAD (Optional) */
			/* Assign NAD is used to resolve conflicting NADs in LIN clusters
			 * built using off-the-shelves slave nodes or reused slave nodes.
			 * This request uses the initial NAD (or the NAD wild-card); this
			 * is to avoid the risk of losing the address of a slave node.
			 * The NAD used for the response shall be the same as in the
			 * request, i.e. the initial NAD.
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xB0 | Supplier | Supplier | Function | Function |  New NAD |
			 *	|     |     |      | ID (LSB) | ID (MSB) | ID (LSB) | ID (MSB) |          |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			if ( ValidSupplierFunctionID( (pDiag->byD1) | ((uint16)(pDiag->byD2) << 8), (pDiag->byD3) | ((uint16)(pDiag->byD4) << 8)) )
			{
				uint8 byInitialNAD = g_NvramUser.NAD;
#if ((LINPROT & LINXX) == LIN2J)
				if ( (pDiag->byD5 & (C_STEP_J2602_NAD - 1U)) != 0U ) /*lint !e587 */
				{
					SetupDiagResponse( byInitialNAD, pDiag->bySID, (uint8) C_ERRCODE_INV_MSG_INV_SZ);		/* Status = Negative feedback */
				}
				else
#endif /* ((LINPROT & LINXX) == LIN2J) */
				{
					SetupDiagResponse( byInitialNAD, pDiag->bySID, (uint8) C_ERRCODE_PENDING);				/* Status = Pending */
					g_NvramUser.NAD = pDiag->byD5;
					g_u8NAD = g_NvramUser.NAD;
					/* Store NVRAM */
					if ( (NVRAM_Store( (uint16) C_NVRAM_USER_PAGE_ALL) == C_NVRAM_STORE_OKAY) && (g_NvramUser.NAD == pDiag->byD5) )
					{
						/* NAD changed */
						SetupDiagResponse( byInitialNAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE);/* Status = Positive feedback */
					}
					else
					{
						/* NAD couldn't be changed */
						SetupDiagResponse( byInitialNAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);		/* Status = Negative feedback */
						SetLastError( (uint8) C_ERR_LIN2X_B0);
					}
				}
			}
		}
#if ((LINPROT & LINXX) == LIN2X)
		else if ( u16DiagPCI_SID == C_PCI_SID_ASSIGN_FRAME_ID )
		{
			/* Assign (16-bit) Message-ID to (8-bit) Frame-ID */
			/*
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | SID |    D1    |    D2    |    D3    |    D4    |    D5    |
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xB1| Supplier | Supplier |  Message |  Message |   Frame  |
			 *	|     |     |     | ID (LSB) | ID (MSB) | ID (LSB) | ID (MSB) |    ID    |
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 * Only LIN 2.0 (Obsolete in LIN 2.1)
			 */
			if ( CheckSupplier( (pDiag->byD1) | ((uint16)(pDiag->byD2) << 8)) )
			{
				uint16 wMessageID = (((uint16) pDiag->byD4) << 8) | ((uint16) pDiag->byD3);
				if ( wMessageID == MSG_CONTROL )
				{
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_PENDING);				/* Status = Pending */
					g_NvramUser.ControlFrameID = pDiag->byD5;
					(void) ml_Disconnect();
					(void) ml_AssignFrameToMessageID( MSG_CONTROL, g_NvramUser.ControlFrameID);
					(void) ml_Connect();
					/* Store NVRAM */
					if ( (NVRAM_Store( C_NVRAM_USER_PAGE_ALL) == C_NVRAM_STORE_OKAY) && (g_NvramUser.ControlFrameID == pDiag->byD5) )
					{
						/* Control Frame-ID changed */
						SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE);	/* Status = Positive feedback */
					}
					else
					{
						/* Control Frame-ID couldn't be changed */
						SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);		/* Status = Negative feedback */
						SetLastError( (uint8) C_ERR_LIN2X_B1);
					}
				}
				else if ( wMessageID == MSG_STATUS )
				{
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_PENDING);				/* Status = Pending */
					g_NvramUser.StatusFrameID = pDiag->byD5;
					(void) ml_Disconnect();
					(void) ml_AssignFrameToMessageID( MSG_STATUS, g_NvramUser.StatusFrameID);
					(void) ml_Connect();
					/* Store NVRAM */
					if ( (NVRAM_Store( C_NVRAM_USER_PAGE_ALL) == C_NVRAM_STORE_OKAY) && (g_NvramUser.StatusFrameID == pDiag->byD5) )
					{
						/* Status Frame-ID changed */
						SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE);	/* Status = Positive feedback */
					}
					else
					{
						/* Status Frame-ID couldn't be changed */
						SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);		/* Status = Negative feedback */
						SetLastError( (uint8) C_ERR_LIN2X_B1);
					}
				}
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
				else if ( wMessageID == MSG_GROUP_CONTROL )
				{
					SetupDiagResponse( g_u8NAD, pDiag->bySID, C_ERRCODE_PENDING);				/* Status = Pending */
					g_NvramUser.GroupControlFrameID = pDiag->byD5;
					(void) ml_Disconnect();
					(void) ml_AssignFrameToMessageID( MSG_GROUP_CONTROL, g_NvramUser.GroupControlFrameID);
					(void) ml_Connect();
					/* Store NVRAM */
					if ( (NVRAM_Store( C_NVRAM_USER_PAGE_ALL) == C_NVRAM_STORE_OKAY) && (g_NvramUser.GroupControlFrameID == pDiag->byD5) )
					{
						/* Control Frame-ID changed */
						SetupDiagResponse( g_u8NAD, pDiag->bySID, C_ERRCODE_POSITIVE_RESPONSE);	/* Status = Positive feedback */
					}
					else
					{
						/* Control Frame-ID couldn't be changed */
						SetupDiagResponse( g_u8NAD, pDiag->bySID, C_ERRCODE_SFUNC_NOSUP);		/* Status = Negative feedback */
						SetLastError( (uint8) C_ERR_LIN2X_B1);
					}
				}
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
				else
				{
					/* Wrong Message-ID */
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);			/* Status = Negative feedback */
					SetLastError( (uint8) C_ERR_LIN2X_B1);
				}
			}
		}
#endif /* ((LINPROT & LINXX) == LIN2X) */
		else if ( u16DiagPCI_SID == C_PCI_SID_READ_BY_ID )
		{
			/* Read-by-Identifier (Mandatory) */
			/*
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | SID |    D1    |    D2    |    D3    |    D4    |    D5    |
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xB2|  ReadID  | Supplier | Supplier | Function | Function |
			 *	|     |     |     |          | ID (LSB) | ID (MSB) | ID (LSB) | ID (MSB) |
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 * It is possible to read the supplier identity and other properties from a slave node using this request
			 * ReadID	Description
			 * 0x00 (M)	LIN Product identification
			 * 0x01 (O) Serial number (32-bits)
			 * (0x02-0x1F: Reserved)
			 * (0x20-0x3F: User defined)
			 * 0x30 (U) Firmware version
			 * (0x40-0xFF: Reserved)
			 * (M) = Mandatory
			 * (O) = Optional
			 * (U) = User defined
			 */
			if ( ValidSupplierFunctionID( (pDiag->byD2) | ((uint16)(pDiag->byD3) << 8), (pDiag->byD4) | ((uint16)(pDiag->byD5) << 8)) )
			{
				if ( pDiag->byD1 == (uint8) C_LIN_PROD_ID )
				{
					/* LIN Product identification
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xF2 | Supplier | Supplier | Function | Function |  Variant |
					 *	|     |     |      | ID (LSB) | ID (MSB) | ID (LSB) | ID (MSB) |    ID    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					g_DiagResponse.byNAD = g_u8NAD;
					g_DiagResponse.byPCI = (uint8) C_RPCI_READ_BY_ID_00;
					g_DiagResponse.byRSID = (uint8) C_RSID_READ_BY_ID;
					g_DiagResponse.byD5 = (uint8) C_VARIANT;	/* g_NvramUser.Variant; (MMP160613-3) */
					StoreD1to4( C_SUPPLIER_ID, C_FUNCTION_ID);					/* Supplier & Function-ID */
				}
				else if ( pDiag->byD1 == C_SERIAL_NR_ID )
				{
					/* (Optional) Serial number
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x05| 0xF2 | SerialNr | SerialNr | SerialNr | SerialNr | Reserved |
					 *	|     |     |      |   (LSB)  |		     |          |   (MSB)  |          |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					g_DiagResponse.byNAD = g_u8NAD;
					g_DiagResponse.byPCI = (uint8) C_RPCI_READ_BY_ID_01;
					g_DiagResponse.byRSID = (uint8) C_RSID_READ_BY_ID;
					StoreD1to4( g_NvramUser.SerialNumberLSW, g_NvramUser.SerialNumberMSW);	/* Serial-number */
				}
#if (LINPROT == LIN2J_VALVE_VW)
				else if ( pDiag->byD1 == (uint8) C_SVN_ID )
				{
					g_DiagResponse.byNAD = g_u8NAD;
					g_DiagResponse.byPCI = (uint8) C_RPCI_READ_BY_ID_30;
					g_DiagResponse.byRSID = (uint8) C_RSID_READ_BY_ID;
					StoreD1to4( C_SVN, 0xFFFFU);								/* Firmware SVN */
				}
#endif /* (LINPROT == LIN2J_VALVE_VW) */
#if (LINPROT == LIN2X_ACT44)
				else if ( pDiag->byD1 == (uint8) C_VERIFY_NAD )
				{
					/* Last NAD, Frame-ID for Control message & Status-message
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x04| 0xF2 |    NAD   |  Control |  Status  | Reserved | Reserved |
					 *	|     |     |      |   NVRAM  | Frame ID | Frame ID |		   |          |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					g_DiagResponse.byNAD = g_u8NAD;
					g_DiagResponse.byPCI = (uint8) C_RPCI_READ_BY_ID_21;
					g_DiagResponse.byRSID = (uint8) C_RSID_READ_BY_ID;
					g_DiagResponse.byD1 = (uint8) g_NvramUser.NAD;				/* Stored NAD (NVRAM) */
					g_DiagResponse.byD2 = (uint8) g_NvramUser.ControlFrameID;	/* Frame-ID for Control-message */
					g_DiagResponse.byD3 = (uint8) g_NvramUser.StatusFrameID;	/* Frame-ID for Status-message */
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
					g_DiagResponse.byD4 = g_NvramUser.GroupControlFrameID;		/* Frame-ID for Group Control-message */
					g_DiagResponse.byD5 = g_NvramUser.GAD;						/* Group-address */
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
					g_u8BufferOutID = (uint8) QR_RFR_DIAG;						/* LIN Output buffer is valid (RFR_DIAG) */
				}
				else if ( pDiag->byD1 == (uint8) C_SW_HW_REF )
				{
					/* SW & HW reference
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x03| 0xF2 | Software | Hardware | Reserved | Reserved | Reserved |
					 *	|     |     |      |  Ref. ID |  Ref. ID |          |	       |          |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					g_DiagResponse.byNAD = g_u8NAD;
					g_DiagResponse.byPCI = (uint8) C_RPCI_READ_BY_ID_2A;
					g_DiagResponse.byRSID = (uint8) C_RSID_READ_BY_ID;
					g_DiagResponse.byD1 = (uint8) C_SW_REF;	/* g_NvramUser.SwRef; */	/* SW-reference */
					g_DiagResponse.byD2 = (uint8) g_NvramUser.HwRef;					/* HW-reference */
					g_u8BufferOutID = (uint8) QR_RFR_DIAG;								/* LIN Output buffer is valid (RFR_DIAG) */
				}
				else if ( pDiag->byD1 == (uint8) C_MLX_HW_SW_REF )
				{
					/* MLX-chip HW & SW ID
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x03| 0xF2 |  Chip HW |  Chip SW | Reserved | Reserved | Reserved |
					 *	|     |     |      |    ID    |    ID    |          | 	       |          |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					g_DiagResponse.byNAD = g_u8NAD;
					g_DiagResponse.byPCI = (uint8) C_RPCI_READ_BY_ID_3C;
					g_DiagResponse.byRSID = (uint8) C_RSID_READ_BY_ID;
					StoreD1to2( *((uint16 *) C_ADDR_MLX_HWSWID));				/* Chip HW/SW-ID */
				}
				else if ( pDiag->byD1 == (uint8) C_LIN_CUST_ID )
				{
					/* Customer ID
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x03| 0xF2 | Customer | Customer | Reserved | Reserved | Reserved |
					 *	|     |     |      | ID (LSB) | ID (MSB) |          | 	       |          |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					g_DiagResponse.byNAD = g_u8NAD;
					g_DiagResponse.byPCI = (uint8) C_RPCI_READ_BY_ID_3D;
					g_DiagResponse.byRSID = (uint8) C_RSID_READ_BY_ID;
					StoreD1to2( g_NvramUser.CustomerID);						/* Customer-ID */
				}
				else if ( pDiag->byD1 == (uint8) C_PROD_DATE )
				{
					/* Production Date
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x03| 0xF2 |Production|Production| Reserved | Reserved | Reserved |
					 *	|     |     |      |Date (LSB)|Date (MSB)|          | 	       |          |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					g_DiagResponse.byNAD = g_u8NAD;
					g_DiagResponse.byPCI = (uint8) C_RPCI_READ_BY_ID_3E;
					g_DiagResponse.byRSID = (uint8) C_RSID_READ_BY_ID;
					StoreD1to2( g_NvramUser.ProductionDate);					/* Production Date */
				}
#endif /* (LINPROT == LIN2X_ACT44) */
				else
				{
					/* Identifier not supported */
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);	/* Status = Negative feedback */
					SetLastError( (uint8) C_ERR_LIN2X_B2);
				}
			}
			else
			{
				/* Invalid vendor/Function ID (MMP170329-1) */
				SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);	/* Status = Negative feedback */
				SetLastError( (uint8) C_ERR_LIN2X_B2);
			}
		}
#if ((LINPROT & LINXX) == LIN2X)
		else if ( u16DiagPCI_SID == C_PCI_SID_CC_NAD )
		{
			/* (Optional) Conditional change NAD */
			/*
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | SID |    D1    |    D2    |    D3    |    D4    |    D5    |
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xB3|Identifier|   Byte   |   Mask   |  Invert  | New NAD  |
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 */
			/* Get the identifier of possible read by ID response and selected by Id */
			/* Extract the data byte selected by Byte */
			uint8 u8DataByte = 0x00U;
			uint8 u8Error = (uint8) C_ERR_NONE;
			if ( pDiag->byD1 == 0x00U ) /* Requested Id = LIN Product Identification */
			{
				if ( pDiag->byD2 == 1U )
				{
					u8DataByte = (uint8) (C_SUPPLIER_ID & 0xFFU);				/* LSB of Supplier-ID */
				}
				else if ( pDiag->byD2 == 2U )
				{
					u8DataByte = (uint8) (C_SUPPLIER_ID >> 8);					/* MSB of Supplier-ID */
				}
				else if ( pDiag->byD2 == 3U )
				{
					u8DataByte = (uint8) (C_FUNCTION_ID & 0xFFU);				/* LSB of Function-ID */
				}
				else if ( pDiag->byD2 == 4U )
				{
					u8DataByte = (uint8) (C_FUNCTION_ID >> 8); /*lint !e572 */	/* MSB of Function-ID */
				}
				else if ( pDiag->byD2 == 5U )
				{
					u8DataByte = g_NvramUser.Variant;
				}
				else
				{
					u8Error = (uint8) C_ERRCODE_INV_MSG_INV_SZ;					/* Selected byte not in range, not valid => no response */
				}
			}
			else if ( pDiag->byD1 == 0x01U ) /* Requested Id = Serial number (optional) */
			{
				if ( (pDiag->byD2 == 0U) || (pDiag->byD2 > 4U) )
				{
					/* Selected byte not in range, not valid => no response */
					u8Error = (uint8) C_ERRCODE_INV_MSG_INV_SZ;					/* Status = Invalid Format */
				}
				else
				{
					uint8 *pu8Nvram = (uint8 *) &g_NvramUser.SerialNumberLSW;
					u8DataByte = pu8Nvram[pDiag->byD2 - 1U];						/* Serial-number[n] */
				}
			}
			else
			{
				/* Identifier not supported */
				u8Error = (uint8) C_ERRCODE_SFUNC_NOSUP;						/* Status = Negative feedback */
			}

			if ( u8Error != (uint8) C_ERR_NONE )
			{
				SetupDiagResponse( g_u8NAD, pDiag->bySID, u8Error);
				SetLastError( (uint8) C_ERR_LIN2X_B3);
			}
			else
			{
				/* Do a bitwise XOR with Invert */
				u8DataByte ^= pDiag->byD4;

				/* Do a bitwise AND with Mask */
				u8DataByte &= pDiag->byD3;

				if ( u8DataByte == 0U )											/* Condition PASSED */
				{
					uint8 byInitialNAD = g_NvramUser.NAD;
					SetupDiagResponse( byInitialNAD, pDiag->bySID, (uint8) C_ERRCODE_PENDING);	/* Status = Pending */
					g_NvramUser.NAD = pDiag->byD5;								/* New NAD */
					g_u8NAD = g_NvramUser.NAD;
					/* Store NVRAM */
					if ( (NVRAM_Store( C_NVRAM_USER_PAGE_ALL) == C_NVRAM_STORE_OKAY) && (g_NvramUser.NAD == pDiag->byD5) )
					{
						/* NAD changed */
						SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE);	/* Status = Positive feedback */
					}
					else
					{
						/* NAD couldn't be changed */
						SetupDiagResponse( byInitialNAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);	/* Status = Negative feedback */
						SetLastError( (uint8) C_ERR_LIN2X_B3);
					}
				}
			}
		}
		else if ( u16DiagPCI_SID == C_PCI_SID_DATA_DUMP )
		{
			/* Assign Variant-ID, HW-Reference and SW-Reference (Supplier specification, not LIN specification) */
			/*
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | SID |    D1    |    D2    |    D3    |    D4    |    D5    |
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xB4| Supplier | Supplier |  Variant |  HW-Ref  |  SW-Ref  |
			 *	|     |     |     | ID (LSB) | ID (MSB) |    ID    |    ID    |    ID    |
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 */
			if ( CheckSupplier( (pDiag->byD1) | ((uint16)(pDiag->byD2) << 8)) )
			{
				SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_PENDING);	/* Status = Pending */
				if ( pDiag->byD3 != 0xFFU )
				{
					g_NvramUser.Variant = pDiag->byD3;							/* Set new Variant-ID */
				}
				if ( pDiag->byD4 != 0xFFU )
				{
					g_NvramUser.HwRef = pDiag->byD4;							/* Set new HW-Reference */
				}
				if ( pDiag->byD5 != 0xFFU )
				{
					/* -=#=- Note: SW-Ref should not be changed by this function, but be reprogramming the flash */
					/* g_NvramUser.SwRef = pDiag->byD5;	*/						/* Set new SW-Reference */
				}
				/* Store NVRAM */
				if ( (NVRAM_Store( C_NVRAM_USER_PAGE_ALL) == C_NVRAM_STORE_OKAY)
					&& ((pDiag->byD3 == 0xFFU) || (g_NvramUser.Variant == pDiag->byD3))
					&& ((pDiag->byD4 == 0xFFU) || (g_NvramUser.HwRef == pDiag->byD4))
					/* && ((pDiag->byD5 == 0xFF) || (g_NvramUser.SwRef == pDiag->byD5)) */ /* SW-Reference */
				   )
				{
					/* Variant-ID and/or HW-reference and/or SW-reference changed */
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE);	/* Status = Positive feedback */
				}
				else
				{
					/* Variant-ID and/or HW-reference and/or SW-reference counldn't be changed */
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);	/* Status = Negative feedback */
					SetLastError( (uint8) C_ERR_LIN2X_B4);
				}
			}
		}
#if _SUPPORT_LIN_AA
		else if ( u16DiagPCI_SID == C_PCI_SID_ASSIGN_NAD )
		{
			/* Assign NAD (Slave Node Position Detection, SNPD) */
			/* This is a broadcast LIN-command; Therefore no feedback is returned */
			/* Beginning with the BSM-Initialisation, all SNPD nodes with BSM
			 * capability start their measurement sequence within the next
			 * break field.
			 */
			if ( ( CheckSupplier( (pDiag->byD1) | ((uint16)(pDiag->byD2) << 8)) != FALSE ) && ((pDiag->byD4 == (uint8) C_SNPD_METHOD_BSM) || (pDiag->byD4 == (uint8) C_SNPD_METHOD_BSM2)) )	/* SNPD Method ID */
			{
				if ( (pDiag->byD3 == (uint8) C_SNPD_SUBFUNC_START) && (g_u8LinAAMode == 0U) )	/* Sub-function ID */
				{
					/* BSM Initialisation (only if not already in AA-mode) */
					/*
					 *	+-----+-----+-----+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+-----+----------+----------+----------+----------+----------+
					 *	|     |     |     | Supplier | Supplier | Function | Function |  Unused  |
					 *	| NAD | 0x06| 0xB5| ID (LSB) | ID (MSB) | ID (LSB) | ID (MSB) |          |
					 *	|     |     |     |  (0xFF)  |  (0x7F)  |  (0x01)  |0x02||0xF1|  (0xFF)  |
					 *	+-----+-----+-----+----------+----------+----------+----------+----------+
					 * All SNPD slaves with BSM capability start their measurement
					 * sequence with the next break field.
					 * Function ID (MSB): 0x02 = Bus Shunt Method
					 * Function ID (LSB): 0x01 = BSM Initialisation
					 */
					if ( g_e8MotorStatusMode == (uint8) C_MOTOR_STATUS_RUNNING )	/* If actuator is not stopped ... */
					{
						MotorDriverStop( (uint16) C_STOP_RAMPDOWN);				/* ... stop actuator NOW (LIN-AA) */
					}
					g_u8ChipResetOcc = FALSE;
					g_u8StallOcc = FALSE;
					g_u8StallTypeComm &= ~M_STALL_MODE;
					g_u8EmergencyRunOcc = FALSE;
					g_e8MotorDirectionCCW = (uint8) C_MOTOR_DIR_UNKNOWN;		/* Direction is unknown (9.5.3.13) */

					ml_SetSlaveNotAddressed();									/* (Test) Allow re-addressing */
#if LIN_AA_INFO
					ClearAAData();												/* Clear AA-ModuleScreening data */
#endif /* LIN_AA_INFO */
					g_u16LinAATicker = PI_TICKS_PER_SECOND;						/* Re-start g_u16LinAATicker to time LIN-AA timeout of 40sec */
					g_u8LinAATimeout = (uint8) C_LINAA_TIMEOUT;					/* LIN-AA time-out counter (seconds) */
					g_u8LinAAMode = (uint8) C_SNPD_SUBFUNC_START;				/* LIN-AA mode (BSM-init) */
					if ( pDiag->byD4 == (uint8) C_SNPD_METHOD_BSM2 )
					{
						g_u8LinAAMode |= (uint8) C_SNPD_METHOD_2;
					}
#if (LINAA_BSM_SNPD_R1p0 != FALSE)
					ml_InitAutoAddressing();
#endif /* (LINAA_BSM_SNPD_R1p0 != FALSE) */
				}
				else if ( ((pDiag->byD4 == (uint8) C_SNPD_METHOD_BSM ) && ((g_u8LinAAMode & (uint8) C_SNPD_METHOD_2) == 0)) ||
						  ((pDiag->byD4 == (uint8) C_SNPD_METHOD_BSM2) && ((g_u8LinAAMode & (uint8) C_SNPD_METHOD_2) != 0)) )
				{
					if ( (pDiag->byD3 == (uint8) C_SNPD_SUBFUNC_ADDR) && ((g_u8LinAAMode & (uint8) M_SNPD_SUBFUNC) == (uint8) C_SNPD_SUBFUNC_START) )
					{
						/* Assign NAD to last not-addressed slave (only in AA-mode) */
						/*
						 *	+-----+-----+-----+----------+----------+----------+----------+----------+
						 *	| NAD | PCI | SID |    D1    |    D2    |    D3    |    D4    |    D5    |
						 *	+-----+-----+-----+----------+----------+----------+----------+----------+
						 *	|     |     |     | Supplier | Supplier | Function | Function |          |
						 *	| NAD | 0x06| 0xB5| ID (LSB) | ID (MSB) | ID (LSB) | ID (MSB) |  New NAD |
						 *	|     |     |     |  (0xFF)  |  (0x7F)  |  (0x02)  |0x02||0xF1|          |
						 *	+-----+-----+-----+----------+----------+----------+----------+----------+
						 * All SNPD slaves with BSM capability start their measurement sequence
						 * within the break field; after the break the selected SNPD slave takes
						 * the NAD.
						 */
						if ( ml_GetAutoaddressingStatus() )
						{
							g_u8NAD = (pDiag->byD5);								/* New NAD (into RAM) */
							(void) ml_SetLoaderNAD( g_u8NAD);						/* Inform new NAD to LIN */
							ml_SetSlaveAddressed();
						}
					}
					else if ( (pDiag->byD3 == (uint8) C_SNPD_SUBFUNC_STORE) && ((g_u8LinAAMode & (uint8) M_SNPD_SUBFUNC) == (uint8) C_SNPD_SUBFUNC_START) )
					{
						/* Store NAD in slave (optional) (only in AA-mode) */
						/*
						 *	+-----+-----+-----+----------+----------+----------+----------+----------+
						 *	| NAD | PCI | SID |    D1    |    D2    |    D3    |    D4    |    D5    |
						 *	+-----+-----+-----+----------+----------+----------+----------+----------+
						 *	|     |     |     | Supplier | Supplier | Function | Function |  Unused  |
						 *	| NAD | 0x06| 0xB5| ID (LSB) | ID (MSB) | ID (LSB) | ID (MSB) |          |
						 *	|     |     |     |  (0xFF)  |  (0x7F)  |  (0x03)  |0x02||0xF1|  (0xFF)  |
						 *	+-----+-----+-----+----------+----------+----------+----------+----------+
						 * All SNPD slaves with BSM capability store their new NAD from the
						 * RAM in to the NVM, if available.
						 */
						g_NvramUser.NAD = g_u8NAD;
						(void) NVRAM_Store( (uint16) C_NVRAM_USER_PAGE_ALL);
						g_u8LinAAMode = (g_u8LinAAMode & (uint8) ~M_SNPD_SUBFUNC) | (uint8) C_SNPD_SUBFUNC_STORE;
					}
					else if ( (pDiag->byD3 == (uint8) C_SNPD_SUBFUNC_FINISH) && ((g_u8LinAAMode & (uint8) M_SNPD_SUBFUNC) != 0) )
					{
						/* Assign NAD Finished (Only in LIN-AA mode) */
						/*
						 *	+-----+-----+-----+----------+----------+----------+----------+----------+
						 *	| NAD | PCI | SID |    D1    |    D2    |    D3    |    D4    |    D5    |
						 *	+-----+-----+-----+----------+----------+----------+----------+----------+
						 *	|     |     |     | Supplier | Supplier | Function | Function |  Unused  |
						 *	| NAD | 0x06| 0xB5| ID (LSB) | ID (MSB) | ID (LSB) | ID (MSB) |          |
						 *	|     |     |     |  (0xFF)  |  (0x7F)  |  (0x04)  |0x02||0xF1|  (0xFF)  |
						 *	+-----+-----+-----+----------+----------+----------+----------+----------+
						 * All SNPD slaves with BSM capability stop their measurement sequence
						 * in the break field.
						 */
#if (LINAA_BSM_SNPD_R1p0 != FALSE)
						ml_StopAutoAddressing();
#endif /* (LINAA_BSM_SNPD_R1p0 != FALSE) */
						g_u16LinAATicker = 0U;
						g_u8LinAAMode = 0U;
					}
				}
			}
		}
#endif /* _SUPPORT_LIN_AA */
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
		else if ( u16DiagPCI_SID == C_PCI_SID_ASSIGN_GROUPADDRESS )
		{
			/* Assign Group-address to a NAD
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | SID |    D1    |    D2    |    D3    |    D4    |    D5    |
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	|     |     |     | Supplier | Supplier | Function | Function |  Group-  |
			 *	| NAD | 0x06| 0xB6| ID (LSB) | ID (MSB) | ID (LSB) | ID (MSB) |  Address |
			 *	|     |     |     |  (0xFF)  |  (0x7F)  |          |          |          |
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 */
			SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_PENDING);	/* Status = Pending */
			l_u8GAD = pDiag->byD5;
			g_NvramUser.GAD = l_u8GAD;
			if ( (NVRAM_Store( C_NVRAM_USER_PAGE_ALL) == C_NVRAM_STORE_OKAY) )
			{
				/* Variant-ID and/or HW-reference and/or SW-reference changed */
				SetupDiagResponse( g_u8NAD, pDiag->bySID, C_ERRCODE_POSITIVE_RESPONSE);	/* Status = Positive feedback */
			}
			else
			{
				/* Variant-ID and/or HW-reference and/or SW-reference counldn't be changed */
				SetupDiagResponse( g_u8NAD, pDiag->bySID, C_ERRCODE_SFUNC_NOSUP);	/* Status = Negative feedback */
				SetLastError( (uint8) C_ERR_LIN2X_B6);
			}
		}
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
		else if ( u16DiagPCI_SID == C_PCI_SID_ASSIGN_FRAME_ID_RNG )
		{
			/* Assign frame ID range */
			/* Assign frame ID range is used to set or disable PIDs up to four frames.
			 * The request shall be structured as shown
			 *
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | SID |    D1    |    D2    |    D3    |    D4    |    D5    |
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xB7|  start   |    PID   |    PID   |    PID   |    PID   |
			 *	|     |     |     |  index   |  (index) | (index+1)| (index+2)| (index+3)|
			 *	+-----+-----+-----+----------+----------+----------+----------+----------+
			 * The start index specifies which is the first frame to assign a PID.
			 * The first frame in the list has index 0 (zero).
			 * The PIDs are an array of four PID values that will be used in the
			 * configuration request. Valid PID values here are the PID values for
			 * signal carrying frames, the unassigns value 0 (zero) and the do not
			 * care value 0xFF. The unassigns value is used to invalidate this frame
			 * for transportation on the bus. The do not care is used to keep the
			 * previous assigned value of this frame.
			 * In case the slave cannot fulfil all of the assignments set PID or
			 * unassigns or do not care, the slave shall reject the request. The do
			 * not care can always be fulfilled.
			 * The slave node will not validate the given PIDs (i.e. validating the
			 * parity flags), the slave node relies on that the master sets the
			 * correct PIDs.
			 *
			 * A response shall only be sent if the NAD match.
			 */
			/* Since the slave node has only two frames the last two must be set to do not care (0xFF), otherwise the request will fail. */
			if ( (pDiag->byD4 != 0xFFU) || (pDiag->byD5 != 0xFFU) || (pDiag->byD1 > 1U) || ((pDiag->byD1 == 1U) && (pDiag->byD3 != 0xFFU)) )
			{
				/* Negative feedback */
				SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);	/* Status = Negative feedback */
			}
			else
			{
				uint16 u16NvramStoreResult;
				SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_PENDING);		/* Status = Pending */

				u16NvramStoreResult = ~C_NVRAM_STORE_OKAY;
				if ( pDiag->byD1 == 0U )
				{
					/* Starting with first message-index */
					if ( pDiag->byD2 != 0xFFU )
					{
						/* First Frame-ID is Control-message Frame-ID */
						g_NvramUser.ControlFrameID = pDiag->byD2;
						(void) ml_Disconnect();
						if ( g_NvramUser.ControlFrameID != 0x00U )
						{
							(void) ml_AssignFrameToMessageID( MSG_CONTROL, g_NvramUser.ControlFrameID);
						}
						else
						{
							(void) ml_DisableMessage( MSG_CONTROL);
						}
						u16NvramStoreResult = C_NVRAM_STORE_OKAY;
					}
					if ( pDiag->byD3 != 0xFFU )
					{
						/* Second Frame-ID is Status-message Frame-ID */
						g_NvramUser.StatusFrameID = pDiag->byD3;
						(void) ml_Disconnect();
						if ( g_NvramUser.StatusFrameID != 0x00U )
						{
							(void) ml_AssignFrameToMessageID( MSG_STATUS, g_NvramUser.StatusFrameID);
						}
						else
						{
							(void) ml_DisableMessage( MSG_STATUS);
						}
						u16NvramStoreResult = C_NVRAM_STORE_OKAY;
					}
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
					if ( pDiag->byD4 != 0xFFU )
					{
						/* Third Frame-ID is Group Control-message Frame-ID */
						g_NvramUser.GroupControlFrameID = pDiag->byD4;
						(void) ml_Disconnect();
						if ( g_NvramUser.GroupControlFrameID != 0U )
						{
							(void) ml_AssignFrameToMessageID( MSG_GROUP_CONTROL, g_NvramUser.GroupControlFrameID);
						}
						else
						{
							(void) ml_DisableMessage( MSG_GROUP_CONTROL);
						}
						u16NvramStoreResult = C_NVRAM_STORE_OKAY;
					}
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
				}
				else if ( pDiag->byD1 == 1U )
				{
					if ( pDiag->byD2 != 0xFFU )
					{
						/* Starting with second message-index; First Frame-ID is Status-message Frame-ID */
						g_NvramUser.StatusFrameID = pDiag->byD2;
						(void) ml_Disconnect();
						if ( g_NvramUser.StatusFrameID != 0x00U )
						{
							(void) ml_AssignFrameToMessageID( MSG_STATUS, g_NvramUser.StatusFrameID);
						}
						else
						{
							(void) ml_DisableMessage( MSG_STATUS);
						}
						u16NvramStoreResult = C_NVRAM_STORE_OKAY;
					}
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
					if ( pDiag->byD3 != 0xFFU )
					{
						/* Third Frame-ID is Group Control-message Frame-ID */
						g_NvramUser.GroupControlFrameID = pDiag->byD3;
						(void) ml_Disconnect();
						if ( g_NvramUser.GroupControlFrameID != 0U )
						{
							(void) ml_AssignFrameToMessageID( MSG_GROUP_CONTROL, g_NvramUser.GroupControlFrameID);
						}
						else
						{
							(void) ml_DisableMessage( MSG_GROUP_CONTROL);
						}
						u16NvramStoreResult = C_NVRAM_STORE_OKAY;
					}
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
				}
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
				else if ( pDiag->byD1 == 2U )
				{
					/* Starting with index 2 (MSG_GROUP_CONTROL) */
					if ( pDiag->byD2 != 0xFFU )
					{
						/* Third Frame-ID is Group Control-message Frame-ID */
						g_NvramUser.GroupControlFrameID = pDiag->byD2;
						(void) ml_Disconnect();
						if ( g_NvramUser.GroupControlFrameID != 0U )
						{
							(void) ml_AssignFrameToMessageID( MSG_GROUP_CONTROL, g_NvramUser.GroupControlFrameID);
						}
						else
						{
							(void) ml_DisableMessage( MSG_GROUP_CONTROL);
						}
						u16NvramStoreResult = C_NVRAM_STORE_OKAY;
					}
				}
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
				if ( u16NvramStoreResult == C_NVRAM_STORE_OKAY )
				{
					(void) ml_Connect();
					u16NvramStoreResult = NVRAM_Store( C_NVRAM_USER_PAGE_ALL);		/* Store NVRAM */
				}

				if ( (u16NvramStoreResult == C_NVRAM_STORE_OKAY) &&
					(
					((pDiag->byD1 == 0U)														/* Start-index = MSG_CONTROL */
					&& ((pDiag->byD2 == 0xFFU) || (g_NvramUser.ControlFrameID == pDiag->byD2))	/* Max. three Frame-ID's */
					&& ((pDiag->byD3 == 0xFFU) || (g_NvramUser.StatusFrameID == pDiag->byD3))
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
					&& ((pDiag->byD4 == 0xFFU) || (g_NvramUser.GroupControlFrameID == pDiag->byD4))
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
					)
					|| ((pDiag->byD1 == 1U)														/* Start-index = MSG_STATUS */
					&& ((pDiag->byD2 == 0xFFU) || (g_NvramUser.StatusFrameID == pDiag->byD2))	/* Max. two Frame-ID's */
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
					&& ((pDiag->byD3 == 0xFFU) || (g_NvramUser.GroupControlFrameID == pDiag->byD3))
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
					)
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)
					|| ((pDiag->byD1 == 2U)														/* Start-index = MSG_GROUP_CONTROL */
					&& ((pDiag->byD2 == 0xFFU) || (g_NvramUser.GroupControlFrameID == pDiag->byD2))
					)
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
					) )
				{
					/* Positive feedback */
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE);/* Status = Positive feedback */
				}
				else
				{
					/* Negative feedback */
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);	/* Status = Negative feedback */
				}
			}
		}
		else if ( u16DiagPCI_SID == C_PCI_SID_WRITE_BY_ID )
		{
			/* Write-by-Identifier (0xCB) */
			/* Request:
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xCB |Identifier| ID Data1 | ID Data2 | ID Data3 | ID Data4 |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *
			 * Response (OK):
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0x0B |Identifier| ID Data1 | ID Data2 | ID Data3 | ID Data4 |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 * Identifier: 00
			 * ID data 1-4
			 */
			uint16 u16SupplierID = (((uint16) pDiag->byD3) << 8) | ((uint16) pDiag->byD2);
			uint16 u16ParamID = (((uint16) pDiag->byD5) << 8) | ((uint16) pDiag->byD4);
			if ( pDiag->byD1 == (uint8) C_LIN_PROD_ID )
			{
				/* Write Function ID */
				if ( (u16SupplierID == C_SUPPLIER_ID) && (u16ParamID == C_FUNCTION_ID) )
				{
					/* Correct Supplier ID; Change Function ID allowed */
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_PENDING);			/* Status = Pending */
					g_NvramUser.FunctionID = u16ParamID;
					/* Store NVRAM */
					if ( (NVRAM_Store( C_NVRAM_USER_PAGE_ALL) == C_NVRAM_STORE_OKAY) && (g_NvramUser.FunctionID == u16ParamID) )
					{
						/* Function-ID changed */
						g_DiagResponse.byNAD = g_u8NAD;
						g_DiagResponse.byPCI = (uint8) C_RSID_WRITE_BY_ID;							/* Positive feedback */
						g_DiagResponse.byRSID = (uint8) C_RPCI_WRITE_BY_ID;
						g_DiagResponse.byD1 = pDiag->byD1;
						g_DiagResponse.byD2 = pDiag->byD2;
						g_DiagResponse.byD3 = pDiag->byD3;
						g_DiagResponse.byD4 = pDiag->byD4;
						g_DiagResponse.byD5 = pDiag->byD5;

						g_u8BufferOutID = (uint8) QR_RFR_DIAG;										/* LIN Output buffer is valid (RFR_DIAG) */
					}
					else
					{
						/* Function-ID couldn't be changed */
						SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);	/* Status = Negative feedback */
						SetLastError( (uint8) C_ERR_LIN2X_CB);
					}
				}
				else
				{
					/* Wrong Supplier ID (Wild-card not allowed) */
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_INV_MSG_INV_SZ);	/* Status = Negative feedback */
					SetLastError( (uint8) C_ERR_LIN2X_CB);
				}
			}
			else if ( pDiag->byD1 == (uint8) C_LIN_CUST_ID )
			{
				/* Write Customer ID */
				if ( u16SupplierID == C_SUPPLIER_ID )
				{
					/* Correct Supplier ID; Change Function ID allowed */
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_PENDING);			/* Status = Pending */
					g_NvramUser.CustomerID = u16ParamID;
					/* Store NVRAM */
					if ( (NVRAM_Store( C_NVRAM_USER_PAGE_ALL) == C_NVRAM_STORE_OKAY) && (g_NvramUser.CustomerID == u16ParamID) )
					{
						/* Customer-ID changed */
						g_DiagResponse.byNAD = g_u8NAD;
						g_DiagResponse.byPCI = (uint8) C_RSID_WRITE_BY_ID;							/* Positive feedback */
						g_DiagResponse.byRSID = (uint8) C_RPCI_WRITE_BY_ID;
						g_DiagResponse.byD1 = pDiag->byD1;
						g_DiagResponse.byD2 = pDiag->byD2;
						g_DiagResponse.byD3 = pDiag->byD3;
						g_DiagResponse.byD4 = pDiag->byD4;
						g_DiagResponse.byD5 = pDiag->byD5;

						g_u8BufferOutID = (uint8) QR_RFR_DIAG;										/* LIN Output buffer is valid (RFR_DIAG) */
					}
					else
					{
						/* Customer-ID couldn't be changed */
						SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);	/* Status = Negative feedback */
						SetLastError( (uint8) C_ERR_LIN2X_CB);
					}
				}
				else
				{
					/* Wrong Supplier ID (Wild-card not allowed) */
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_INV_MSG_INV_SZ);	/* Status = Negative feedback */
					SetLastError( (uint8) C_ERR_LIN2X_CB);
				}
			}
			else if ( pDiag->byD1 == (uint8) C_PROD_DATE )
			{
				/* Write Customer ID */
				if ( u16SupplierID == C_SUPPLIER_ID )
				{
					/* Correct Supplier ID; Change Function ID allowed */
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_PENDING);			/* Status = Pending */
					g_NvramUser.ProductionDate = u16ParamID;
					/* Store NVRAM */
					if ( (NVRAM_Store( C_NVRAM_USER_PAGE_ALL) == C_NVRAM_STORE_OKAY) && (g_NvramUser.ProductionDate == u16ParamID) )
					{
						/* Customer-ID changed */
						g_DiagResponse.byNAD = g_u8NAD;
						g_DiagResponse.byPCI = (uint8) C_RSID_WRITE_BY_ID;							/* Positive feedback */
						g_DiagResponse.byRSID = (uint8) C_RPCI_WRITE_BY_ID;
						g_DiagResponse.byD1 = pDiag->byD1;
						g_DiagResponse.byD2 = pDiag->byD2;
						g_DiagResponse.byD3 = pDiag->byD3;
						g_DiagResponse.byD4 = pDiag->byD4;
						g_DiagResponse.byD5 = pDiag->byD5;

						g_u8BufferOutID = (uint8) QR_RFR_DIAG;										/* LIN Output buffer is valid (RFR_DIAG) */
					}
					else
					{
						/* Customer-ID couldn't be changed */
						SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);	/* Status = Negative feedback */
						SetLastError( (uint8) C_ERR_LIN2X_CB);
					}
				}
				else
				{
					/* Wrong Supplier ID (Wild-card not allowed) */
					SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_INV_MSG_INV_SZ);	/* Status = Negative feedback */
					SetLastError( (uint8) C_ERR_LIN2X_CB);
				}
			}
			else
			{
				/* Identifier not supported */
				SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);			/* Status = Negative feedback */
				SetLastError( (uint8) C_ERR_LIN2X_CB);
			}
		}
#endif /* ((LINPROT & LINXX) == LIN2X) */
#if ((LINPROT & LINXX) == LIN2J)
		else if ( u16DiagPCI_SID == C_SID_PCI_RESET )	/* Targeted or Broadcast reset */
		{
			/* Reset Target */
			MLX4_RESET();														/* Reset the Mlx4   */
			bistResetInfo = C_CHIP_STATE_LIN_CMD_RESET;
			MLX16_RESET();														/* Reset the Mlx16  */
			/* This reset restart the chip as POR, and doesn't come back (no answer) */
		}
#endif /* ((LINPROT & LINXX) == LIN2J) */
#if _SUPPORT_MLX_DEBUG_MODE
		else if ( pDiag->bySID == C_SID_MLX_DEBUG )
		{
			/*
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier | Param #1 | Param #2 | Function |
			 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |          |          |    ID    |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			uint16 u16SupplierID = (((uint16) pDiag->byD2) << 8) | ((uint16) pDiag->byD1);
			if ( u16SupplierID == C_SUPPLIER_ID )
			{
				/* Reply diagnostics response with NAD, length and RSID.*/
				g_DiagResponse.byNAD = g_u8NAD;
				g_DiagResponse.byPCI = 0x06U;
				g_DiagResponse.byRSID = (uint8) C_SID_MLX_DEBUG;

				/* Function-ID and Description
				 * 0x00: Supported Function-ID's
				 * 0x5D: Stall detector
				 * 0x5E: Get currents buffer
				 * 0xA0: LIN Auto-Addressing Test module - Ish2/Ish3 setting
				 * 0xA1: LIN-AA BSM Ishunt #1,2 & 3 and flags
				 * 0xA2: LIN-AA BSM Common-mode & Differential-mode levels #1
				 * 0xA3: LIN-AA BSM Common-mode & Differential-mode levels #2
				 * 0xA4: LIN-AA BSM Common-mode & Differential-mode levels #3
				 * 0xA5: Application State
				 * 0xA6: LIN Slave Baudrate
				 * 0xAC: ADC Temperature/Voltage sensor (raw)
				 * 0xAE: Ambient-environment: Temperature, S/W Build ID
				 * 0xAF: Get PWM-IN Period & Low-Time
				 * 0xC0: Get CPU-clock
				 * 0xC1: Get Chip-ID
				 * 0xC2: Get HW/SW-ID of chip
				 * 0xC5: Get _DEBUG options
				 * 0xC6: Get _SUPPORT options
				 * 0xC7: MLX4 F/W & Loader
				 * 0xC8: Get Platform version
				 * 0xC9: Get application version
				 * 0xCA: Get Melexis NVRAM page info
				 * 0xCB: Get PID g_u16PidCtrlRatio & g_u16PID_I
				 * 0xCC: Get NVRAM stored errorcode's
				 * 0xCD: Clear NVRAM error-logging
				 * 0xCE: Chip-environment: Temperature, Motor driver current, Supply-voltage
				 * 0xCF: Chip functions
				 * 0xD0-0xD7: Set ANA_OUT[A:H]
				 * 0xF0: Get/Set FlashTrimming info
				 * 0xF1: Start Flash/ROM-Sum Calculation
				 * 0xF2: Get Flash/ROM Calculation Result
				 * 0xF8: NVRAM Clear function
				 * 0xFC: Clear Fatal-handler error logging
				 * 0xFD: Get I/O-register value (16-bits)
				 * 0xFE: Get Fatal-error: error-code, info and address
				 */
				if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_SUPPORT )
				{
					/* Get MLX Debug Support
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier |   index  | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |          |   0xFF   |   0x00   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB |   index  |MLX DBG[i]|MLX DBG[i]| Reserved | Reserved |
					 *	|     |     |      |          |   (LSB)  |   (MSB)  |          |          |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					uint16 u16Index = (uint16) (pDiag->byD3 & 0x0FU);
					StoreD1to2( tMlxDbgSupport[u16Index]);
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_STALLDET )
				{
					/* Stall detector
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |   0x5D   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+------------+------------+------------+------------+----------+
					 *	| NAD | PCI | RSID |     D1     |     D2     |     D3     |     D4     |    D5    |
					 *	+-----+-----+------+------------+------------+------------+------------+----------+
					 *	| NAD | 0x06| 0xDB |Stallcurrent|Stallcurrent|Motorcurrent|Motorcurrent|StallFlags|
					 *	|     |     |      |Thrshld(LSB)|Thrshld(LSB)|MovAvg (LSB)|MovAvg (MSB)|          |
					 *	+-----+-----+------+------------+------------+------------+------------+----------+
					 */
					uint16 u16Value = (uint16) ((mulU32_U16byU16( (l_u16MotorCurrentStallThrshldxN >> C_MOVAVG_SSZ), g_u16MCurrgain) + (C_GMCURR_DIV/2)) / C_GMCURR_DIV); /* Stall motor-current threshold [mA] */
					g_DiagResponse.byD1 = (uint8) (u16Value & 0xFFU);
					g_DiagResponse.byD2 = (uint8) (u16Value >> 8);
					u16Value = (uint16) ((mulU32_U16byU16( (g_u16MotorCurrentMovAvgxN >> C_MOVAVG_SSZ), g_u16MCurrgain) + (C_GMCURR_DIV/2)) / C_GMCURR_DIV); /* Moving average-motor current [mA] */
					g_DiagResponse.byD3 = (uint8) (u16Value & 0xFFU);
					g_DiagResponse.byD4 = (uint8) (u16Value >> 8);
#if _SUPPORT_STALLDET_O
					if ( g_u8StallTypeComm & C_STALL_FOUND_O )
					{
						g_DiagResponse.byD5 = (g_u8StallTypeComm & M_STALL_MODE) | (l_u8StallCountO & 0x07U);	/* Stall detection & count */
					}
					else
#endif /* _SUPPORT_STALLDET_O */
					{
						g_DiagResponse.byD5 = (g_u8StallTypeComm & M_STALL_MODE) | (l_u8StallCountA & 0x07U);	/* Stall detection & count */
					}
					if ( g_e8StallDetectorEna != C_STALLDET_NONE )
					{
						g_u8StallTypeComm = (uint8) C_STALL_NOT_FOUND;
					}
					g_u8BufferOutID = (uint8) QR_RFR_DIAG;						/* LIN Output buffer is valid (RFR_DIAG) */
				}
#if _SUPPORT_LIN_AA && (LIN_AA_INFO != FALSE)
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_LINAA_1 )
				{
					/* LIN-AA BSM Ishunt #1,2 & 3 and flags
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |   0xA1   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB |   Step   |  Ishunt1 |  Ishunt2 |  Ishunt3 | AA-Flags |
					 *	|     |     |      |CycleCount|          |          |          |          |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					PSNPD_DATA pSNPD_Data = LIN_AA_DATA + l_u8SNPD_CycleCountComm;
					g_DiagResponse.byD1 = (uint8) ((pSNPD_Data->byStepAndFlags << 1) & 0xF0U) | (l_u8SNPD_CycleCountComm & 0x0FU);
					g_DiagResponse.byD2 = pSNPD_Data->byIshunt1;
					g_DiagResponse.byD3 = pSNPD_Data->byIshunt2;
					g_DiagResponse.byD4 = pSNPD_Data->byIshunt3;
					g_DiagResponse.byD5 = (pSNPD_Data->byStepAndFlags & 0x87U);
					l_u8SNPD_CycleCountComm++;
					if ( l_u8SNPD_CycleCountComm >= LIN_AA_INFO_SZ )								/* Don't increase index in case last AA-structure index */
					{
						l_u8SNPD_CycleCountComm = 0;
					}
					g_u8BufferOutID = (uint8) QR_RFR_DIAG;											/* LIN Output buffer is valid (RFR_DIAG) */
				}
#if (LIN_AA_SCREENTEST != FALSE)
				else if ( (pDiag->byD5 >= (uint8) C_DBG_SUBFUNC_LINAA_2) && (pDiag->byD5 <= (uint8) C_DBG_SUBFUNC_LINAA_4) )
				{
					/* LIN-AA BSM Common-mode & Differential-mode levels #1, #2 or #3
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |0xA2/A3/A4|
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB |CycleCount|CommonMode|CommonMode|DifferMode|DifferMode|
					 *	|     |     |      |          |   (LSB)  |   (MSB)  |   (LSB)  |   (MSB)  |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					PSNPD_DATA pSNPD_Data = LIN_AA_DATA + l_u8SNPD_CycleCountComm;
					uint16 *pu16CMDM;
					g_DiagResponse.byD1 = l_u8SNPD_CycleCountComm;
					if ( (pDiag->byD5 == (uint8) C_DBG_SUBFUNC_LINAA_2))
					{
						pu16CMDM = (uint16 *) &(pSNPD_Data->u16CM_1);
					}
					else if (pDiag->byD5 >= (uint8) C_DBG_SUBFUNC_LINAA_3)
					{
						pu16CMDM = (uint16 *) &(pSNPD_Data->u16CM_2);
					}
					else
					{
						pu16CMDM = (uint16 *) &(pSNPD_Data->u16CM_3);
					}
					StoreD2to5( pu16CMDM[0], pu16CMDM[1]); /*lint !e415 */
				}
#endif /* (LIN_AA_SCREENTEST != FALSE) */
#endif /* _SUPPORT_LIN_AA && (LIN_AA_INFO != FALSE) */
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_APPLSTATE )
				{
					/* Application state
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |   0xA5   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB |   Motor  |  Actual  |  Actual  |   Motor  |ErrorFlags|
					 *	|     |     |      |StatusMode| Pos (LSB)| Pos (MSB)|  Request |		  |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * ErrorFlags:
					 *	bit 7: Chip Reset occurred
					 *	bit 6: Stall occurred
					 *	bit 5: Emergency Run occurred
					 *	bit 4: Over-temperature
					 *  bit 3:2: Voltage (In-range, UV and OV)
					 *  bit 1:0: Electric Error (Ok, Error, Permanent)
					 */
					g_DiagResponse.byD1 = g_e8MotorStatusMode;
					{
						uint16 u16CopyPosition = g_u16ActualPosition;
						g_DiagResponse.byD2 = (uint8) (u16CopyPosition & 0xFFU);
						g_DiagResponse.byD3 = (uint8) (u16CopyPosition >> 8);
					}
					g_DiagResponse.byD4 = (g_e8MotorRequest & 0x0F);
					{
						uint8 u8D5 = ((g_e8ErrorVoltage & 0x03U) << 2) | (g_e8ErrorElectric & 0x03U);
						if ( g_e8ErrorOverTemperature != FALSE )
						{
							u8D5 |= 0x10U;
						}
						if ( g_u8EmergencyRunOcc != FALSE )
						{
							u8D5 |= 0x20U;
						}
						if ( g_u8StallOcc != FALSE )
						{
							u8D5 |= 0x40U;
						}
						if (g_u8ChipResetOcc != FALSE)
						{
							u8D5 |= 0x80U;
						}
						g_DiagResponse.byD5 = u8D5;
					}
					g_u8BufferOutID = (uint8) QR_RFR_DIAG;											/* LIN Output buffer is valid (RFR_DIAG) */
				}
#if (LIN_AA_INFO && LIN_AA_SCREENTEST)
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_LIN_BAUDRATE )
				{
					/* LIN Slave baudrate
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |   0xA6   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB |  MCU_PLL |NomLINBaud|NomLINBaud|ActLINBaud|ActLINBaud|
					 *	|     |     |      |   _MULT  |rate (LSB)|rate (MSB)|rate (LSB)|rate (MSB)|
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					g_DiagResponse.byD1 = (uint8) MCU_PLL_MULT;
#if (__MLX_PLTF_VERSION_MAJOR__ == 4)
					StoreD2to5( 0xFFFF, ml_GetBaudRate());
#elif (__MLX_PLTF_VERSION_MAJOR__ == 3)
					uint16 u16Baudrate = divU16_U32byU16( (PLL_freq/2U), (((uint16) u8NominalBaudRateDiv) << (u8BaudRatePreScaler >> 4)));
					g_DiagResponse.byD2 = (uint8) (u16Baudrate & 0xFFU);
					g_DiagResponse.byD3 = (uint8) (u16Baudrate >> 8);
					u16Baudrate = divU16_U32byU16( (PLL_freq/2), (((uint16) u8ActualBaudRateDiv) << (u8BaudRatePreScaler >> 4)));
					g_DiagResponse.byD4 = (uint8) (u16Baudrate & 0xFFU);
					g_DiagResponse.byD5 = (uint8) (u16Baudrate >> 8);
					g_u8BufferOutID = (uint8) QR_RFR_DIAG;						/* LIN Output buffer is valid (RFR_DIAG) */
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 3) */
				}
#endif /* (LIN_AA_INFO && LIN_AA_SCREENTEST) */
#if (_SUPPORT_AUTO_BAUDRATE != FALSE)
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_RESTART_AUTO_BAUDRATE )
				{
					/* Restart auto baudrate detection
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |   0xA7   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response (none)
					 */
#if ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 1))
					__asm__("clrb dp:_LinBusStatus.1");							/* LinBusStatus &= ~ML_LIN_BAUDRATE_DETECTED; */
#endif /* ((__MLX_PLTF_VERSION_MAJOR__ == 3) && (__MLX_PLTF_VERSION_MINOR__ >= 1)) */
#if (__MLX_PLTF_VERSION_MAJOR__ == 4)
					(void)ml_SetAutoBaudRateMode( ML_ABR_ON_FIRST_FRAME);
#endif /* (__MLX_PLTF_VERSION_MAJOR__ == 4) */
#if _SUPPORT_LIN_BUS_ACTIVITY_CHECK
					g_u8Mlx4ErrorState = C_MLX4_STATE_IMMEDIATE_RST;			/* Reset MLX4 too */
#endif /* _SUPPORT_LIN_BUS_ACTIVITY_CHECK */
				}
#endif /* (_SUPPORT_AUTO_BAUDRATE != FALSE) */
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_ADC_RAW )
				{
					/* ADC Raw data (Temperature and Supply)
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |   0xAC   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+-----------+-----------+---------+---------+----------+
					 *	| NAD | PCI | RSID |     D1    |     D2    |   D3    |   D4    |    D5    |
					 *	+-----+-----+------+-----------+-----------+---------+---------+----------+
					 *	| NAD | 0x06| 0xDB |Temperature|Temperature| Supply  | Supply  | Reserved |
					 *	|     |     |      |   (LSB)   |   (MSB)   |  (LSB)  |  (MSB)  |          |
					 *	+-----+-----+------+-----------+-----------+---------+---------+----------+
					 */
					StoreD1to4( GetRawTemperature(), GetRawChipSupply());
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_AMBJENV )
				{
					/* Ambient-environment: Temperature
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |   0xAE   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB | Ambj-Temp|Motor-volt|Motor-volt| Reserved | Reserved |
					 *	|     |     |      |          |  (LSB)   |   (MSB)  |          |          |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
#if _SUPPORT_AMBIENT_TEMP
					uint16 u16Value = (uint16) (g_i16AmbjTemperature + C_TEMPOFF);				/* Ambient Junction temperature + offset (C_TEMPOFF); Range: -C_TEMPOFF .. +(255-C_TEMPOFF) */
					g_DiagResponse.byD1 = (uint8) (u16Value & 0xFFU);
#endif /* _SUPPORT_AMBIENT_TEMP */
#if _SUPPORT_PHASE_SHORT_DET
					StoreD2to5( (uint16) g_i16MotorVoltage, (uint16) g_i16PhaseVoltage);
#else  /* _SUPPORT_PHASE_SHORT_DET */
					StoreD2to5( (uint16) g_i16MotorVoltage, 0xFFFFU);
#endif /* _SUPPORT_PHASE_SHORT_DET */
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_MLX16_CLK )
				{
					/* Get MLX16 Clock
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |   0xC0   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB |MLX16Clock|MLX16Clock| Reserved | Reserved | Reserved |
					 *	|     |     |      |[kHz](LSB)|[kHz](MSB)|   0xFF   |   0xFF   |   0xFF   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					uint16 u16RC_Clock = muldivU16_U16byU16byU16( (2048U + EE_OCLOCK), 1000U, 2048U);
					int16 i16ADC_Temp = (int16) (GetRawTemperature() - EE_OTEMP);
					int16 i16Coef;
					if ( i16ADC_Temp <= 0 )
					{
						/* ((dTemp * Gp) * 1000)/131072 --> ((dTemp * Gp) * 125)/16384 */
						i16Coef = EE_GPCLOCK;
					}
					else
					{
						/* ((dTemp * Gn) * 1000)/131072 --> ((dTemp * Gn) * 125)/16384 */
						i16Coef = EE_GNCLOCK;
					}
					i16Coef = (125 * i16Coef);
					u16RC_Clock += muldivI16_I16byI16byI16( i16ADC_Temp, i16Coef, 16384);
					StoreD1to2( (uint16) (mulU32_U16byU16( u16RC_Clock, ((PLL_CTRL >> 8) + 1U)) >> 2));
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_CHIPID )
				{
					/* Get Chip-ID
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier |   index  | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |          |   0xFF   |   0xC1   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB |   index  | NVRAM[i] | NVRAM[i] |NVRAM[i+1]|NVRAM[i+1]|
					 *	|     |     |      |          |   (LSB)  |   (MSB)  |   (LSB)  |   (MSB)  |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					uint16 u16Index = (uint16) (pDiag->byD3 & 0x02U);
					g_DiagResponse.byD1 = (uint8) u16Index;
					{
						uint16 *pu16NvramData = ((uint16 *) C_ADDR_MLX_CHIPID) + u16Index;			/* NVRAM 16-bit pointer */
						StoreD2to5( pu16NvramData[0], pu16NvramData[1]);
					}
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_HWSWID )
				{
					/* Get HW/SW-ID of chip
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |   0xC2   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB | HW/SW ID | HW/SW ID | CPU-Clock| Reserved | Reserved |
					 *	|     |     |      |   (LSB)  |   (MSB)  |          |   0xFF   |   0xFF   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					g_DiagResponse.byD3 = (uint8) MCU_PLL_MULT;
					StoreD1to2( *((uint16 *) C_ADDR_MLX_HWSWID));
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_SUPPORT_OPTIONS )
				{
					/* Get _SUPPORT options
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |   0xC6   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB | SUPPORT  | SUPPORT  |  SUPPORT |  SUPPORT | Reserved |
					 *	|     |     |      |   (LSB)  |          |          |   (MSB)  |   0xFF   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					g_DiagResponse.byD1 = (uint8) (C_DIAG_RES
#if _SUPPORT_NVRAM_BACKUP
													& ~(1U << 0)				/* bit 0: NVRAM Backup support */
#endif /* _SUPPORT_NVRAM_BACKUP */
#if _SUPPORT_DOUBLE_MOTOR_CURRENT
													& ~(1U << 1)				/* bit 1: Double motor-current support */
#endif /* _SUPPORT_DOUBLE_MOTOR_CURRENT */
#if _SUPPORT_WD_RST_RECOVERY
													& ~(1U << 3)				/* bit 3: Watchdog reset fast recovery support */
#endif /* _SUPPORT_WD_RST_RECOVERY */
#if _SUPPORT_CRASH_RECOVERY
													& ~(1U << 4)				/* bit 4: Crash recovery support */
#endif /* _SUPPORT_CRASH_RECOVERY */
#if _SUPPORT_TESTMODE_OFF
													& ~(1U << 5)				/* bit 5: Debug-I/F off support */
#endif /* _SUPPORT_TESTMODE_OFF */
#if _SUPPORT_MLX16_HALT
													& ~(1U << 6)				/* bit 6: MLX16 enters HALT during holding mode (power-safe) support */
#endif /* _SUPPORT_MLX16_HALT */
#if _SUPPORT_CHIP_TEMP_PROFILE
													& ~(1U << 7)				/* bit 7: Chip temperature profile check (dT/dt) support */
#endif /* _SUPPORT_CHIP_TEMP_PROFILE */
																);
					g_DiagResponse.byD2 = (uint8) (C_DIAG_RES
#if _SUPPORT_AUTO_BAUDRATE
													& ~(1U << 0)				/* bit 0: Auto-detection of baudrate support */
#endif /* _SUPPORT_AUTO_BAUDRATE */
#if _SUPPORT_LIN_UV
													& ~(1U << 1)				/* bit 1: LIN UV check (reset Bus-time-out) support */
#endif /* _SUPPORT_LIN_UV */
#if _SUPPORT_LINNETWORK_LOADER
													& ~(1U << 2)				/* bit 2: Network Flash-loading (NAD) support */
#endif /* _SUPPORT_LINNETWORK_LOADER */
#if _SUPPORT_BUSTIMEOUT_SLEEP
													& ~(1U << 3)				/* bit 3: Bus-time-out to sleep support */
#endif /* _SUPPORT_BUSTIMEOUT_SLEEP */
																);
					g_DiagResponse.byD3 = (uint8) (C_DIAG_RES
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
													& ~(1U << 0)				/* bit 0: Mirror mode m-PWM support */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
#if (MOTOR_PHASES == 4) && (_SUPPORT_PWM_MODE != BIPOLAR_PWM_DOUBLE_MIRROR)
													& ~(1U << 1)				/* bit 1: Single P-FET-Switching support */
#endif /* (MOTOR_PHASES == 4) && (_SUPPORT_PWM_MODE != BIPOLAR_PWM_DOUBLE_MIRROR) */
#if (MOTOR_PHASES == 3) && (_SUPPORT_TWO_PWM != FALSE)
													& ~(1U << 1)				/* bit 1: Two phase PWM support */
#endif /* (MOTOR_PHASES == 3) && (_SUPPORT_TWO_PWM != FALSE) */
#if _SUPPORT_PWM_DC_RAMPUP
													& ~(1U << 3)				/* bit 3: Increasing mPWM-DC at ramp-up support */
#endif /* _SUPPORT_PWM_DC_RAMPUP */
#if _SUPPORT_PWM_DC_RAMPDOWN
													& ~(1U << 4)				/* bit 4: Decrease mPWM-DC at ramp-down support */
#endif /* _SUPPORT_PWM_DC_RAMPDOWN */
#if _SUPPORT_MOTOR_SELFTEST
													& ~(1U << 5)				/* bit 5: Motor driver check at POR support */
#endif /* _SUPPORT_MOTOR_SELFTEST */
#if _SUPPORT_PHASE_SHORT_DET
													& ~(1U << 6)				/* bit 6: Phase-short-to-GND detection support */
#endif /* _SUPPORT_PHASE_SHORT_DET */
#if _SUPPORT_STALLDET_O
													& ~(1U << 7)				/* bit 7: Current-oscillation stall-detection support */
#endif /* _SUPPORT_STALLDET_O */
																);
					g_DiagResponse.byD4 = (uint8) (C_DIAG_RES
#if _SUPPORT_DIAG_OC
													& ~(1U << 0)				/* bit 0: Diagnostic OC support */
#endif /* _SUPPORT_DIAG_OC */
#if (defined __MLX81315_A__) && _SUPPORT_QUADRUPLE_MOTOR_CURRENT
													& ~(1U << 1)				/* bit 1: Quadruple motor-current support (MMP141209-4) */
#endif /* (defined __MLX81315_A__) && _SUPPORT_QUADRUPLE_MOTOR_CURRENT */
#if _SUPPORT_DOUBLE_USTEP
													& ~(1U << 3)				/* bit 3: Double uStep support */
#endif /* _SUPPORT_DOUBLE_USTEP */
																);
					g_DiagResponse.byD5 = (uint8) C_DIAG_RES;
					g_u8BufferOutID = (uint8) QR_RFR_DIAG;						/* LIN Output buffer is valid (RFR_DIAG) */
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_MLX4_VERSION )
				{
					/* Get Platform version
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |   0xC7   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB | MLX4 F/W | MLX4 F/W |  Loader  |  Loader  | Reserved |
					 *	|     |     |      |   (LSB)  |   (MSB)  |   (LSB)  |   (MSB)  |   0xFF   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					StoreD1to4( *((uint16 *) 0x4018U), *((uint16 *) 0x401AU));
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_PLTF_VERSION )
				{
					/* Get Platform version
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |   0xC8   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB | PLTF Ver | PLTF ver | PLTF ver | PLTF ver | Reserved |
					 *	|     |     |      |  (Major) |  (Minor) |   (Rev)  |  (Build) |   0xFF   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					StoreD1to4( (__MLX_PLTF_VERSION_MAJOR__ | (__MLX_PLTF_VERSION_MINOR__ << 8)),
								(__MLX_PLTF_VERSION_REVISION__ | (__MLX_PLTF_VERSION_CUSTOMER_BUILD__ << 8)));
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_APP_VERSION )
				{
					extern const uint8 product_id[8];
					/* Get Application version
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier |   Index  | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |          |   0xFF   |   0xC9   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB | Appl ver | Appl ver | Appl ver | Appl ver | Appl ver |
					 *	|     |     |      |  (Major) |  (Minor) | (Rev LSB)| (Rev MSB)|   0xFF   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					if ( pDiag->byD3 == 0U )
					{
						StoreD1to4( (__APP_VERSION_MAJOR__ | (__APP_VERSION_MINOR__ << 8)), __APP_VERSION_REVISION__);
					}
					else if ( pDiag->byD3 == 1U )
					{
						StoreD1to4( *((uint16 *) &product_id[0]), *((uint16 *) &product_id[2]));
					}
					else if ( pDiag->byD3 == 2U )
					{
						StoreD1to4( *((uint16 *) &product_id[4]), *((uint16 *) &product_id[6]));
					}
					else
					{
						/* Nothing */
					}
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_MLXPAGE )
				{
					/* Get Melexis NVRAM page info
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier |   index  | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |          |   0xFF   |   0xCA   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB |   index  | NVRAM[i] | NVRAM[i] |NVRAM[i+1]|NVRAM[i+1]|
					 *	|     |     |      |          |   (LSB)  |   (MSB)  |   (LSB)  |   (MSB)  |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					uint16 u16Index = (uint16) (pDiag->byD3 & 0x3EU);
					g_DiagResponse.byD1 = (uint8) u16Index;
					{
						uint16 *pu16NvramData = ((uint16 *) C_ADDR_MLXF_PAGE) + u16Index;			/* NVRAM 16-bit pointer */
						StoreD2to5( pu16NvramData[0], pu16NvramData[1]);
					}
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_MLXPID )
				{
					/* Get PID info
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier |   index  | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |          |   0xFF   |   0xCB   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB | Reserved | CtrlRatio| CtrlRatio|   PID_I  |   PID_I  |
					 *	|     |     |      |          |   (LSB)  |   (MSB)  |   (LSB)  |   (MSB)  |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					StoreD2to5( g_u16PidCtrlRatio, g_u16PID_I);
				}
#if _SUPPORT_LOG_NVRAM
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_NVRAM_ERRORCODES )
				{
					/* Get NVRAM stored errorcode's[index .. index+3]
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier |   Index  | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |          |          |   0xCC   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB |   Index  | ErrorCode| ErrorCode| ErrorCode| ErrorCode|
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					uint16 u16Index = (uint16) (pDiag->byD3 & 0x1CU);
					if ( u16Index < (2U * (C_MAX_ERRORS_PER_PAGE - 1U)) )
					{
						uint16 *pu16ErrorCode;
						g_DiagResponse.byD1 = (uint8) (u16Index & 0xFFU);
						u16Index = u16Index >> 1;
						if ( u16Index < (C_MAX_ERRORS_PER_PAGE/2U) )
						{
							pu16ErrorCode = (uint16 *) &(((PNVRAM_ERRORLOG) (C_ADDR_USERPAGE1 + sizeof(NVRAM_USER)))->ErrorLog[u16Index]);
						}
						else
						{
							u16Index -= (C_MAX_ERRORS_PER_PAGE/2);
							pu16ErrorCode = (uint16 *) &(((PNVRAM_ERRORLOG) (C_ADDR_USERPAGE2 + sizeof(NVRAM_USER)))->ErrorLog[u16Index]);
						}
						StoreD2to5( *pu16ErrorCode, *(pu16ErrorCode+1U)); /*lint !e661 */
					}
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_CLR_NVRAM_ERRORCODES )
				{
					/* Clear NVRAM error-logging
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |          |          |   0xCD   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					NVRAM_ClearErrorLog();
				}
#endif /* _SUPPORT_LOG_NVRAM */
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_CHIPENV )
				{
					/* Temperature, Motor driver current, Supply-voltage
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   0xFF   |   0xFF   |   0xCE   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB | Chip-Temp|  Current |  Current |  Voltage |  Voltage |
					 *	|     |     |      |          |   (LSB)  |   (MSB)  |   (LSB)  |   (MSB)  |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					uint16 u16Value = (uint16) (g_i16ChipTemperature + C_TEMPOFF);					/* Chip Junction temperature + offset (C_TEMPOFF); Range: -C_TEMPOFF .. +(255-C_TEMPOFF) */
					g_DiagResponse.byD1 = (uint8) (u16Value & 0xFFU);
					StoreD2to5( (uint16) g_i16Current, (uint16) g_i16SupplyVoltage); /* Motor driver current [mA] & Supply voltage [10mV] */
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_FUNC )
				{
					/* Chip functions
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | Function | Function |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) | ID (LSB) | ID (MSB) |   0xCF   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * (No response)
					 */
					uint16 u16FunctionID = (((uint16) pDiag->byD4) << 8) | ((uint16) pDiag->byD3);

					if ( u16FunctionID == C_DBG_DBGFUNC_RESET )
					{
						/* Function ID = Chip reset */
						(void) mlu_ApplicationStop();
						MLX4_RESET();											/* Reset the Mlx4   */
						bistResetInfo = C_CHIP_STATE_LIN_CMD_RESET;
						MLX16_RESET();											/* Reset the Mlx16  */
					}
				}
				else if ( (pDiag->byD5 >= (uint8) C_DBG_SUBFUNC_SET_ANAOUTA) && (pDiag->byD5 <= (uint8) C_DBG_SUBFUNC_SET_ANAOUTH) )
				{
					/* Set ANA_OUT[A:H]
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier |   Value  |   Value  |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   (LSB)  |   (MSB)  | 0xD0-0xD7|
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					uint16 *pu16IoReg = (uint16*) au16AnaOutRegs[pDiag->byD5 & 0x07U];
					CONTROL |= (OUTA_WE | OUTB_WE | OUTC_WE);					/* Grant access to ANA_OUTx registers */
					*pu16IoReg = (((uint16) pDiag->byD4) << 8) | ((uint16) pDiag->byD3);
					CONTROL &= ~(OUTA_WE | OUTB_WE | OUTC_WE);
				}
#if _DEBUG_MOTOR_CURRENT_FLT
				else if ( pDiag->byD5 == (uint8) C_DBG_MOTOR_CURR_RAW )
				{
					/* Get Motor current (raw) data
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier |   Index  |   Index  |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   (LSB)  |   (MSB)  |   0xDC   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB | Curr[i]  | Curr[i+1]| Curr[i+2]| Curr[i+3]| Reserved |
					 *	|     |     |      |          |          |          |          |          |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					uint16 u16Index = (((uint16) pDiag->byD4) << 8) | ((uint16) pDiag->byD3);
					if ( u16Index == 0xFFFFU )
					{
						StoreD1to4( C_MOTOR_CURR_SZ, l_u16MotorCurrIdx);
					}
					else if ( u16Index <= (C_MOTOR_CURR_SZ - 4U) )
					{
						uint16 *pu16MotorCurrRaw = (uint16*) &l_au8MotorCurrRaw[u16Index];
						StoreD1to4( pu16MotorCurrRaw[0], pu16MotorCurrRaw[1]);
					}
					else
					{
						/* Nothing */
					}
				}
#endif /* _DEBUG_MOTOR_CURRENT_FLT */
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_FILLNVRAM )
				{
					/* Fill NVRAM
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier | NVRAM ID |  Pattern |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |          |          |   0xF8   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 * (No response)
					 */
					uint8 u8NvramID = pDiag->byD3;
					uint16 u16Pattern = (((uint16) pDiag->byD4) << 8) | ((uint16) pDiag->byD4);
					if ( (u8NvramID & 0x01U) != 0U )
					{
						/* Fill NVRAM #1, 1 */
						uint16 *pu16NvramData = ((uint16 *) BGN_NVRAM1_PAGE1_ADDRESS);
						do
						{
							*pu16NvramData++ = u16Pattern;
						} while (pu16NvramData < (uint16 *) END_NVRAM1_PAGE1_ADDRESS);
						NVRAM_SavePage( NVRAM1_PAGE1);
					}
					if ( (u8NvramID & 0x02) != 0U )
					{
						/* Fill NVRAM #1, 2 (Don't overwrite the NVRAM1 trim value) */
						uint16 *pu16NvramData = ((uint16 *) BGN_NVRAM1_PAGE2_ADDRESS);
						do
						{
							*pu16NvramData++ = u16Pattern;
						} while (pu16NvramData < (uint16 *) END_NVRAM1_PAGE2_ADDRESS);
						NVRAM_SavePage( NVRAM1_PAGE2);
					}
					if ( (u8NvramID & 0x04U) != 0U )
					{
						/* Fill NVRAM #2, 1 */
						uint16 *pu16NvramData = ((uint16 *) BGN_NVRAM2_PAGE1_ADDRESS);
						do
						{
							*pu16NvramData++ = u16Pattern;
						} while (pu16NvramData < (uint16 *) END_NVRAM2_PAGE1_ADDRESS);
						NVRAM_SavePage( NVRAM2_PAGE1);
					}
					if ( (u8NvramID & 0x80U) != 0U )
					{
						NVRAM_LoadUserPage();
					}
				}
				else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_GET_IO_REG )
				{
					/* Get I/O-register value (16-bits)
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| Debug| Supplier | Supplier |  I/O-reg |  I/O-reg |   FUNC   |
					 *	|     |     | 0xDB | ID (LSB) | ID (MSB) |   (LSB)  |   (MSB)  |   0xFD   |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *
					 * Response
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 *	| NAD | 0x06| 0xDB |  I/O-reg |  I/O-reg | I/O-value| I/O-value| Reserved |
					 *	|     |     |      |   (LSB)  |   (MSB)  |   (LSB)  |   (MSB)  |  (0xFF)  |
					 *	+-----+-----+------+----------+----------+----------+----------+----------+
					 */
					uint16 u16IoAddress = (((uint16) pDiag->byD4) << 8) | ((uint16) pDiag->byD3);
					if ( ((u16IoAddress >= 0x2000U) && (u16IoAddress <= 0x2056U)) ||	/* System I/O */
						  (u16IoAddress <= 0x07FEU) ||									/* System RAM */
						  ((u16IoAddress >= 0x2800U) && (u16IoAddress <= 0x28DAU)) ||	/* User I/O */
						  ((u16IoAddress >= 0x1000U) && (u16IoAddress <= 0x11FEU)) )	/* NVRAM */
					{
						StoreD1to4( u16IoAddress, *((uint16 *) u16IoAddress));
					}
				}
			}
			else
			{
				SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);			/* Status = Negative feedback */
			}
		}
#endif /* _SUPPORT_MLX_DEBUG_MODE */
		else if ( pDiag->bySID == (uint8) C_SID_MLX_ERROR_CODES )
		{
			/* Error-codes support (RAM-stored)
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xEC | Reserved | Reserved | Reserved | Reserved | Reserved |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *
			 * Response
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |    D1    |    D2    |    D3    |    D4    |    D5    |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xEC | Error[0] | Error[1] | Error[2] | Error[3] | Error[4] |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			g_DiagResponse.byNAD = g_u8NAD;
			g_DiagResponse.byPCI = 0x06U;
			g_DiagResponse.byRSID = (uint8) C_SID_MLX_ERROR_CODES;
			g_DiagResponse.byD1 = GetLastError();													/* Oldest Error-code */
			g_DiagResponse.byD2 = GetLastError();
			g_DiagResponse.byD3 = GetLastError();
			g_DiagResponse.byD4 = GetLastError();
			g_DiagResponse.byD5 = GetLastError();

			g_u8BufferOutID = (uint8) QR_RFR_DIAG;													/* LIN Output buffer is valid (RFR_DIAG) */
		}
		else if ( (pDiag->bySID == (uint8) C_SID_MLX_EE_PATCH) && ((FL_CTRL0 & FL_DETECT) == 0U) )
		{
			/* EEPROM/NVRAM Patch support
			 * D1.bit 7 = 0 : Read Patch area
			 *			  1 : Write Patch area
			 * D1.bit[6:0] : 16-bit data index. Valid 0x00 through 0x3D.
			 */
			uint16 u16Index = (uint16) (pDiag->byD1 & 0x3F);
			if ( (pDiag->byD1 & 0x80U) != 0U )
			{
				/* Write EEPROM/NVRAM Patch Page
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | 0x06| 0xED |  Write   | W[index] | W[index] |W[index+1]|W[index+1]|
				 *	|     |     |      |   Index  |  (LSB)   |  (MSB)   |   (LSB)  |   (MSB)  |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 * No Response
				 */
				uint16 *pu16NvramData = ((uint16 *) C_ADDR_PATCHPAGE) + u16Index;	/* NVRAM 16-bit pointer */
				*pu16NvramData = (((uint16) pDiag->byD3) << 8) | ((uint16) pDiag->byD2);
				pu16NvramData++;
				*pu16NvramData = (((uint16) pDiag->byD5) << 8) | ((uint16) pDiag->byD4);
			}
			else
			{
				/* Read EEPROM/NVRAM Patch Page
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | 0x06| 0xED |Read Index| Reserved | Reserved | Reserved | Reserved |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *
				 * Response
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | 0x06| 0xED |Read Index| R[index] | R[index] |R[index+1]|R[index+1]|
				 *	|     |     |      |          |  (LSB)   |  (MSB)   |   (LSB)  |   (MSB)  |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 */
				g_DiagResponse.byNAD = g_u8NAD;
				g_DiagResponse.byPCI = 0x06U;
				g_DiagResponse.byRSID = (uint8) C_SID_MLX_EE_PATCH;
				g_DiagResponse.byD1 = (uint8) u16Index;
				{
					uint16 *pu16NvramData = ((uint16 *) C_ADDR_PATCHPAGE) + u16Index;	/* NVRAM 16-bit pointer */
					StoreD2to5( pu16NvramData[0], pu16NvramData[1]);
				}
			}
		}
		else if ( pDiag->bySID == (uint8) C_SID_MLX_EE_USERPG1 )
		{
			/* EEPROM/NVRAM User-page #1 support
			 * D1.bit 7 = 0 : Read User-page #1
			 *			  1 : Write User-page #1
			 * D1.bit[6:0] : 16-bit data index. Valid 0x00 through 0x3F.
			 */
			uint16 u16Index = (uint16) (pDiag->byD1 & 0x3FU);
			if ( (pDiag->byD1 & 0x80U) != 0U )
			{
				/* Write EEPROM/NVRAM UserPage #1
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | 0x06| 0xEE |  Write   | W[index] | W[index] |W[index+1]|W[index+1]|
				 *	|     |     |      |   Index  |  (LSB)   |  (MSB)   |   (LSB)  |   (MSB)  |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 * No Response
				 */
				uint16 *pu16NvramData = ((uint16 *) &g_NvramUser) + u16Index;	/* NVRAM 16-bit pointer */
				pu16NvramData[0] = (((uint16) pDiag->byD3) << 8) | ((uint16) pDiag->byD2);
				pu16NvramData[1] = (((uint16) pDiag->byD5) << 8) | ((uint16) pDiag->byD4);
			}
			else
			{
				/* Read EEPROM/NVRAM UserPage #1
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | 0x06| 0xEE |Read Index| Reserved | Reserved | Reserved | Reserved |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *
				 * Response
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | 0x06| 0xEE |Read Index| R[index] | R[index] |R[index+1]|R[index+1]|
				 *	|     |     |      |          |  (LSB)   |  (MSB)   |   (LSB)  |   (MSB)  |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 */
				g_DiagResponse.byNAD = g_u8NAD;
				g_DiagResponse.byPCI = 0x06U;
				g_DiagResponse.byRSID = (uint8) C_SID_MLX_EE_USERPG1;
				g_DiagResponse.byD1 = (uint8) u16Index;
				{
					uint16 *pu16NvramData = ((uint16 *) &g_NvramUser) + u16Index;	/* NVRAM 16-bit pointer */
					StoreD2to5( pu16NvramData[0], pu16NvramData[1]);
				}
			}
		}
		else if ( pDiag->bySID == (uint8) C_SID_MLX_EE_STORE )
		{
			if ( pDiag->byD1 == (uint8) C_EE_STORE_USERPG1 )
			{
				/* Store RAM to NVRAM
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | 0x06| 0xEF |   0xEE   | Pages &  |          |          |          |
				 *	|     |     |      |          | ResetFlg |          |          |          |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 * No Response
				 */
				(void) NVRAM_Store( pDiag->byD2);
				if ( (pDiag->byD2 != 0xFFU) && ((pDiag->byD2 & C_NVRAM_USER_PAGE_RESET) != 0U) )
				{
					(void) mlu_ApplicationStop();
					MLX4_RESET();												/* Reset the Mlx4   */
					MLX16_RESET();												/* Reset the Mlx16  */
				}
			}
			else if ( (pDiag->byD1 == (uint8) C_EE_STORE_PATCH) && ((FL_CTRL0 & FL_DETECT) == 0U) )
			{
				/* Store Patch SRAM into NVRAM
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | PCI |  SID |    D1    |    D2    |    D3    |    D4    |    D5    |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 *	| NAD | 0x06| 0xEF |   0xED   |          |          |          |          |
				 *	|     |     |      |          |          |          |          |          |
				 *	+-----+-----+------+----------+----------+----------+----------+----------+
				 * No Response
				 */
				NVRAM_StorePatch();
			}
		}
	}
} /* End of HandleDfrDiag() */

#if _SUPPORT_MLX_DEBUG_MODE || ((LINPROT & LINXX) == LIN2J)
/* ****************************************************************************	*
 * RfrDiagReset()
 *
 * ****************************************************************************	*/
void RfrDiagReset()
{
#if ((LINPROT & LINXX) == LIN2X)
	if ( g_u8NAD != (uint8) C_BROADCAST_NAD )
#endif /* ((LINPROT & LINXX) == LIN2X) */
#if ((LINPROT & LINXX) == LIN2J)
	DFR_DIAG *pDiag = &g_LinCmdFrameBuffer.Diag;
	if ( pDiag->byNAD != C_BROADCAST_J2602_NAD )
#endif /* ((LINPROT & LINXX) == LIN2J) */
	{
		/* Positive Response */
		g_DiagResponse.byNAD = g_u8NAD;
		g_DiagResponse.byPCI = 0x06U;
#if (LINPROT == LIN2J_VALVE_VW)
		g_DiagResponse.byRSID = (uint8) (C_SID_RESET | C_RSID_OK);
#else  /* (LINPROT == LIN2J_VALVE_VW) */
		g_DiagResponse.byRSID = (uint8) C_SID_MLX_DEBUG;
#endif /* (LINPROT == LIN2J_VALVE_VW) */
		g_DiagResponse.byD5 = (uint8) g_NvramUser.Variant;
		StoreD1to4( C_SUPPLIER_ID, C_FUNCTION_ID);
	}
} /* End of RfrDiagReset() */
#endif /* _SUPPORT_MLX_DEBUG_MODE || ((LINPROT & LINXX) == LIN2J) */

#endif /* ((LINPROT & LINXX) == LIN2X) */

/* EOF */
