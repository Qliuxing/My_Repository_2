/*
 * Copyright (C) 2013 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 *  Test functios used during validation
 */

#include <plib.h>
#include <coretimerlib.h>

#define TMR_PERIOD_10MS         (10000UL / 1)       /* one tick 1 us */

/*
 * Initialze test pin
 */
void MLX_InitTestFramework (void)
{
    CORE_TIMER_INIT(TMR_PERIOD_10MS);   /* start timer */
    CORE_TIMER_INT_ENABLE(3);


    IO_EXTIO = IO4_ENABLE;  /* Init IO4 pin */
}

/*
 * Toggle test pin
 */
void MLX_ToggleTestPin (void)
{
    IO_EXTIO ^= IO4_OUT;    /* toggle IO4 pin */
}

/*
 *
 */
__interrupt__ void MLX_TestFrameworkIsr (void)
{
    WDG_Manager();  /* unconditionally acknowledge watchdog in the test */
}



/* EOF */
