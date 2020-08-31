/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef ITC2_HELPER_H_
#define ITC2_HELPER_H_


/*-----------------------------------------------------------------------------
 * Indirect call of the function which address is stored at location vect_addr
 *
 * Arguments:   vect_addr   address of the vector; second level handler will
 *                          be called via this vector
 *
 * Returns:     none
 *
 * Notes:
 *      1. This function emulates call [x] asm instruction using jmp [x]
 *         instruction.
 *      2. Warning: Compiler is not aware of indirect function call via jmp[x].
 *         Thus it will not preserve any registers allocated for local objects
 *         for the duration of function call (all registers of MLX16 are
 *         call-used registers and might be freely used by callee). So function
 *         ITC_CallHandler should itself take care of register preserving.
 *         Clobber list partly solves this (except X register).
 */
inline __attribute__((always_inline))
static  void ITC2_IndirectCall (const uint16 vect_addr)
{

    __asm__ __volatile__ (
                    "jmp L1_%=" "\n\t"
        "L0_%=: "   "jmp [%0]"  "\n\t"
        "L1_%=: "   "call L0_%="
                    :
                    : "x" (vect_addr)
                    : "Y", "A", "memory"    /* all registers should be saved,
                                             * since indirect function call
                                             * jmp[x] is used in this function
                                             */
    );
}


/*----------------------------------------------------------------------------
 * First level interrupt handler template
 *
 * Notes:
 *  1. Warning! Check assembler listing after compilation to be sure that
 *     content of the X register is NOT used after ITC2_IndirectCall invocation.
 *     If it is used then the X register must be preserved/restored (or reloaded)
 *     after the ITC2_IndirectCall funtion call. It is because the function
 *     invoked via ITC2_IndirectCall could potentially use the X register for
 *     own purposes and thus corrupt it's previous content.
 */
#define ISR_L1_HANDLER(name, jump_vector)       \
__interrupt__ void name (void)                  \
{                                               \
    uint16 vect_addr;                           \
                                                \
    vect_addr = jump_vector;                    \
                                                \
    while ( vect_addr != 0 ) {                  \
        ITC2_IndirectCall(vect_addr);           \
        vect_addr = jump_vector;                \
    };                                          \
}


#endif /* ITC2_HELPER_H_ */
