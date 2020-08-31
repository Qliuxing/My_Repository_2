/*
 * Copyright (C) 2005-2015 Melexis N.V.
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
#include "lin.h"

/*
 *
 */
__MLX_TEXT__  void ml_LinInit(void)
{
	(void)ml_InitLinModule();          /* Start and initialize the LIN Module */

	/* The LIN Module is now in the DISCONNECTED state */

#if STANDALONE_LOADER == 0
	/* Configure the Mlx4 software */
    (void)ml_SetOptions (1U,        /* IDStopBitLength = 1.5 Bit (Melexis LIN Master has 1.5 Tbit stop bit */
                    0U,             /* TXStopBitLength = 1 Bit */
                    ML_ENABLED,     /* StateChangeSignal */
                    ML_LIGHTSLEEP   /* SleepMode: lightsleep mode */
                   );
    (void)ml_SetSlewRate(ML_SLEWHIGH);
#else
    (void)ml_SetFastLoaderBaudRate();   /* default Fast Loader baud rate : defined by ML_FAST_BAUDRATE in Chip.mk */

    (void)ml_SetSlewRate(ML_SLEWFAST);
#endif /* STANDALONE_LOADER */
}

/* EOF */
