/*
 * Copyright (C) 2011-2015 Melexis N.V.
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
#include <plib.h>


#define DWD_TIMER_MODE_MAX_RESET_MS     1


/* ----------------------------------------------------------------------------
 * Reset MLX16 CPU
 *
 * The analog WD is not recommended for device reset due to issue (see MLX82050-20)
 *
 * The digital WD can be used for device reset if
 *     - Digital WD isn't enabled
 *     - Window mode is enabled
 *     - Intelligent mode is enabled
 *     - Timer mode is enabled with satisfied timeout (timeout < DWD_TIMER_MODE_MAX_RESET_MS)
 *
 * NOTE:
 *      Digital WD can't be used for reset if Timer mode is enabled
 *      with NOT satisfied timeout (timeout > DWD_TIMER_MODE_MAX_RESET_MS)
 *      because reset can't be caused rather quickly.
 *      In this case analog WD is used for reset. The PLL disabling is done
 *      to decrease AWD reset failure rate
 */
__MLX_TEXT__  void MLX16_RESET (void)
{
    ATOMIC_CODE (
        switch ( WD_GET_MODE() )
        {
            case WD_MODE_WINDOW: /* Digital WD Window mode is enabled */
                /* Do acknowledge before 'window' opening */
                for ( ; ; ) {
                    WD_RESTART();
                }
                break;

            case WD_MODE_INTELLIGENT: /* Digital WD Intelligent mode is enabled */
                /* Reinit digital WD with minimal timeout */
                WD_T = 0;
                for ( ; ; ) {
                    /* wait for reset */
                }
                break;

            case WD_MODE_DISABLED: /* Digital WD is disabled */
                /* Set 1:8 prescaler and minimal period;
                 * Digital WD timeout will be 32us */
                WD_INIT(WD_MODE_TIMER, WD_DIV_8, 1);
                for ( ; ; ) {
                    /* wait for reset */
                }
                break;

            case WD_MODE_TIMER: /* Digital WD Timer mode is enabled */
            {
                /* Calculate (Timeout_ticks / 2) which is used for DWD */
                uint16 DWD_Timeout_tck = ((WD_T << 2) << ((WD_CTRL & WD_DIV) << 1));

                /* Estimate MAX satisfied (Timeout_ticks / 2) which can be used for device reset */
                uint16 DWD_Max_Timeout_tck = WD_CLK_KHZ * DWD_TIMER_MODE_MAX_RESET_MS / 2;

                /* Check DWD is enabled with satisfied timeout */
                if ( DWD_Timeout_tck < DWD_Max_Timeout_tck )
                {
                    for ( ; ; ) {
                        /* wait for reset (worst case is DWD_TIMER_MODE_MAX_RESET_MS delay) */
                    }
                }
                /* Digital WD Timer mode is enabled with NOT satisfied timeout */
                else {
                    /* Use analog WD for reset with disabled PLL */
                    do {
                        /* Should be called with period > 200us,
                         * otherwise bit AWD_WRITE_FAIL will be set
                         * and further acknowledgment will fail during next 200 us */
                        DELAY_US(250);

                        /* Set 1:1 prescaler and minimal period;
                         * AWD timeout will be 100 us */
                        AWD_CTRL = (AWD_ATT | AWD_WRITE_FAIL | (3u << 8) | 1);
                    } while ( AWD_CTRL & (AWD_ATT | AWD_WRITE_FAIL) );

                    /* Turn off PLL */
                    PLL_CTRL &= ~PLL_EN;

                    for ( ; ; ) {
                        /* wait for reset */
                    }
                }
                break;
            }
            default:
                /* Do nothing */
                break;
        }
    );
}

/* EOF */
