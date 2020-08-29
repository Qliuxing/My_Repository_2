/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

/* 
 * TEST DESCRIPTION:
 *   This validation test target is interrupt controller issue detection.
 *   Due to this issue XIn_PEND register could be cleaned by another External Interrupt source.
 *   If this test FAIL, then two External Interrupt sources couldn't be resolved in a safe way.
 *
 * TEST DETAILS:
 *   For this validation test two timers with the different periods were used.
 *   During the WaitTime = Timer1_Period * Timer2_Period the moment exists when interrupts
 *   collision was occurred and, perhaps, few of the interrupt handlers weren't processed
 *   because of the issue. Every time, when interrupts comes, the counter inside each of
 *   them handlers is incremented. After the WaitTime, when issue should come, this counters
 *   are checked. This counters values will be different from estimated values if few
 *   interrupt handlers were missed.
 *
 *   As the result, if some of the interrupts during the WaitTime wasn't processed
 *   then the _fatal() is called and the IO4 output always high (if DEBUG_IOs_ENABLE).
 */
#include <syslib.h>
#include <plib.h>       /* product libs */

#include <itc2.h>
#include <timerlib.h>

/*
 * Timers options
 */
#define TIMER_MASTER_INTERVAL    5000
#define TIMERS_DIFF              1
#define TIMER_SLAVE_INTERVAL     (TIMER_MASTER_INTERVAL - TIMERS_DIFF)

/*
 * Enable debug markers
 */
#define DEBUG_IOs_ENABLE

/* Generate warning for debug options */
#if defined (DEBUG_IOs_ENABLE)
# warning "DEBUG_IOs_ENABLE is enabled: IO4 and IO5 are configured as outputs!"
#endif

/* External functions */
extern void _fatal(void);

/*
 * Prototypes
 */
static void TMR1_Init (void);
static void TMR2_Init (void);

/*
 * Globals
 */
static volatile uint16 timer1_pulses_cntr;
static volatile uint16 timer2_pulses_cntr;

/*
 * main
 */
int main (void)
{
#if defined(DEBUG_IOs_ENABLE)
    IO_EXTIO = IO4_ENABLE | IO5_ENABLE;
#endif

    TMR1_Init();
    TMR2_Init();

    timer1_pulses_cntr = 0;
    timer2_pulses_cntr = 0;

    while (1) {          /* idle loop */
        WDG_Manager();
    }
    return 0;
}

/*
 * Timer1 interrupt
 */
void TMR1_Compare_B_Interrupt (void)
{
    timer1_pulses_cntr++;

#if defined(DEBUG_IOs_ENABLE)
    IO_EXTIO |= IO5_OUT;
    DELAY_US(7);
    IO_EXTIO ^= IO5_OUT;
#endif
}

/*
 * Timer2 interrupt
 */
void TMR2_Compare_B_Interrupt (void)
{
    timer2_pulses_cntr++;

    /*
     * After WaitTime = Timer1_Period * Timer2_Period
     * 'timer2_pulses_cnt' should be equal to Timer1_Period (TIMER_MASTER_INTERVAL) and
     * 'timer1_pulses_cnt' should be equal to Timer2_Period (TIMER_SLAVE_INTERVAL)
     * Otherwise, some of interrupts handlers weren't processed!
     *
     * According to the issue behavior, it looks like low priority interrupt spoils XIn_PEND
     * of high priority interrupt. So, the next algorithm is rightful:
     *     1. check that WaitTime was expired by comparison 'timer2_pulses_cntr' with
     *        estimated value;
     *     2. if 'timer1_pulses_cntr' is different from estimated value, then some
     *        of Timer1 interrupt handlers weren't processed --> error output
     */
    if (timer2_pulses_cntr >= TIMER_MASTER_INTERVAL) {
        if (timer1_pulses_cntr != TIMER_SLAVE_INTERVAL) {
            /* Error output */
            ATOMIC_CODE( IO_EXTIO |= IO4_OUT;
                         _fatal();
                       );
        }
        timer1_pulses_cntr = 0;
        timer2_pulses_cntr = 0;
    }

#if defined(DEBUG_IOs_ENABLE)
    IO_EXTIO |= IO4_OUT;
    DELAY_US(7);
    IO_EXTIO ^= IO4_OUT;
#endif
}

/*
 * Timer1 init
 */
static  void TMR1_Init (void)
{
    TIMER1_AUTOLOAD_INIT (TIMER_DIV_1, TIMER_MASTER_INTERVAL);
    EXT0_INT_ENABLE(3);         /* Enable EXT0 on 1st level interrupt controller */
    TIMER1_INT_RELOAD_ENABLE(); /* enable timer interrupts */
}

/*
 * Timer2 init
 */
static  void TMR2_Init (void)
{
    TIMER2_AUTOLOAD_INIT (TIMER_DIV_1, TIMER_SLAVE_INTERVAL);
    EXT1_INT_ENABLE(6);         /* Enable EXT1 on 1st level interrupt controller */
    TIMER2_INT_RELOAD_ENABLE(); /* enable timer interrupts */
}


/* EOF */
