/*! \file		MotorStall.h
 *  \brief		MLX81310 Motor Stall handling
 *
 * \note		project MLX81310
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
 * Copyright (C) 2012-2013 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * ****************************************************************************	*/

#ifndef MOTOR_STALL_H_
#define MOTOR_STALL_H_

#ifndef SYSLIB_H_
#include "syslib.h"
#endif /* SYSLIB_H_ */

#define C_STALL_NOT_FOUND			0x00
#define C_STALL_BUSY				0x01
#define C_STALL_FOUND				0x02
/* Stall detector levels
 */
/* #define C_STALL_NOT_FOUND		0x00 */										/* No stall detected */
#define M_STALL_MODE				0x78										/* bit 6:3: Stall-mode */
#define C_STALL_FOUND_A				0x40										/* bit 6: Current(Amps) stall detected */
#define C_STALL_FOUND_B				0x20										/* bit 5: BEMF-based stall detected */
#define C_STALL_FOUND_H				0x10										/* bit 4: hall-sensor stall detected */
#define C_STALL_FOUND_O				0x08										/* bit 3: Current Oscillation stall detected */
#define C_MOVAVG_6STEP_SZ			8

#define C_MIN_MOTORCURRENT			10											/* Minimum current [ADC_LSB] */

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
extern void   MotorStallInitA( void);
extern uint16 MotorStallCheckA( void);
#if _SUPPORT_STALLDET_O
extern void   MotorStallInitO( void);
extern uint16 MotorStallCheckO( void);
#endif /* _SUPPORT_STALLDET_O */
#if _SUPPORT_STALLDET_H
extern void MotorStallInitH( void);
extern uint16 MotorStallCheckH( void);
#endif /* _SUPPORT_STALLDET_H */

/* ****************************************************************************	*
 *	P u b l i c   v a r i a b l e s												*
 * ****************************************************************************	*/
#pragma space dp
extern uint8  l_u8StallCountA;													/* only used for debugging */
#if _SUPPORT_STALLDET_O
extern uint8 l_u8StallCountO;													/* only used for debugging */
#endif /* _SUPPORT_STALLDET_O */
#pragma space none

#pragma space nodp
extern uint16 l_u16MotorCurrentStallThrshldxN;									/* Stall-detector current-threshold (x 4..16) */
extern uint16 g_u16CurrStallO;
#pragma space none

#endif /* MOTOR_STALL_H_ */

/* EOF */
