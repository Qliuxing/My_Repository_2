/*
 * Copyright (C) 2011-2015 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Timer test example
 *
 * Toggles IO4 output every 10 ms from Timer's compare interrupt
 *
 */
#include <syslib.h>
#include <plib.h>       /* product libs */

#include <itc2.h>
#include <timerlib.h>

/*
 * Prototypes
 */
static void TMR_Init (void);
static void LED_Init (void);
static void LED_Toggle (void);

#define TIMEOUT_IN_MS   10

#if ((TIMEOUT_IN_MS * FPLL + 128) / 256) > 0xFFFFu
#error "Specified TIMEOUT_IN_MS timeout can not be used with prescaler 1:256"
#endif


/*
 *
 */
int main (void)
{
    LED_Init();
    TMR_Init();

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
    LED_Toggle();  /* toggle IO */
}


/*
 *
 */
static  void TMR_Init (void)
{
    TIMER1_AUTOLOAD_INIT (  TIMER_DIV_256,
                            (TIMEOUT_IN_MS * FPLL + 128) / 256 - 1);

    EXT0_INT_ENABLE(3);         /* Enable EXT0 on 1st level interrupt controller */
    TIMER1_INT_RELOAD_ENABLE(); /* enable timer interrupts */
}

/*
 * Initialize LED
 */
static void LED_Init (void)
{
    IO_EXTIO = IO4_ENABLE;
}


/*
 * LED Toggle
 */
static void LED_Toggle (void)
{
    IO_EXTIO ^= IO4_OUT;
}

/* EOF */
