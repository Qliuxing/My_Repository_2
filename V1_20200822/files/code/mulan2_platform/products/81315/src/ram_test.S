/* ------------------------------------------------------------------------------
 * Copyright (C) 2012-2015 Melexis N.V.
 *
 * RAM test
 *
 */


#define _SUPPORT_RAM_TEST_3LOOPS   1


.global _RAM_Test

    .section .mlx_text,"xa",@progbits

    _tRamTestPattern:
        .word 0xB66D    /* 0110 1101 1011 0110 */
        .word 0x6DDB    /* 1101 1011 0110 1101 */
        .word 0xDBB6    /* 1011 0110 1101 1011 */
#if _SUPPORT_RAM_TEST_3LOOPS
        .word 0xB66D    /* 0110 1101 1011 0110 */
        .word 0x6DDB    /* 1101 1011 0110 1101 */
#endif


/* ----------------------------------------------------------------------------
 * Destructive RAM test
 *  \note:
 *  1. RAM test must be performed before MLX4 is started
 *  2. Stack shall not be used
 *  3. Total test time is between 1.4 - 1.8 ms (45k Clocks) or 5ms for 3 loops
 */
_RAM_Test:  /* void RAM_Test (void); */

    lod y, #_tRamTestPattern
_lbl_LoopPattern:
    mov d, ya
    /* Fill RAM with pattern */
    lod x, #0
    movsw [x++], [y++]                                          /* Write first three 16-bits words with test-pattern */
    movsw [x++], [y++]
    movsw [x++], [y++]
    lod y, #0
_lbl_LoopFill:
    movsw [x++], [y++]                                          /* Repeat test-pattern for the rest of the RAM area */
	cmp x, #0x07FE
	jule _lbl_LoopFill

    /* Check RAM pattern */
    mov ya, d
    lod x, #0
    lod a, [x++]
    cmp a, [y++]
    jne _lbl_LoopFail

    lod a, [x++]
    cmp a, [y++]
    jne _lbl_LoopFail

    lod a, [x++]
    cmp a, [y++]
    jne _lbl_LoopFail

    lod y, #0
_lbl_LoopCheck:
    lod a, [x++]                                                /* Check remaining RAM area against previously check pattern */
    cmp a, [y++]
    je _lbl_LoopNext

_lbl_LoopFail:
    mov yl, #0xF3                                               /* RAM-failure */
	jmp __fatal

_lbl_LoopNext:
	cmp x, #0x07FE
	jule _lbl_LoopCheck

#if _SUPPORT_RAM_TEST_3LOOPS
    mov ya, d
    add y, #2
    cmp y, #_tRamTestPattern + 6
    jne _lbl_LoopPattern
#endif /* _SUPPORT_RAM_TEST_3LOOPS */

    jmp _start                                                  /* continue initialization .. */
    end

/* EOF */
