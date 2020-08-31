/*
 * Copyright (C) 2009-2010 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Toggling IO with 1 second period
 */
#include <ioports.h>
#include <syslib.h>

#include <iolib.h>

/*
 *
 */
int main (void)
{



    for (;;) {
        
        ATOMIC_CODE(
            IO_TOGGLE(0);
        );
        
        MSEC_DELAY(500);
    }

    return 0;
}  


/* EOF */
