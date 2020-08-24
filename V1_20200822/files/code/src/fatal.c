/*
 * Copyright (C) 2009-2011 Melexis N.V.
 *
 * Software Platform
 *
 * Revision $Name:
 *
 * File $RCSfile: fatal.c $
 *
 */

/*
 *  NOTES:
 *      1.  All functions in this module should be defined with __MLX_TEXT__
 *          attribute. Thus they will be linked in first half of the Flash.
 *
 *      2.  This module should NOT use _initialized_ global and static variables!
 *          Such variables are linked into .data or .dp.data sections and their
 *          initialisation lists are stored at load address (LMA) in the Flash.
 *          Since there is no control on the position of the load address, the
 *          linker might link it to second half of the Flash and thus
 *          initialisation values will be overwritten by the loader during
 *          programming of the a new application. As a result, variables in .data
 *          sections will not be correctly initialised.
 *          Use uninitialised variables instead (will be linked to .bss section).
 */

#include "Build.h"
#include <ioports.h>
#include <nvram.h>
#if _DEBUG_FATAL
#include "NVRAM_UserPage.h"
#endif /* _DEBUG_FATAL */

/* Linker symbols (these objects are not created in the memory) */
extern uint16 stack;
#include "main.h"

/* ****************************************************************************	*
 *	Internal function prototypes												*
 * ****************************************************************************	*/
void STACK_IT(void);
void _fatal (void);
uint16 FlashBackgroundTest( uint16 u16Size);

/* ------------------------------------------------------------------------- 
 * Fatal program termination (Stack IRQ)
 */
__MLX_TEXT__ void STACK_IT(void)
{
//	SET_STACK( &stack);
	/* Chip header is valid and chip successfully initialised; LIN Command Reset use AWD to reset chip */
	if ( (bistHeader == C_CHIP_HEADER) && ((bistResetInfo == C_CHIP_STATE_LIN_CMD_RESET) || (bistResetInfo == C_CHIP_STATE_LOADER_PROG_RESET)) )
	{
		/* INLINE MLX16_RESET (Don't use stack) */
		do
		{
			if ( (PLL_CTRL & PLL_EN) != 0 )								/* Only delay in case of PLL is active */
			{
				DELAY_US( 250);											/* Should be called with period > 200us, otherwise bit AWD_WRITE_FAIL will be set and further acknowledgment will fail during next 200 us */
			}
			AWD_CTRL = (AWD_ATT | AWD_WRITE_FAIL | (3u << 8) | 1);		/* Set 1:1 pre-scaler and minimal period; AWD timeout will be 100 us */
		} while ( AWD_CTRL & (AWD_ATT | AWD_WRITE_FAIL) );
		for ( ; ; ) {
			/* wait for reset */
		}
	}
	__asm__( "mov yl, #01");
	__asm__( "jmp __fatal");

} /* End of _STACK_IT() */


/* ------------------------------------------------------------------------- 
 * Fatal program termination
 *
 * Pre:		YL = Error-code
 *			CPU-protection-mode: 0
 * Post;	Nothing
 *
 * 0x079A: Error-reason (IRQ-vector ID)
 * 0x079B: PLL-Status (bit[5:4]) and Flash-Control (bit[2:0])
 * 0x079C: Address of failure (from stack)
 */
__MLX_TEXT__ void _fatal (void)
{
	/* YL = Error-reason; Don't use stack (MMP151125-1) */
	__asm__("lod X, 0x2026");													/* X = [FL_CTRL0] */
	__asm__("and X, #0x07");													/* X[2:0] = ([FL_CTRL0] & (FL_DBE | FL_SBE | FL_DETECT)) */
	__asm__("lod A, 0x2040");													/* A = [PLL_STAT] */
	__asm__("and A, #0x03");													/* A[1:0] = (([PLL_STAT] & (PLL_CM | PLL_LOCKED)) */
	__asm__("asl A, #2");
	__asm__("asl A, #2");														/* A[5:4] = (([PLL_STAT] & (PLL_CM | PLL_LOCKED)) << 4) */
	__asm__("or  A, X");														/* A(L) = ([FL_CTRL0] & (FL_DBE | FL_SBE | FL_DETECT)) | (([PLL_STAT] & (PLL_CM | PLL_LOCKED)) << 4) */
	__asm__("lod YH, AL");														/* YH = ([FL_CTRL0] & (FL_DBE | FL_SBE | FL_DETECT)) | (([PLL_STAT] & (PLL_CM | PLL_LOCKED)) << 4) */
	__asm__("mov _bistError, Y");
	__asm__("lod A, [S-2]");													/* Save address of failed instruction */
	__asm__("mov _bistErrorInfo, A");											/* Failure address */

#if _SUPPORT_CRASH_RECOVERY
	/* Crash recovery */
	if ( (bistResetInfo == C_CHIP_STATE_FATAL_RECOVER_ENA) && ((uint8)(bistError & 0xFF) < 0x05) )	/* MMP151125-1 */
	{
		/* Crash recovery is enabled, and type of IRQ is between 0x01 and 0x04 */
		bistResetInfo = C_CHIP_STATE_FATAL_CRASH_RECOVERY;						/* Start recovery (on-going) */
		SET_STACK( &stack);														/* Re-initialise stack */
		ENTER_SYSTEM_MODE();													/* Protected mode, highest priority (0) (MMP141023-1) */
		XI0_PEND = CLR_T1_INT4;
		XI2_PEND = 0xFFFF;														/* Clear all XI2_PEND flags */
		XI4_PEND = (XI4_OVT | XI4_UV | XI4_OV | XI4_OC_DRV);
		g_u8Mlx4ErrorState = (uint8) C_MLX4_STATE_IMMEDIATE_RST;				/* Reset MLX4 always */
		PEND = CLR_TIMER_IT;													/* Core-Timer */
		SET_PRIORITY( 7);														/* Protected mode, low priority (7) */
		(void) FlashBackgroundTest( 0);

#if (_DEBUG_FATAL != FALSE)
		if ( (FL_CTRL0 & FL_DETECT) != 0 )										/* MMP150603-2 */
		{
			/* Only in case of a FLASH-device, store FATAL error info */
			uint16 u16NVRAM_FatalCount = *((uint16 *) C_ADDR_FATALPAGE);
			u16NVRAM_FatalCount++;
			if ( u16NVRAM_FatalCount <= 30 )
			{
				uint16 *pu16NV;
				*((uint16 *) C_ADDR_FATALPAGE) = u16NVRAM_FatalCount;
				u16NVRAM_FatalCount <<= 1;
				pu16NV = ((uint16 *) C_ADDR_FATALPAGE + u16NVRAM_FatalCount);
				pu16NV[0] = bistError;
				pu16NV[1] = bistErrorInfo;
				NVRAM_SavePage( NVRAM1_PAGE2 | NVRAM_PAGE_WR_SKIP_WAIT);
			}
		}
#endif /* (_DEBUG_FATAL != FALSE) */

		(void) main();

		/* Should never come here, as main should not be left */
		__asm__( "mov YL, #0x19");												/* C_MLX16_MAIN_FATAL */
		/* lint -e{974} */ _fatal();
	}
#endif /* _SUPPORT_CRASH_RECOVERY */

	SET_STACK( &stack);															/* Re-initialise stack (MMP141023-1) */
	ENTER_SYSTEM_MODE();														/* Protected mode, highest priority (0) (MMP141023-1) */
	FL_CTRL0 &= ~(FL_DBE | FL_SBE);												/* Clear DBE and SBE errors (MMP141023-1) */

#if (_DEBUG_FATAL != FALSE)
	if ( (FL_CTRL0 & FL_DETECT) != 0 )											/* MMP150603-2 */
	{
		/* Only in case of a FLASH-device, store FATAL error info */
		uint16 u16NVRAM_FatalCount = *((uint16 *) C_ADDR_FATALPAGE);
		u16NVRAM_FatalCount++;
		if ( u16NVRAM_FatalCount <= 30 )
		{
			uint16 *pu16NV;
			*((uint16 *) C_ADDR_FATALPAGE) = u16NVRAM_FatalCount;
			u16NVRAM_FatalCount <<= 1;
			pu16NV = ((uint16 *) C_ADDR_FATALPAGE + u16NVRAM_FatalCount);
			pu16NV[0] = bistError;
			pu16NV[1] = bistErrorInfo;
			NVRAM_SavePage( NVRAM1_PAGE2);
		}
	}
#endif /* (_DEBUG_FATAL != FALSE) */

	/* Disable motor driver first, before waiting for watchdog */
	DRVCFG_DIS_UVWT();															/* Tri-state (disconnect) the phase U, V, W and T (MMP130919-1) */

	for (;;) {
		/* loop forever */
	}
} /* End of _fatal() */

/* EOF */
