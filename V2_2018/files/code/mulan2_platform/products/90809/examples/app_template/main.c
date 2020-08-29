/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Blinks LED with 1 second period
 */
#include <ioports.h>
#include <syslib.h>

#define LED_TOGGLE()

/*
 *
 */
int main (void)
{



    for (;;) {
        
        ATOMIC_CODE(
            LED_TOGGLE();
        );
        
        MSEC_DELAY(500);
    }

    return 0;
}  


/* EOF */
