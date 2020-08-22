/*! ----------------------------------------------------------------------------
 * \file		MotorDriver.c
 * \brief		MLX81300 Motor driver tables
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
 * ****************************************************************************	*
 * Resources:
 *
 * ****************************************************************************	*/

/* Set-Test - Part A: Single FET ON */
extern uint8 const c_au8DrvCfgSelfTestA[8];
extern uint16 const c_au16DrvAdcSelfTestA[4][2];

extern uint8 const c_au8DrvCfgSelfTestB4[10];
extern int16 const c_ai16MicroStepVector4PH[SZ_MICRO_VECTOR_TABLE_4PH];

