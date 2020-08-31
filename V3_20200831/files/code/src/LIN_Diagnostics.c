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
#include "LIN_Diagnostics.h"
#include "LIN_Communication.h"

#include <lin.h>
#include "lin_internal.h"														/* LinFrame (MMP140417-1) */

#include <syslib.h>
#include <plib.h>

#include "MotorDriver.h"

#include <nvram.h>
#include "NVRAM_UserPage.h"														/* NVRAM Functions & Layout */

#include "app_version.h"														/* MMP140519-1 */
#include "ErrorCodes.h"															/* Error-logging support */

#include "Timer.h"
#include "private_mathlib.h"
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION ( TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	( NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp																/* __NEAR_SECTION__ */
#if _SUPPORT_MLX_DEBUG_MODE
uint16 l_u16FlashCRC = 0u;
#endif /* _SUPPORT_MLX_DEBUG_MODE */
uint8 l_u8SNPD_CycleCountComm = 0u;												/* Communication Cycle counter LIN-AA info */
#pragma space none																/* __NEAR_SECTION__ */

#if _SUPPORT_MLX_DEBUG_MODE
/*								 ANA_OUTA,ANA_OUTB,ANA_OUTC,ANA_OUTD,ANA_OUTE,ANA_OUTF,ANA_OUTG,ANA_OUTH */
const uint16 au16AnaOutRegs[] = {  0x201C,  0x201E,  0x2020,  0x204A,  0x204C,  0x204E,  0x28CC,  0x28CE};

const uint16 tMlxDbgSupport[] = {												/* MMP140519-2 - Begin */
	0xFFFE, 0x0007, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, C_DBG_SUBFUNC_SUPPORT_A, C_DBG_SUBFUNC_SUPPORT_B,
	C_DBG_SUBFUNC_SUPPORT_C, C_DBG_SUBFUNC_SUPPORT_D, C_DBG_SUBFUNC_SUPPORT_E, C_DBG_SUBFUNC_SUPPORT_F
};																				/* MMP140519-2 - End */
#endif /* _SUPPORT_MLX_DEBUG_MODE */

void SetupDiagResponse( uint8 u8NAD, uint8 u8SID, uint8 u8ResponseCode);
uint16 CheckSupplier( uint16 const u16SupplierID);
uint16 ValidSupplierFunctionID( uint16 const u16SupplierID, uint16 const u16FunctionID );

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
		g_DiagResponse.byD1 = (uint8) C_DIAG_RES;								/* Clear Pending feedback (MMP151130-1) */
		g_DiagResponse.byD2 = (uint8) C_DIAG_RES;								/* Clear Pending feedback (MMP151130-1) */
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



void handleReassignNAD(const DFR_DIAG *pDiag)
{
	uint16 buf[2];
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
	if ( ValidSupplierFunctionID( (pDiag->byD1) | ((uint16)(pDiag->byD2) << 8u), (pDiag->byD3) | ((uint16)(pDiag->byD4) << 8u)) != FALSE)
	{
#if ((LINPROT & LINXX) == LIN2J)
		if ( (pDiag->byD5 & (C_STEP_J2602_NAD - 1u)) != 0u )
		{
			/* TODO: Check response correct in case of invalid NAD */
			SetupDiagResponse( C_DEFAULT_NAD, pDiag->bySID, (uint8) C_ERRCODE_INV_MSG_INV_SZ);		/* Status = Negative feedback */
		}
		else
		{
			if((pDiag->byD5 >= C_MIN_J2602_NAD) && (pDiag->byD5 <= C_MAX_J2602_NAD))
			{
				g_u8NAD = pDiag->byD5;
				/* J2602: Fixed DNN SNPD */
				g_u8ControlFrameID = ((g_u8NAD & 0x0Fu) << 2u) + 0x00u;
				g_u8StatusFrameID = ((g_u8NAD & 0x0Fu) << 2u) + 0x01u;
				(void) ml_Disconnect();
				(void) ml_AssignFrameToMessageID( MSG_CONTROL, g_u8ControlFrameID);
				(void) ml_AssignFrameToMessageID( MSG_STATUS, g_u8StatusFrameID);
				(void) ml_Connect();
				/* setup positive response */
				SetupDiagResponse( C_DEFAULT_NAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE);/* Status = Positive feedback */
				/* Store NVRAM */
				buf[0] = (uint16)g_u8NAD | ((uint16)g_u8ControlFrameID << 8u);
				buf[1] = (uint16)g_u8StatusFrameID | ((uint16)C_VARIANT << 8u);
				(void)NVRAM_Write(0x1000, buf, 2);
			}
		}
#endif

#if ((LINPROT & LINXX) == LIN20)
		/* 0x00:goto sleep;0x01-0x7D:NAD;0x7E:functional NAD;0x7F:broadcast NAD;0x80 to 0xFF is free usage for user defined diagnostic */
		g_u8NAD = pDiag->byD5;
		/* Set up positive response */
		SetupDiagResponse( C_DEFAULT_NAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE);/* Status = Positive feedback */
		/* Store NVRAM */
		buf[0] = (uint16)g_u8NAD | ((uint16)g_u8ControlFrameID << 8u);
		buf[1] = (uint16)g_u8StatusFrameID | ((uint16)C_VARIANT << 8u);
		(void)NVRAM_Write(0x1000, buf, 2);
#endif
			/* J2602,LIN2.0:automatic save confifuration */
#if ( (LINPROT & LINXX) == LIN21 )
		g_u8NAD = pDiag->byD5;
		SetupDiagResponse( C_DEFAULT_NAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE);	/* Status = Positive feedback */
		
#endif

	}
}

void handleAssignFrameID(const DFR_DIAG *pDiag)
{
	uint16 wMessageID;
	uint16 buf[2];
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
	if ( CheckSupplier( (pDiag->byD1) | ((uint16)(pDiag->byD2) << 8u)) != FALSE )
	{
		wMessageID = ((uint16) pDiag->byD4 << 8u) | ((uint16) pDiag->byD3);
		if ( wMessageID == MSG_CONTROL )
		{
			g_u8ControlFrameID = pDiag->byD5;
			(void) ml_Disconnect();
			(void) ml_AssignFrameToMessageID( MSG_CONTROL, g_u8ControlFrameID);
			(void) ml_Connect();
			/* Control Frame-ID changed */
			SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE);	/* Status = Positive feedback */
			/* Store NVRAM */
			buf[0] = (uint16)g_u8NAD | ((uint16)g_u8ControlFrameID << 8u);
			buf[1] = (uint16)g_u8StatusFrameID | ((uint16)C_VARIANT << 8u);
			(void)NVRAM_Write(0x1000, buf, 2);
		}
		else if ( wMessageID == MSG_STATUS )
		{
			g_u8StatusFrameID = pDiag->byD5;
			(void) ml_Disconnect();
			(void) ml_AssignFrameToMessageID( MSG_STATUS, g_u8StatusFrameID);
			(void) ml_Connect();
			/* Status Frame-ID changed */
			SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE);	/* Status = Positive feedback */
			/* Store NVRAM */
			buf[0] = (uint16)g_u8NAD | ((uint16)g_u8ControlFrameID << 8u);
			buf[1] = (uint16)g_u8StatusFrameID | ((uint16)C_VARIANT << 8u);
			(void)NVRAM_Write(0x1000, buf, 2);
		}
		else
		{
			/* Wrong Message-ID */
			SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);			/* Status = Negative feedback */
		}
	}
}

void handleReadByIdentifier(const DFR_DIAG *pDiag)
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
	if ( ValidSupplierFunctionID( (pDiag->byD2) | ((uint16)(pDiag->byD3) << 8u), (pDiag->byD4) | ((uint16)(pDiag->byD5) << 8u)) != FALSE )
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
			g_DiagResponse.byD5 = (uint8) C_VARIANT;	/* (MMP160613-3) */
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
			StoreD1to4( C_SERIAL_NO_LSW, C_SERIAL_NO_MSW);	/* Serial-number */
		}
		else if ( pDiag->byD1 == (uint8) C_SVN_ID )
		{
			g_DiagResponse.byNAD = g_u8NAD;
			g_DiagResponse.byPCI = (uint8) C_RPCI_READ_BY_ID_30;
			g_DiagResponse.byRSID = (uint8) C_RSID_READ_BY_ID;
			StoreD1to4( C_SVN, 0xFFFFU);											/* Firmware SVN */
		}
		else if( pDiag->byD1 == (uint8) C_SW_VER_ID )
		{
			g_DiagResponse.byNAD = g_u8NAD;
			g_DiagResponse.byPCI = (uint8) C_RPCI_READ_BY_ID_32;
			g_DiagResponse.byRSID = (uint8) C_RSID_READ_BY_ID;
			StoreD1to4( C_SW_VER, 0xFFFFU);											/* Firmware Software version */
		}
#if ((LINPROT & LINXX) == LIN21)
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
			g_DiagResponse.byD1 = (uint8) g_u8NAD;						/* Stored NAD (NVRAM) */
			g_DiagResponse.byD2 = (uint8) g_u8ControlFrameID;	/* Frame-ID for Control-message */
			g_DiagResponse.byD3 = (uint8) g_u8StatusFrameID;	/* Frame-ID for Status-message */
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
			g_DiagResponse.byD1 = (uint8) C_SW_REF;					/* SW-reference */
			g_DiagResponse.byD2 = (uint8) C_HW_REF;					/* HW-reference */
			g_u8BufferOutID = (uint8) QR_RFR_DIAG;					/* LIN Output buffer is valid (RFR_DIAG) */
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
			StoreD1to2( C_CUSTOMER_ID );						/* Customer-ID */
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
			StoreD1to2( 0x100C );					/* Production Date */
		}
#endif /* ((LINPROT & LINXX) == LIN2X) */
		else
		{
			/* Identifier not supported */
			SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);	/* Status = Negative feedback */
		}
	}
			
}

/* not supported */
void handleConditionalChangeNAD(const DFR_DIAG *pDiag)
{
	/* (Optional) Conditional change NAD */
	/*
	 *	+-----+-----+-----+----------+----------+----------+----------+----------+
	 *	| NAD | PCI | SID |    D1	 |	  D2	|	 D3    |	D4	  |    D5	 |
	 *	+-----+-----+-----+----------+----------+----------+----------+----------+
	 *	| NAD | 0x06| 0xB3|Identifier|	 Byte	|	Mask   |  Invert  | New NAD  |
	 *	+-----+-----+-----+----------+----------+----------+----------+----------+
	 */
	/* Get the identifier of possible read by ID response and selected by Id */
	/* Extract the data byte selected by Byte */
	uint16 buf[2];
	uint8 u8SerialNo[4];
	uint8 u8DataByte = 0x00;
	uint8 u8Error = (uint8) C_ERR_NONE;
	
	if ( pDiag->byD1 == 0x00u ) /* Requested Id = LIN Product Identification */
	{
		if ( pDiag->byD2 == 1u )
		{
			u8DataByte = (uint8) ((uint16)C_SUPPLIER_ID & 0xFFu);				/* LSB of Supplier-ID */
		}
		else if ( pDiag->byD2 == 2u )
		{
			u8DataByte = (uint8) ((uint16)C_SUPPLIER_ID >> 8u);					/* MSB of Supplier-ID */
		}
		else if ( pDiag->byD2 == 3u )
		{
			u8DataByte = (uint8) ((uint16)C_FUNCTION_ID & 0xFFu);				/* LSB of Function-ID */
		}
		else if ( pDiag->byD2 == 4u )
		{
			u8DataByte = (uint8) ((uint16)C_FUNCTION_ID >> 8u); 					/*lint !e572 */	/* MSB of Function-ID */
		}
		else if ( pDiag->byD2 == 5u )
		{
			u8DataByte = C_VARIANT;
		}
		else
		{
			u8Error = (uint8) C_ERRCODE_INV_MSG_INV_SZ; 				/* Selected byte not in range, not valid => no response */
		}
	}
	else if ( pDiag->byD1 == 0x01u ) 									/* Requested Id = Serial number (optional) */
	{
		if ( (pDiag->byD2 == 0u) || (pDiag->byD2 > 4u) )
		{
			/* Selected byte not in range, not valid => no response */
			u8Error = (uint8) C_ERRCODE_INV_MSG_INV_SZ; 				/* Status = Invalid Format */
		}
		else
		{
			u8SerialNo[0] = (uint8)C_SERIAL_NO_LSW;
			u8SerialNo[1] = (uint8)((uint16)C_SERIAL_NO_LSW >> 8u);
			u8SerialNo[2] = (uint8)C_SERIAL_NO_MSW;
			u8SerialNo[3] = (uint8)((uint16)C_SERIAL_NO_MSW >> 8u);
			
			u8DataByte = u8SerialNo[pDiag->byD2 - 1u]; 					/* Serial-number[n] */
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
	}
	else
	{
		/* Do a bitwise XOR with Invert */
		u8DataByte ^= pDiag->byD4;

		/* Do a bitwise AND with Mask */
		u8DataByte &= pDiag->byD3;

		if ( u8DataByte == 0u )												/* Condition PASSED */
		{
			uint8 byInitialNAD = g_u8NAD;
			g_u8NAD = pDiag->byD5;
			/* NAD changed */
			SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE); /* Status = Positive feedback */
			/* Store NVRAM */
			buf[0] = (uint16)g_u8NAD | ((uint16)g_u8ControlFrameID << 8u);
			buf[1] = (uint16)g_u8StatusFrameID | ((uint16)C_VARIANT << 8u);
			(void)NVRAM_Write(0x1000, buf, 2);
		}
	}

}

/* not supported */
void handleDataDump(const DFR_DIAG *pDiag)
{

	uint16 buf[2];
	/* Assign Variant-ID, HW-Reference and SW-Reference (Supplier specification, not LIN specification) */
	/*
	 *	+-----+-----+-----+----------+----------+----------+----------+----------+
	 *	| NAD | PCI | SID |    D1	 |	  D2	|	 D3    |	D4	  |    D5	 |
	 *	+-----+-----+-----+----------+----------+----------+----------+----------+
	 *	| NAD | 0x06| 0xB4| Supplier | Supplier |  Variant |  HW-Ref  |  SW-Ref  |
	 *	|	  | 	|	  | ID (LSB) | ID (MSB) |	 ID    |	ID	  |    ID	 |
	 *	+-----+-----+-----+----------+----------+----------+----------+----------+
	 */
	if ( CheckSupplier( (pDiag->byD1) | ((uint16)(pDiag->byD2) << 8u)) != FALSE)
	{
		(void)NVRAM_Read(0x1001,buf,2);										/* little endian or big endian */
		if ( pDiag->byD3 != 0xFFu )
		{
			buf[0] = (buf[0] & 0xFF00u) | (pDiag->byD3);					/* Set new Variant-ID */
		}
		
		if ( pDiag->byD4 != 0xFFu )
		{
			buf[1] = (buf[1] & 0x00FFu) | ((uint16)pDiag->byD4 << 8u);										/* Set new HW-Reference */
		}

		if ( pDiag->byD5 != 0xFFu )
		{
			/* -=#=- Note: SW-Ref should not be changed by this function, but be reprogramming the flash */
			 buf[2] = (buf[1] & 0xFF00u) | (pDiag->byD5); 										/* Set new SW-Reference */
		}
		/* Variant-ID and/or HW-reference and/or SW-reference changed */
		SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE); /* Status = Positive feedback */
		/* Store NVRAM */
		(void)NVRAM_Write(0x1000, buf, 2);
		
	}

}

void handleTargetReset(const DFR_DIAG *pDiag)
{
	/* Reset Target */
	MLX4_RESET();														/* Reset the Mlx4	*/
	bistResetInfo = C_CHIP_STATE_LIN_CMD_RESET;
	MLX16_RESET();														/* Reset the Mlx16	*/
	/* This reset restart the chip as POR, and doesn't come back (no answer) */

}

void handleSaveConfig(const DFR_DIAG *pDiag)
{
	uint16 buf[2];
	/* Save Configuration */
	/*
	 *	+-----+-----+-----+----------+----------+----------+----------+----------+
	 *	| NAD | PCI | SID |    D1    |    D2    |    D3    |    D4    |    D5    |
	 *	+-----+-----+-----+----------+----------+----------+----------+----------+
	 *	| NAD | 0x01| 0xB6| 0xFF | 0xFF |  0xFF |  0xFF  |  0xFF |
	 *	+-----+-----+-----+----------+----------+----------+----------+----------+
	 *	Only LIN 2.1
	 */
	/* LIN2.1:After reception of the service and the NAD is correct the slave node shall response(not wait until the configuration is saved) */
	SetupDiagResponse( C_DEFAULT_NAD, pDiag->bySID, (uint8) C_ERRCODE_POSITIVE_RESPONSE);/* Status = Positive feedback */
	/* store to NVRAM */
	buf[0] = (uint16)g_u8NAD | ((uint16)g_u8ControlFrameID << 8u);
	buf[1] = (uint16)g_u8StatusFrameID | ((uint16)C_VARIANT << 8u);
	(void)NVRAM_Write(0x1000, buf, 2);
}

void handleAssignFrameIDRange(const DFR_DIAG *pDiag)
{
	uint16 u16NvramStoreResult;
	/* Assign frame ID range */
	/* Assign frame ID range is used to set or disable PIDs up to four frames.
	 * The request shall be structured as shown
	 *
	 *	+-----+-----+-----+----------+----------+----------+----------+----------+
	 *	| NAD | PCI | SID |    D1	 |	  D2	|	 D3    |	D4	  |    D5	 |
	 *	+-----+-----+-----+----------+----------+----------+----------+----------+
	 *	| NAD | 0x06| 0xB7|  start	 |	  PID	|	 PID   |	PID   |    PID	 |
	 *	|	  | 	|	  |  index	 |	(index) | (index+1)| (index+2)| (index+3)|
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
	if ( (pDiag->byD4 != 0xFFu) || (pDiag->byD5 != 0xFFu) || (pDiag->byD1 > 1u) || ((pDiag->byD1 == 1u) && (pDiag->byD3 != 0xFFu)) )
	{
		/* Negative feedback */
		SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);	/* Status = Negative feedback */
	}
	else
	{
		SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_PENDING);		/* Status = Pending */

		u16NvramStoreResult = (uint8)~C_NVRAM_STORE_OKAY;
		if ( pDiag->byD1 == 0u )
		{
			/* Starting with first message-index */
			if ( pDiag->byD2 != 0xFFu )
			{
				/* First Frame-ID is Status-message Frame-ID */
				g_u8StatusFrameID = pDiag->byD2;
				(void) ml_Disconnect();
				if ( g_u8StatusFrameID != 0x00u )				/* MMP130913-1 - Begin */
				{
					(void) ml_AssignFrameToMessageID( MSG_STATUS, g_u8StatusFrameID);
				}
				else
				{
					(void) ml_DisableMessage( MSG_STATUS);
				}														/* MMP130913-1 - End */
				u16NvramStoreResult = C_NVRAM_STORE_OKAY;
			}
			
			if ( pDiag->byD3 != 0xFFu )
			{
				/* Second Frame-ID is Control-message Frame-ID */
				g_u8ControlFrameID = pDiag->byD3;
				(void) ml_Disconnect();
				if ( g_u8ControlFrameID != 0x00u )				/* MMP130913-1 - Begin */
				{
					(void) ml_AssignFrameToMessageID( MSG_CONTROL, g_u8ControlFrameID);
				}
				else
				{
					(void) ml_DisableMessage( MSG_CONTROL);
				}														/* MMP130913-1 - End */
				u16NvramStoreResult = C_NVRAM_STORE_OKAY;
			}
		}
		else if ( pDiag->byD1 == 1u )
		{
			/* Second Frame-ID is Control-message Frame-ID */
			g_u8ControlFrameID = pDiag->byD2;
			(void) ml_Disconnect();
			if ( g_u8ControlFrameID != 0x00u )				/* MMP130913-1 - Begin */
			{
				(void) ml_AssignFrameToMessageID( MSG_CONTROL, g_u8ControlFrameID);
			}
			else
			{
				(void) ml_DisableMessage( MSG_CONTROL);
			}														/* MMP130913-1 - End */
			u16NvramStoreResult = C_NVRAM_STORE_OKAY;
		}
		else
		{
			
		}
		/* Assign frame ID range success */
		if ( u16NvramStoreResult == C_NVRAM_STORE_OKAY )
		{
			(void) ml_Connect();
			/* save configuration is implemented by save configuration service */
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

void handleWriteByIdentifier(const DFR_DIAG *pDiag)
{
	/* Write-by-Identifier (0xCB) */
	/* Request:
	 *	+-----+-----+------+----------+----------+----------+----------+----------+
	 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
	 *	+-----+-----+------+----------+----------+----------+----------+----------+
	 *	| NAD | 0x06| 0xCB |Identifier| ID Data1 | ID Data2 | ID Data3 | ID Data4 |
	 *	+-----+-----+------+----------+----------+----------+----------+----------+
	 *
	 * Response (OK):
	 *	+-----+-----+------+----------+----------+----------+----------+----------+
	 *	| NAD | PCI | RSID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
	 *	+-----+-----+------+----------+----------+----------+----------+----------+
	 *	| NAD | 0x06| 0x0B |Identifier| ID Data1 | ID Data2 | ID Data3 | ID Data4 |
	 *	+-----+-----+------+----------+----------+----------+----------+----------+
	 * Identifier: 00
	 * ID data 1-4
	 */
	uint16 u16SupplierID = (((uint16) pDiag->byD3) << 8u) | ((uint16) pDiag->byD2);
	uint16 u16ParamID = (((uint16) pDiag->byD5) << 8u) | ((uint16) pDiag->byD4);
	
	if ( pDiag->byD1 == (uint8) C_LIN_PROD_ID )
	{
		/* Write Function ID */
		if ( (u16SupplierID == C_SUPPLIER_ID) && (u16ParamID == C_FUNCTION_ID) )			/* MMP130626-1 */
		{
			/* Correct Supplier ID; Change Function ID allowed */
			/* Store NVRAM */
		
			{
				/* Function-ID changed:set up response */
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
		}
		else
		{
			/* Wrong Supplier ID (Wild-card not allowed) */
			SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_INV_MSG_INV_SZ);	/* Status = Negative feedback */
		}
	}
	else if ( pDiag->byD1 == (uint8) C_LIN_CUST_ID )
	{
		/* Write Customer ID */
		if ( u16SupplierID == C_SUPPLIER_ID )
		{
			/* Correct Supplier ID; Change Function ID allowed */
			/* Store NVRAM */
			{
				/* Customer-ID changed:set up response */
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
		}
		else
		{
			/* Wrong Supplier ID (Wild-card not allowed) */
			SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_INV_MSG_INV_SZ);	/* Status = Negative feedback */
		}
	}
	else if ( pDiag->byD1 == (uint8) C_PROD_DATE )
	{
		/* Write Customer ID */
		if ( u16SupplierID == C_SUPPLIER_ID )
		{
			/* Correct Supplier ID; Change Function ID allowed */
			/* Customer-ID changed:set up response */
			{
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
		}
		else
		{
			/* Wrong Supplier ID (Wild-card not allowed) */
			SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_INV_MSG_INV_SZ);	/* Status = Negative feedback */
		}
	}
	else
	{
		/* Identifier not supported */
		SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);			/* Status = Negative feedback */
	}

}

/* not tested for MISRA2012 */
#if _SUPPORT_MLX_DEBUG_MODE
void handleMLXDebug(const DFR_DIAG *pDiag)
{
	/*
	 *	+-----+-----+------+----------+----------+----------+----------+----------+
	 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
	 *	+-----+-----+------+----------+----------+----------+----------+----------+
	 *	| NAD | 0x06| Debug| Supplier | Supplier | Param #1 | Param #2 | Function |
	 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |			|		   |	ID	  |
	 *	+-----+-----+------+----------+----------+----------+----------+----------+
	 */
	uint16 u16SupplierID = (((uint16) pDiag->byD2) << 8) | ((uint16) pDiag->byD1);
	if ( u16SupplierID == C_SUPPLIER_ID )
	{
		/* MMP131024-1: Reply diagnostics response with NAD, length and RSID.*/
		g_DiagResponse.byNAD = g_u8NAD;
		g_DiagResponse.byPCI = 0x06;
		g_DiagResponse.byRSID = (uint8) C_SID_MLX_DEBUG;

		/* Function-ID and Description
		 * 0x00: Supported Function-ID's (MMP140519-2)
		 * -0x5D: Stall detector
		 * -0x5E: Get currents buffer (_SHRINK_CODE_SIZE == FALSE)
		 * -0xA0: LIN Auto-Addressing Test module - Ish2/Ish3 setting
		 * -0xA1: LIN-AA BSM Ishunt #1,2 & 3 and flags
		 * -0xA2: LIN-AA BSM Common-mode & Differential-mode levels #1
		 * -0xA3: LIN-AA BSM Common-mode & Differential-mode levels #2
		 * -0xA4: LIN-AA BSM Common-mode & Differential-mode levels #3
		 * -0xA5: Application State
		 * -0xA6: LIN Slave Baudrate
		 * -0xAC: ADC Temperature/Voltage sensor (raw)
		 * -0xAE: Ambient-environment: Temperature, S/W Build ID
		 * -0xAF: Get PWM-IN Period & Low-Time (_SHRINK_CODE_SIZE == FALSE)
		 * 0xC0: Get CPU-clock (MMP140527-1)
		 * 0xC1: Get Chip-ID
		 * 0xC2: Get HW/SW-ID of chip
		 * 0xC5: Get _DEBUG options (MMP140905-1)
		 * 0xC6: Get _SUPPORT options (MMP140904-1)
		 * 0xC7: MLX4 F/W & Loader (MMP140523-1)
		 * 0xC8: Get Platform version (MMP140519-1)
		 * 0xC9: Get application version (MMP140519-1)
		 * 0xCA: Get Melexis NVRAM page info
		 * -0xCB: Get PID g_u16PidCtrlRatio & g_u16PID_I
		 * 0xCC: Get NVRAM stored errorcode's
		 * 0xCD: Clear NVRAM error-logging
		 * -0xCE: Chip-environment: Temperature, Motor driver current, Supply-voltage
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
		if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_SUPPORT ) 			/* MMP140519-2 - Begin */
		{
			/* Get MLX Debug Support
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier |	 index	| Reserved |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |			|	0xFF   |   0x00   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 * Response
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xDB |   index  |MLX DBG[i]|MLX DBG[i]| Reserved | Reserved |
			 *	|	  | 	|	   |		  |   (LSB)  |	 (MSB)	|		   |		  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			uint16 u16Index = (uint16) (pDiag->byD3 & 0x0F);
			StoreD1to2( tMlxDbgSupport[u16Index]);
		}																/* MMP140519-2 - End */
		else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_MLX16_CLK )		/* MMP140527-1 - Begin */
		{
			/* Get MLX16 Clock
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |	 0xFF	|	0xFF   |   0xC0   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 * Response
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xDB |MLX16Clock|MLX16Clock| Reserved | Reserved | Reserved |
			 *	|	  | 	|	   |[kHz](LSB)|[kHz](MSB)|	 0xFF	|	0xFF   |   0xFF   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			uint16 u16RC_Clock = muldivU16_U16byU16byU16( (2048 + EE_OCLOCK), 1000, 2048);
			int16 i16Coef;
			/* ((dTemp * Gp) * 1000)/131072 --> ((dTemp * Gp) * 125)/16384 */
			//i16Coef = EE_GPCLOCK;

			/* ((dTemp * Gn) * 1000)/131072 --> ((dTemp * Gn) * 125)/16384 */
			i16Coef = EE_GNCLOCK;
			i16Coef = (125 * i16Coef);
			u16RC_Clock += muldivI16_I16byI16byI16( 25, i16Coef, 16384);
			StoreD1to2( (uint16) (mulU32_U16byU16( u16RC_Clock, ((PLL_CTRL >> 8) + 1)) >> 2));
		}																/* MMP140527-1 - End */
		else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_CHIPID )
		{
			/* Get Chip-ID
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier |	 index	| Reserved |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |			|	0xFF   |   0xC1   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 * Response
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xDB |   index  | NVRAM[i] | NVRAM[i] |NVRAM[i+1]|NVRAM[i+1]|
			 *	|	  | 	|	   |		  |   (LSB)  |	 (MSB)	|	(LSB)  |   (MSB)  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			uint16 u16Index = (uint16) (pDiag->byD3 & 0x02);
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
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |	 0xFF	|	0xFF   |   0xC2   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 * Response
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xDB | HW/SW ID | HW/SW ID | CPU-Clock| Reserved | Reserved |
			 *	|	  | 	|	   |   (LSB)  |   (MSB)  |			|	0xFF   |   0xFF   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			g_DiagResponse.byD3 = (uint8) MCU_PLL_MULT;
			StoreD1to2( *((uint16 *) C_ADDR_MLX_HWSWID));
		}
		else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_SUPPORT_OPTIONS )	/* MMP140904-1 - Begin */
		{
			/* Get _SUPPORT options
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |	 0xFF	|	0xFF   |   0xC6   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 * Response
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xDB | SUPPORT  | SUPPORT  |	SUPPORT |  SUPPORT | Reserved |
			 *	|	  | 	|	   |   (LSB)  | 		 |			|	(MSB)  |   0xFF   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			g_DiagResponse.byD1 = (uint8) (C_DIAG_RES
											& ~(1U << 0)				/* bit 0: NVRAM Backup support */

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

#if (MOTOR_PHASES == 4U) && (_SUPPORT_PWM_MODE != BIPOLAR_PWM_DOUBLE_MIRROR)
											& ~(1U << 1)				/* bit 1: Single P-FET-Switching support */
#endif /* (MOTOR_PHASES == 4) && (_SUPPORT_PWM_MODE != BIPOLAR_PWM_DOUBLE_MIRROR) */

#if (MOTOR_PHASES == 3U) && (_SUPPORT_TWO_PWM != FALSE)
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

#if _SUPPORT_DOUBLE_USTEP
											& ~(1U << 3)				/* bit 3: Double uStep support */
#endif /* _SUPPORT_DOUBLE_USTEP */
														);
			g_DiagResponse.byD5 = (uint8) C_DIAG_RES;
			g_u8BufferOutID = (uint8) QR_RFR_DIAG;						/* LIN Output buffer is valid (RFR_DIAG) */
		}																/* MMP140904-1 - End */
		else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_MLX4_VERSION )	/* MMP140523-1 - Begin */
		{
			/* Get Platform version
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |	 0xFF	|	0xFF   |   0xC7   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 * Response
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xDB | MLX4 F/W | MLX4 F/W |	Loader	|  Loader  | Reserved |
			 *	|	  | 	|	   |   (LSB)  |   (MSB)  |	 (LSB)	|	(MSB)  |   0xFF   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			StoreD1to4( *((uint16 *) 0x4018), *((uint16 *) 0x401A));
		}																/* MMP140523-1 - End */
		else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_PLTF_VERSION )	/* MMP140519-1 - Begin */
		{
			/* Get Platform version
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |	 0xFF	|	0xFF   |   0xC8   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 * Response
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xDB | PLTF Ver | PLTF ver | PLTF ver | PLTF ver | Reserved |
			 *	|	  | 	|	   |  (Major) |  (Minor) |	 (Rev)	|  (Build) |   0xFF   |
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
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier |	 Index	| Reserved |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |			|	0xFF   |   0xC9   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 * Response
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xDB | Appl ver | Appl ver | Appl ver | Appl ver | Appl ver |
			 *	|	  | 	|	   |  (Major) |  (Minor) | (Rev LSB)| (Rev MSB)|   0xFF   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			if ( pDiag->byD3 == 0 ) 								/* MMP140618-2 */
			{
				StoreD1to4( (__APP_VERSION_MAJOR__ | (__APP_VERSION_MINOR__ << 8)), __APP_VERSION_REVISION__);
			}
			else if ( pDiag->byD3 == 1 )								/* MMP140618-2 - Begin */
			{
				StoreD1to4( *((uint16 *) &product_id[0]), *((uint16 *) &product_id[2]));
			}
			else if ( pDiag->byD3 == 2 )
			{
				StoreD1to4( *((uint16 *) &product_id[4]), *((uint16 *) &product_id[6]));
			}
		}																/* MMP140519-1 - End */
		else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_MLXPAGE )
		{
			/* Get Melexis NVRAM page info
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier |	 index	| Reserved |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |			|	0xFF   |   0xCA   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 * Response
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xDB |   index  | NVRAM[i] | NVRAM[i] |NVRAM[i+1]|NVRAM[i+1]|
			 *	|	  | 	|	   |		  |   (LSB)  |	 (MSB)	|	(LSB)  |   (MSB)  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			uint16 u16Index = (uint16) (pDiag->byD3 & 0x3E);
			g_DiagResponse.byD1 = (uint8) u16Index;
			{
				uint16 *pu16NvramData = ((uint16 *) C_ADDR_MLXF_PAGE) + u16Index;			/* NVRAM 16-bit pointer */
				StoreD2to5( pu16NvramData[0], pu16NvramData[1]);
			}
		}
		else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_NVRAM_ERRORCODES )
		{
			/* Get NVRAM stored errorcode's[index .. index+3]
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier |	 Index	| Reserved |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |			|		   |   0xCC   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *
			 * Response
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xDB |   Index  | ErrorCode| ErrorCode| ErrorCode| ErrorCode|
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			uint16 u16Index = (uint16) (pDiag->byD3 & 0x1C);
			if ( u16Index < (2 * (C_MAX_ERRORS_PER_PAGE - 1)) )
			{
				uint16 *pu16ErrorCode;
				g_DiagResponse.byD1 = (uint8) (u16Index & 0xFF);
				u16Index = u16Index >> 1;

				pu16ErrorCode =  &((PNVRAM_ERRORLOG ) (C_ADDR_USERPAGE2 + C_NVRAM_ERRLOG_OFFSET))->ErrorLog[u16Index];
				
				StoreD2to5( *pu16ErrorCode, *(pu16ErrorCode + 1)); /*lint !e661 */
			}
		}
		else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_CLR_NVRAM_ERRORCODES )
		{
			/* Clear NVRAM error-logging
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |			|		   |   0xCD   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			NVRAM_ClearErrorLog();
		}
		else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_FUNC )
		{
			/* Chip functions
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier | Function | Function |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) | ID (LSB) | ID (MSB) |   0xCF   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 * (No response)
			 */
			uint16 u16FunctionID = (((uint16) pDiag->byD4) << 8) | ((uint16) pDiag->byD3);

			if ( u16FunctionID == C_DBG_DBGFUNC_RESET )
			{
				/* Function ID = Chip reset */
				(void) mlu_ApplicationStop();
				MLX4_RESET();											/* Reset the Mlx4	*/
				bistResetInfo = C_CHIP_STATE_LIN_CMD_RESET;
				MLX16_RESET();											/* Reset the Mlx16	*/
			}
#if _SUPPORT_LINCMD_CRASH
			else if ( u16FunctionID == C_CHIP_STATE_FATAL_CRASH_RECOVERY )
			{
extern uint16 stack;
				SET_PRIORITY( 0);										/* Protected mode, highest priority (0) */
				SET_STACK( &stack);
				__asm__( "mov yl, #01");
				__asm__( "jmpf __fatal");
			}
#endif /* _SUPPORT_LINCMD_CRASH */
#if _SUPPORT_LINCMD_WD_RST
			else if ( u16FunctionID == C_CHIP_STATE_WATCHDOG_RESET )
			{
				MLX16_RESET();											/* Reset the Mlx16	*/
			}
#endif /* _SUPPORT_LINCMD_WD_RST */
		}
		else if ( (pDiag->byD5 >= (uint8) C_DBG_SUBFUNC_SET_ANAOUTA) && (pDiag->byD5 <= (uint8) C_DBG_SUBFUNC_SET_ANAOUTH) )
		{
			/* Set ANA_OUT[A:H]
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier |	 Value	|	Value  |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |	 (LSB)	|	(MSB)  | 0xD0-0xD7|
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			uint16 *pu16IoReg = (uint16*) au16AnaOutRegs[pDiag->byD5 & 0x07];
			CONTROL |= (OUTA_WE | OUTB_WE | OUTC_WE);					/* Grant access to ANA_OUTx registers */
			*pu16IoReg = (((uint16) pDiag->byD4) << 8) | ((uint16) pDiag->byD3);
			CONTROL &= ~(OUTA_WE | OUTB_WE | OUTC_WE);
		}
		else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_FILLNVRAM )		/* MMP140407-1 - Begin */
		{
			/* Fill NVRAM
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier | NVRAM ID |  Pattern |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |			|		   |   0xF8   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 * (No response)
			 */
			uint8 u8NvramID = pDiag->byD3;
			uint16 u16Pattern = (((uint16) pDiag->byD4) << 8) | ((uint16) pDiag->byD4);
			if ( u8NvramID & 0x01 )
			{
				/* Fill NVRAM #1, 1 */
				uint16 *pu16NvramData = ((uint16 *) BGN_NVRAM1_PAGE1_ADDRESS);
				do
				{
					*pu16NvramData++ = u16Pattern;
				} while (pu16NvramData < (uint16 *) END_NVRAM1_PAGE1_ADDRESS);
				NVRAM_SavePage( NVRAM1_PAGE1);
			}
			if ( u8NvramID & 0x02 )
			{
				/* Fill NVRAM #1, 2 (Don't overwrite the NVRAM1 trim value) */
				uint16 *pu16NvramData = ((uint16 *) BGN_NVRAM1_PAGE2_ADDRESS);
				do
				{
					*pu16NvramData++ = u16Pattern;
				} while (pu16NvramData < (uint16 *) END_NVRAM1_PAGE2_ADDRESS);
				NVRAM_SavePage( NVRAM1_PAGE2);
			}
			if ( u8NvramID & 0x04 )
			{
				/* Fill NVRAM #2, 1 */
				uint16 *pu16NvramData = ((uint16 *) BGN_NVRAM2_PAGE1_ADDRESS);
				do
				{
					*pu16NvramData++ = u16Pattern;
				} while (pu16NvramData < (uint16 *) END_NVRAM2_PAGE1_ADDRESS);
				NVRAM_SavePage( NVRAM2_PAGE1);
			}
		}																/* MMP140407-1 - End */
#if (_DEBUG_FATAL != FALSE)
		else if ( (pDiag->byD5 == (uint8) C_DBG_SUBFUNC_CLR_FATAL_ERRORCODES) && ((FL_CTRL0 & FL_DETECT) != 0) )	/* MMP150603-2 */
		{
			/* Clear Fatal-handler error logging
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier | Reserved | Reserved |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |			|		   |   0xFC   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			uint16 *pu16NvramData = ((uint16 *) C_ADDR_FATALPAGE);		/* NVRAM 16-bit pointer */
			do
			{
				*pu16NvramData = 0x0000;
				pu16NvramData++;
			} while ( (uint16) pu16NvramData < (C_ADDR_FATALPAGE + 0x7C));
			NVRAM_StorePatch();
		}
#endif /* (_DEBUG_FATAL != FALSE) */
		else if ( pDiag->byD5 == (uint8) C_DBG_SUBFUNC_GET_IO_REG )
		{
			/* Get I/O-register value (16-bits)
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier |	I/O-reg |  I/O-reg |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |	 (LSB)	|	(MSB)  |   0xFD   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *
			 * Response
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xDB |  I/O-reg |  I/O-reg | I/O-value| I/O-value| Reserved |
			 *	|	  | 	|	   |   (LSB)  |   (MSB)  |	 (LSB)	|	(MSB)  |  (0xFF)  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			uint16 u16IoAddress = (((uint16) pDiag->byD4) << 8) | ((uint16) pDiag->byD3);
			if ( ((u16IoAddress >= 0x2000) && (u16IoAddress <= 0x2056)) ||	/* System I/O */
				  (u16IoAddress <= 0x07FE) ||								/* System RAM */
				  ((u16IoAddress >= 0x2800) && (u16IoAddress <= 0x28DA)) )
			{
				StoreD1to4( u16IoAddress, *((uint16 *) u16IoAddress));
			}
		}
#if (_DEBUG_FATAL != FALSE)
		else if ( (pDiag->byD5 == (uint8) C_DBG_SUBFUNC_FATAL_ERRORCODES) && ((FL_CTRL0 & FL_DETECT) != 0) )	/* MMP150603-2 */
		{
			/* Get Fatal-handler[index]: error-code, info and address
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI |  SID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| Debug| Supplier | Supplier |	 Index	| Reserved |   FUNC   |
			 *	|	  | 	| 0xDB | ID (LSB) | ID (MSB) |			|		   |   0xFE   |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *
			 * Response
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | PCI | RSID |	D1	  |    D2	 |	  D3	|	 D4    |	D5	  |
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 *	| NAD | 0x06| 0xDB |   Index  | ErrorCode|	 Info	|AddressLSB|AddressMSB|
			 *	+-----+-----+------+----------+----------+----------+----------+----------+
			 */
			uint16 u16Index = (uint16) (pDiag->byD3 & 0x1F);
			uint16 u16NVRAM_FatalCount = *((uint16 *) C_ADDR_FATALPAGE);
			if ( u16Index <= u16NVRAM_FatalCount )
			{
				g_DiagResponse.byD1 = (uint8) (u16Index & 0xFF);
				{
					uint16 *pu16NV = ((uint16 *) C_ADDR_FATALPAGE + (u16Index << 1));
					StoreD2to5( pu16NV[0], pu16NV[1]);
				}
			}
		}
#endif /* (_DEBUG_FATAL != FALSE) */
	}
	else
	{
		SetupDiagResponse( g_u8NAD, pDiag->bySID, (uint8) C_ERRCODE_SFUNC_NOSUP);			/* Status = Negative feedback */
	}

}

#endif /* _SUPPORT_MLX_DEBUG_MODE */


void handleReadErrorCodes(const DFR_DIAG *pDiag)
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
	g_DiagResponse.byPCI = 0x06;
	g_DiagResponse.byRSID = (uint8) C_SID_MLX_ERROR_CODES;
	g_DiagResponse.byD1 = GetLastError();													/* Oldest Error-code */
	g_DiagResponse.byD2 = GetLastError();
	g_DiagResponse.byD3 = GetLastError();
	g_DiagResponse.byD4 = GetLastError();
	g_DiagResponse.byD5 = GetLastError();

	g_u8BufferOutID = (uint8) QR_RFR_DIAG;													/* LIN Output buffer is valid (RFR_DIAG) */
}

void handleMLXEEPatch(const DFR_DIAG *pDiag)
{
	/* EEPROM/NVRAM Patch support
	 * D1.bit 7 = 0 : Read Patch area
	 *			  1 : Write Patch area
	 * D1.bit[6:0] : 16-bit data index. Valid 0x00 through 0x3D.
	 */
	uint16 u16Index = (uint16) (pDiag->byD1 & 0x3Fu);
	if ( (pDiag->byD1 & 0x80u) != 0u )
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
		*pu16NvramData = (((uint16) pDiag->byD3) << 8u) | ((uint16) pDiag->byD2);
		pu16NvramData++;
		*pu16NvramData = (((uint16) pDiag->byD5) << 8u) | ((uint16) pDiag->byD4);
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
		g_DiagResponse.byPCI = 0x06;
		g_DiagResponse.byRSID = (uint8) C_SID_MLX_EE_PATCH;
		g_DiagResponse.byD1 = (uint8) u16Index;
		{
			uint16 *pu16NvramData = ((uint16 *) C_ADDR_PATCHPAGE) + u16Index;	/* NVRAM 16-bit pointer */
			StoreD2to5( pu16NvramData[0], pu16NvramData[1]);
		}
	}
}

void handleMLXEEUserPage(const DFR_DIAG *pDiag)
{
	/* EEPROM/NVRAM User-page #1 support
	 * D1.bit 7 = 0 : Read User-page #1
	 *			  1 : Write User-page #1
	 * D1.bit[6:0] : 16-bit data index. Valid 0x00 through 0x3F.
	 */
	uint16 u16Index = (uint16) (pDiag->byD1 & 0x3Fu);
	if ( (pDiag->byD1 & 0x80u) != 0x00u )
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
		uint16 *pu16NvramData = ((uint16 *) C_ADDR_USERPAGE1) + u16Index;	/* NVRAM 16-bit pointer */
		pu16NvramData[0] = (((uint16) pDiag->byD3) << 8u) | ((uint16) pDiag->byD2);
		pu16NvramData[1] = (((uint16) pDiag->byD5) << 8u) | ((uint16) pDiag->byD4);
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
		g_DiagResponse.byPCI = 0x06;
		g_DiagResponse.byRSID = (uint8) C_SID_MLX_EE_USERPG1;
		g_DiagResponse.byD1 = (uint8) u16Index;
		{
			uint16 *pu16NvramData = ((uint16 *) C_ADDR_USERPAGE1) + u16Index;	/* NVRAM 16-bit pointer */
			StoreD2to5( pu16NvramData[0], pu16NvramData[1]);
		}
	}
}

void handleMLXEEStore(const DFR_DIAG *pDiag)
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
		if ( (pDiag->byD2 != 0xFFu) && ((pDiag->byD2 & C_NVRAM_USER_PAGE_RESET) != 0u) )
		{
			(void) mlu_ApplicationStop();
			MLX4_RESET();												/* Reset the Mlx4   */
			MLX16_RESET();												/* Reset the Mlx16  */
		}
	}
	else if ( (pDiag->byD1 == (uint8) C_EE_STORE_PATCH) && ((FL_CTRL0 & FL_DETECT) == 0u) )	/* MMP150603-2 */
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
	else
	{
		
	}
}

/* ****************************************************************************	*
 * Diagnostic
 * ****************************************************************************	*/
void HandleDfrDiag( void)
{
	DFR_DIAG *pDiag = &g_LinCmdFrameBuffer.Diag;
	uint16 u16DiagPCI_SID;

#if ((LINPROT & LINXX) == LIN21) || ((LINPROT & LINXX) == LIN22) 			/* LIN 2.1, LIN 2.2 */
	if ( pDiag->byNAD != 0x7Eu )
	{
		g_u8BufferOutID = (uint8) QR_INVALID;
	}
#endif 	/* LIN 2.1, LIN 2.2 */

	if ( pDiag->byNAD == 0x00u )	/* Other bytes should be 0xFF, and are ignored */
	{
		/* ACT_DFR_DIAG_SLEEP: Sleep request (Optional) */
	}
	/* assign NAD service shall be used with initial NAD */
	if((pDiag->byNAD == (uint8)C_DEFAULT_NAD) || (pDiag->byNAD == (uint8)C_BROADCAST_NAD))
	{
		u16DiagPCI_SID = (((uint16)pDiag->byPCI) << 8u) | ((uint16)pDiag->bySID);
		/* support padding */
		g_DiagResponse.byD1 = (uint8) C_DIAG_RES;
		g_DiagResponse.byD2 = (uint8) C_DIAG_RES;
		g_DiagResponse.byD3 = (uint8) C_DIAG_RES;
		g_DiagResponse.byD4 = (uint8) C_DIAG_RES;
		g_DiagResponse.byD5 = (uint8) C_DIAG_RES;
		if(u16DiagPCI_SID == C_PCI_SID_REASSIGN_NAD)
		{
			handleReassignNAD(pDiag);
		}
	}
	/* other service shall use configure NAD */	
	if ( (pDiag->byNAD == g_u8NAD) || (pDiag->byNAD == (uint8) C_BROADCAST_NAD) )
	{
		u16DiagPCI_SID = (((uint16)pDiag->byPCI) << 8u) | ((uint16)pDiag->bySID);

		/* support padding */
		g_DiagResponse.byD1 = (uint8) C_DIAG_RES;
		g_DiagResponse.byD2 = (uint8) C_DIAG_RES;
		g_DiagResponse.byD3 = (uint8) C_DIAG_RES;
		g_DiagResponse.byD4 = (uint8) C_DIAG_RES;
		g_DiagResponse.byD5 = (uint8) C_DIAG_RES;

		if ( (u16DiagPCI_SID == C_PCI_SID_STOP_ACTUATOR) && (pDiag->byD5 == 0xFEu) )
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
			if ( ValidSupplierFunctionID( (pDiag->byD1) | ((uint16)(pDiag->byD2) << 8u), ((uint16)pDiag->byD3) | ((uint16)(pDiag->byD4) << 8u)) != FALSE )
			{
				
			}
		}
#if ((LINPROT & LINXX) == LIN20)
		else if ( u16DiagPCI_SID == C_PCI_SID_ASSIGN_FRAME_ID )
		{
			handleAssignFrameID(pDiag);
		}
#endif /* ((LINPROT & LINXX) == LIN2X) */
		else if ( u16DiagPCI_SID == C_PCI_SID_READ_BY_ID )
		{
			handleReadByIdentifier(pDiag);
		}
#if ((LINPROT & LINXX) == LIN20)
		else if ( u16DiagPCI_SID == C_PCI_SID_CC_NAD )
		{
			handleConditionalChangeNAD(pDiag);
		}
		else if ( u16DiagPCI_SID == C_PCI_SID_DATA_DUMP )
		{
			handleDataDump(pDiag);
		}
#endif
#if ((LINPROT & LINXX) == LIN21)
		else if ( u16DiagPCI_SID == C_PCI_SID_SAVE_CONFIG)
		{
			handleSaveConfig(pDiag);
		}
		else if ( u16DiagPCI_SID == C_PCI_SID_ASSIGN_FRAME_ID_RNG )
		{
			handleAssignFrameIDRange(pDiag);
		}
#endif
		else if ( u16DiagPCI_SID == C_PCI_SID_WRITE_BY_ID )
		{
			handleWriteByIdentifier(pDiag);
		}
#if ((LINPROT & LINXX) == LIN2J)
		else if ( u16DiagPCI_SID == C_SID_PCI_RESET )							/* Targeted or Broadcast reset */
		{
			handleTargetReset(pDiag);	
		}
#endif /* ((LINPROT & LINXX) == LIN2J) */

#if _SUPPORT_MLX_DEBUG_MODE
		else if ( pDiag->bySID == C_SID_MLX_DEBUG )
		{
			handleMLXDebug(pDiag);
		}
#endif /* _SUPPORT_MLX_DEBUG_MODE */

		else if ( pDiag->bySID == (uint8) C_SID_MLX_ERROR_CODES )
		{
			handleReadErrorCodes(pDiag);
		}
		else if ( (pDiag->bySID == (uint8) C_SID_MLX_EE_PATCH) && ((FL_CTRL0 & FL_DETECT) == 0u) )	/* MMP150603-2 */
		{
			handleMLXEEPatch(pDiag);
		}
		else if ( pDiag->bySID == (uint8) C_SID_MLX_EE_USERPG1 )
		{
			handleMLXEEUserPage(pDiag);
		}
		else if ( pDiag->bySID == (uint8) C_SID_MLX_EE_STORE )
		{
			handleMLXEEStore(pDiag);
		}
		else
		{
			
		}
	}
} /* End of HandleDfrDiag() */

/* ****************************************************************************	*
 * RfrDiagReset()
 *
 * ****************************************************************************	*/
void RfrDiagReset(void)
{
	DFR_DIAG *pDiag = &g_LinCmdFrameBuffer.Diag;
	
	if ( pDiag->byNAD != C_BROADCAST_NAD )
	{
		/* Positive Response */
		g_DiagResponse.byNAD = g_u8NAD;
		g_DiagResponse.byPCI = 0x06;
		g_DiagResponse.byRSID = (uint8) (C_SID_RESET | C_RSID_OK);
		g_DiagResponse.byD5 = (uint8) C_VARIANT;
		StoreD1to4( C_SUPPLIER_ID, C_FUNCTION_ID);
	}
} /* End of RfrDiagReset() */

/* EOF */
