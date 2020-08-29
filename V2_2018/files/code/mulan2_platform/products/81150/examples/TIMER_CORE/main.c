/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

/* 
 * Core timer test example
 *
 * Toggles PWM1 output from Core timer interrupt which is generated every 1 ms.
 *
 */
#include <plib.h>       /* product libs */
#include <ioports.h>
#include <syslib.h>
#include <coretimerlib.h>

/*
 * IO macros
 * 
 * Output pin is PWM1
 */
#define  IO_INIT()                                      \
    do {                                                \
        ANA_OUTG = 7;   /* enable PS and PWM1, PWM2 */  \
        ANA_OUTK = PWMMODE;                             \
    } while (0)

#define  IO_OFF()       ( ANA_OUTK &= ~OPWM0 )
#define  IO_ON()        ( ANA_OUTK |=  OPWM0 )


static void core_timer_init (void);

/*
 *
 */
int main (void)
{
    IO_INIT();
    core_timer_init();

    for (;;) {
        WDG_Manager();
    }

    return 0;
}  

/*
 * Initializes core timer
 */
static void core_timer_init (void)
{
    CORE_TIMER_INIT(1000);      /* period 1000 uS */
    CORE_TIMER_INT_ENABLE(4);   /* enable timer's interrupt with SW priority 4 */
}


/*
 * Core Timer ISR
 */
__interrupt__ void CORE_TIMER_ISR (void)  /* priority inside interrupt is 3 */
{
    static uint8_t led_state __attribute__((dp));


    if ( 0 == led_state ) {  /* change output depending on previous state */
        IO_ON();       
    }
    else {   
        IO_OFF();
    }

    led_state++;        /* toggle the state */
    led_state &= 1;
}

/* EOF */
