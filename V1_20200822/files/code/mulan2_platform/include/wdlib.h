/*
 * Copyright (C) 2005-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef WDLIB_H_
#define WDLIB_H_

#include "syslib.h"

/*
 *  Intelligent watchdog unit
 * 
 * Watchdog uses 250 kHz clock derived from calibrated 1 MHz clock
 *
 *  WD_timeout = 1/250 kHz * Prescaler *  Timer = 
 *  = 1/250 kHz * 2^(2*WD_DIV[1:0] + 3) * WD_T[7:0]
 *
 * NOTES:
 *  - Once WD_MODE is defined it cannot be overwritten by another value
 *    (protection error is generated)
 *  - WD_DIV can only be changed from WD_MODE_DISABLED or WD_MODE_INTELLIGENT.
 *    Note that for latter case MODE[1:0] shall be '11' while writing to
 *    WD_CTRL register
 *  - WD_T register can only be written from WD_MODE_DISABLED or WD_MODE_INTELLIGENT
 *    modes. Writing to this register in other modes should be avoided since
 *    will be considered as watchog acknowledgment
 *  - After a watchdog reset, all watchdog registers are restored to the state
 *    they had at power on reset
 *  - In WD_MODE_INTELLIGENT, writing to any port of the watchdog is
 *    only valid in system mode of CPU
 */
#define WD_CLK_KHZ           250    /* watchdog module clock */


/*-----------------------------------------------------------------------------
 * WD_INIT
 * Initializes watchdog unit
 *
 * NOTES:
 *  - Once started watchdog can not be disabled or mode changed
 *  - Function can be called in WD_MODE_INTELLIGENT mode to set
 *    new 'div' and 'timeout'
 */
#define WD_INIT(mode, div, timeout) \
do  {                               \
    WD_T = timeout;                 \
    WD_CTRL = (mode) | (div);       \
} while (0)

/*-----------------------------------------------------------------------------
 * WD_RESTART
 * Restarts (acknowledges) watchdog unit
 *
 * NOTES:
 *     - Writing to WD_T a dummy value resets the watchdog
 *     - Applicable only for WD_MODE_TIMER and WD_MODE_WINDOW modes
 */
#define WD_RESTART()        (WD_T = 0)

/*-----------------------------------------------------------------------------
 * WD_BOOT_CHECK
 * Tests if the watchdog was a source of reset.
 * Returns true if it was watchdog reset.
 */
#define WD_BOOT_CHECK()     ((CONTROL & WD_BOOT) != 0)

/*-----------------------------------------------------------------------------
 * WD_GET_MODE
 * Returns WD mode
 *
 * NOTES:
 *  Applicable in any mode
 */
#define WD_GET_MODE()     (WD_CTRL & WD_MODE)

/*-----------------------------------------------------------------------------
 * WD_IS_WINDOW_OPEN
 * Returns true if acknowledgment window is open.
 *
 * NOTES:
 *  Applicable in window mode
 */
#define WD_IS_WINDOW_OPEN()     ((WD_CTRL & WD_WND) != 0)

/* -------------------------------------------------------------------
 * WD_INT_ENABLE
 * Enables watchdog attention interrupt
 * SW priority: 1 (fixed)
 *
 * MASK[2], PEND[2]
 */
#define WD_INT_ENABLE()         \
do {                            \
    PEND  = CLR_WD_ATT_IT;      \
    MASK |= EN_WD_ATT_IT;       \
} while(0)

/* -------------------------------------------------------------------
 * WD_INT_DISABLE
 * Disables (masks) watchdog attention interrupt
 */
#define WD_INT_DISABLE()     ( MASK &= ~EN_WD_ATT_IT )

#endif /* WDLIB_H_ */
