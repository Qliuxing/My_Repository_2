/*! \file		NVRAM_UserPage.h
 *  \brief		MLX81310 NVRAM USer Page handling
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
 * Copyright (C) 2008-2015 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * ****************************************************************************	*/

#ifndef NVRAM_USER_PAGE_H_
#define NVRAM_USER_PAGE_H_

#include "Build.h"

#define C_NVRAM_USER_REV		0x02u

#define C_NVRAM_USER_PAGE_1		0x01u						/* NVRAM User Page #1,calibration info NVRAM #1,Page #1 */
#define C_NVRAM_USER_PAGE_2		0x02u						/* NVRAM User Page #2,user real time info NVRAM #2,Page #1 */
#define C_NVRAM_USER_PAGE_ALL	(C_NVRAM_USER_PAGE_1 | C_NVRAM_USER_PAGE_2)
#define C_NVRAM_USER_PAGE_RESET	0x20u
#define C_NVRAM_USER_PAGE_FORCE	0x40u						/* NVRAM Write w/o pre-verify */
#define C_MVRAM_USER_PAGE_NoCRC	0x80u						/* No CRC Update */

/* Note: NVRAM is currently LIN communication protocol depended !! */

#define C_ADDR_USERPAGE1		0x1000u
#define C_SIZE_USERPAGE1		0x0080u
#define C_ADDR_PATCHPAGE		0x1080u
#define C_SIZE_PATCHPAGE		0x007Cu

#if _DEBUG_FATAL
#define C_ADDR_FATALPAGE		0x1080u
#define C_SIZE_FATALPAGE		0x007Cu
#endif /* _DEBUG_FATAL */

#define C_ADDR_USERPAGE2		0x1100u
#define C_SIZE_USERPAGE2		0x0080u

#define C_ADDR_MLXF_PAGE		0x1180u
#define C_ADDR_MLX_HWSWID		0x1182u
#define C_ADDR_MLX_CHIPID		0x1188u 
#define C_ADDR_MLX_TESTID		0x11A0u
#define C_SIZE_MLXF_PAGE		0x0080u


/* NVRAM memory structure/distribution,total 256-2*4=248 0xF8 bytes, half word */
#define C_NVRAM_AREA1_ADDR		0x1000u
#define C_NVRAM_AREA1_SIZE		0x3Eu
#define C_NVRAM_AREA2_ADDR		0x103Eu
#define C_NVRAM_AREA2_SIZE		0x3Eu

/* NVRAM error type */
#define NVRAM_E_OK					0u
#define NVRAM_E_INVALID_DATA		1u
#define NVRAM_E_FAIL				2u

typedef struct
{
	uint16 CRC8_Revision;									/* 0x00: CRC-8 & Revision */
	uint16 ProgramCount;									/* 0x02: Program Count */
}NVRAM_PAGEINTEGRITY,*PNVRAM_PAGEINTEGRITY;


typedef struct _NVRAM_ERRORLOG
{
	uint16	NvramProgramCycleCount;							/* 0x70: 16-bits Program cycle counter */
	uint16	ErrorLogIndex_CRC;								/* 0x72: Error-log index (0-11) & (Optional-CRC) */
	uint16	ErrorLog[6];									/* 0x74-0x7F: Error-log */
} NVRAM_ERRORLOG, *PNVRAM_ERRORLOG;

#define C_NVRAM_ERRLOG_OFFSET			0x70u

#define C_MAX_NVRAM_PROGRAM_COUNT		65000u				/* Maximum 65000 Write-cycles */
#define C_MAX_ERRORS_PER_PAGE			12u

#define C_NVRAM_STORE_OKAY				0x00u
#define C_NVRAM_STORE_MAX_WRITE_CYCLE	0x01u
#define C_NVRAM_STORE_INVALID_COUNTER	0x02u


#pragma space dp
#pragma space none


#pragma space nodp
#pragma space none


/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
extern void NVRAM_Init(void);
extern uint8 NVRAM_Read(uint16 addr,uint16 buf[],uint16 size);
extern uint8 NVRAM_Write(uint16 addr,const uint16 buf[],uint16 size);

extern uint16 NVRAM_Store(uint16 u16Page);
extern void NVRAM_StorePatch( void);											/* Store patch NVRAM page (shadow RAM to NVRAM) and load all NVRAM pages */

/* error log */
extern uint16 NVRAM_LogError( uint8 u8ErrorCode);								/* Store servire error code */
extern uint8 NVRAM_GetLastError( void);										/* Get last NVRAM error code */
extern void NVRAM_ClearErrorLog( void);										/* Clear NVRAM error log */


#endif /* NVRAM_USER_PAGE_H_ */

/* EOF */
