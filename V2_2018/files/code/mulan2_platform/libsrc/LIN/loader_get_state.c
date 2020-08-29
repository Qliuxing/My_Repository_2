/*
 * Copyright (C) 2011-2013 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <loader_state.h>

/*
 * Loader state calculation:
 *  1) sum = app_disabled + app_enabled + loader_rst_state @ 0xBF66
 *  2) loader_state = sum & 3
 *
 * app_disabled  app_enabled  0xBF66   sum      loader_state  notes
 * ---------------------------------------------------------------------------------------
 * 0             1            3        4        0             application mode
 * 1             1            3        5        1             loading loaderB.hex helper to high part of the Flash
 * 0             0            2        2        2             loading new application (low part)
 * 0             0            3        3        3             loading new application (high part)
 * 0             1            3        4        0             application mode
 */
__MLX_TEXT__  ml_uint8 LDR_GetState (void)
{
    ml_uint8 loader_state = (loader_flags.app_disabled
            + loader_flags.app_enabled
            + loader_rst_state) & 3;

    return loader_state;
}

/* EOF */
