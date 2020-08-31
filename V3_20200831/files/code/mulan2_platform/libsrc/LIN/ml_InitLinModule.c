/*
 * Copyright (C) 2005-2015 Melexis N.V.
 *
 * Software Platform
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
#include <plib.h>   /* product libraries */
#include <lin.h>

#if (LIN_PIN_LOADER != 0)
#include "flashupload.h"
#include "lin_nad.h"
#endif

/* ----------------------------------------------------------------------------
 * Initializes the LIN Module
 *
 * Initializes the LIN Module and switches to the DISCONNECTED state.
 * The LIN Module may need to be (re)configured, as no previous configuration
 * data is retained
 *
 * Param:   none
 * Return:  command status (not used so far)
 *
 * \note
 *  1. LIN interrupt priority (if available for the LIN interrupt) is
 *     application specific and thus shall be set by the application before
 *     calling ml_InitLinModule() function. The ml_InitLinModule() function
 *     doesn't change the LIN interrupt priority settings.
 *
 *  2. The ml_InitLinModule command can be used only after reset of the MLX4
 *     processor, i.e. at the startup or after an explicit reset of the MLX4
 *     from the application with e.g.:
 *          MLX4_RESET();
 *          NOP ();
 *          NOP ();
 *          NOP ();
 *          MLX4_START ();
 */
__MLX_TEXT__  ml_Status ml_InitLinModule(void)
{
    /*
     * Assuming that LIN interrupt priority (if availabe for configuration)
     * is set by the application
     */

#if (LIN_PIN_LOADER != 0)
    if (   (0 == LDR_GetState())
#if defined (LDR_RESET_ON_ENTER_PROG_MODE)
        && (bistResetInfo != C_CHIP_STATE_LOADER_PROG_RESET)
#endif /* LDR_RESET_ON_ENTER_PROG_MODE */
       )
    {  /* if we're in loader state 0 (application mode) .. */
        ENABLE_MLX4_INT();      /* .. enable LIN interrupt                          */
    }
    /* else : for other loader state use LIN interrupt polling */
#else
    ENABLE_MLX4_INT();          /* enable LIN interrupt                             */
#endif /* LIN_PIN_LOADER */


    /* MLX4/MLX16 synchronization.
     * If the Mlx16 is here before the Mlx4, it will wait for MLX4.
     * If the Mlx4 reached this point before MLX16, then the Mlx16 does not wait
     */
    WDG_Manager();              /* keep system alive    */
    SLVCMD = 0x84U;             /* signal to MLX4       */

    while ( (SLVCMD & 0x04U) == 0U ) {
        /* Busy waiting for acknowledgement from MLX4, but no more than
         * AWDG watchdog period. If acknowledgent is not received within
         * this time, the AWDG watchdog will reset the MLX16.
         */
    }

    /* The Mlx4 goes to the DISCONNECTED state, where it will wait for a command
     * (event). Before that, it is going to send an event to indicate a state
     * change. Now that the initialization of both chips is done (the LIN part
     * still needs to be configured!) enable the interrupts to allow inter-chip
     * communication
     */
    SLVIT = 0xABU;          /* Enable Mlx4 Event interrupt */

#if (LIN_PIN_LOADER != 0)
    ml_driver_mode = kLinAppMode;

    /* Initial NAD */
    if (stFixedRamNAD.key == _mlx_NAD_Security_Key)
    {
        LIN_nad = stFixedRamNAD.nad;
    }
    else
    {
        LIN_nad = MLX_NAD_DEFAULT;
    }
#endif /* LIN_PIN_LOADER */

    return ML_SUCCESS;
}

/* EOF */
