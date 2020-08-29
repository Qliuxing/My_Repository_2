/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef EXTINTLIB_H_
#define EXTINTLIB_H_

#include "ioports.h"
#include "syslib.h"

/* -------------------------------------------------------------------
 * Enable External Interrupt 0 and set its SW priority
 * Parameter:
 *      prio: 3..6
 *
 * PRIO[7:6], MASK[8], PEND[8]
 */
#define EXT0_INT_ENABLE(prio)               \
    do {                                    \
        PRIO = ((((prio) - 3u) & 3) << 6)   \
                     | (PRIO & ~(3u << 6)); \
        PEND  = (1u << 8);                  \
        MASK |= (1u << 8);                  \
    } while (0)

/* -------------------------------------------------------------------
 * Disable (mask) External Interrupt 0
 */
#define EXT0_INT_DISABLE()      \
    (MASK &= ~(1u << 8))


/* -------------------------------------------------------------------
 * Enable External Interrupt 1 and set its SW priority
 * Parameter:
 *      prio: 3..6
 *
 * PRIO[9:8], MASK[9], PEND[9]
 */
#define EXT1_INT_ENABLE(prio)               \
    do {                                    \
        PRIO = ((((prio) - 3u) & 3) << 8)   \
                     | (PRIO & ~(3u << 8)); \
        PEND  = (1u << 9);                  \
        MASK |= (1u << 9);                  \
    } while (0)

/* -------------------------------------------------------------------
 * Disable (mask) External Interrupt 1
 */
#define EXT1_INT_DISABLE()      \
    (MASK &= ~(1u << 9))


/* -------------------------------------------------------------------
 * Enable External Interrupt 2 and set its SW priority
 * Parameter:
 *      prio: 3..6
 *
 * PRIO[11:10], MASK[10], PEND[10]
 */
#define EXT2_INT_ENABLE(prio)                   \
    do {                                        \
        PRIO = ((((prio) - 3u) & 3) << 10)      \
                     | (PRIO & ~(3u << 10));    \
        PEND  = (1u << 10);                     \
        MASK |= (1u << 10);                     \
    } while (0)

/* -------------------------------------------------------------------
 * Disable (mask) External Interrupt 2
 */
#define EXT2_INT_DISABLE()      \
    (MASK &= ~(1u << 10))


/* -------------------------------------------------------------------
 * Enable External Interrupt 3 and set its SW priority
 * Parameter:
 *      prio: 3..6
 *
 * PRIO[13:12], MASK[11], PEND[11]
 */
#define EXT3_INT_ENABLE(prio)                   \
    do {                                        \
        PRIO = ((((prio) - 3u) & 3) << 12)      \
                     | (PRIO & ~(3u << 12));    \
        PEND  = (1u << 11);                     \
        MASK |= (1u << 11);                     \
    } while (0)

/* -------------------------------------------------------------------
 * Disable (mask) External Interrupt 3
 */
#define EXT3_INT_DISABLE()      \
    (MASK &= ~(1u << 11))


/* -------------------------------------------------------------------
 * Enable External Interrupt 4 and set its SW priority
 * Parameter:
 *      prio: 3..6
 *
 * PRIO[15:14], MASK[12], PEND[12]
 */
#define EXT4_INT_ENABLE(prio)                   \
    do {                                        \
        PRIO = ((((prio) - 3u) & 3) << 14)      \
                     | (PRIO & ~(3u << 14));    \
        PEND  = (1u << 12);                     \
        MASK |= (1u << 12);                     \
    } while (0)

/* -------------------------------------------------------------------
 * Disable (mask) External Interrupt 4
 */
#define EXT4_INT_DISABLE()      \
    (MASK &= ~(1u << 12))


#endif /* EXTINTLIB_H_ */
