/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Application:
 *  Toggling IO with 1 second period using timer1 interrupt in autoreload mode
 *
 * Usage notes:
 *
 *  1. Find the name of 2nd level interrupt handler in file itc2.h
 *     E.g. for irq4 of timer1 the name is ITC2_T1_IRQ4
 *     Names are predefined and can not be changed. If name is misspelled for
 *     some reason, hanlder will not be called (default handler will be called
 *     instead).
 *
 *  2. Define 2nd level handler function using the name obtained at step (1),
 *     e.g. ITC2_T1_IRQ4. Hanlder function should be defined as a function without
 *     arguments and return value: void <name> (void).
 *     Please note, that __interrupt__ attribute should NOT be used for these
 *     2nd level functions.  Also it is important to call acknowledge
 *     function <name>_ACKNOWLEDGE  somewhere in the handler to acknowledge
 *     the 2nd level interrupt. Otherwise handler will be reentered again.
 * 
 *  3. Find the name of 1st level dispatcher which corresponds to target 2nd level
 *     interrupt (in file itc2.h). E.g. for ITC2_T1_IRQ4 interrupt the dispatcher's
 *     name is ITC1_TIMER1_INT. Add this dispatcher's name to corresponding position of
 *     1st level vector table in file vectors.S. Also specify ISR-priority for this
 *     dispatcher (usually one level down compared to runtime priority of the
 *     interrupt).
 *     E.g. for ITC1_TIMER1_INT:
 *          CALLVECTOR (0x0040, ITC1_TIMER1_INT, 1)
 *
 *  4. During initialization make calls:
 *          ITC2_Init           to initialize 2nd level interrupt controller
 *          <L2_name>_ENABLE    to enable target 2nd level interrupt
 *          <L1_name>_ENABLE    to enable 1st level dispatcher and set it's SW
 *                              priority (if any)
 */

#include <ioports.h>

#include <syslib.h>
#include <itc2.h>
#include <timerlib.h>

#include <iolib.h>

/*
 *
 */
int main (void)
{
    TIMER1_AUTOLOAD_INIT(TIMER_DIV_256, 0.5 * FPLL * 1000 / 256 - 0.5); /* setup 0.5 sec period */

    ITC2_Init();
    ITC2_T1_IRQ4_ENABLE();      /* enable reload interrupt (irq4) at 2nd level ITC  */
    ITC1_TIMER1_INT_ENABLE();   /* enable at 1st level ITC for timer1               */

	while (1) {
	   /* idle loop */
    }

	return 0;
}

/*----------------------------------------------------------
 * Reload mode interrupt handler
 *
 */
void ITC2_T1_IRQ4 (void)
{
	IO_TOGGLE(0);

    ITC2_T1_IRQ4_ACKNOWLEDGE();
}


/* EOF */
