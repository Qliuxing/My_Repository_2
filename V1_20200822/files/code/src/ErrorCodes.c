/*! ----------------------------------------------------------------------------
 * \file		ErrorCodes.c
 * \brief		MLX81300 Error handling
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	ErrorLogInit()
 *				SetLastError()
 *				GetLastError()
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

#include "Build.h"
#include "ErrorCodes.h"
#include "NVRAM_UserPage.h"

#define C_ERR_LOG_SZ	10
uint8 l_au8FiFoErrorLog[C_ERR_LOG_SZ];
uint8 l_u8ErrorLogIdx = 0;

/* ****************************************************************************	*
 * ErrorLogInit
 *
 *	Pre:		Nothing
 *	Post:		Nothing
 *
 *	Comments:	Clear Error-FiFo-buffer, in case watchdog reset occurred, 
 *				otherwise leave untouched.
 * ****************************************************************************	*/
void ErrorLogInit( void)
{
	if ( (AWD_CTRL & AWD_RST) != 0 )
	{
		uint16 i;
		for ( i = 0; i < C_ERR_LOG_SZ; i++ )
		{
			l_au8FiFoErrorLog[i] = C_ERR_NONE;
		}
		l_u8ErrorLogIdx = 0;
	}
} /* End of ErrorLogInit() */

/* ****************************************************************************	*
 * SetLastError
 *
 *	Pre:		uint8 u8ErrorCode: Error-code (8-bits)
 *	Post:		Nothing
 *
 *	Comments:	Save error-code in Error-FiFo-buffer, unless last error is the 
 *				same as error posted.
 * ****************************************************************************	*/
void SetLastError( uint8 u8ErrorCode)
{
	if ( (l_u8ErrorLogIdx == 0) || (l_au8FiFoErrorLog[l_u8ErrorLogIdx - 1] != u8ErrorCode) )
	{
		/* Don't log the same error over and over again */
		l_au8FiFoErrorLog[l_u8ErrorLogIdx] = u8ErrorCode;
		if ( l_u8ErrorLogIdx < (C_ERR_LOG_SZ - 1) )
		{
			l_u8ErrorLogIdx++;
		}

		/* Log serious error-codes also in NVRAM */
		/* Serious errors are:
		  	  Unsupported IRQ's   or C_ERR_INV_MLXPAGE_CRC1..4, CAL_GN or Over-temperature                   or 'Fatal'-errors */
		if ( (u8ErrorCode < 0x20) || ((u8ErrorCode & 0xC8) == 0xC8) || (u8ErrorCode == (uint8) C_ERR_DIAG_OVER_TEMP) || ((u8ErrorCode & 0xF0) == 0xF0) ) /*lint !e845 */
		{
			(void) NVRAM_LogError( u8ErrorCode);
		}
	}
} /* End of SetLastError() */

uint8 GetLastError( void)
{
	uint8 u8Reply = l_au8FiFoErrorLog[0];
	if ( l_u8ErrorLogIdx != 0 )
	{
		uint16 i;
		ATOMIC_CODE
		(
			for ( i = 1; i < l_u8ErrorLogIdx; i++ )
			{
				l_au8FiFoErrorLog[i - 1] = l_au8FiFoErrorLog[i];
			}
			l_u8ErrorLogIdx--;
			l_au8FiFoErrorLog[l_u8ErrorLogIdx] = C_ERR_NONE;
		);
	}
	return ( u8Reply );
} /* End of GetLastError() */


/* EOF */
