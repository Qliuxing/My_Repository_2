/*
 * Copyright (C) 2008-2015 Melexis N.V.
 *
 * Software Platform
 * Product/board specific functions
 *
 */

#ifndef LIB_MLX81310_WDG_H_
#define LIB_MLX81310_WDG_H_

#include "awdg.h"

/* ----------------------------------------------------------------------------
 * Function is call in the idle loop of flash loader
 *
 * \note
 * 1. Digital Watchdog acknowledgement by "WD_T = 0" is only valid for Timer and
 *    Window mode (not for the Intelligent mode).
 */
__MLX_TEXT__  static INLINE  void WDG_Manager (void)
{
    awdg_restart();

    /* Request [PLTF-626]: Acknowledge Digital Watchdog
     */
    if ((WD_CTRL & WD_MODE) != 0) {     /* if digital watchdog is enabled .. */
        WD_T = 0;                       /* .. acknowledge the watchdog */
    }
    /* else: digital watchdog is not enabled */
}


#endif /* LIB_MLX81310_WDG_H_ */
