/*! \file		NVRAM_UserPage.c
 *  \brief		MLX81310 NVRAM User Page
 *
 * \note		project MLX81310
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-16
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	NVRAM_CRC8()
 *				NVRAM_CountCRC8()
 *				NVRAM_PageVerify()
 *				NVRAM_Store()
 *				NVRAM_LoadUserPage()
 *				PlaceError()
 *				NVRAM_LogError()
 *				NVRAM_GetLastError()
 *				NVRAM_ClearErrorLog()
 *				NVRAM_EmergencyStore()
 *				NVRAM_StorePatch()
 *				NVRAM_MlxCalibrationAreaCheck()
 *
 * MELEXIS Microelectronic Integrated Systems
 * 
 * Copyright (C) 2012-2015 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * ****************************************************************************	*/
#include "NVRAM_UserPage.h"
#include <plib.h>
#include <nvram.h>																/* NVRAM support */
#include "LIN_Protocol.h"														/* LIN configuration default */
#include "MotorParams.h"														/* Motor-driver support */
#include "ErrorCodes.h"															/* Error-logging support */


/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION (@TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp

#pragma space none

#pragma space nodp

#pragma space none

const NVRAM_PAGEINTEGRITY PageIntegerityDefault = 
{
	C_NVRAM_USER_REV,										/* 0x00: CRC-8 & 0x01:NVRAM Structure revision */
	0														/* 0x02: Program Count */
};


/***********************private functions declaration ***************************/
uint8 NVRAM_CRC8( const uint16 *pRAM, uint16 size);
/* error log */
uint8 NVRAM_CountCRC8( PNVRAM_ERRORLOG pNVERRLOG, uint8 byReplaceCRC);
void PlaceError( uint16 *pu16ErrorElement, uint16 u16OddEven, uint8 u8ErrorCode);


/***********************public functions ***************************/
void NVRAM_Init(void)
{
	uint16 *pMRAM;
	PNVRAM_PAGEINTEGRITY pIntegrity;
	uint16 i;
	uint16 u16ErrorFlag; 
	uint8  u8CRC;
	
	/* dump to memory */
	NVRAM_LoadAll();
	u16ErrorFlag = (VARIOUS_L & EENV_DED);										/* Double-bit error state */
	/* Check Double-bit NVRAM set, User-NVRAM structure-revision and User-NVRAM Checksum */
	/* 1.check user page 1,if fail use default */
	pMRAM = (uint16 *)C_ADDR_USERPAGE1;
	pIntegrity = (PNVRAM_PAGEINTEGRITY)(C_ADDR_USERPAGE1);
	u8CRC = (uint8)NVRAM_CRC8(pMRAM,C_SIZE_USERPAGE1 / 2u);
	if(((uint8)pIntegrity->CRC8_Revision != C_NVRAM_USER_REV) || (u8CRC != 0xFFu))
	{
		/* UniROM */
		pIntegrity->CRC8_Revision = C_NVRAM_USER_REV;		/* CRC8=0x00,Revision=C_NVRAM_USER_REV */
		pIntegrity->ProgramCount = 0u;

		/* nvram may be corrupted,filled with default  */
		for(i = 0;i < (C_SIZE_USERPAGE2 / 2u);i++)
		{
			pMRAM[i] = 0xFFFFu;
		}
	}
	/* 2.check user page 2:if fail padding with 0xFF */
	pMRAM = (uint16 *)C_ADDR_USERPAGE2;
	pIntegrity = (PNVRAM_PAGEINTEGRITY)(C_ADDR_USERPAGE2);
	u8CRC = NVRAM_CRC8(pMRAM,C_SIZE_USERPAGE2 / 2u);
	if(((uint8)pIntegrity->CRC8_Revision != C_NVRAM_USER_REV) || (u8CRC != 0xFFu))
	{
		/* UniROM */
		pIntegrity->CRC8_Revision = C_NVRAM_USER_REV;		/* CRC8=0x00,Revision=C_NVRAM_USER_REV */
		pIntegrity->ProgramCount = 0u;
		
		/* nvram may be corrupted,filled with padding  */
		for(i = 0;i < (C_SIZE_USERPAGE2 / 2u);i++)
		{
			pMRAM[i] = 0xFFFFu;
		}
	}

}


uint8 NVRAM_Read(uint16 addr,uint16 buf[],uint16 size)
{
	uint16 *pMRAM;
	uint16 i;
	uint8 ret = NVRAM_E_OK;
	
	if((addr >= C_NVRAM_AREA1_ADDR) && ((addr + size) < (C_NVRAM_AREA1_ADDR + C_NVRAM_AREA1_SIZE) ))
	{
		pMRAM = (uint16 *)C_ADDR_USERPAGE1 + sizeof(NVRAM_PAGEINTEGRITY)/sizeof(uint16) + addr - C_NVRAM_AREA1_ADDR;
		for(i = 0;i < size;i++)
		{
			buf[i] = pMRAM[i];
		}
	}
	else if((addr >= C_NVRAM_AREA2_ADDR) && ((addr + size) < (C_NVRAM_AREA2_ADDR + C_NVRAM_AREA2_SIZE)))
	{
		pMRAM = (uint16 *)C_ADDR_USERPAGE2 + sizeof(NVRAM_PAGEINTEGRITY)/sizeof(uint16) + addr - C_NVRAM_AREA2_ADDR;
		for(i = 0;i < size;i++)
		{
			buf[i] = pMRAM[i];
		}
	}
	else
	{
		ret = NVRAM_E_INVALID_DATA;
	}

	return ret;
}


uint8 NVRAM_Write(uint16 addr,const uint16 buf[],uint16 size)
{
	uint16  *pMRAM;
	uint8  u8VerifyRes,u8CRC8;
	PNVRAM_PAGEINTEGRITY pIntegrity;
	uint16  i;
	uint8  ret = NVRAM_E_OK;

	/* address should be even */	
	if((addr >= C_NVRAM_AREA1_ADDR) && ((addr + size) < (C_NVRAM_AREA1_ADDR + C_NVRAM_AREA1_SIZE)))
	{
		pMRAM = (uint16 *)C_ADDR_USERPAGE1 + sizeof(NVRAM_PAGEINTEGRITY) / sizeof(uint16) + (addr - C_NVRAM_AREA1_ADDR);

		/* copy with verifing */
		u8VerifyRes = 0u;
		for(i = 0;i < size;i++)
		{
			if(buf[i] != pMRAM[i])
			{
				u8VerifyRes = 1u;
				pMRAM[i] = buf[i];
			}
		}

		/* page verify result need saving  */
		if(u8VerifyRes == 1u)
		{
			/* program count and nvram user version */
			pIntegrity = (PNVRAM_PAGEINTEGRITY)(C_ADDR_USERPAGE1);
			pIntegrity->CRC8_Revision = C_NVRAM_USER_REV;					/* CRC8=0x00,Revision=C_NVRAM_USER_REV */
			pIntegrity->ProgramCount++;
			if(pIntegrity->ProgramCount >= C_MAX_NVRAM_PROGRAM_COUNT)
			{
				pIntegrity->ProgramCount = C_MAX_NVRAM_PROGRAM_COUNT;
			}
			
			/* page intergrity */
			u8CRC8 = (uint8)NVRAM_CRC8((uint16 *)C_ADDR_USERPAGE1, C_SIZE_USERPAGE1 / 2u);	/* MMP151202-1 */
			u8CRC8 = 0xFFu - u8CRC8;
			pIntegrity->CRC8_Revision = (pIntegrity->CRC8_Revision & 0x00FFu) | ((uint16)u8CRC8 << 8u);
			
			NVRAM_SavePage( NVRAM1_PAGE1);
		}
		
	}
	else if((addr >= C_NVRAM_AREA2_ADDR) && ((addr + size) < (C_NVRAM_AREA2_ADDR + C_NVRAM_AREA2_SIZE)))
	{
		pMRAM = (uint16 *)C_ADDR_USERPAGE2 + sizeof(NVRAM_PAGEINTEGRITY) / sizeof(uint16) + (addr - C_NVRAM_AREA2_ADDR);

		/* copy and verify */
		u8VerifyRes = 0u;

		for(i = 0;i < size;i++)
		{
			if(buf[i] != pMRAM[i])
			{
				u8VerifyRes = 1u;
				pMRAM[i] = buf[i];
			}
		}

		/* page verify result need saving,then start save process  */
		if(u8VerifyRes == 1u)
		{
			pIntegrity = (PNVRAM_PAGEINTEGRITY)(C_ADDR_USERPAGE2);
			pIntegrity->CRC8_Revision = C_NVRAM_USER_REV;					/* CRC8=0x00,Revision=C_NVRAM_USER_REV */
			pIntegrity->ProgramCount++;
			if(pIntegrity->ProgramCount >= C_MAX_NVRAM_PROGRAM_COUNT)
			{
				pIntegrity->ProgramCount = C_MAX_NVRAM_PROGRAM_COUNT;
			}
			/* page intergrity */
			u8CRC8 = (uint8)NVRAM_CRC8((uint16 *)C_ADDR_USERPAGE2, C_SIZE_USERPAGE2 / 2u);	/* MMP151202-1 */
			u8CRC8 = 0xFFu - u8CRC8;
			pIntegrity->CRC8_Revision = (pIntegrity->CRC8_Revision & 0x00FFu) | ((uint16)u8CRC8 << 8u);
			
			NVRAM_SavePage( NVRAM2_PAGE1);
		}
	}
	else
	{
		ret = NVRAM_E_INVALID_DATA;
	}

	return ret;
	
}

/* LIN diagnostic compatiable interface:for system configuration page */
uint16 NVRAM_Store( uint16 u16Page)
{
	uint16 u16Result = C_NVRAM_STORE_OKAY;
	uint8 u8CRC8;
	PNVRAM_PAGEINTEGRITY pIntegrity;

	if((u16Page & C_NVRAM_USER_PAGE_1) != 0u)
	{
		if ( (u16Page & C_MVRAM_USER_PAGE_NoCRC) == 0u )
		{
			pIntegrity = (PNVRAM_PAGEINTEGRITY)(C_ADDR_USERPAGE1);
			u8CRC8 = (uint8)NVRAM_CRC8((uint16 *)C_ADDR_USERPAGE1, C_SIZE_USERPAGE1 / 2u);	/* MMP151202-1 */
			u8CRC8 = 0xFFu - u8CRC8;
			pIntegrity->CRC8_Revision = (pIntegrity->CRC8_Revision & 0x00FFu) | ((uint16)u8CRC8 << 8u);
			
		}
		NVRAM_SavePage( NVRAM1_PAGE1);
	}

	if((u16Page & C_NVRAM_USER_PAGE_2) != 0u)
	{
		if ( (u16Page & C_MVRAM_USER_PAGE_NoCRC) == 0u )
		{
			pIntegrity = (PNVRAM_PAGEINTEGRITY)(C_ADDR_USERPAGE2);
			u8CRC8 = (uint8)NVRAM_CRC8((uint16 *)C_ADDR_USERPAGE2, C_SIZE_USERPAGE2 / 2u);	/* MMP151202-1 */
			u8CRC8 = 0xFFu - u8CRC8;
			pIntegrity->CRC8_Revision = (pIntegrity->CRC8_Revision & 0x00FFu) | ((uint16)u8CRC8 << 8u);
		}
		NVRAM_SavePage( NVRAM2_PAGE1);
	}

	return u16Result;
}

/* ****************************************************************************	*
 * void NVRAM_StorePatch
 *
 * Store Patch NVRAM page (shadow RAM to NVRAM) and load all NVRAM pages.
 *
 * C_ADDR_PATCHPAGE+0x00:0x6B:	Patch-code
 * C_ADDR_PATCHPAGE+0x6C:0x6D:	PATCH0_I
 * C_ADDR_PATCHPAGE+0x6E:0x6F:	PATCH1_I
 * C_ADDR_PATCHPAGE+0x70:0x71:	PATCH2_I
 * C_ADDR_PATCHPAGE+0x72:0x73:	PATCH3_I
 * C_ADDR_PATCHPAGE+0x74:0x75:	PATCH0_A
 * C_ADDR_PATCHPAGE+0x76:0x77:	PATCH1_A
 * C_ADDR_PATCHPAGE+0x78:0x79:	PATCH2_A
 * C_ADDR_PATCHPAGE+0x7A:0x7B:	PATCH3_A
 *
 * ****************************************************************************	*/
void NVRAM_StorePatch( void)
{
	NVRAM_SavePage( NVRAM1_PAGE2);
} /* End of NVRAM_StorePatch() */


uint8 NVRAM_CRC8( const uint16 *pRAM, uint16 size)
{
	uint16 u16CRC;

	u16CRC = nvram_CalcCRC(pRAM, size);	/* MMP151202-1 */

	return ( (uint8) u16CRC );
} /* End of NVRAM_CRC8() */



/* ****************************************************************************	*
 *  NVRAM_CountCRC8
 *
 *	Pre:	Pointer to NVRAM-structure of Write-cycle counter
 *			byReplaceCRC = FALSE: Check CRC
 *							TRUE: Replace CRC (calculate)
 *	Post:	(uint8) XOR result of calculated CRC8 and stored CRC8.
 *				byReplaceCRC = FALSE: When correct, the result = 0xFF, otherwise != 0xFF
 *								TRUE: Calculated CRC8.
 *
 *	Calculate CRC8 on Write-cycle counter.
 * ****************************************************************************	*/
uint8 NVRAM_CountCRC8( PNVRAM_ERRORLOG pNVERRLOG, uint8 byReplaceCRC)
{
	uint16 u16CRC = nvram_CalcCRC( (uint16 *) &pNVERRLOG->NvramProgramCycleCount, 1);	/* MMP151202-1 */

	if ( byReplaceCRC != FALSE )
	{
		pNVERRLOG->ErrorLogIndex_CRC = (pNVERRLOG->ErrorLogIndex_CRC & 0x00FFu) | ((0xFFu - u16CRC) << 8u);
	}
	return ( (uint8) ((pNVERRLOG->ErrorLogIndex_CRC >> 8u) ^ u16CRC) );
} /* End of NVRAM_CountCRC8() */


/* ****************************************************************************	*
 * void PlaceError
 *
 *	Pre:	*pu16ErrorElement: Pointer to NVRAM 16-bit Word address
 *			u16OddEven: FALSE: LSB of 16-bits Word, TRUE: MSB of 16-bits Word
 *			u8ErrorCode: Error-code
 *	Post:	-
 *	Comments: Write error-code into NVRAM (16-bits words based)
 * ****************************************************************************	*/
void PlaceError( uint16 *pu16ErrorElement, uint16 u16OddEven, uint8 u8ErrorCode)
{
	if ( u16OddEven != 0u )
	{
		/* Odd index: MSB of uint16 */
		*pu16ErrorElement = (uint8)(*pu16ErrorElement) | (((uint16) u8ErrorCode) << 8u);
	}
	else
	{
		/* Even index: LSB of uint16 */
		*pu16ErrorElement = (*pu16ErrorElement & 0xFF00u) | ((uint16) u8ErrorCode);
	}
} /* End of PlaceError() */

/* ****************************************************************************	*
 * NVRAM_LogError
 *
 *	Pre:	8-bit error-code
 *	Post:	(uint16) C_NVRAM_STORE_OKAY : Stored successfully
 *					 C_NVRAM_STORE_MAX_WRITE_CYCLE : NVRAM Write-cycle reached maximum
 *
 * Store severe error-code. Circular buffer. Write error-code at 'empty/over-write'
 *	index and update index. In case index beyond array-size, set to 0xFF. In case both 
 *	arrays on both pages full, set index to 0xFF or 0xFE. The XOR-of both page indexes
 *	result is either 0 or 1, indicating which page should be written next-time.
 * Index at user-page #1/#2: 0-11: Empty/over-write index, FF: Full
 * If index user-page #1 is not 0xFF (Full), write error at page #1, otherwise page #2
 * ****************************************************************************	*/
uint16 NVRAM_LogError( uint8 u8ErrorCode)
{
	uint16 u16Result = C_NVRAM_STORE_OKAY;										/* MMP150219-1 */
	
	PNVRAM_ERRORLOG pNVERRLOG_UPG = (PNVRAM_ERRORLOG) (C_ADDR_USERPAGE2 + C_NVRAM_ERRLOG_OFFSET);
	uint16 u16ErrorLogIdx = (uint8) (pNVERRLOG_UPG->ErrorLogIndex_CRC);
	
	
	if ( (u16ErrorLogIdx & 0x80u) == 0x00u )
	{
		/* Store error on User-Page #1 */										/* MMP150219-1 - Begin */
		if ( u16ErrorLogIdx >= C_MAX_ERRORS_PER_PAGE )						/* Check against array overflow */
		{
			u16ErrorLogIdx = 0u;
		}
		if ( NVRAM_CountCRC8( pNVERRLOG_UPG, FALSE) == 0x00u )
		{
			pNVERRLOG_UPG->NvramProgramCycleCount++;
		}
		else
		{
			pNVERRLOG_UPG->NvramProgramCycleCount = 1U;
		}																		/* MMP150219-1 - End */
		/* store error log */
		if ( pNVERRLOG_UPG->NvramProgramCycleCount < (C_MAX_NVRAM_PROGRAM_COUNT - 1000u) )
        {
			(void) NVRAM_CountCRC8( pNVERRLOG_UPG, TRUE);
			PlaceError( (uint16 *) &(pNVERRLOG_UPG->ErrorLog[u16ErrorLogIdx >> 1u]), u16ErrorLogIdx & 0x01u, u8ErrorCode);
			pNVERRLOG_UPG->ErrorLogIndex_CRC = ((pNVERRLOG_UPG->ErrorLogIndex_CRC) & 0xFF00u) | u16ErrorLogIdx;
			/* Save (NV)RAM to NV(RAM) */
			NVRAM_SavePage( NVRAM2_PAGE1 | NVRAM_PAGE_WR_SKIP_WAIT);
		}
		else
		{
			u16Result = C_NVRAM_STORE_MAX_WRITE_CYCLE;
		}
		
	}
	return ( u16Result );														/* MMP150219-1 */
} /* End of NVRAM_LogError() */

/* ****************************************************************************	*
 * uint8 NVRAM_GetLastError
 *
 *	Pre:	-
 *	Post:	(uint8) Last logged error-code
 * ****************************************************************************	*/
uint8 NVRAM_GetLastError( void)
{
	uint16 u16ErrorLogIdx;
	uint8 u8Result = 0x00u;														/* No error's */

	PNVRAM_ERRORLOG pNVERRLOG_UPG = (PNVRAM_ERRORLOG) (C_ADDR_USERPAGE2 + C_NVRAM_ERRLOG_OFFSET);
	u16ErrorLogIdx = (uint8) (pNVERRLOG_UPG->ErrorLogIndex_CRC);

	if ( u16ErrorLogIdx != 0x00u )
	{																			/* MMP140218-1 */
		/* Get last error from User-Page #2 */
		u16ErrorLogIdx--;
		{
			uint16 u16ErrorCodes = pNVERRLOG_UPG->ErrorLog[u16ErrorLogIdx >> 1u];
			if ( (u16ErrorLogIdx & 0x01u) != 0u )
			{
				u8Result = (uint8) (u16ErrorCodes >> 8u);
			}
			else
			{
				u8Result = (uint8) u16ErrorCodes;
			}
		}
	}
	return( u8Result );

} /* End of NVRAM_GetLastError() */

/* ****************************************************************************	*
 * void NVRAM_ClearErrorLog
 *
 *	Pre:	-
 *	Post:	-
 *	Comments: Clear Application Error logging (in both User-NVRAM pages)
 * ****************************************************************************	*/
void NVRAM_ClearErrorLog( void)
{
	uint16 i;
	PNVRAM_ERRORLOG pNVERRLOG_UPG = (PNVRAM_ERRORLOG) (C_ADDR_USERPAGE2 + C_NVRAM_ERRLOG_OFFSET);
	pNVERRLOG_UPG->ErrorLogIndex_CRC = ((pNVERRLOG_UPG->ErrorLogIndex_CRC) & 0xFF00u) | 0x00u;	/* Set index at 0x00 */
	for ( i = 0; i < (C_MAX_ERRORS_PER_PAGE / 2u); i++ )
	{
		pNVERRLOG_UPG->ErrorLog[i] = 0x0000u;
	}
	(void) NVRAM_CountCRC8( pNVERRLOG_UPG, TRUE);
	NVRAM_SavePage( NVRAM2_PAGE1);
} /* End of NVRAM_ClearErrorLog() */

/* EOF */
