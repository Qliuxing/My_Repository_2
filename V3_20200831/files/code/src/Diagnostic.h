/*! \file		Diagnostic.h
 *  \brief		MLX81300 Diagnostic handling
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
 * Copyright (C) 2012-2013 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * ****************************************************************************	*/

#ifndef DIAGNOSTIC_H_
#define DIAGNOSTIC_H_

#include "syslib.h"
#include "Build.h"

#if (_SUPPORT_DIAG_OC == FALSE) && (_SUPPORT_DIAG_OVT == FALSE)
#define C_DIAG_MASK			(XI4_UV | XI4_OV)
#elif (_SUPPORT_DIAG_OVT == FALSE)
#define C_DIAG_MASK			(XI4_UV | XI4_OV | XI4_OC_DRV)
#elif (_SUPPORT_DIAG_OC == FALSE)
#define C_DIAG_MASK			(XI4_OVT | XI4_UV | XI4_OV)
#else
#define C_DIAG_MASK			(XI4_OVT | XI4_UV | XI4_OV | XI4_OC_DRV)
#endif /* (_SUPPORT_DIAG_OVT != FALSE) */

#define C_UOV_DEBOUNCE_THR					6U
#define C_OVT_DEBOUNCE_THR					6U
#define C_DRIFT_DEBOUNCE_THR				5U

/* coil open debounce */
#define C_MIN_COIL_CURRENT					10U									/* ADC-LSB */
#define C_COIL_ZERO_CURRENT_COUNT			32U									/* 32 micro-steps */
#define C_COIL_OVER_CURRENT_COUNT			32U									/* 32 micro-steps */
#define C_COIL_CURRENT_START_DELAY			128U


/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
extern void DiagnosticsInit( void);

extern void MotorDiagnosticSelfTest( void);
void MotorDiagnosticVsupplyAndTemperature(void);
void MotorDiagnosticCheckInit(void);
uint8 MotorDiagnosticOpenCheck(void);


/* ****************************************************************************	*
 *	P u b l i c   v a r i a b l e s												*
 * ****************************************************************************	*/
#pragma space nodp
extern uint16 g_u16HallMicroStepIdx;
#pragma space none

#endif /* DIAGNOSTIC_H_ */

/* EOF */
