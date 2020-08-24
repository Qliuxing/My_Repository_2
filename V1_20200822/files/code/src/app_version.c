/*! ----------------------------------------------------------------------------
 * \file		app-version.c
 * \brief		MLX81310 application version
 *
 * \note		project MLX81310
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2013-09-18
 *   
 * \version 	1.0 - preliminary
 *
 * \functions					
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
#include "syslib.h"
#include "app_version.h"
#include "Build.h"

/* Clear Flash Key */
const uint16 Flash_Key[4] __attribute__((section(".protection_key"))) =			/* MMP150407-1 - Begin */
{
	0x0000,	//C_VENDOR_ID,
	0x0000,	//C_PRODUCT_ID
	0x0000,
	0x0000
};																				/* MMP150407-1 - End */

/* Clear Flash CRC */
const uint16 Flash_CRC __attribute__((section(".flash_crc"))) = 0x0000;			/* MMP131126-4 */

/* Set application version */
const uint32 application_version __attribute__((section(".app_version"))) =
    (__APP_VERSION_MAJOR__) |
    (__APP_VERSION_MINOR__ << 8) |
    (__APP_VERSION_REVISION__ << 16);

/* Set application string */
const uint8 product_id[8] __attribute__((section(".product_no"))) =
	{'3','H','U','A','D','E','M','O'};

/* EOF */
