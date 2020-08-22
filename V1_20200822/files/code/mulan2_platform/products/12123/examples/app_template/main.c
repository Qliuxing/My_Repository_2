/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <ioports.h>
#include <syslib.h>
#include "ext_module.h"

/*
 *  Test dp variables
 */
volatile uint16_t var_default;
volatile uint16_t var_dp   __attribute__((dp));
volatile uint16_t var_nodp __attribute__((nodp));


/*
 *
 */
int main (void)
{

    var_default = 0x0123;
    var_dp      = 0x4567;
    var_nodp    = 0x89AB;


    while (1) {
        /* main loop */
    }

    return 0;
}

/* EOF */
