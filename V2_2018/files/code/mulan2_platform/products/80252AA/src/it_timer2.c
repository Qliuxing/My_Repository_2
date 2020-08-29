/*
 * Copyright (C) 2009 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Timer2 interrupt handlers (connected to 2nd level interrupt controller)
 */
#include <syslib.h>
#include <itc2.h>

#define XI_PEND_REG         SPI_TIMERS_IRQ_STAT     /* bits [12:8]        */
#define XI_MASK_REG         SPI_TIMERS_IRQ_MASK     /* bits [12:8]        */
#define XI_BASE_BIT         8                       /* starts from bit 8  */
#define XI_POSITION_MASK    (0x1Fu << XI_BASE_BIT)  /* bits [12:8]        */

typedef void (* const Int_notificationFunctionType) (void);


extern void _fatal (void);
static void _unexpected_interrupt (void);

/* Link all interrupts to unexpected interrupt handler by default */
void TMR2_Int1Notification (void)       __attribute__((weak,alias("_unexpected_interrupt")));
void TMR2_Int2Notification (void)       __attribute__((weak,alias("_unexpected_interrupt")));
void TMR2_Int3Notification (void)       __attribute__((weak,alias("_unexpected_interrupt")));
void TMR2_Int4Notification (void)       __attribute__((weak,alias("_unexpected_interrupt")));
void TMR2_Int5Notification (void)       __attribute__((weak,alias("_unexpected_interrupt")));

/*
 * Second level interrupt handlers
 */
static const Int_notificationFunctionType notificationFunctions[5] = {
    TMR2_Int1Notification,
    TMR2_Int2Notification,
    TMR2_Int3Notification,
    TMR2_Int4Notification,
    TMR2_Int5Notification   /* highest priority */
};

/*
 * Handler of unexpected interrupt
 */
static void _unexpected_interrupt (void)
{
    _fatal();
}

/*
 * TIMER2_IRQ handler
 *
 * First level interrupt handler
 *
 * NOTES:
 *
 *  1. Raising of 1st level interrupt request with no XPEND bit set on 2nd level
 *     Such situation is quite possible. The 2nd level interrupt request or several
 *     requests (XPEND) are ORed and generate 1st level interrupt request (PEND).
 *     CPU automatically clears 1st level request (in PEND register) and calls
 *     1st level interrupt handler.
 *     When another 2nd level requests come after entering 1st level interrupt
 *     handler, but before coping XPEND register to local variable 'pending' they
 *     will be also copied and processed already with this handler invocation.
 *     After exiting ISR it will be entered again (since PEND bit is active),
 *     but 2nd level requests have been already process by previous ISR invocation.
 *
 *  2. The 2nd level XPEND registers need to be handled completely by
 *     software, there is no automatic clear.
 *
 */
__interrupt__ void TIMER2_IRQ (void)
{
    uint16 pending;


    pending  = XI_PEND_REG;         /* copy interrupt requests from HW register */
    pending &= XI_POSITION_MASK;    /* mask requests from other peripherals     */
    pending &= XI_MASK_REG;         /* leave only request which are enabled     */
    
    XI_PEND_REG = pending;               /* clear requests which are going to be processed
                                          * Note: Any masked and in-between requests are still pending
                                          */

    /*
     * Call notification for all requests in pending
     * Requests are checked starting from msbit (highest priority)
     */
    while (0 != pending) {
        uint16 m;

        m = __fsb (pending);
        pending &= ~(1u << m);

        /* assert ((m - XI_BASE_BIT) < (sizeof(notificationFunctions) / sizeof (Int_notificationFunctionType)); */

        /* call the 2nd level notification */
        notificationFunctions[m - XI_BASE_BIT] ();
    }
}


/* EOF */
