/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef LIB_MLX8110X_WAKEUP_H_
#define LIB_MLX8110X_WAKEUP_H_

#include "ioports.h"

/* WU .. Register: ANA_OUTG .. Bit 5-6 */
#define WUI     (uint16)~(3u << 5)
#define WU_TIMER_DISABLE (0u << 5)
#define WU_TIMER_0s4     (1u << 5)
#define WU_TIMER_0s8     (2u << 5)
#define WU_TIMER_1s6     (3u << 5)

/* INTERNAL_WU .. Register: ANA_INB .. Bit 10 */
#define INTERNAL_WU      (1u << 10)

/* LOCAL_WU .. Register: ANA_INB .. Bit 9 */
#define LOCAL_WU         (1u << 9)

/* LIN_WU .. Register: ANA_INB .. Bit 8 */
#define LIN_WU           (1u << 8)

/** Set Wake-up timer
 * @param
 * @return void
 * Register: ANA_OUTG, Bit WUI, Bit positions [6:5]
 */
#define WU_SET_TIMER(WU_TIMER)                     \
do {                                               \
       ANA_OUTG = ((ANA_OUTG & WUI) | (WU_TIMER)); \
	} while (0)

/** Check for internal wake-up
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INB, Bit INTERNAL_WU, Bit positions [10]
 */
#define WU_STATE_INTERNAL() ((ANA_INB & INTERNAL_WU) >> 10)

/** Check for local wake-up
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INB, Bit LOCAL_WU, Bit positions [9]
 */
#define WU_STATE_LOCAL() ((ANA_INB & LOCAL_WU) >> 9)

/** Check for lin wake-up
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INB, Bit LIN_WU, Bit positions [8]
 */
#define WU_STATE_LIN() ((ANA_INB & LIN_WU) >> 8)

#endif /* LIB_MLX8110X_WAKEUP_H_ */
