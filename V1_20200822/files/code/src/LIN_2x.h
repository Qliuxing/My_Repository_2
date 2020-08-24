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

#ifndef LIN_2X_H_
#define LIN_2X_H_

#include <syslib.h>
#include <lin.h>

#define C_DEFAULT_NAD						0x7FU								/* Default (unprogrammed) NAD */
#define C_BROADCAST_NAD						0x7FU								/* Broadcast NAD */
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)										/* MMP150125-1 - Begin */
#define C_INVALD_GAD						0xFF								/* Default/not-programmed Group-address */
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */								/* MMP150125-1 - End */

/* Supplier ID */
#define C_WILDCARD_SUPPLIER_ID				0x7FFFU
#define C_MLX_SUPPLIER_ID					((('M'-'@')<<10) | (('L'-'@')<<5) | ('X'-'@'))	/* MeLeXis (test) */
#define C_SUPPLIER_ID						C_MLX_SUPPLIER_ID

/* Function ID */
#define C_WILDCARD_FUNCTION_ID				0xFFFFU
#if (LINPROT == LIN2X_ACT44)
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)										/* MMP150125-1 - Begin */
#define C_FUNCTION_ID						0x0002U								/* Function-ID for a Group-Actuator */
#else  /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
#define C_FUNCTION_ID						0x0001U								/* Function-ID for an Actuator */
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */								/* MMP150125-1 - End */
#endif /* (LINPROT == LIN2X_ACT44) */

/* Variant ID */
#if (LINPROT == LIN2X_ACT44)
#define C_VARIANT							0x01U								/* First version ID */
#endif /* (LINPROT == LIN2X_ACT44) */

#define C_SW_REF							0x88U								/* UniROM SW revision 1.0 */
#define C_HW_REF							0x10U								/* HW Revision 1.0 */
#if (LINPROT == LIN2X_ACT44)
#define C_PROJECT_ID						0x01U								/* MES-EVO */
#endif /* (LINPROT == LIN2X_ACT44) */

#define MSG_CONTROL							0x0001U								/* Actuator Control Message-ID */
#define MSG_STATUS							0x0002U								/* Actuator Status Message-ID */
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)										/* MMP150125-1 - Begin */
#define MSG_GROUP_CONTROL					0x0003U								/* Actuator Group Control Message-ID */
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */								/* MMP150125-1 - End */

#define C_MIN_POS							0x0000U
#define C_MAX_POS							0xFFFEU

#define C_LINAA_TIMEOUT						40U									/* LIN-AutoAddressing time-out of 40 sec */

#define mlxCONTROL							0xC1U								/* Actuator Control Frame-ID */
#define mlxSTATUS							0x42U								/* Actuator Status Frame-ID */
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)										/* MMP150125-1 - Begin */
#define mlxGROUP_CONTROL					0x03U								/* Actuator Group-Control Frame-ID */
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */								/* MMP150125-1 - End */

#define QR_INVALID							0xFFU

#if (LINPROT == LIN2X_ACT44)
typedef struct _ACT_CTRL							/* Description of ACT_CONTROL LIN-Frame */
{
	uint16 byNAD				: 8;				/* Byte 1 */
	uint16 byProgram			: 2;				/* Byte 2 */
#define C_CTRL_PROGRAM_DIS			0U				/* Store in NVRAM: Disabled */
#define C_CTRL_PROGRAM_ENA			1U				/* Store in NVRAM: Enabled */
#define C_CTRL_PROGRAM_RES			2U				/* Store in NVRAM: Reserved */
#define C_CTRL_PROGRAM_INV			3U				/* Store in NVRAM: Invalid */
	uint16 byStallDetector		: 2;
#define C_CTRL_STALLDET_DIS			0U				/* Stall-detector: Disabled */
#define C_CTRL_STALLDET_ENA			1U				/* Stall-detector: Enabled */
#define C_CTRL_STALLDET_RES			2U				/* Stall-detector: Reserved */
#define C_CTRL_STALLDET_INV			3U				/* Stall-detector: Invalid */
	uint16 byClearEventFlags	: 4;
#define C_CTRL_CLREVENT_NONE		0x0U			/* Clear-Event: None */
#define C_CTRL_CLREVENT_RESET		0x8U			/* Clear-Event: Reset */
#define C_CTRL_CLREVENT_STALL		0x4U			/* Clear-Event: Stall */
#define C_CTRL_CLREVENT_EMRUN		0x2U			/* Clear-Event: Emergency-run */
#define C_CTRL_CLREVENT_RES			0x1U			/* Clear-Event: Reserved */
#define C_CTRL_CLREVENT_INV			0xFU			/* Clear-Event: Invalid */
	uint16 byHoldingCurrent		: 2;				/* Byte 3 */
#define C_CTRL_MHOLDCUR_DIS			0U				/* Motor holding-current: Disabled */
#define C_CTRL_MHOLDCUR_ENA			1U				/* Motor holding-current: Enabled */
#define C_CTRL_MHOLDCUR_RES			2U				/* Motor holding-current: Reserved */
#define C_CTRL_MHOLDCUR_INV			3U				/* Motor holding-current: Invalid */
	uint16 byPositionType			: 2;
#define C_CTRL_POSITION_TARGET		0U				/* Position: Target */
#define C_CTRL_POSITION_INITIAL		1U				/* Position: Initial */
#define C_CTRL_POSITION_NONE		2U				/* Position: None */
#define C_CTRL_POSITION_INV			3U				/* Position: Invalid */
	uint16 bySpeed				: 4;
#define C_CTRL_SPEED_RES			0U				/* Motor speed: Reserved */
#define C_CTRL_SPEED_LOW			1U				/* Motor speed: 1 */
#define C_CTRL_SPEED_MID_LOW		2U				/* Motor speed: 2 */
#define C_CTRL_SPEED_MID			3U				/* Motor speed: 3 */
#define C_CTRL_SPEED_MID_HIGH		4U				/* Motor speed: 4 */
#define C_CTRL_SPEED_HIGH			5U				/* Motor speed: 5 */
/* #define C_CTRL_SPEED_RES			6..14	*/		/* Motor speed: Reserved */
#define C_CTRL_SPEED_INV			15U				/* Motor speed: Invalid */
	uint16 byTargetPositionLSB	: 8;				/* Byte 4 */
	uint16 byTargetPositionMSB	: 8;				/* Byte 5 */
	uint16 byStartPositionLSB	: 8;				/* Byte 6 */
	uint16 byStartPositionMSB	: 8;				/* Byte 7 */
	uint16 byEmergencyRun		: 2;				/* Byte 8 */
#define C_CTRL_EMRUN_DIS			0U				/* Emergency-run: Disabled */
#define C_CTRL_EMRUN_ENA			1U				/* Emergency-run: Enabled */
#define C_CTRL_EMRUN_RES			2U				/* Emergency-run: Reserved */
#define C_CTRL_EMRUN_INV			3U				/* Emergency-run: Invalid */
	uint16 byEmergencyEndStop	: 2;
#define C_CTRL_ENRUN_ENDSTOP_LO		0U				/* Emergency-run End-stop: Low */
#define C_CTRL_ENRUN_ENDSTOP_HI		1U				/* Emergency-run End-stop: High */
#define C_CTRL_ENRUN_ENDSTOP_RES	2U				/* Emergency-run End-stop: Reserved */
#define C_CTRL_ENRUN_ENDSTOP_INV	3U				/* Emergency-run End-stop: Invalid */
	uint16 byRotationDirection	: 2;
#define C_CTRL_DIR_CW				0U				/* Rotational-direction: ClockWise */
#define C_CTRL_DIR_CCW				1U				/* Rotational-direction: CounterClockWise */
#define C_CTRL_DIR_RES				2U				/* Rotational-direction: Reserved */
#define C_CTRL_DIR_INV				3U				/* Rotational-direction: Invalid */
	uint16 byStopMode			: 2;
#define C_CTRL_STOPMODE_NORMAL		0U				/* Stop-mode: Normal */
#define C_CTRL_STOPMODE_STOP		1U				/* Stop-mode: Stop */
#define C_CTRL_STOPMODE_RES			2U				/* Stop-mode: Reserved */
#define C_CTRL_STOPMODE_INV			3U				/* Stop-mode: Invalid */
} ACT_CTRL;	

typedef struct _ACT_STATUS							/* Description of ACT_STATUS LIN-Frame */
{
	uint16 byResponseError		: 1;				/* Byte 1 */
#define C_STATUS_ERROR_NONE			0U				/* Error: None */
#define C_STATUS_ERROR				1U				/* Error: Available */
	uint16 byReserved1			: 1;
	uint16 byOverTemperature	: 2;
#define C_STATUS_OTEMP_NO			0U				/* Over-temperature: No */
#define C_STATUS_OTEMP_YES			1U				/* Over-temperature: Yes */
#define C_STATUS_OTEMP_RES			2U				/* Over-temperature: Reserved */
#define C_STATUS_OTEMP_INV			3U				/* Over-temperature: Invalid */
	uint16 byElectricDefect		: 2;
#define C_STATUS_ELECDEFECT_NO		0U				/* Electric-defect: No */
#define C_STATUS_ELECDEFECT_YES		1U				/* Electric-defect: Yes */
#define C_STATUS_ELECDEFECT_PERM	2U				/* Electric-defect: Yes, permanent */
#define C_STATUS_ELECDEFECT_INV		3U				/* Electric-defect: Invalid */
	uint16 byVoltageError		: 2;
#define C_STATUS_VOLTAGE_OK			0U				/* Voltage-range: Ok */
#define C_STATUS_VOLTAGE_UNDER		1U				/* Voltage-range: Under-voltage */
#define C_STATUS_VOLTAGE_OVER		2U				/* Voltage-range: Over-voltage */
#define C_STATUS_VOLTAGE_INV		3U				/* Voltage-range: Invalid */
	uint16 byEmergencyOccurred	: 2;				/* Byte 2 */
#define C_STATUS_EMRUNOCC_NO		0U				/* Emergency-run occurred: No */
#define C_STATUS_EMRUNOCC_YES		1U				/* Emergency-run occurred: Yes */
#define C_STATUS_EMRUNOCC_RES		2U				/* Emergency-run occurred: Reserved */
#define C_STATUS_EMRUNOCC_INV		3U				/* Emergency-run occurred: Invalid */
	uint16 byStallDetector		: 2;
#define C_STATUS_STALLDET_DIS		0U				/* Stall Detector: Disabled */
#define C_STATUS_STALLDET_ENA		1U				/* Stall Detector: Enabled */
#define C_STATUS_STALLDET_RES		2U				/* Stall Detector: Reserved */
#define C_STATUS_STALLDET_INV		3U				/* Stall Detector: Invalid */
	uint16 byStallOccurred		: 2;
#define C_STATUS_STALLOCC_NO		0U				/* Stall occurred: No */
#define C_STATUS_STALLOCC_YES		1U				/* Stall occurred: Yes */
#define C_STATUS_STALLOCC_RES		2U				/* Stall occurred: Reserved */
#define C_STATUS_STALLOCC_INV		3U				/* Stall occurred: Invalid */
	uint16 byReset				: 2;
#define C_STATUS_RESETOCC_NO		0U				/* Reset occurred: No */
#define C_STATUS_RESETOCC_YES		1U				/* Reset occurred: Yes */
#define C_STATUS_RESETOCC_RES		2U				/* Reset occurred: Reserved */
#define C_STATUS_RESETOCC_INV		3U				/* Reset occurred: Invalid */
	uint16 byHoldingCurrent		: 2;				/* Byte 3 */
#define C_STATUS_MHOLDCUR_DIS		0U				/* Motor holding-current: No */
#define C_STATUS_MHOLDCUR_ENA		1U				/* Motor holding-current: Yes */
#define C_STATUS_MHOLDCUR_RES		2U				/* Motor holding-current: Reserved */
#define C_STATUS_MHOLDCUR_INV		3U				/* Motor holding-current: Invalid */
	uint16 byPositionTypeStatus	: 2;
#define C_STATUS_POSITION_ACTUAL	0U				/* Valid position: Actual */
#define C_STATUS_POSITION_INIT		1U				/* Valid position: Initial */
#define C_STATUS_POSITION_NONE		2U				/* Valid position: None */
#define C_STATUS_POSITION_INV		3U				/* Valid position: Invalid */
	uint16 bySpeedStatus		: 4;
#define C_STATUS_SPEED_STOP			0U				/* Actual speed: Stop */
#define C_STATUS_SPEED_1			1U				/* Actual speed: 1 (2.25 RPM) */
#define C_STATUS_SPEED_2			2U				/* Actual speed: 2 (2.25 RPM .. 3.00 RPM) */
#define C_STATUS_SPEED_3			3U				/* Actual speed: 3 (3.00 RPM .. 4.00 RPM) */
#define C_STATUS_SPEED_4			4U				/* Actual speed: 4 (> 4.00 RPM) */
#define C_STATUS_SPEED_AUTO			5U				/* Actual speed: Auto */
#define C_STATUS_SPEED_INV			15U				/* Actual speed: Invalid */
	uint16 byActualPositionLSB	: 8;				/* Byte 4 */
	uint16 byActualPositionMSB	: 8;				/* Byte 5 */
	uint16 byActualRotationalDir : 2;				/* Byte 6 */
#define C_STATUS_ACT_DIR_CLOSING	0U				/* Actual/last direction is: Closing */
#define C_STATUS_ACT_DIR_OPENING	1U				/* Actual/last direction is: Opening */
#define C_STATUS_ACT_DIR_UNKNOWN	2U				/* Actual/last direction is: Unknown */
#define C_STATUS_ACT_DIR_INV		3U				/* Actual/last direction is: Invalid */
	uint16 bySelfHoldingTorque	: 2;
#define C_STATUS_HOLDING_TORQUE_DIS	0U				/* Self Holding Torque: Disabled */
#define C_STATUS_HOLDING_TORQUE_ENA	1U				/* Self Holding Torque: Enabled */
#define C_STATUS_HOLDING_TORQUE_RES	2U				/* Self Holding Torque: Reserved */
#define C_STATUS_HOLDING_TORQUE_INV	3U				/* Self Holding Torque: Invalid */
	uint16 bySpecialFunctionActive: 2;
#define C_STATUS_SFUNC_ACTIVE_NO	0U				/* Special Function Active: No */
#define C_STATUS_SFUNC1_ACTIVE_YES	1U				/* Special Function #1 Active: Yes */
#define C_STATUS_SFUNC_ACTIVE_RES	2U				/* Special Function Active: Reserved */
#define C_STATUS_SFUNC_ACTIVE_INV	3U				/* Special Function Active: Invalid */
	uint16 byReserved			: 2;
	uint16 byNAD				: 8;				/* Byte 7 */
	uint16 byEmergencyRun		: 2;				/* Byte 8 */
#define C_STATUS_EMRUN_DIS			0U				/* Emergency-run: Disabled */
#define C_STATUS_EMRUN_ENA			1U				/* Emergency-run: Enabled */
#define C_STATUS_EMRUN_RES			2U				/* Emergency-run: Reserved */
#define C_STATUS_EMRUN_INV			3U				/* Emergency-run: Invalid */
	uint16 byEmergencyRunEndStop : 2;
#define C_STATUS_EMRUN_ENDPOS_LO	0U				/* Emergency-run End-position: Low */
#define C_STATUS_EMRUN_ENDPOS_HI	1U				/* Emergency-run End-position: High */
#define C_STATUS_EMRUN_ENDPOS_RES	2U				/* Emergency-run End-position: Reserved */
#define C_STATUS_EMRUN_ENDPOS_INV	3U				/* Emergency-run End-position: Invalid */
	uint16 byRotationDirection	: 2;
#define C_STATUS_DIRECTION_CW		0U				/* Rotational direction: ClockWise */
#define C_STATUS_DIRECTION_CCW		1U				/* Rotational direction: Counter-ClockWise */
#define C_STATUS_DIRECTION_RES		2U				/* Rotational direction: Reserved */
#define C_STATUS_DIRECTION_INV		3U				/* Rotational direction: Invalid */
	uint16 byStopMode			: 2;
#define C_STATUS_STOPMODE_NORMAL	0U				/* Stop-mode: Normal */
#define C_STATUS_STOPMODE_STOP		1U				/* Stop-mode: Stop */
#define C_STATUS_STOPMODE_RES		2U				/* Stop-mode: Reserved */
#define C_STATUS_STOPMODE_DEGRADED	2U				/* Stop-mode: Degraded */
#define C_STATUS_STOPMODE_INV		3U				/* Stop-mode: Invalid */
} ACT_STATUS;	
#endif /* (LINPROT == LIN2X_ACT44) */


/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
void LIN_2x_Init( uint16 u16WarmStart);											/* LIN 2.x initialisation */
#if (LINPROT == LIN2X_ACT44)
#if (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE)										/* MMP150125-1 - Begin */
void HandleActCtrl( uint16 u16Group);											/* HVAC Actuator or AGS Control (with Group-support) */
#else  /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */
void HandleActCtrl( void);														/* HVAC Actuator or AGS Control */
#endif /* (_SUPPORT_HVAC_GROUP_ADDRESS != FALSE) */								/* MMP150125-1 - End */
void HandleActStatus( void);													/* HVAC Actuator or AGS Status */
#endif /* (LINPROT == LIN2X_ACT44) */
void HandleBusTimeout( void);													/* Bus time-out */
void LinAATimeoutControl( void);												/* LIN-AA time-out */
void LIN2x_ErrorHandling( ml_LinError Error);									/* LIN2x Error handling */

#pragma space dp
extern uint8 g_u8NAD;															/* Actual NAD */
#pragma space none

#pragma space nodp																/* __NEAR_SECTION__ */
extern uint8 g_u8LinAATimeout;													/* LIN-AA Timeout counter (seconds) */
extern volatile uint8 g_u8LinAAMode;											/* LIN-AA mode */
extern uint16 g_u16LinAATicker;													/* LIN-AA Ticker counter */
#pragma space none																/* __NEAR_SECTION__ */

#endif /* LIN_2X_H_ */

/* EOF */
