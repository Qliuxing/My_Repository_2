/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <syslib.h>

/* ----------------------------------------------------------------------------
 * Reset MLX16 CPU
 *
 * Notes:
 *  1. There is no reset activation bit to trigger HW reset from SW.
 *     Thus HW reset will be triggered by the watchdog.
 *  2. Since watchdog mode can not be changed once initially set,
 *     the common approach for all modes should be used. This will be
 *     locking execution flow in the infinity loop and waiting for the
 *     watchdog reset.
 *  3. Once set, the watchodg period can not be changed in Timer and
 *     Window mode. So, in worst case the loop can take up to 0.5 seconds
 *  4. This functions assumes that watchdog is already running in some mode.
 */
void MLX16_RESET (void)
{
    for ( ; ; ) {
        /* no watchdog ack */
    }

}

/* EOF */
