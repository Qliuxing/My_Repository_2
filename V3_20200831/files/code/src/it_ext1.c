/*
 * Copyright (C) 2009 Melexis N.V.
 *
 * Software Platform
 *
 * Revision $Name: $
 *
 * File $RCSfile: it_ext1.c $
 *
 */

#include <syslib.h>
#include <itc2.h>
#include "Build.h"

extern void _fatal (void);

/* #define _SUPPORT_2ND_LEVEL_IRQ */
#ifdef _SUPPORT_2ND_LEVEL_IRQ

#define XI_PEND_REG     XI1_PEND
#define XI_MASK_REG     XI1_MASK

typedef void (* const Int_interruptFunctionType) (void);


static void _unexpected_interrupt (void);

/* Link all interrupts to unexpected interrupt handler by default */
void TMR2_Capture_A_Interrupt (void)  __attribute__((weak,alias("_unexpected_interrupt")));
void TMR2_Capture_B_Interrupt (void)  __attribute__((weak,alias("_unexpected_interrupt")));
void TMR2_Compare_A_Interrupt (void)  __attribute__((weak,alias("_unexpected_interrupt")));
void TMR2_Compare_B_Interrupt (void)  __attribute__((weak,alias("_unexpected_interrupt")));
void TMR2_Overflow_Interrupt  (void)  __attribute__((weak,alias("_unexpected_interrupt")));

/*
 * Second level interrupt handlers
 */
static const Int_interruptFunctionType intFunctions[16] = {
    _unexpected_interrupt,    /* reserved for Timer 4 */
    _unexpected_interrupt,
    _unexpected_interrupt,
    _unexpected_interrupt,
    _unexpected_interrupt,

    TMR2_Overflow_Interrupt,  /* T2_INT3 */
    TMR2_Compare_B_Interrupt, /* T2_INT4 */
    TMR2_Compare_A_Interrupt, /* T2_INT2 */
    TMR2_Capture_B_Interrupt, /* T2_INT5 */
    TMR2_Capture_A_Interrupt, /* T2_INT1 : highest priority */

    _unexpected_interrupt,    /* not used */
    _unexpected_interrupt,
    _unexpected_interrupt,
    _unexpected_interrupt,
    _unexpected_interrupt,
    _unexpected_interrupt
};

/*-----------------------------------------------------------------------------
 * Handler of unexpected interrupt
 */
static void _unexpected_interrupt (void)
{
	asm( "mov yl, #14");
    _fatal();
}
#endif /* _SUPPORT_2ND_LEVEL_IRQ */

/* PWM-support using MLX16 use other interrupt service routine (See PWM_Communication.c) */
__interrupt__ void EXT1_IT( void)
{
	asm( "mov yl, #14");
	_fatal();
}

/* EOF */
