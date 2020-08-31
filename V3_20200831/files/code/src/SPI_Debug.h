/*! \file		SPI_Debug.h
 *  \brief		MLX813xx SPI Debugging interface
 *
 * \note		project MLX813xx
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2015-12-30
 *   
 * \version 	1.0 - preliminary
 *
 *
 * MELEXIS Microelectronic Integrated Systems
 * 
 * Copyright (C) 2012-2016 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * ****************************************************************************	*/

#ifndef SPI_DEBUG_H
#define SPI_DEBUG_H

#include "Build.h"

#if MCU_ASSP_MODE
/* ASSP-mode: IO[0], IO[1] and IO[2] can be used for debugging */
#define DEBUG_INI_IO_ABC()			{ ANA_OUTM = (IO2_OUTCFG_SOFT | IO1_OUTCFG_SOFT | IO0_OUTCFG_SOFT); \
										ANA_OUTF = (IO2_ENA | IO1_ENA | IO0_ENA); }
#define DEBUG_CLR_IO_A()			{ANA_OUTN &= ~(1u << 1);}							/* Clear  IO[1] */
#define DEBUG_SET_IO_A()			{ANA_OUTN |= (1u << 1);}							/* Set    IO[1] */
#define DEBUG_TOG_IO_A()			{ANA_OUTN ^= (1u << 1);}							/* Toggle IO[1] */
#define DEBUG_CLR_IO_B()			{ANA_OUTN &= ~(1u << 2);}							/* Clear  IO[2] */
#define DEBUG_SET_IO_B()			{ANA_OUTN |= (1u << 2);}							/* Set    IO[2] */
#define DEBUG_TOG_IO_B()			{ANA_OUTN ^= (1u << 2);}							/* Toggle IO[2] */
#define DEBUG_SET_IO_AB_00()		{ANA_OUTN &= ~(3u << 1);}							/* Clear  IO[2:1] = 00 */
#define DEBUG_SET_IO_AB_01()		{ANA_OUTN = (ANA_OUTN & ~(3u << 1)) | (1u << 1);}	/* Set    IO[2:1] = 01 */
#define DEBUG_SET_IO_AB_10()		{ANA_OUTN = (ANA_OUTN & ~(3u << 1)) | (1u << 2);}	/* Set    IO[2:1] = 10 */
#define DEBUG_SET_IO_AB_11()		{ANA_OUTN |= (3u << 1);}							/* Set    IO[2:1] = 11 */
#define DEBUG_CLR_IO_C()			{ANA_OUTN &= ~(1u << 0);}							/* Clear  IO[0] */
#define DEBUG_SET_IO_C()			{ANA_OUTN |= (1u << 0);}							/* Set    IO[0] */
#define DEBUG_TOG_IO_C()			{ANA_OUTN ^= (1u << 0);}							/* Toggle IO[0] */
#else  /* MCU_ASSP_MODE */	
#define DEBUG_CLR_IO_A()			{IO_EXTIO &= ~(1u << 1);}							/* Clear  IO[4] */
#define DEBUG_SET_IO_A()			{IO_EXTIO |= (3u << 0);}							/* Set    IO[4] */
#define DEBUG_TOG_IO_A()			{IO_EXTIO ^= (1u << 1);}							/* Toggle IO[4] */
#define DEBUG_CLR_IO_B()			{IO_EXTIO &= ~(1u << 5);}							/* Clear  IO[5] */
#define DEBUG_SET_IO_B()			{IO_EXTIO |= (3u << 4);}							/* Set    IO[5] */
#define DEBUG_TOG_IO_B()			{IO_EXTIO ^= (1u << 5);}							/* Toggle IO[5] */
#define DEBUG_CLR_IO_AB()			{IO_EXTIO &= ~((1u << 5) | (1u << 1));}				/* Clear  IO[5:4] */
#define DEBUG_SET_IO_AB()			{IO_EXTIO |= ((1u << 5) | (1u << 1));}				/* Set    IO[5:4] */
#define DEBUG_SET_IO_AB_00()		{IO_EXTIO = ((1u << 4) | (1u << 0));}				/* Set    IO[5:4] = 00 */
#define DEBUG_SET_IO_AB_01()		{IO_EXTIO = ((1u << 4) | (3u << 0));}				/* Set    IO[5:4] = 01 */
#define DEBUG_SET_IO_AB_10()		{IO_EXTIO = ((3u << 4) | (1u << 0));}				/* Set    IO[5:4] = 10 */
#define DEBUG_SET_IO_AB_11()		{IO_EXTIO = ((3u << 4) | (3u << 0));}				/* Set    IO[5:4] = 11 */
#endif /* MCU_ASSP_MODE */


#define C_DBG_INIT	0xA55Au

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
void SpiDebugInit( void);
void SpiDebugWriteFirst( uint16 u16Data);
void SpiDebugWriteNext( uint16 u16Data);

#endif /* SPI_DEBUG_H */

/* EOF */
