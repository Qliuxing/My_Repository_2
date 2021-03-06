/*! \file		Main.h
 *  \brief		MLX81300 app-version
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2013-09-18
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	main()
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
 * ****************************************************************************	*/

#ifndef APP_VERSION_H_
#define APP_VERSION_H_

/*
 * Application version
 */
#define __APP_VERSION_MAJOR__      	1UL				/* Major version: 1 */
#define __APP_VERSION_MINOR__      	0UL				/* Minor version: 0 */
#define __APP_VERSION_REVISION__   	0xE008UL		/* Engineering build #0008 */

#define __APP_VERSION_STRING__		"1.0.E008"


extern const uint16 Flash_Key[4] __attribute__((section(".protection_key")));
extern const uint16 Flash_CRC __attribute__((section(".flash_crc")));
extern const uint32 application_version __attribute__((section(".app_version")));
extern const uint8 product_id[8] __attribute__((section(".product_no")));


#endif /* APP_VERSION_H_ */

/* EOF */

