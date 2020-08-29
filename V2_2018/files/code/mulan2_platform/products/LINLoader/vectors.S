/* ------------------------------------------------------------------------------
 * Copyright (C) 2011-2015 Melexis N.V.
 *
 * Interrupt vector table
 * Software Platform
 */

#include <ioports.h>

;
; Helper macros to place vector to vector table
;
;   The format is:
;       FATAL_VECTOR (offset)
;       where <offset> is an offset of the vector
;
; @note:
;   - # Interrupt handler routine invoked by FATAL_VECTOR is executed
;       in USER mode with priority 0.

#define USER_MODE   (1 << 3)

    .macro FATAL_VECTOR offset
    .org    \offset, 0x0
    mov UPR, #(USER_MODE | 0)   ; USER mode with priority 0
    .word   0x76DA              ; JMPF but used as word to evade relaxation
    .word   (. - 2)             ; loop forever
    .endm

;-------------------------------------------------------------
; Interrupt vector table
    .section .vectors,"xa",@progbits
    .global __vectors


__vectors:
    .org 0                                      ; RST_WD_IT
    mov UPR, #0
#if defined (USE_PRESTART)
    jmp substart
#else
    jmp prestart
#endif /* USE_PRESTART */

    FATAL_VECTOR (0x0008)                       ; STACK_IT      ; abort current instruction, return NOT possible
    FATAL_VECTOR (0x0010)                       ; PROT_ERR_IT   ; abort current instruction, return NOT possible

/*
 *  For loaderB.hex, to fit into a single Flash page (0xBF00, 128 bytes) only
 *  RESET, STACK_IT and PROT_ERR_IT vectors are included.
 *  1) Vectors for non-maskable interrupt INV_AD_IT and PROG_ERR_IT are not provided.
 *     In case such interrupts (INV_AD_IT or PROG_ERR_IT) will be invoked for
 *     any reason, the sequence of the NOP intructions will be executed as a handler,
 *     until execution flow will reach the end of the Flash memory and, eventually,
 *     PROT_ERR_IT handler will be invoked. The NOP instructions in the area of
 *     interrupt handlers is a result of block-erase operation which leaves zeros in the
 *     erased areas and opcode of NOP instruction is 0.
 *  2) Other (maskable) interrupts are not used and masked in loaderB.hex
 *
 */
#if 0
    FATAL_VECTOR (0x0018)                       ; INV_AD_IT     ; abort current instruction, return NOT possible
    FATAL_VECTOR (0x0020)                       ; PROG_ERR_IT   ; abort current instruction, return NOT possible
    FATAL_VECTOR (0x0028)                       ; EXCHANGE_IT
    FATAL_VECTOR (0x0030)                       ; TASK_RST_IT
    FATAL_VECTOR (0x0038)                       ; WD_ATT_IT
    FATAL_VECTOR (0x0040)                       ; M4_MUTEX_IT
    FATAL_VECTOR (0x0048)                       ; M4_SHE_IT
    FATAL_VECTOR (0x0050)                       ; TIMER_IT
    FATAL_VECTOR (0x0058)                       ; ADC_IT
    FATAL_VECTOR (0x0060)                       ; EE_IT
    FATAL_VECTOR (0x0068)                       ; EXT0_IT ; Timer 1
    FATAL_VECTOR (0x0070)                       ; EXT1_IT ; Timer 2
    FATAL_VECTOR (0x0078)                       ; EXT2_IT ; PWMs
    FATAL_VECTOR (0x0080)                       ; EXT3_IT ; SPI
    FATAL_VECTOR (0x0088)                       ; EXT4_IT ; Analog + Custom
    FATAL_VECTOR (0x0090)                       ; SOFT_IT
#endif

    .text
#if defined (USE_PRESTART)
    substart:
#else
    prestart:
#endif /* USE_PRESTART */
        mov     A, FL_CTRL0     /* relocate MLX4 flash */
        or      A, #(1 << 9)
        mov     FL_CTRL0, A
        .word   0x76DA          /* JMPF but used as word to evade relaxation */
#if defined (USE_PRESTART)
        .word   __prestart
#else
        .word   _start
#endif

/* EOF */
