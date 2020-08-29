/*
 * Copyright (C) 2005-2013 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef SYSLIB_H_
#define SYSLIB_H_

#include "typelib.h"
#include <ioports.h>
#include <mlx16_cfg.h>
#include <static_assert.h>

/* Abstraction for inlining */
#define INLINE  __attribute__((always_inline)) inline

/* define function as interrupt handler */
#define __interrupt__ __attribute__((interrupt))

/* place variable in direct page */
#define __DPAGE__ __attribute__((dp))

/* do not place variable in direct page */
#define __NODPAGE__ __attribute__((nodp))

/* section in the first half of the flash */
#define __MLX_TEXT__ __attribute__((section(".mlx_text")))

/* ---------------------------------------------------------------------------
 * Bit manipulations
 */
/* Bit value */
#define _BV(bit)    (1u << (bit))

/*
 * Find first set bit (starting from msbit)
 * fsb(0) = 0
 * fsb(1) = 0
 * fsb(0xffff) = 15
 */
#if defined(HAS_MLX16_FSB_SFB_INSTRUCTIONS)
static __inline__ int16  __fsb(uint16 v)  __attribute__ ((always_inline));
static __inline__ int16  __fsb(uint16 v)
{
    /* fsb(0) = 0 */
    __asm__ ("fsb %[res]" : [res] "=b" (v) : "0"(v));
    return (v);
}
#else
static __inline__ int16  __fsb(uint16 v)  __attribute__ ((always_inline));
static __inline__ int16  __fsb(uint16 v)
{
    int16 first_bit;
    /* fsb(0) = 0 */
    __asm__ __volatile__ (
         "mov X, #15\n"
         "Next_%=:\t"
         "lsl %1\n\t"
         "jc Done_%=\n\t"
         "djnz X, Next_%=\n"
         "Done_%=:"
         : "=x" (first_bit)
         : "b" (v)
         );

    return first_bit;
}
#endif  /* HAS_MLX16_FSB_SFB_INSTRUCTIONS */


/* ----------------------------------------------------------------------------
 * Blocking delays
 *
 */
#define DELAY(loops)            \
    __asm__ __volatile__ (      \
        "mov  X, %[cnt]\n\t"    \
        "djnz X,."              \
        :                       \
        : [cnt] "ri" (loops)    \
        : "X"                   \
    )


/*-----------------------------------------------------------------------------
 * Delays for 'us' microseconds
 */
#define DELAY_US(us)    DELAY((FPLL * (uint32)(us) + 2000) / 4000)


/*
 * Blocking delay for 'msec' milliseconds
 *
 * NOTES:
 *  1. Macro assumes 4 clocks per instruction cycle. However, in certain cases
 *     instruction cycle is extended by 1 or 2 additional clocks. In these
 *     cases accuracy of MSEC_DELAY macro is not guaranteed:
 *      1.1 in case of MLX4 activity the instruction cycle is equal
 *          to 5-6 clocks
 *      1.2 in some products the instruction cycle can be configured to be
 *          equal to 5 clocks
 */
static INLINE void MSEC_DELAY(int16 msec)
{
    int16 i;
    for(i = 0; i < msec; i++)
    {
        __asm__ __volatile__ (
            "mov X, %[cnt]\n\t"
            "djnz X,."
            :
            : [cnt] "i" (FPLL/4)
            : "X"
        );
    }
}

#define LOOP_FOREVER()  __asm__ __volatile__ ("jmp .")


/* ----------------------------------------------------------------------------
 * MLX16 helper functions
 *
 * NOTE: stack - should be in a register
 */
#define SET_STACK(stack)        \
do  {                           \
    __asm__ __volatile__ (      \
        "mov S, %[oper]"        \
		:                       \
		: [oper] "r" (stack)	\
        : "S"                   \
    );                          \
}	while(0)


#define SET_PRIORITY(prio)      \
do  {                           \
    __asm__ __volatile__ (      \
        "mov UPR, %[oper]"      \
		:                       \
        : [oper] "ri" (prio)    \
    );                          \
}   while(0)


#define ENTER_USER_MODE()   __asm__ __volatile__ ("setb MH.3")

/*
 * Switch to system mode keeping Priority register value
 */
#define ENTER_SYSTEM_MODE_KEEP_PRIO() __SYS_ENTER_PROTECTED_MODE  /* depends on ioports.h */

/*
 * Switch to system mode with zeroing of Priority register value
 */
#define ENTER_SYSTEM_MODE_PRIO_0()                                      \
do {                                                                    \
    ENTER_SYSTEM_MODE_KEEP_PRIO();                                      \
    SET_PRIORITY(0);                                                    \
} while(0)

/* Deprecated; use ENTER_SYSTEM_MODE_PRIO_0 or ENTER_SYSTEM_MODE_KEEP_PRIO instead */
#define ENTER_SYSTEM_MODE() ENTER_SYSTEM_MODE_PRIO_0()

#define NOP()               __asm__ __volatile__ ("nop"::) /* no operation */


#define MLX16_MASK_ALL_INT()    \
    do {                        \
        MASK = 0;               \
    } while (0)

#define MLX16_HALT()            \
        do {                    \
            CONTROL |= HALT;    \
            NOP();              \
        } while (0)

/*
 *
 */
extern void MLX16_RESET (void);  /* MLX16 reset is core/project specific */

/*
 * Return M register value
 */
static INLINE uint16 SYS_getCpuStatus (void)
{
    uint16 status;

    __asm__ __volatile__ (
        "mov %[dest], M" "\n\t"
        : [dest] "=r"(status)
        :
    );
    
    return status;
}

/*
 * Set M register value
 */
static INLINE void SYS_setCpuStatus (uint16 status)
{
    __asm__ __volatile__ (
        "mov M, %[src]"
        :
        : [src] "r" (status)
    );
}

/*
 * Clears M register
 */
static INLINE void SYS_clearCpuStatus (void)
{
    __asm__ __volatile__ (
        "mov A, #0" "\n\t"
        "mov M, A"  "\n\t"
        :
        :
        : "A"
    );
}


/* ----------------------------------------------------------------------------
 * ATOMIC_CODE
 * Wrapper to get atomic code execution block
 *
 * Notes:
 *  1. Local variable which stores CPU M register should have quite unique
 *     name (currently _mreg_saved) in order NOT to override any other
 *     variables with the same name in application code. If application code
 *     had variable with  _mreg_saved name and wrote to it inside ATOMIC_CODE
 *     block then local variable of ATOMIC_CODE would be written instead.
 *     Thus stored CPU status would be corrupted.
 */
static INLINE void __statusCleanup (const  uint16 *p)
{
    SYS_setCpuStatus(*p);
}

#define ATOMIC_CODE(__code__)                                           \
do  {                                                                   \
    uint16 _mreg_saved __attribute__((cleanup(__statusCleanup)));       \
                                                                        \
    _mreg_saved = SYS_getCpuStatus();                                   \
    ENTER_SYSTEM_MODE_PRIO_0();                                         \
    __code__                                                            \
} while (0)

/*
 * Execute code in system mode keeping Priority register value
 */
#define SYSTEM_CODE(__code__)                                           \
do  {                                                                   \
    uint16 _mreg_saved __attribute__((cleanup(__statusCleanup)));       \
                                                                        \
    _mreg_saved = SYS_getCpuStatus();                                   \
    ENTER_SYSTEM_MODE_KEEP_PRIO();                                      \
    __code__                                                            \
} while (0)


/* ----------------------------------------------------------------------------
 * MLX4 helper functions
 */
#define MLX4_RESET()        (CONTROL &= ~M4_RB)
#define MLX4_START()        (CONTROL |=  M4_RB)

#define ENABLE_MLX4_INT()   (MASK |=  EN_M4_SHE_IT)
#define DISABLE_MLX4_INT()  (MASK &= ~EN_M4_SHE_IT)

#define CLEAR_MLX4_INT()    (PEND = CLR_M4_SHE_IT)

#define GET_STATUS_MLX4_INT()   (MASK & EN_M4_SHE_IT)


#endif /* SYSLIB_H_ */
