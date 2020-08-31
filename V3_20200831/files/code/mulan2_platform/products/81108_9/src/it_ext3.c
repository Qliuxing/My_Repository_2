/*
 * Copyright (C) 2009 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <syslib.h>
#include <itc2.h>

#define XI_PEND_REG     XI3_PEND
#define XI_MASK_REG     XI3_MASK

typedef void (* const Int_interruptFunctionType) (void);


extern void _fatal (void);
static void _unexpected_interrupt (void);

/* Link all interrupts to unexpected interrupt handler by default */
void SPI1_RX_Interrupt (void)  __attribute__((weak,alias("_unexpected_interrupt")));
void SPI1_TX_Interrupt (void)  __attribute__((weak,alias("_unexpected_interrupt")));


/*
 * Second level interrupt handlers
 */
static const Int_interruptFunctionType intFunctions[16] = {

    _unexpected_interrupt,    /* reserved for UART 2 */
    _unexpected_interrupt,
    _unexpected_interrupt,
    _unexpected_interrupt,
    _unexpected_interrupt,
    _unexpected_interrupt,

    _unexpected_interrupt,    /* reserved for UART 1 */
    _unexpected_interrupt,
    _unexpected_interrupt,
    _unexpected_interrupt,
    _unexpected_interrupt,
    _unexpected_interrupt,

    _unexpected_interrupt,    /* reserved for SPI 2 */
    _unexpected_interrupt,

    SPI1_TX_Interrupt,
    SPI1_RX_Interrupt   /* highest priority */
};

/*-----------------------------------------------------------------------------
 * Handler of unexpected interrupt
 */
static void _unexpected_interrupt (void)
{
    _fatal();
}

/*-----------------------------------------------------------------------------
 * EXT3_IT handler
 *
 * NOTES:
 *
 *  1. Raising of 1st level interrupt request EXT<n>_IT with no XPEND bit
 *     set on 2nd level
 *     Such situation is quite possible. The 2nd level interrupt request or several
 *     requests (XPEND) are ORed and generate 1st level interrupt request (PEND).
 *     CPU automatically clears 1st level request (in PEND register) and calls
 *     1st level interrupt handler EXT<n>_IT.
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
__interrupt__ void EXT3_IT (void)
{
    uint16 pending;


    pending = XI_PEND_REG & XI_MASK_REG; /* copy interrupt requests which are not masked   */
    XI_PEND_REG = pending;               
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

        /* assert (m < (sizeof(intFunctions) / sizeof (Int_interruptFunctionType)); */

        /* call the 2nd level ISR/notification */
        intFunctions[m] ();
    }
}


/* EOF */
