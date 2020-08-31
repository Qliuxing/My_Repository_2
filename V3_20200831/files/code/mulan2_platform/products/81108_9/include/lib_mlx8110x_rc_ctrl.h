/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef _LIB_MLX8110X__RC_CTRL__H_
#define _LIB_MLX8110X__RC_CTRL__H_

#include "ioports.h"

/* RC_EN .. Register: RC_CTRL .. Bit 0 */
#define RC_EN     (1u << 0)


/** Enable highspeed RC
 * @param
 * @return void
 * Register: RC_CTRL, Bit RC_EN, Bit positions [0]
 */
#define RC_ENABLE() RC_CTRL |= RC_EN

/** Enable disable RC
 * @param
 * @return void
 * Register: RC_CTRL, Bit RC_EN, Bit positions [0]
 */
#define RC_DISABLE() RC_CTRL &= ~(RC_EN)


#endif /* _LIB_MLX8110X_RC_CTRL_H_ */
