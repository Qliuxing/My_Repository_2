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

#include <alib.h>
#include <plib.h>

/* ------------------------------------------------------------------------- 
 *  Fatal program termination
 *  Input:
 *  -
 *  Output:
 *  -
 *  Return:
 *  -
 *  Notes:
 *  1. This function never returns, so can be defined w/o interrupt attribute
 *     (no need to save registers)
 *  2. Program counter was saved to Y register by JMPUSERVECTOR before jump to Fatal Handler
 */
__MLX_TEXT__ void _fatal (void)
{
    __asm__ __volatile__ ( "mov %0,Y \n\t" : "=m" (bistError) );
    bistErrorInfo = (uint16)__builtin_return_address(0);

    for (;;) {
        /* loop forever */
    }
}

/* EOF */
