;
; Copyright (C) 2008-2013 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

#if defined (HAS_MLX16_FSB_SFB_INSTRUCTIONS)
    /* table is not needed */
#else
	;; MLX16
#if 0
	const uint16 sfb2_table16[8] = {
	0x0001, /* 1 <<  0 */
	0x0004, /* 1 <<  2 */
	0x0010, /* 1 <<  4 */
	0x0040, /* 1 <<  6 */
	0x0100, /* 1 <<  8 */
	0x0400, /* 1 << 10 */
	0x1000, /* 1 << 12 */
	0x4000, /* 1 << 14 */
	};
#endif

	.section .rodata	; read-only data (should be in ROM)
	.balign	 2
	.type	__sfb2_table16, @object
__sfb2_table16:
	.word	1
	.word	4
	.word	16
	.word	64
	.word	256
	.word	1024
	.word	4096
	.word	16384
#endif /* HAS_MLX16_FSB_SFB_INSTRUCTIONS */

	.text
	.global	_isqrt32_helper
	
#include "math.mac"

	.macro SFB_X
	;; pre: X != 0
	;; pre: X < 16
	push	A			; mov Cx, X not available
	mov	A, X
	mov	Cx, Al
	mov	X, #1
	lsl	X
	djnz	Cx, .-2
	pop	A
	.endm

	.macro SFB2_X
	;; pre: X != 0
	;; pre: X < 16
	;; pre: X is even
	push	A			; mov Cx, X not available
	mov	A, X
	lsr	A, #1		;X even, shift by 2 at once
	mov	Cx, Al		
	mov	X, #1
	lsl	X, #2
	djnz	Cx, .-2
	pop	A
	.endm

	.macro SFB22_X
	;; pre: X < 16
	;; pre: X is even
	;; x = sfb2_table[x>>1]
	;; sfb2_table is a table of 8 words (uint16) -> x/2
	;; as table of uint16, pointer arithmetic -> *2
	add	X, #__sfb2_table16
	mov	X, [X]
	.endm

	

;
;	isqrt32 helper function

;	Algorithm:	
;	one = 1 << idx;
;	res = 0;
;	while (one != 0) {
;	  if (op >= (res + one)) {
;	    op -= res + one;
;	    res = res +  2 * one;
;	  }
;	  res >>= 1;
;	  one >>= 2;
;	}
;
;	->
;	one = 1 << idx;
;	res = 0;
;	while (one != 0) {
;	  res += one;
;	  if (op >= res) {
;	    op -= res;
;	    res += one;
;	  } else {
;	    res -= one;
;	  }
;	  res >>= 1;
;	  one >>= 2;
;	}
;
;	NB. += one -> |= one
;
;	Need 2 32 bit variables: op, res
;	split loop in two:
;	- one loop for MSW of "one"
;	- second loop for LSW of "one"
;	to avoid need for third 32 bit variable and stack storage
;	also optimize 32 bit add/subtract res +-= one

; Input:
;	A	idx (even; <= 30)
;	[S-6]	op low part (16 LSW) op - 32 bit unsigned operand
;	[S-4]	op high part (16 MSW)
;
; Output:
;	A      16 bit unsigned integer result
;
; Temporary:
;	YA	res

#if defined (HAS_MLX16_COPROCESSOR)
_isqrt32_helper:	
	;; isqrt (0) -> = 0
	;; isqrt (1) -> isqrt_helper(0 (idx)) -> 1
	;; isqrt (2) -> isqrt_helper(0 (idx)) -> 1
	;; isqrt (3) -> isqrt_helper(0 (idx)) -> 1

	;; avoids unnecessary first time loop bounds
	cmp	A, #1
	jug	iinit
	mov	A, #1
	ret			;return A

iinit:
	;; variable initialization
	mov	X, A		; idx
	mov	Y, [S-4] 	; op
	mov	A, [S-6]
	mov	D, YA		; op
	mov	YA, #0		; res = 0

	;; split in 2 loops
	cmp	X, #16
	jul	start2
	sub	X, #16
	sfb	X		; one = 1 << (idx-16)

	;cmp	X, #0
	;jne	loop2
loop1:
	;add	YA, X<<16
	add	Y, X		; res' = res + one
	cmp	D, YA
	juge	then1
	;sub	YA, X<<16
	sub	Y, X		; res = res' - one
loop1_next:
	;LSR	YA, #1		; res >>= 1
	lsr	Y, #1
	lsr	X, #2		; one >>= 2
	;cmp	X, #0		
	;jne	loop1
	jnz	loop1
end1:	

	;; initialize second loop
	mov	X, #14
start2:	
	sfb	X		; one = 1 << (idx-16)

	;cmp	X, #0
	;jne	loop2
loop2:
	;addu	YA, X
	add	A, X		; res' = res + one
	cmp	D, YA
	juge	then2
	;subu	YA, X
	sub	A, X		; res = res' - one
loop2_next:
	LSR	YA, #1		; res >>= 1
	lsr	X, #2		; one >>= 2
	;cmp	X, #0
	;jne	loop2
	jnz	loop2

	ret			;return A (Y == 0)

then1:
	sub	D, YA		; op = op - res' = op - (res + one)
	;add	YA, X<<16
	add	Y, X		; res = res' + one = res + 2 * one
	jmp	loop1_next
then2:
	sub	D, YA		; op = op - res' = op - (res + one)
	;addu	YA, X
	add	A, X		; res = res' + one = res + 2 * one
	jmp	loop2_next
	.size	_isqrt32_helper, .-_isqrt32_helper

#else
_isqrt32_helper:	
	;; isqrt (0) -> = 0
	;; isqrt (1) -> isqrt_helper(0 (idx)) -> 1
	;; isqrt (2) -> isqrt_helper(0 (idx)) -> 1
	;; isqrt (3) -> isqrt_helper(0 (idx)) -> 1

	;; avoids unnecessary first time loop bounds
	cmp	A, #1
	jug	iinit
	mov	A, #1
	ret			;return A

iinit:	
	;; variable initialization
	mov	X, A		; idx
	mov	Y, [S-4] 	; op
	mov	A, [S-6]

	mov	Y, #0		; res = 0 -> [S-2] [S-4]
				; op -> [S-8] [S-10]
	mov	A, Y
	push	A
	push	Y

	;; split in 2 loops
	cmp	X, #16
	jul	start2
	sub	X, #16
#ifdef HAS_MLX16_FSB_SFB_INSTRUCTIONS
	sfb	X		; also ok for X = 0
#else
	jz	start1
	SFB22_X			; one = 1 << (idx-16)
#endif
	
	;cmp	X, #0
	;jne	loop2
	;; NB: A remains 0 during first loop
loop1:
	;add	YA, X<<16
	add	Y, X		; res' = res + one
	cmp	Y, [S-8]	; if (op >= res')
	jul	then1
	jne	else1
	cmp	A, [S-10]
	jule	then1
else1:
	;sub	YA, X<<16
	sub	Y, X		; res' = res - one
loop1_next:
	;LSR	YA, #1		; res >>= 1
	lsr	Y, #1
	;rrc	A, #1
	lsr	X, #2		; one >>= 2
	;cmp	X, #0		
	;jne	loop1
	jnz	loop1
end1:	
	;; initialize second loop
	mov	X, #16384	; 1 << 14
	jmp	loop2

#ifndef HAS_MLX16_FSB_SFB_INSTRUCTIONS
start1:
	mov	X, #1		;sfb 0
	jmp	loop1
#endif
	
start2:
	;; X != 0
#ifdef HAS_MLX16_FSB_SFB_INSTRUCTIONS
	sfb	X
#else
	SFB22_X			; one = 1 << (idx-16)
#endif

	;cmp	X, #0
	;jne	loop2
loop2:
	;addu	YA, X
	add	A, X		; res' = res + one
	cmp	Y, [S-8]	; if (op >= res')
	jul	then2
	jne	else2
	cmp	A, [S-10]
	jule	then2
else2:
	;subu	YA, X
	sub	A, X		; res' = res - one
loop2_next:
	;LSR	YA, #1		; res >>= 1
	lsr	Y
	rrc	A
	lsr	X, #2		; one >>= 2
	;cmp	X, #0
	;jne	loop2
	jnz	loop2

	ret	#6		;return A (Y == 0)

then1:
	mov	[S-2], Y	; save res
	;mov	[S-4], A	; NB. A = 0

	;; op = op - res' = op - (res + one)
	;; = - ((res + one) - op)
	sub	A, [S-10]
	subc	Y, [S-8]
	NEG_YA
	mov	[S-10], A
	mov	[S-8], Y

	mov	Y, [S-2]	; restore res
	mov	A, [S-4]	; NB. A, [S-4] = 0
	;add	YA, X<<16
	add	Y, X		; res = res' + one = res + 2 * one
	jmp	loop1_next

then2:
	mov	[S-2], Y	; save res
	mov	[S-4], A

	;; op = op - res' = op - (res + one)
	;; = - ((res + one) - op
	sub	A, [S-10]
	subc	Y, [S-8]
	NEG_YA
	mov	[S-10], A
	mov	[S-8], Y

	mov	Y, [S-2]	; restore res
	mov	A, [S-4]

	;addu	YA, X
	add	A, X		; res = res' + one = res + 2 * one
	jmp	loop2_next
	.size	_isqrt32_helper, .-_isqrt32_helper
#endif /* HAS_MLX16_COPROCESSOR */
	
	end
