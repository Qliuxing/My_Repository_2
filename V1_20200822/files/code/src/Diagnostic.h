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

#ifndef SYSLIB_H_
#include "syslib.h"
#endif /* SYSLIB_H_ */
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

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
extern void DiagnosticsInit( void);
extern void HandleDiagnosticEvent( uint16 u16Event);

/* ****************************************************************************	*
 *	P u b l i c   v a r i a b l e s												*
 * ****************************************************************************	*/
#pragma space nodp
extern uint8 g_u8HallSwitchState;
extern uint16 g_u16HallMicroStepIdx;
#pragma space none

#endif /* DIAGNOSTIC_H_ */

/* EOF */
