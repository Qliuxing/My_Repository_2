/*
 * Copyright (C) 2009-2015 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 *  NOTES:
 *      1.  All functions in this module should be defined with __MLX_TEXT__
 *          attribute. Thus they will be linked in first half of the Flash.
 *
 *      2.  This module should NOT use _initialized_ global and static variables!
 *          Such variables are linked into .data or .dp.data sections and their
 *          initialization lists are stored at load address (LMA) in the Flash.
 *          Since there is no control on the position of the load address, the
 *          linker might link it to second half of the Flash and thus
 *          initialization values will be overwritten by the loader during
 *          programming of the a new application. As a result, variables in .data
 *          sections will not be correctly initialized.
 *          Use uninitialized variables instead (will be linked to .bss section).
 */

#include <syslib.h>
#include <plib.h>       /* product libs */

#if (LIN_PIN_LOADER != 0)
#include <flashupload.h>

#if defined (SUPPORT_LINNETWORK_LOADER)
#include "lin.h"
#include "lin_nad.h"
#include "flash_cfg.h"
#endif /* SUPPORT_LINNETWORK_LOADER */

extern void ml_LinInit(void);

#endif  /* LIN_PIN_LOADER */

/*-----------------------------------------------------------------------------
 * Function premain() is invoked by start-up code just before calling main()
 * At this point all runtime-initialization is done thus variables/objects 
 * get their initial values
 *
 * NOTE: Function with the same name (i.e. premain) linked from
 * application directory overrides this function
 */
__MLX_TEXT__ void _premain (void)
{

#if (LIN_PIN_LOADER != 0)

#if defined (LDR_HAS_PAGE_BUFFER_ON_STACK)
    /*
     * Here page_buffer on the stack is initialised.
     * Reset on EnterProgMode is needed to run Loader routine from zero stack when LDR_GetState = 0.
     */
    ml_uint8 page_buffer_stack[128] __attribute__((aligned(2)));
    page_buffer = page_buffer_stack;
#endif /* LDR_HAS_PAGE_BUFFER_ON_STACK */

    if (   (LDR_GetState() != 0)
#if defined (LDR_RESET_ON_ENTER_PROG_MODE)
        || (bistResetInfo == C_CHIP_STATE_LOADER_PROG_RESET)
#endif /* LDR_RESET_ON_ENTER_PROG_MODE */
       )
    {
#if defined (SUPPORT_LINNETWORK_LOADER)
	    if ( stFixedRamNAD.key != _mlx_NAD_Security_Key )
		{
			uint8_t u8NAD = (uint8_t) *((uint16_t*) 0xBF76);					/* Get NAD from Flash at STACK_IT Segment */
			if ( (u8NAD & 0x80) || (u8NAD == 0x00) )
			{
				u8NAD = 0x7F;													/* Invalid NAD; Use default NAD */
			}
			stFixedRamNAD.nad = u8NAD;
			stFixedRamNAD.key = _mlx_NAD_Security_Key;
		}
#endif /* SUPPORT_LINNETWORK_LOADER */

        SET_PRIORITY(7);                /* System mode, low priority (7) */

        ml_LinInit();
        (void)ml_Connect();

#if defined (LDR_RESET_ON_ENTER_PROG_MODE)
        if (bistResetInfo == C_CHIP_STATE_LOADER_PROG_RESET) {
            ml_ldr_SwitchToProgMode(ML_FALSE);
            bistResetInfo = C_CHIP_STATE_WATCHDOG_RESET;
        }
#endif /* LDR_RESET_ON_ENTER_PROG_MODE */
        for (;;) {
            WDG_Manager();
            
            if (PEND & CLR_M4_SHE_IT) { /* If LIN interrupt requested */

                /*
                 * LIN interrupt pending bit will be cleared in ml_GetLinEventData
                 */

                ml_GetLinEventData();
                ml_ProccessLinEvent();
            }
        }
    }
#endif /* LIN_PIN_LOADER */

    SET_PRIORITY(7);                    /* System mode, low priority (7) */
}

/* EOF */
