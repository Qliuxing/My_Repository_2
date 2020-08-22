/*
 * Copyright (C) 2011-2013 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <loader_state.h>

const volatile LoaderFlagsPage_Type loader_flags __attribute__((section(".loader_flags"))) =
{
        .app_disabled = 0,
        .app_enabled  = 1
        /* 0 for the rest of the structure */
};

/*
 *  Variable loader_rst_state indicates the state of the Reset vector:
 *      3: Reset vector points to the low part of the Flash
 *      2: Reset vector points to the high part of the Flash (means that loaderB code is active now)
 */
const volatile uint16_t loader_rst_state  __attribute__((section(".loader_rst_state"))) = 3;    /* state 3 @ 0xBF66 */

/* EOF */
