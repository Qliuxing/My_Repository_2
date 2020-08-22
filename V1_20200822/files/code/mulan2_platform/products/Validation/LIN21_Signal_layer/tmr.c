/*
 * Timer module
 * Copyright (C) 2008 Melexis N.V.
 */

#include <plib.h>       /* product libs */
#include <coretimerlib.h>

#include "lin_api.h"
#include "tmr.h"

#define TIMEOUT_IN_MS   10
#define TO_N_AS_LIN     100 /* x10 mS = 1000 mS */

/*
 *  Module's variables
 */
static volatile uint8 tmrTL;

/*
 *  Configuration check
 */

/* --- TO_N_AS_LIN --------------------------------------------------------- */
#if (TO_N_AS_LIN < 1) || (TO_N_AS_LIN > 255)
#error "TO_N_AS_LIN must be in range of [1..255]"
#endif

/*
 ******************************************************************************
 * Timer initialization
 ******************************************************************************
 */
void tmr_init (void)
{
    CORE_TIMER_INIT(1000 * TIMEOUT_IN_MS);      /* period 10 mS = 10000 us */
       ATOMIC_CODE(
               CORE_TIMER_INT_ENABLE(3U);   /* enable timer's interrupt with SW priority 4 */
       );
}

/*
 ******************************************************************************
 * Starts Transport Layer N_As_LIN timeout (1000 mS)
 * See LIN2.1 spec section 3.2.5 TIMING CONSTRAINTS
 ******************************************************************************
 */
void mlu_TL_TimeoutStart (void)
{
    tmrTL = TO_N_AS_LIN; /* MUST be compiled into atomic operation */
}

/*
 ******************************************************************************
 * TimerA ISR
 * period: 10 mS
 ******************************************************************************
 */
__interrupt__ void CORE_TIMER_ISR (void)  /* priority inside interrupt is 3 */
{
    /* 
     * Note that 'tmrTL' is also accessed from LIN ISR:
     *   LIN_ISR -> mlu_MessageReceived -> ml_DiagMasterRequest -> ...
     *           ... -> mlu_TL_TimeoutStart -> tmrTL
     * To guarantee ATOMIC access to 'tmrTL' variable disable interrupts
     * by raising CPU priority
     */
    SET_PRIORITY(1);
    if (tmrTL != 0) {
        --tmrTL;
        if (tmrTL == 0) {
            ml_DiagClearPendingResponse();
        }
        /* else : in progress */
    }
    /* else : timeout is not started yet  */
}

/* EOF */
