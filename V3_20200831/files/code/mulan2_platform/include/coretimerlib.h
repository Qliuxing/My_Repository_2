/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * MULAN core timer
 */
#ifndef CORETIMERLIB_H_
#define CORETIMERLIB_H_

#include <ioports.h>
#include "syslib.h"

/* -------------------------------------------------------------------
 * Initialize and start 15-bit core's timer
 * Timer is down counting loadable binary counter clocked from 1MHz
 * Each time it reaches 0x0000 it is reloaded by the value 'n'
 */
#define CORE_TIMER_INIT(n)   ( TIMER = TMR_EN | ( n ) )

/* -------------------------------------------------------------------
 * Get current timer value
 * NOTES:
 * 1. This macro cannot be called when timer is stopped (TMR_EN = 0),
 *    otherwise returns undefined value (see MULAN datasheet)
 * 2. Reading TIMER just after setting TMR_EN can read garbage value
 *    as TIMER is only updated every 1uS
 */
#define CORE_TIMER_VALUE()   ( TIMER & ~TMR_EN )

/* -------------------------------------------------------------------
 * Stop the timer; to start the timer again CORE_TIMER_INIT should be
 * called
 */                                                                                                                  
#define CORE_TIMER_STOP()    ( TIMER &= ~TMR_EN )

/* -------------------------------------------------------------------
 * CORE_TIMER_START() is impossible while it requires read-modify-write
 * operation on TIMER port (|=). But if TMR_EN = 0 reading from
 * TIMER port returns undefined value (see MULAN datasheet)
 */

/* -------------------------------------------------------------------
 * Enable timer interrupt and set priority
 * prio: 3 .. 6
 *
 * PRIO[1:0], MASK[5], PEND[5]
 */
#define CORE_TIMER_INT_ENABLE(prio) \
do {                                \
    PRIO = (((prio) - 3u) & 3)      \
                 | (PRIO & ~3u);    \
    PEND  = CLR_TIMER_IT;           \
    MASK |= EN_TIMER_IT;            \
} while(0)


/* -------------------------------------------------------------------
 * Disable (mask) timer interrupt
 */
#define CORE_TIMER_INT_DISABLE()    ( MASK &= ~EN_TIMER_IT )


/* -------------------------------------------------------------------
 * Clear timer interrupt
 */
#define CORE_TIMER_INT_CLEAR()      ( PEND = CLR_TIMER_IT )


#endif /* CORETIMERLIB_H_ */
