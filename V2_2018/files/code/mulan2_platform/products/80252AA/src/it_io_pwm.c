/*
 * Copyright (C) 2009 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * External IO and PWM interrupt handlers (connected to 2nd level interrupt controller)
 */
#include <syslib.h>
#include <itc2.h>

#define XI_PEND_REG         PWM_IO_IRQ_STAT         /* bits [7:0]        */
#define XI_MASK_REG         PWM_IO_IRQ_MASK         /* bits [7:0]        */

typedef void (* const Int_notificationFunctionType) (void);


extern void _fatal (void);
static void _unexpected_interrupt (void);

/* Link all interrupts to unexpected interrupt handler by default */
void IO_Ext0Notification (void)     __attribute__((weak,alias("_unexpected_interrupt")));
void IO_Ext1Notification (void)     __attribute__((weak,alias("_unexpected_interrupt")));
void IO_Ext2Notification (void)     __attribute__((weak,alias("_unexpected_interrupt")));
void IO_Ext3Notification (void)     __attribute__((weak,alias("_unexpected_interrupt")));
void IO_Ext4Notification (void)     __attribute__((weak,alias("_unexpected_interrupt")));
void IO_Ext5Notification (void)     __attribute__((weak,alias("_unexpected_interrupt")));
void PWMI_Notification (void)       __attribute__((weak,alias("_unexpected_interrupt")));
void PWMA_Notification (void)       __attribute__((weak,alias("_unexpected_interrupt")));


/*
 * Second level interrupt handlers
 */
static const Int_notificationFunctionType notificationFunctions[8] = {
    IO_Ext0Notification,
    IO_Ext1Notification,
    IO_Ext2Notification,
    IO_Ext3Notification,
    IO_Ext4Notification,
    IO_Ext5Notification,
    PWMI_Notification,
    PWMA_Notification   /* highest priority */
};

/*
 * Handler of unexpected interrupt
 */
static void _unexpected_interrupt (void)
{
    _fatal();
}

/*
 * IO_PWM_IRQ handler
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
__interrupt__ void IO_PWM_IRQ (void)
{
    uint16 pending;


    pending  = (uint16)XI_PEND_REG; /* copy interrupt requests from HW register */
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

        /* assert (m < (sizeof(notificationFunctions) / sizeof (Int_notificationFunctionType)); */

        /* call the 2nd level notification */
        notificationFunctions[m] ();
    }
}


/* EOF */
