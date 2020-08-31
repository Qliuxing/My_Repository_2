/*! \file		PID_Control.h
 *  \brief		MLX81310 PID Controller handling
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

#ifndef PID_CONTROL_H_
#define PID_CONTROL_H_

#ifndef SYSLIB_H_
#include "syslib.h"
#endif /* SYSLIB_H_ */
#include "Build.h"

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
extern void PID_Init( void);
extern void PID_Control( void);
extern void VoltageCorrection( void);
extern void ThresholdControl( void);
#if _SUPPORT_AMBIENT_TEMP
extern void SelfHeatCompensation( void);
#endif /* _SUPPORT_AMBIENT_TEMP */

/* ****************************************************************************	*
 *	P u b l i c   v a r i a b l e s												*
 * ****************************************************************************	*/
#pragma space dp 																/* __TINY_SECTION__ */
extern uint16 g_u16PID_I;
#pragma space none																/* __TINY_SECTION__ */

#pragma space nodp																/* __NEAR_SECTION__ */
extern uint16 g_u16PidCtrlRatio;															
extern int16 g_i16PID_D;
extern int16 g_i16PID_E;
#if _SUPPORT_AMBIENT_TEMP
extern uint16 g_u16SelfHeating;													/* Chip elf-heating temperature increase */
extern uint16 g_u16SelfHeatingCounter;											/* Self-heating counter */
#endif /* _SUPPORT_AMBIENT_TEMP */
extern uint16 g_u16PidRunningThreshold;											/* Motor current threshold (running) */
extern uint16 g_u16PidRunningThresholdADC;										/* Motor current threshold (running) (ADC) */
extern uint16 g_u16PidHoldingThreshold;											/* Motor holding current threshold */
extern uint16 g_u16MotorRefVoltage;												/* Motor reference voltage */

#if _DEBUG_VOLTAGE_COMPENSATION
#define SZ_MOTOR_VOLT_COMP	64u
extern int16 l_ai16MotorVolt[SZ_MOTOR_VOLT_COMP];
extern uint16 u16MotorVoltIdx;
#endif /* _DEBUG_VOLTAGE_COMPENSATION */

#pragma space none																/* __NEAR_SECTION__ */

#endif /* PID_CONTROL_H_ */

/* EOF */
