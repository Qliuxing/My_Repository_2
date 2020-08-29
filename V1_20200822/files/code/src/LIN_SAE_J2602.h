/*! \file		LIN_SAE_J2602.h
 *  \brief		MLX81300 LIN SAE J2602 communication handling
 *				Based on SAE J2603-1 Revised SEP2005 standard
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-14
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

#ifndef LIN_SAE_J2602_H_
#define LIN_SAE_J2602_H_

#include <syslib.h>
#include <lin.h>

#define C_WILDCARD_SUPPLIER_ID				0x7FFFU
#define C_GM_SUPPLIER_ID					0x0124U
#define C_MLX_SUPPLIER_ID					((('M'-'@')<<10) | (('L'-'@')<<5) | ('X'-'@'))
#define C_SUPPLIER_ID						C_GM_SUPPLIER_ID

#define C_WILDCARD_FUNCTION_ID				0xFFFFU
#define C_GM_FUNCTION_ID					0x00B0U
#define C_MLX_FUNCTION_ID					((('V'-'@')<<10) | (('L'-'@')<<5) | ('V'-'@'))
#define C_FUNCTION_ID						C_GM_FUNCTION_ID

#define C_VARIANT							0xB0U

#define C_SVN								0x0001U

#define C_SW_REF							0x88U								/* UniROM SW revision 1.0 */
#define C_HW_REF							0x10U								/* HW Revision 1.0 */
#if (LINPROT == LIN2J_VALVE_GM)
#define C_PROJECT_ID						0xCBU								/* Sanhua GM (Water)VALVE */
#endif /* (LINPROT == LIN2J_HVAC_GM) */

#define C_MIN_J2602_NAD						0x60U				/* Lowest J2602 NAD */
#define C_MAX_J2602_NAD						0x6DU				/* Highest J2602 NAD */
#define C_STEP_J2602_NAD					0x01U				/* All NAD's */
#define C_DIAG_J2602_NAD					0x6EU				/* J2602 NAD may be used for 0x3C and 0x3E frame's */
#define C_DEFAULT_J2602_NAD					0x6CU				/* Default (not programmed) J2602 NAD */
#define C_BROADCAST_J2602_NAD				0x7FU

#define C_MIN_POS							0x00U
#define C_MAX_POS							0xFFU

#if (LINPROT == LIN2J_VALVE_GM)
#define mlxACT_CTRL							0x00U				/* Actuator CfrCtrl */
#define mlxACT_STATUS						0x01U				/* Actuator RfrSta */
#endif /* (LINPROT == LIN2J_VALVE_GM) */

/* Special SAE J2602 */
#define C_PCI_RESET								0x01U
#define C_SID_RESET								0xB5U			/* Reset */
#define C_SID_PCI_RESET							0x01B5U

//#define QR_RFR_DIAG							7
#define QR_INVALID							0xFFU


#if (LINPROT == LIN2J_VALVE_GM)
/* GM VALVE V1.0 */
typedef struct __attribute__((packed)) _ACT_CFR_INI
{
	uint8 byPositionLSB			: 8;							/* Byte 0 */
	uint8 byPositionMSB			: 2;							/* Byte 1.[1:0] */
	uint8 byReserved1_6			: 6;							/* Byte 1.[7:2] */
	uint8 byMovEn				: 1;							/* Byte 2.[0] */
#define C_CTRL_MOVE_DIS				0U							/* Move disabled */
#define C_CTRL_MOVE_ENA				1U							/* Move enabled */
	uint8 byTorqueLevel			: 2;							/* Byte 2.[2:1] */
#define C_CTRL_TORQUE_NO			0U							/* No Torque Value Requested */
#define C_CTRL_TORQUE_NOMINAL		1U							/* Nominal Torque */
#define C_CTRL_TORQUE_BOOST_20PCT	2U							/* 20% Boost */
#define C_CTRL_TORQUE_BOOST_40PCT	3U							/* 40% Boost */
	uint8 byStallEnable			: 1;							/* Byte 2.[3] */
#define C_CTRL_STALL_DISABLE		0U
#define C_CTRL_STALL_ENABLE			1U
	uint8 byReserved2_4			: 4;							/* Byte 2.[7:4] */
	uint8 byReserved3			: 4;							/* Byte 3 */
} ACT_CFR_CTRL, *PACT_CFR_CTRL;

typedef struct _ACT_RFR_STA
{
	uint8 byLinErr				: 1;							/* Byte 1.[0] */
#define C_STATUS_LIN_OK				0U
#define C_STATUS_LIN_ERR			1U
	uint8 byFaultState			: 4;							/* Byte 1.[4:1] */
#define C_STATUS_NO_FAULT						0U
#define C_STATUS_FAULT_OVER_VOLTAGE				1U
#define C_STATUS_FAULT_UNDER_VLOTAGE 			2U
#define C_STATUS_FAULT_COIL_SHORT				3U
#define C_STATUS_FAULT_COIL_OPEN				4U
#define C_STATUS_FAULT_OVER_TEMP_SHUTDOWN		5U
#define C_STATUS_FAULT_UNEXPECT_STALL			6U
#define C_STATUS_FAULT_INDETERMINATE			7U
//#define C_STATUS_FAULT_INVALID					8U
//#define C_STATUS_FAULT_INVALID					9U
#define C_STATUS_FAULT_OVER_TEMP_WARNING		10U
//#define C_STATUS_FAULT_INVALID					11U
//#define C_STATUS_FAULT_INVALID					12U
//#define C_STATUS_FAULT_INVALID					13U
//#define C_STATUS_FAULT_INVALID					14U
//#define C_STATUS_FAULT_INVALID					15U
	uint8 byMoveState			: 1;							/* Byte 1.[5] */
#define C_STATUS_MOVE_IDLE			0U
#define C_STATUS_MOVE_ACTIVE		1U
	uint8 byTorqueLevel			: 2;							/* Byte 1.[7:6] */
	uint8 byInitState			: 2;							/* Byte 2.[1:0] */
#define C_STATUS_NOT_INIT			0U
#define C_STATUS_INIT_BUSY			1U
#define C_STATUS_INIT_DONE			2U
#define C_STATUS_INIT_INV			3U
	uint8 byArcState			: 2;							/* Byte 2.[3:2] */
	uint8 byStallDetectStatus	: 1;							/* Byte 2.[4] */
	uint8 byReserved2_4			: 3;							/* Byte 2.[7:5] */
	uint8 byActPositionLSB		: 8;							/* Byte 3: Actual position */
	uint8 byActPositionMSB		: 2;							/* Byte 4: Actual position */
	uint8 byReserved4_6			: 6;							/* Byte 4:  */
} ACT_RFR_STA, *PACT_RFR_STA;
#endif /* (LINPROT == LIN2J_VALVE_GM) */

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
extern void LIN_SAE_J2602_Init( uint16 u16WarmStart);							/* LIN SAE J2602 initialisation */

#if (LINPROT == LIN2J_VALVE_GM)
extern void LIN_SAE_J2602_Store( void);
extern void HandleActCfrCtrl( void);											/* Actuator Initialisation */
extern void HandleActRfrSta( void);												/* Actuator Status response */
#endif /* (LINPROT == LIN2J_VALVE_GM) */
extern void HandleBusTimeout( void);											/* Bus time-out */
extern void LIN2J_ErrorHandling( ml_LinError Error);							/* LIN Error handling */

extern uint8 g_u8NAD;															/* Actual NAD */
#if (LINPROT != LIN2J_VALVE_GM)
extern uint8 g_u8SAE_ErrorFlags;												/* LIN communication error-flags */
extern uint8 g_u8SAE_SendErrorState;											/* Send error-state (Copy of g_u8SAE_ErrorFlags) */
#else  /* (LINPROT != LIN2J_VALVE_GM) */
extern uint8 g_u8SAE_SendErrorState;											/* Send error-state (Copy of g_u8ErrorCommunicatio) */
#endif /* (LINPROT != LIN2J_VALVE_GM) */

#endif /* LIN_SAE_J2602_H_ */

/* EOF */

