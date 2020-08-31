/*
 * Copyright (C) 2005-2009 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <ioports.h>
#include <syslib.h>
#include <coretimerlib.h>
#include <wdlib.h>

/*
 * LED indication
 */
#define LED_WD_ACK()
#define LED_WD_RST()

/* Task should be executed at least TASK_TH times by the time 
 * of watchdog attention interrupt
 */
#define TASK_TH     (FPLL / 10) /*3000*/    /* assuming 7.5 MIPS, 1 mS watchdog period, 0.8 safety margin */
#define TO_BLOCK    200                     /* x20mS = 4 seconds */

volatile uint16_t task_cnt __attribute__((dp)); /* main task heartbeat */

static void tmr_init (void);

/*
 * MAIN
 */
int main (void)
{
    tmr_init();

    WD_INIT(WD_MODE_WINDOW, WD_DIV_8, 31);  /* WD period 1 mS */
    WD_INT_ENABLE();

    if ( WD_BOOT_CHECK() ) {
        LED_WD_RST();
    }
    /* else : power on reset */


    for (;;) {

        ATOMIC_CODE (
            task_cnt++;
        );

    }

    return 0;
}

/*
 * Watchdog attention (1/2 WD period)
 * 
 * The task which is allowed to acknowledge the watchdog
 */
__interrupt__ void WD_ATT_IT (void)
{
    /* check the health of main task */
    if (task_cnt > TASK_TH) {
        task_cnt = 0;

        WD_RESTART();
        LED_WD_ACK();
    }
    /* else :
     *  Failure detected. Task is not executed as often as expected
     *  Stop watchdog acknowledgement.
     *  Reset will be applied in 1/8 WD period
     */
}

/*
 * Period 20 mS
 */
void __interrupt__ CORE_TIMER_ISR (void)
{
    static uint16_t sw_tmr __attribute__((dp)) = TO_BLOCK; /* sw extension of the timer */

    if(--sw_tmr == 0) {

        for (;;) {
            /* block main task */
        }
    }
    /* else: timeout isn't expired yet */
}

/*
 * 
 */
static void tmr_init (void)
{
    CORE_TIMER_INIT(20000);   /* period 20 mS */
    CORE_TIMER_INT_ENABLE(4);
}

/* EOF */
