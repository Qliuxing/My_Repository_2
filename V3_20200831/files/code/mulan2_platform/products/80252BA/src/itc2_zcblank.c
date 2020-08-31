/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <ioports.h>
#include <syslib.h>
#include <itc2.h>
#include "itc2_helper.h"


/* -----------------------------------------------------------------------------
 * Define first level handler
 */
ISR_L1_HANDLER (ITC1_ZCBLANK_INT, ZC_BLANK_JMP_VECTOR);


/* EOF */
