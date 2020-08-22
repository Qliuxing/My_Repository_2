/*
 * Copyright (C) 2009-2010 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <syslib.h>
#include <itc2.h>
#include <timerlib.h>

#include <iolib.h>

static void hw_init (void);

/*
 *
 */
int main (void)
{
	hw_init();

	for (;;) {
	   /* idle loop */
    }

	return 0;
}

/*
 *
 */
void TMR1_Int4Notification (void)
{
	IO_TOGGLE(0);
}


/*
 *
 */
static  void hw_init (void)
{
    /* setup 0.5 sec period */
	TIMER1_AUTOLOAD_INIT(TIMER_DIV_256, 0.5 * FPLL * 1000 / 256 - 0.5);

    /* Enable TIMER1_IRQ on 1st level interrupt controller */
    TIMER1_IRQ_ENABLE();

    /* enable timer interrupts */
	TIMER1_INT_RELOAD_ENABLE();
}

/* EOF */
