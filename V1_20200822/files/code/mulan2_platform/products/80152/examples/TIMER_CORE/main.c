/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Application:
 *  Toggling IO with 1 second period using 15-bit core timer's interrupt
 */
#include <ioports.h>

#include <syslib.h>
#include <coretimerlib.h>
#include <iolib.h>

#define TO_BLINK    125     /* x4mS = 500 mS */
uint16_t sw_tmr __attribute__((dp));  /* sw extension of the timer  */

static void init_timer (void);

/*
 *
 */
int main (void)
{
    init_timer();
    
    for (;;) {
        /* empty */        
    }

    return 0;
}  

/*
 * Initializes core timer
 */
static void init_timer (void)
{
    sw_tmr = TO_BLINK;

    CORE_TIMER_INIT(4000);   /* period 4 mS */
    CORE_TIMER_INT_ENABLE(4);
}

/*
 * Period 4 mS
 */
__interrupt__ void CORE_TIMER_INT (void)
{
    static uint8_t io_state __attribute__((dp));


    if(--sw_tmr == 0) {
        sw_tmr = TO_BLINK;      /* reload timeout */

        if ( 0 == io_state ) {  /* change output depending on previous state */
            IO_SET(0);
        }
        else {   
            IO_RESET(0);
        }

        io_state++;        /* toggle the state */
        io_state &= 1;
    }   
}

/* EOF */
