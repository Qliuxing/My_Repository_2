/*
 * Copyright (C) 2009 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Diagnostic interrupt handlers (connected to 2nd level interrupt controller)
 */
#include <syslib.h>
#include <itc2.h>

#define XI_PEND_REG         DIAG_IRQ_STAT       /* bits [8:0]  */
#define XI_MASK_REG         DIAGNOSTIC_MASK     /* bits [8:0]  */
#define XI_POSITION_MASK    (0x1FFu << 0)       /* bits [8:0]  */

typedef void (* const Int_notificationFunctionType) (void);


extern void _fatal (void);
static void _unexpected_interrupt (void);

/* Link all interrupts to unexpected interrupt handler by default */
void DIAG_OvercurrentNotification (void)    __attribute__((weak,alias("_unexpected_interrupt")));
void DIAG_VgsErrorNotification (void)       __attribute__((weak,alias("_unexpected_interrupt")));
void DIAG_VdsErrorNotification (void)       __attribute__((weak,alias("_unexpected_interrupt")));
void DIAG_OvertempNotification (void)       __attribute__((weak,alias("_unexpected_interrupt")));
void DIAG_VboostUvNotification  (void)      __attribute__((weak,alias("_unexpected_interrupt")));
void DIAG_VregUvNotification (void)         __attribute__((weak,alias("_unexpected_interrupt")));
void DIAG_VsupUvNotification (void)         __attribute__((weak,alias("_unexpected_interrupt")));
void DIAG_VsupOvNotification (void)         __attribute__((weak,alias("_unexpected_interrupt")));
void DIAG_CurrRegNotification (void)        __attribute__((weak,alias("_unexpected_interrupt")));

/*
 * Second level interrupt handlers
 */
static const Int_notificationFunctionType notificationFunctions[9] = {
    DIAG_OvercurrentNotification,  /* OVER_CURRENT */
    DIAG_VgsErrorNotification,     /* VGS_ERROR    */
    DIAG_VdsErrorNotification,     /* VDS_ERROR    */
    DIAG_OvertempNotification,     /* OVER_TEMP    */
    DIAG_VboostUvNotification,     /* VBOOST_UV    */
    DIAG_VregUvNotification,       /* VREG_UV      */
    DIAG_VsupUvNotification,       /* VSUP_UV      */
    DIAG_VsupOvNotification,       /* VSUP_OV      */
    DIAG_CurrRegNotification,      /* CURREG : highest priority */
};

/*
 * Handler of unexpected interrupt
 */
static void _unexpected_interrupt (void)
{
    _fatal();
}

/*
 * DIAG_IRQ handler
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
__interrupt__ void DIAG_IRQ (void)
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

        /* assert (m < (sizeof(notificationFunctions) / sizeof (Int_notificationFunctionType)); */

        /* call the 2nd level notification */
        notificationFunctions[m] ();
    }
}


/* EOF */
