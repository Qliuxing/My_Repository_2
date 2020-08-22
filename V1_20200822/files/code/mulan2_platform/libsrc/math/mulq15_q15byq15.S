;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

	.text
	.global	_mulQ15_Q15byQ15

#include "math.mac"

;
;	Signed integer fraction multiplication
;
; Input:
;	A  	signed Q15 multiplicand
;	[S-4]	signed Q15 multiplier
;
; Output:
;	A     	signed Q15 result
;	

_mulQ15_Q15byQ15:


#if defined (HAS_MLX16_COPROCESSOR)
	mov	Y, [S-4]
	muls	YA, A, Y
	asr	YA, #15
	ret
#else
	mov	X, [S-4]

	;add	X, #0
	jnn	m1

	neg	X
	add	A, #0
	jnn	m2

	neg	A
	;; non-negative multiplicand and multiplier
m0:	
	MULU_X
	;; shift right 15
	asl	A
	rlc	Y
	mov	A, Y

	ret

	;; non-negative multiplier
m1:	add	A, #0
	jnn	m0

	neg	A
m2:
	MULU_X
	NEG_YA
	;; shift right 15
	asl	A
	rlc	Y
	mov	A, Y

	ret
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_mulQ15_Q15byQ15, .-_mulQ15_Q15byQ15

	end
