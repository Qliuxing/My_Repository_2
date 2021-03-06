/*! ----------------------------------------------------------------------------
 * \file		SPI_Debug.c
 * \brief		MLX813xx SPI Debugging interface
 *
 * \note		project MLX813xx
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2015-12-30
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	SpiDebugInit()
 *				SpiDebugWriteFirst()
 *				SpiDebugWriteNext()
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

#include "Build.h"
#include "SPI_Debug.h"


/* ****************************************************************************	*
 * SpiDebugInit
 *
 *	Pre:		Nothing
 *	Post:		Nothing
 *
 *	Comments:	Initialise SPI Interface.
 * ****************************************************************************	*/
void SpiDebugInit( void)
{
	SPI1_PCR  = SPI_CKEN;													/* Set the bit SPI_CKEN to 1 */
	SPI1_BRR  = ((PLL_freq / DEBUG_SPI_BAUDRATE) + 1);						/* 100, 200, 400, 800 or 1000 kBaud */
	SPI1_CTRL = (SPI_FRSSOEN << 8) | (SPI_MSTRONLY << 8) | SPI_RFIE | SPI_TFIE | /* SPI_BYTEMOD | */ SPI_MSTR | SPI_EN | SPI_CKEN;	/* Mode 00 */

	SpiDebugWriteFirst( C_DBG_INIT);
} /* End of SpiDebugInit() */

/* EOF */
