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

#define C_DBG_INIT	0xA55A

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
void SpiDebugInit( void);

/* ****************************************************************************	*
 * SpiDebugWriteFirst
 *
 *	Pre:		uint16 u16Data: Data (16-bit)
 *	Post:		Nothing
 *
 *	Comments:	Send 16-bit SPI-data
 * ****************************************************************************	*/
static INLINE void SpiDebugWriteFirst( uint16 u16Data)
{
	SPI1_PSCR |= SPI_FRSSOEN;													/* Toggle Slave-Select */
	SPI1_PSCR &= ~SPI_FRSSOEN;
	SPI1_DR = u16Data;															/* Send SPI-Data */
} /* End of SpiDebugWriteFirst() */

/* ****************************************************************************	*
 * SpiDebugWriteNext
 *
 *	Pre:		uint16 u16Data: Data (16-bit)
 *	Post:		Nothing
 *
 *	Comments:	Send 16-bit SPI-data
 * ****************************************************************************	*/
static INLINE void SpiDebugWriteNext( uint16 u16Data)
{
	SPI1_DR = u16Data;															/* Send SPI-Data */
} /* End of SpiDebugWriteNext() */


#endif /* SPI_DEBUG_H */

/* EOF */
