/*! \file		LIN_2x.h
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
 * Copyright (C) 2012-2012 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * ****************************************************************************	*/

#ifndef LIN_DIAGNOSTICS_H_
#define LIN_DIAGNOSTICS_H_

#include <syslib.h>
#include <lin.h>

#define mlxDFR_DIAG							0x10U				/* Demand for diagnostic */
#define mlxRFR_DIAG							0x11U				/* Response diagnostic */


/*
 *		+-----+-----+-----+----+----+----+----+----+
 *	SF  | NAD | PCI | SID | D1 | D2 | D3 | D4 | D5 |
 *		+-----+-----+-----+----+----+----+----+----+
 *			 /       \
 *			 7654 3210
 *	SF-Type: 0000 Length
 */
#define M_PCI_TYPE								0xF0U
#define C_PCI_SF_TYPE							0x00U
#define C_PCI_FF_TYPE							0x10U
#define C_PCI_CF_TYPE							0x20U
#define M_PCI_SF_LEN							0x0FU
#define M_PCI_FF_LEN256							0x0FU
#define M_PCI_CF_FRAMECOUNTER					0x0FU
/* Service Identifier (SID) for node configuration (0xB0-0xB7) */
#define C_PCI_REASSIGN_NAD						0x06U
#define C_SID_REASSIGN_NAD						0xB0U			/* (Optional) Reassign NAD */
#define C_PCI_SID_REASSIGN_NAD					0x06B0U
#define C_PCI_ASSIGN_FRAME_ID					0x06U
#define C_SID_ASSIGN_FRAME_ID					0xB1U			/* (Obsolete) Assign Message-ID to Frame-ID */
#define C_PCI_SID_ASSIGN_FRAME_ID				0x06B1U
#define C_PCI_READ_BY_ID						0x06U
#define C_SID_READ_BY_ID						0xB2U			/* (Mandatory) Read by identifier */
#define C_PCI_SID_READ_BY_ID					0x06B2U
#define C_LIN_PROD_ID								0x00U		/* (00, Mandatory) LIN Product Identification */
#define	C_SERIAL_NR_ID								0x01U		/* (01, Optional) Serial number */
#if (LINPROT == LIN2J_VALVE_GM)
#define C_SVN_ID									0x30U		/* (30, User) SVN */
#endif /* (LINPROT == LIN2J_VALVE_GM) */
#define C_PCI_CC_NAD							0x06U
#define C_SID_CC_NAD							0xB3U			/* (Optional) Conditional change NAD */
#define C_PCI_SID_CC_NAD						0x06B3U
#define C_PCI_DATA_DUMP							0x06U
#define C_SID_DATA_DUMP							0xB4U			/* (Optional) Data dump */
#define C_PCI_SID_DATA_DUMP						0x06B4U
#define C_PCI_STOP_ACTUATOR						0x06U
#define C_SID_STOP_ACTUATOR						0xB5U			/* Stop (all) actuators */
#define C_PCI_SID_STOP_ACTUATOR					0x06B5U
#define C_PCI_ASSIGN_NAD						0x06U
#define C_SID_ASSIGN_NAD						0xB5U			/* Assign NAD (Bus Shunt Method) */
#define C_PCI_SID_ASSIGN_NAD					0x06B5U
#define C_SNPD_METHOD_BSM							0x02U		/* SNPD Method: Bus Shunt Method */
#define C_SNPD_METHOD_BSM2							0xF1U		/* HVAC 4.5 SNPD Method: Bus Shunt Method 2 */
#define C_SNPD_SUBFUNC_START						0x01U		/* Start of SNPD */
#define C_SNPD_SUBFUNC_ADDR							0x02U		/* Address last not-addressed NAD */
#define C_SNPD_SUBFUNC_STORE						0x03U		/* Store new NAD */
#define C_SNPD_SUBFUNC_FINISH						0x04U		/* Finish SNPD */
#define M_SNPD_SUBFUNC								0x07U		/* Sub-function mask */
#define C_SNPD_METHOD_2								0x80U		/* Second method */
#define C_PCI_SAVE_CONFIG						0x01U
#define C_SID_SAVE_CONFIG						0xB6U			/* (Optional) Save configuration */
#define C_PCI_ASSIGN_GROUPADDRESS				0x06U			/* MMP150125-1 - Begin */
#define C_SID_ASSIGN_GROUPADDRESS				0xB6U			/* Assign Group-address (HVAC 4.7) */
#define C_PCI_SID_ASSIGN_GROUPADDRESS			0x06B6U			/* MMP150125-1 - End */
#define C_PCI_ASSIGN_FRAME_ID_RNG				0x06U
#define C_SID_ASSIGN_FRAME_ID_RNG				0xB7U			/* (Mandatory) Assign frame ID range */
#define C_PCI_SID_ASSIGN_FRAME_ID_RNG			0x06B7U

#define C_SID_WRITE_BY_ID						0xCBU
#define C_PCI_WRITE_BY_ID						0x06U
#define C_PCI_SID_WRITE_BY_ID					0x06CBU
#define C_RPCI_WRITE_BY_ID						0x0BU
#define C_RSID_WRITE_BY_ID						0x06U

/* *** MES *** */
#define C_SID_MES_GET_KEY						0x27U

/* *** MELEXIS *** */
#define C_SID_MLX_DEBUG							0xDBU			/* Debug Support */
#define C_DBG_SUBFUNC_SUPPORT						0x00U		/* Support 0xA0-0xFE (MMP140519-2) */
#define C_DBG_SUBFUNC_SUPPORT_A						0xD0FF		/* F, E, C, 7, 6, 5, 4, 3, 2, 1, 0 */
#define C_DBG_SUBFUNC_SUPPORT_B						0x0000		/* - */
#define C_DBG_SUBFUNC_SUPPORT_C						0xFF87		/* F, E, D, C, B, A, 9, 8, 7, 2, 1, 0 */
#define C_DBG_SUBFUNC_SUPPORT_D						0xC0FF		/* F, E, (C), (B), 7, 6, 5, 4, 3, 2, 1, 0 */
#define C_DBG_SUBFUNC_SUPPORT_E						0x0001		/* 0 */
#define C_DBG_SUBFUNC_SUPPORT_F						0x7D07		/* E, D, C, B, A, 8, 2, 1, 0 */
#define C_DBG_SUBFUNC_STALLDET						0x5DU		/* Stall Detector */
#define C_DBG_SUBFUNC_LINAA_0						0xA0U		/* LIN Auto-Addressing Test module - Ish2/Ish3 setting */
#define C_DBG_SUBFUNC_LINAA_1						0xA1U		/* LIN Auto-Addressing - Ishunt & flags */
#define C_DBG_SUBFUNC_LINAA_2						0xA2U		/* LIN Auto-Addressing - CM & DM #1 */
#define C_DBG_SUBFUNC_LINAA_3						0xA3U		/* LIN Auto-Addressing - CM & DM #2 */
#define C_DBG_SUBFUNC_LINAA_4						0xA4U		/* LIN Auto-Addressing - CM & DM #3 */
#define C_DBG_SUBFUNC_APPLSTATE						0xA5U		/* Application Status */
#define C_DBG_SUBFUNC_LIN_BAUDRATE					0xA6U		/* LIN Slave baudrate (MMP130810-1) */
#define C_DBG_SUBFUNC_RESTART_AUTO_BAUDRATE			0xA7U		/* Restart Auto baud-rate detection (MMP130828-1) */
#define C_DBG_SUBFUNC_ADC_RAW						0xACU		/* ADC Temperature & voltage (raw) */
#define C_DBG_SUBFUNC_AMBJENV						0xAEU		/* Ambient Environment */
#define C_DBG_SUBFUNC_PWM_IN						0xAFU		/* PWM-In period & duty-cycle */
#define C_DBG_SUBFUNC_MLX16_CLK						0xC0U		/* MLX16 Clock (MMP140527-1) */
#define C_DBG_SUBFUNC_CHIPID						0xC1U		/* Chip ID */
#define C_DBG_SUBFUNC_HWSWID						0xC2U		/* HW/SW ID */
#define C_DBG_SUBFUNC_DEBUG_OPTIONS					0xC5U		/* Get _DEBUG options (MMP140905-1) */
#define C_DBG_SUBFUNC_SUPPORT_OPTIONS				0xC6U		/* Get _SUPPORT options (MMP140904-1) */
#define C_DBG_SUBFUNC_MLX4_VERSION					0xC7U		/* MLX4 F/W & Loader (MMP140523-1) */
#define C_DBG_SUBFUNC_PLTF_VERSION					0xC8U		/* Get Platform version (MMP140519-1) */
#define C_DBG_SUBFUNC_APP_VERSION					0xC9U		/* Get application version (MMP140519-1) */
#define C_DBG_SUBFUNC_MLXPAGE						0xCAU		/* Melexis Calibration page */
#define C_DBG_SUBFUNC_MLXPID						0xCBU		/* Get PID info */
#define C_DBG_SUBFUNC_NVRAM_ERRORCODES				0xCCU		/* NVRAM error-codes */
#define C_DBG_SUBFUNC_CLR_NVRAM_ERRORCODES			0xCDU		/* Clear NVRAM error-codes */
#define C_DBG_SUBFUNC_CHIPENV						0xCEU		/* Chip Environment */
#define C_DBG_SUBFUNC_FUNC							0xCFU		/* Chip (Reset) Function */
#define C_DBG_DBGFUNC_RESET								((('R'-'@')<<10) | (('S'-'@')<<5) | ('T'-'@'))	/* Module Reset */
#define C_DBG_SUBFUNC_SET_ANAOUTA					0xD0U		/* Set ANA_OUTA */
#define C_DBG_SUBFUNC_SET_ANAOUTB					0xD1U		/* Set ANA_OUTB */
#define C_DBG_SUBFUNC_SET_ANAOUTC					0xD2U		/* Set ANA_OUTC */
#define C_DBG_SUBFUNC_SET_ANAOUTD					0xD3U		/* Set ANA_OUTD */
#define C_DBG_SUBFUNC_SET_ANAOUTE					0xD4U		/* Set ANA_OUTE */
#define C_DBG_SUBFUNC_SET_ANAOUTF					0xD5U		/* Set ANA_OUTF */
#define C_DBG_SUBFUNC_SET_ANAOUTG					0xD6U		/* Set ANA_OUTG */
#define C_DBG_SUBFUNC_SET_ANAOUTH					0xD7U		/* Set ANA_OUTH */
#if _DEBUG_MOTOR_CURRENT_FLT
#define C_DBG_MOTOR_CURR_RAW						0xDCU
#endif /* _DEBUG_MOTOR_CURRENT_FLT */
#define C_DBG_RD_TMTR								0xDEU		/* Read TM_TR register */
#define C_DBG_WR_TMTR								0xDFU		/* Write TM_TR register */
//#define C_DBG_SUBFUNC_FLASHTRIM					0xF0U		/* Flash-trimming support */
//#define C_DBG_SUBFUNC_FLASHCRC_CALC				0xF1U		/* Flash-CRC Calculation */
//#define C_DBG_SUBFUNC_FLASHCRC_RES				0xF2U		/* Flash-CRC Result */
#define C_DBG_SUBFUNC_FILLNVRAM						0xF8U		/* Fill NVRAM (MMP140407-1) */
#define	C_DBG_SUBFUNC_HTOL_A						0xFAU		/* HTOL-A Time & Temperature */
#define C_DBG_SUBFUNC_HTOL_B						0xFBU		/* HTOL-B Flash-readwindow & Supply */
#define C_DBG_SUBFUNC_CLR_FATAL_ERRORCODES			0xFCU		/* Clear Fatal-error logging */
#define C_DBG_SUBFUNC_GET_IO_REG					0xFDU		/* Read I/O-register */
#define C_DBG_SUBFUNC_FATAL_ERRORCODES				0xFEU		/* Fatal-error logging */
#define C_SID_MLX_ERROR_CODES					0xECU			/* Error Logging */
#define C_SID_MLX_EE_PATCH						0xEDU			/* EEPROM/NVRAM Patch Read/Write */
#define C_SID_MLX_EE_USERPG1					0xEEU			/* EEPROM/NVRAM UserPage #1 Read/Write */
#define C_SID_MLX_EE_STORE						0xEFU			/* EEPROM/NVRAM Store into */
#define C_EE_STORE_USERPG1							0xEEU
#define C_EE_STORE_PATCH							0xEDU
/*
 *		+-----+-----+------+----+----+----+----+----+
 *	SF  | NAD | PCI | RSID | D1 | D2 | D3 | D4 | D5 |
 *		+-----+-----+------+----+----+----+----+----+
 *			 /       \
 *			 7654 3210
 *	SF-Type: 0000 Length
 *
 *		+-----+-----+------+----+----+----+----+----+
 * NOK	| NAD | 0x03| 0x7F | SID|0x12|0xFF|0xFF|0xFF|
 *		+-----+-----+------+----+----+----+----+----+
 */
#define C_RSID_OK								0x40U			/* Positive feedback: SID | 0x40 */
#define C_RPCI_NOK								0x03U			/* Negative feedback length */
#define C_RSID_NOK								0x7FU			/* Negative feedback RSID */
/* Negative Response Codes */
#define C_ERRCODE_POSITIVE_RESPONSE					0x00U		/* Error-code: Positive Response */
#define C_ERRCODE_GENERAL_REJECT					0x10U		/* Error-code: General reject */
#define C_ERRCODE_SERVNOSUP							0x11U		/* Error-code: Service not supported */
#define C_ERRCODE_SFUNC_NOSUP						0x12U		/* Error-code: Sub-function not supported */
#define C_ERRCODE_INV_MSG_INV_SZ					0x13U		/* Error-code: Incorrect message length or invalid format */
#define C_ERRCODE_RESPONSE_TOO_LONG					0x14U		/* Error-code: Response too long */
#define C_ERRCODE_BUSY_REP_REQ						0x21U		/* Error-code: Busy repeat request */
#define C_ERRCODE_COND_SEQ							0x22U		/* Error-code: Conditions Not Correct */
#define C_ERRCODE_REQ_SEQ							0x24U		/* Error-code: Request Sequence error */
#define C_ERRCODE_NO_RESP_FROM_SUBNET				0x25U		/* Error-code: No response from sub-net component */
#define C_ERRCODE_FAIL_PREV_EXE_REQ					0x26U		/* Error-code: Failure prevents execution of requested action */
#define C_ERRCODE_REQ_OUT_OF_RANGE					0x31U		/* Error-code: Request out of range */
#define C_ERRCODE_SEC_ACC_DENIED					0x33U		/* Error-code: Security access denied */
#define C_ERRCODE_INV_KEY							0x35U		/* Error-code: Invalid Key */
#define C_ERRCODE_EXEED_NR_ATTEMPTS					0x36U		/* Error-code: Exeed number of attemps (to get security access) */
#define C_ERRCODE_REQ_TIME_DLY_NOT_EXP				0x37U		/* Error-code: Required time delay not expired */
#define C_ERRCODE_UP_DOWNLOAD_NOT_ACC				0x70U		/* Error-code: Upload/Download not accepted */
#define C_ERRCODE_TRANS_DATA_SUSP					0x71U		/* Error-code: Transfer data suspended */
#define C_ERRCODE_GEN_PROG_FAIL						0x72U		/* Error-code: General Programming Failure */
#define C_ERRCODE_WRONG_BLOCK_SEQ_CNT				0x73U		/* Error-code: Wrong block sequence counter */
#define C_ERRCODE_PENDING							0x78U		/* Error-code: Request Correctly Received / Response Pending */
#define C_ERRCODE_SUBFUNC_NOT_SUPP_IN_ACT_SESSION	0x7EU		/* Error-code: Sub-function not supported in active session */
#define C_ERRCODE_SERV_NOT_SUPP_IN_ACT_SESSION		0x7FU		/* Error-code: Service not supported in active session */
#define C_RPCI_REASSIGN_NAD						0x01U
#define C_RSID_REASSIGN_NAD						(C_SID_REASSIGN_NAD | C_RSID_OK)
#define C_RPCI_READ_BY_ID_00					0x06U			/* Response-PCI: LIN Product Identification */
#define C_RPCI_READ_BY_ID_01					0x05U			/* Response-PCI: Serial number */
#if (LINPROT == LIN2J_VALVE_GM)
#define C_RPCI_READ_BY_ID_30					0x05U			/* Response-PCI: SVN */
#endif /* (LINPROT == LIN2J_VALVE_GM) */
#define C_RSID_READ_BY_ID						(C_SID_READ_BY_ID | C_RSID_OK)
#define C_RPCI_ASSIGN_FRAME_ID					0x01U
#define C_RSID_ASSIGN_FRAME_ID					(C_SID_ASSIGN_FRAME_ID | C_RSID_OK)
#define C_RPCI_CC_NAD							0x01U
#define C_RSID_CC_NAD							(C_SID_CC_NAD | C_RSID_OK)
#define C_RPCI_DATA_DUMP						0x06U
#define C_RPCI_ASSIGN_VARIANT_ID				0x01U
#define C_RSID_DATA_DUMP						(C_SID_DATA_DUMP | C_RSID_OK)
#define C_RPCI_ASSIGN_NAD						0x01U
#define C_RSID_ASSIGN_NAD						(C_SID_ASSIGN_NAD | C_RSID_OK)
#define C_RPCI_STOP_ACTUATOR					0x01U
#define C_RSID_STOP_ACTUATOR					(C_SID_STOP_ACTUATOR | C_RSID_OK)
#define C_RPCI_SAVE_CONFIG						0x01U
#define C_RSID_SAVE_CONFIG						(C_SID_SAVE_CONFIG | C_RSID_OK)
#define C_RPCI_ASSIGN_FRAME_ID_RNG				0x01U
#define C_RSID_ASSIGN_FRAME_ID_RNG				(C_SID_ASSIGN_FRAME_ID_RNG | C_RSID_OK)
#define C_DIAG_RES								0xFFU			/* Reserved fields feedback */

#define QR_RFR_DIAG							0x07U

typedef struct _DFR_DIAG							/* Description of DFR_DIAGNOSTIC LIN-Frame */
{
	uint8 byNAD;
	uint8 byPCI;
	uint8 bySID;
	uint8 byD1;
	uint8 byD2;
	uint8 byD3;
	uint8 byD4;
	uint8 byD5;
} DFR_DIAG;	

typedef struct _RFR_DIAG							/* Description of RDR_DIAGNOSTIC LIN-Frame */
{
	uint8 byNAD;
	uint8 byPCI;
	uint8 byRSID;
	uint8 byD1;
	uint8 byD2;
	uint8 byD3;
	uint8 byD4;
	uint8 byD5;
} RFR_DIAG;	

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
void HandleDfrDiag( void);														/* Diagnostics */
#if _SUPPORT_MLX_DEBUG_MODE
extern void RfrDiagReset( void);
#endif /* _SUPPORT_MLX_DEBUG_MODE */

#pragma space nodp																/* __NEAR_SECTION__ */
extern uint16 g_u16DiagResponseTimeoutCount;									/* LIN Diagnostic response timeout */
#pragma space none																/* __NEAR_SECTION__ */

#endif /* LIN_DIAGNOSTICS_H_ */

/* EOF */
