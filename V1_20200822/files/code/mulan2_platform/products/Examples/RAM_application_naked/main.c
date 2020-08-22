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
 *
 * Notes:
 *    1. The POR address for RAM programm can be found in lss file by
 *    searching _vectors symbol.
 *
 *    2. Generated bin file can be loaded into MLX16-Simulator as RAM data:
 *    Memory Dump > RAM > File... from address 0. The PC register should be
 *    adjusted to 0. After that example can be debugged in MLX16-Simulator.
 */

#include <alib.h>
#include <syslib.h>
#include <plib.h>       /* product libs */

/*
 *
 */
int main (void)
{
    while (1) {            /* idle loop */
        WDG_Manager();
    }

    return 0;
}

/* EOF */
