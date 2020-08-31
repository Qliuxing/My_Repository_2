/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * This example is compiled with a simple startup which only initializes the
 * stack pointer and calls main function. Other initializations should
 * be inside main (including analog trimming, PLL start etc)
 */

#include <alib.h>
#include <syslib.h>

/*
 *
 */
int main (void)
{
    while (1) {            /* idle loop */
        NOP();
    }

    return 0;
}

/* EOF */
