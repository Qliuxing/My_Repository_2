/*
 * Copyright (C) 2011 Melexis N.V.
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
#include "lin.h"


/* ----------------------------------------------------------------------------
 * Default LIN (MLX4) interrupt handler
 *
 * This function is called whenever an EVENT interrupt from the LIN task (Mlx4)
 * occurs
 */
void __interrupt__ ml_LinInterruptHandler(void);
__MLX_TEXT__  void ml_LinInterruptHandler(void)
{
    ml_GetLinEventData();
    ml_ProccessLinEvent();
}

/*
 * Alias of ml_LinInterruptHandler for compatibility with
 * old definition of LIN interrupt handler in vectors.S
 */
void linit (void) __attribute__ ((alias ("ml_LinInterruptHandler")));

/* EOF */
