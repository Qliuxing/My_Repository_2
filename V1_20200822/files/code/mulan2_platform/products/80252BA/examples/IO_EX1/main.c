/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Application:
 *  Toggling IO with 1 second period using SW delay
 */
#include <ioports.h>

#include <syslib.h>

#include <iolib.h>


/*
 *
 */
int main (void)
{
    SEL_IO_CONFIG = 0;  /* All EXT_IOs in Open drain mode */

    for (;;) {
        
        IO_TOGGLE(2);  /* Toggle EXT_IO2 output */
        
        MSEC_DELAY(500);
    }

    return 0;
}  


/* EOF */
