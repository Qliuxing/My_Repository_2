/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

/* 
 * Timer test example
 *
 * Toggles PWM1 output from Timer's compare interrupt which is generated every 1 ms.
 *
 */


#include <syslib.h>
#include <plib.h>       /* product libs */

#include <itc2.h>
#include <timerlib.h>

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
#define  IO_TOGGLE()    ( ANA_OUTK ^=  OPWM0 )

#define TIMEOUT_IN_MS   10

#if ((TIMEOUT_IN_MS * FPLL + 128) / 256) > 0xFFFFu
#error "Specified TIMEOUT_IN_MS timeout can not be used with prescaler 1:256"
#endif


/*
 *
 */
static void hw_init (void);

/*
 *
 */
int main (void)
{
    hw_init();
    IO_INIT();

    while (1) {            /* idle loop */
        WDG_Manager();
    }

    return 0;
}

/*
 *
 */
void TMR1_Compare_B_Interrupt (void)
{
    IO_TOGGLE();  /* toggle IO */
}


/*
 *
 */
static  void hw_init (void)
{
    TIMER1_AUTOLOAD_INIT (  TIMER_DIV_256,
                            (TIMEOUT_IN_MS * FPLL + 128) / 256 - 1);

    EXT0_INT_ENABLE(3);         /* Enable EXT0 on 1st level interrupt controller */
    TIMER1_INT_RELOAD_ENABLE(); /* enable timer interrupts */
}

/* EOF */
