;------------------------------------------------------------------------------
; Copyright (C) 2011 Melexis N.V.
;
; Simple interrupt vector table
;

;-----------------------------------------------------------------------
; Interrupt vector table
    .section .vectors,"xa",@progbits
    .global __vectors


__vectors:
	.org	0x0

    /*
     * A simple startup which only initializes the stack pointer and calls main function.
     * Other initializations should be inside main (including analog trimming, PLL start etc)
     */
	mov UPR, #0
    mov A, #_stack
    mov S, A
    call _main
    jmp .

; EOF
