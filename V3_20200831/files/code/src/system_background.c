#include "MotorDriver.h"
#include <plib.h>
#include "ErrorCodes.h"
#include "Diagnostic.h"
#include "lib_mlx315_misc.h"
#include "Timer.h"
#include "system_background.h"


#define POLY	                    0x1021u
#define FLASH_START_ADDR			0x4000u
#define FLASH_CRC_ADDR				0xBF4Eu
#define FLASH_END_ADDR				0xC000u
#define C_FLASH_SEGMENT_SZ			4u											/* Max 250us (196us), Halt-mode: Full-check is once per 8:40s; Running-mode: 1.5s */

#define C_FLASH_CRC_FAILED			0u
#define C_FLASH_CRC_OK				1u
#define C_FLASH_CRC_CALCULATING		2u

/* private variable definitions */
uint8 l_u8RamPreError = FALSE;													/* RAM vs. NVRAM test first-failure (MMP150925-1) */
uint8 l_u8BackgroundSchedulerTaskID = 0u;

/* private function declaration */
uint16 RamBackgroundTest( uint16 u16Page);
uint16 FlashBackgroundTest( uint16 u16Size);


void System_BackgroundTaskInit(void)
{
  /* *************************************************** */
	/* *** A. Initialise background schedule (Task-ID) *** */
	/* *************************************************** */
	l_u8BackgroundSchedulerTaskID = 0u;

}

/* ********************************** */
/* *** o. Background System check *** */
/* ********************************** */
void System_BackgroundMemoryTest(void)
{
#if (MOTOR_PARAMS == MP_NVRAM)
	/* RAM check vs. NVRAM User Page 1 schedule ID:0,128 */
	if ( (l_u8BackgroundSchedulerTaskID == 0u) || (l_u8BackgroundSchedulerTaskID == 128u) )
	{
		if ( RamBackgroundTest(0) == FALSE )								/* Check RAM against NVRAM User-page */
		{
			/* MMP150925-1: RAM g_NvramUser structure not same as NVRAM Page #1.1.
			 * Either System RAM is corrupted or the NVRAM. Allow one time NVRAM reload */
			if ( l_u8RamPreError == FALSE )
			{
				NVRAM_Init();
				l_u8RamPreError = TRUE;
			}
			else
			{
				SetLastError( (uint8) C_ERR_RAM_BG);						/* Log RAM failure */
				MLX4_RESET();												/* Reset the Mlx4   */
				bistResetInfo = C_CHIP_STATE_LIN_CMD_RESET;
				MLX16_RESET();												/* Reset the Mlx16  */
			}
		}
		else
		{
			l_u8RamPreError = FALSE;										/* Error is gone (caused by wrong NVRAM shadow-RAM) */
		}
	}
#endif	/* (MOTOR_PARAMS == MP_NVRAM) || (VALVE_PARAMS == VP_NVRAM) */
	/* Flash CRC runtime check,schedule ID:[1-127],[129-255] */
	if((l_u8BackgroundSchedulerTaskID != 0u) && (l_u8BackgroundSchedulerTaskID != 128u))
	{
		if ( (FL_CTRL0 & FL_DETECT) != 0u )									/* MMP150603-2 */
		{
			if ( FlashBackgroundTest( C_FLASH_SEGMENT_SZ) == (uint16)C_FLASH_CRC_FAILED )	/* Check Flash/ROM Memory Checksum (max. 250us) */
			{
				SetLastError( (uint8) C_ERR_FLASH_BG);
				MLX4_RESET();													/* Reset the Mlx4   */
				bistResetInfo = C_CHIP_STATE_LIN_CMD_RESET;
				MLX16_RESET();													/* Reset the Mlx16  */
			}
		}
	}
	l_u8BackgroundSchedulerTaskID++; 
}


/* ****************************************************************************	*
 * RamBackgroundTest
 *
 *	Pre:		u16Page: 0 = Check against User-page #1
 *						<>0 = Check against User-page #2
 *	Post:		FALSE : User-page against RAM compare failed
 *				TRUE  : User-page against RAM is correct
 *
 *	Comments:	Calculate a Melexis standard 8-bit CRC as used in NVRAM, for the
 *				Actuator important RAM data block (copied from NVRAM). This data
 *				should be stable during the actuator operation.
 * ****************************************************************************	*/
uint16 RamBackgroundTest( uint16 u16Page)
{
	/* Use compare instead of CRC-check !! */
	u16Page = u16Page;
	
	return TRUE;

} /* End of RamBackgroundTest() */



/* ****************************************************************************	*
 * FlashBackgroundTest
 *
 *	Pre:		uint16 u16Size = Amount of Flash/ROM (in 16-bits words) to add to 
 *				CRC-calculation
 *	Post:		C_FLASH_CRC_FAILED		: CRC calculation failed
 *				C_FLASH_CRC_OK			: CRC calculation is correct
 *				C_FLASH_CRC_CALCULATING	: CRC calculation is ongoing (busy)
 *
 *	Comments:	The CCITT CRC-16 polynomial is X^16 + X^12 + X^5 + 1. The detection 
 *				rate is 99.9984%, worse case. In hex the pattern is 0x11021. A 17-bit 
 *				register is simulated by testing the MSB before shifting the data, 
 *				which affords us the luxury of specifying the polynomial as a 16-bit
 *				value, 0x1021. Because the CRC is process in reverse order, the 
 *				polynomial is reverse too: 0x8408.
 *				To avoid huge delay, the calculation is split into segments of u16Size
 *				16-bits words. When reaching the Flash-end, the checksum is compared
 *				against first calculated Flash CRC.
 * ****************************************************************************	*/
uint16 FlashBackgroundTest( uint16 u16Size)
{
	uint16 u16Result = C_FLASH_CRC_CALCULATING;
	static uint16 *pu16Segment = (uint16 *) FLASH_START_ADDR;
	static uint16 u16FlashCRC = 0u;

	if ( u16Size == 0u )
	{
		pu16Segment = (uint16 *) FLASH_START_ADDR;
		return ( u16Result );
	}
	if ( pu16Segment == (uint16 *) FLASH_START_ADDR )
	{
		u16FlashCRC = 0xFFFFu;													/* Initialise the CRC preset with 0xFFFF */
	}
	if ( ((uint16) pu16Segment + u16Size) > FLASH_END_ADDR )
	{
		u16Size = (FLASH_END_ADDR - (uint16) pu16Segment);
	}
	for ( ; u16Size > 0u; u16Size-- )
	{
		if ( pu16Segment != (uint16 *) FLASH_CRC_ADDR )
		{
			uint8 u8Count;
			uint16 u16Data = *pu16Segment;
			for ( u8Count = 16u; u8Count > 0u; u8Count-- )
			{
				uint16 u16XorFlag = u16FlashCRC & 0x8000U;
				u16FlashCRC = (u16FlashCRC << 1u);
				if ( (u16Data & 0x8000u) != 0u )
				{
					u16FlashCRC++;
				}
				if ( u16XorFlag != 0u)
				{
					u16FlashCRC ^= POLY;
				}
				u16Data <<= 1u;
			}
		}
		pu16Segment++;
	}

	if ( (uint16) pu16Segment >= FLASH_END_ADDR )
	{
		/* CRC fully calculated, check values */
		pu16Segment = (uint16 *) FLASH_START_ADDR;
		if ( *(uint16 *) FLASH_CRC_ADDR != 0u )									/* Flash/ROM Checksum programmed? */
		{
			if ( *(uint16 *) FLASH_CRC_ADDR != u16FlashCRC )
			{
				u16Result = C_FLASH_CRC_FAILED;
			}
			else
			{
				u16Result = C_FLASH_CRC_OK;
			}
		}
	}

	return ( u16Result );

} /* End of FlashBackgroundTest() */



void System_BackgroundIORegTest(void)
{
	/* ************************************ */
	/* *** s. Critical peripheral check *** */
	/* ************************************ */
	/* Check: Motor commutation timer disabled */
	if ( (TMR1_CTRL & TMRx_T_EBLK) == 0u )
	{
		/* Communication timer is disabled; Motor is stopped too */
		if ( g_u8MotorStartupMode != (uint8)MSM_STOP )
		{
			TMR1_CTRL = ((1u * TMRx_DIV0) | (0u * TMRx_MODE0) | TMRx_T_EBLK) | TMRx_START; 				/* Start timer mode 0 */
		}
		else
		{
			TMR1_CTRL = (1u * TMRx_DIV0) | (0u * TMRx_MODE0) | TMRx_T_EBLK;	/* Timer mode 0, Divider 16 */;								/* Timer mode 0 */
		}
		SetLastError( (uint8) C_ERR_IOREG);
	}
	/* Check: Administrative timer disabled */
	if ( (TIMER & TMR_EN) == 0u )							
	{
		TIMER = TMR_EN | CT_PERIODIC_RATE;
		SetLastError( (uint8) C_ERR_IOREG);
	}
	/* Check: IRQ-Mask (Respectively: Diagnostics, Timer1, CoreTimer and LIN-Communication */
	if ( (MASK & (EN_EXT4_IT | EN_EXT0_IT | EN_TIMER_IT | EN_M4_SHE_IT)) != (EN_EXT4_IT | EN_EXT0_IT | EN_TIMER_IT | EN_M4_SHE_IT) )
	{
		PEND = (EN_EXT4_IT | EN_EXT0_IT | EN_TIMER_IT | EN_M4_SHE_IT);
		MASK |= (EN_EXT4_IT | EN_EXT0_IT | EN_TIMER_IT | EN_M4_SHE_IT);
		SetLastError( (uint8) C_ERR_IOREG);
	}
	/* Check: IRQ-priority (Respectively: Diagnostics, Timer1, CoreTimer) */
	if ( (PRIO & (((uint16)3u << 14u) | ((uint16)3u << 6u) | ((uint16)3u << 0u))) != 
		(/*((uint16)(3-3) << 14) |*/ ((uint16)(4u - 3u) << 6u) | ((uint16)(6u - 3u) << 0u)) )
	{
		PRIO &= ~(((uint16)3u << 14u) | ((uint16)3u << 6u) | ((uint16)3u << 0u));
		PRIO |=  (/*((uint16)(3u - 3u) << 14u) |*/ ((uint16)(4u - 3u) << 6u) | ((uint16)(6u - 3u) << 0u));
		SetLastError( (uint8) C_ERR_IOREG);
	}
	/* Check: 2nd level IRQ Timer1 */
	if ( (XI0_MASK & EN_T1_INT4) == 0u )
	{
		XI0_PEND = EN_T1_INT4;
		XI0_MASK = EN_T1_INT4;
		SetLastError( (uint8) C_ERR_IOREG);
	}
	/* Check: 2nd level IRQ Diagnostics */
	if ( (XI4_MASK & (XI4_OVT | XI4_UV | XI4_OV | XI4_OC_DRV)) != C_DIAG_MASK )
	{
		XI4_PEND = C_DIAG_MASK; 										/* MMP150409-1 */
		XI4_MASK = C_DIAG_MASK; 										/* MMP150409-1 */
		SetLastError( (uint8) C_ERR_IOREG);
	}
	/* Check:driver check */
	if ( (g_u8MotorStartupMode != (uint8)MSM_STOP) && ((DRVCFG & (DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U)) == 0u) )
	{
		/* Driver have been disabled */
		SetLastError( (uint8) C_ERR_IOREG);
	}
}
