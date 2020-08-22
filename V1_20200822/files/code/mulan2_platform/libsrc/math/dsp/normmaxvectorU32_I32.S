;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_normmaxvectorU32_I32
;	.global _normmaxvectorU32_I322

#include "../math.mac"


;	Signed integer max of absolute values
;
; Input:
;	A  	pointer to vector of signed 32-bit elements
;	[S-4]	size of vector
;
; Output:
;	Y:A 	unsigned 32-bit result
;	

_normmaxvectorU32_I32:	
#if defined (HAS_MLX16_COPROCESSOR)
	mov	X, A 		; first operand, i.e. vector, in X

	; loop initialization
	mov	Y, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	vm32_start
	mov	YA, #0		; return value
	ret
vm32_start:
#endif
	mov	Cx, YL

	;; first iteration
	mov	YA, [X++]	; result, max
	jnn .+4			; abs
	neg	YA
	djnz	Cx, lms

	; return value
	ret
lms:
	mov	D, YA
lm:
	mov	YA, [X++]	; abs(*vector++)
	jnn .+4			; abs
	neg	YA
	cmp	D, YA		; if (max < abs) { max = abs; }
	juge	le
	mov	D, YA
le:
	djnz	Cx, lm

	; return value
	mov	YA, D
	ret
#else
	mov	X, A 		; first operand, i.e. vector, in X

	; loop initialization
	mov	Y, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	vm32_start
	mov	A, #0		; return value
	mov	Y, A
	ret
vm32_start:
#endif
	mov	Cx, YL

	;; return/max  initialization
	inc	S, #4
	mov	A, #0
	mov	[S-4], A
	mov	[S-2], A
lm:
	mov	A, [X++]	; abs(*vector++)
	mov	Y, [X++]

	;; abs32
	;cmp	Y, #0		; TBD not redundant as lod only sets Z,N
	jnn	la
	NEG_YA
la:
	cmp	Y, [S-2]	; if (abs > max) { max = abs; }
	jul	le
	jne	lm2
	cmp	A, [S-4]
	jule	le
lm2:
	mov	[S-4], A
	mov	[S-2], Y
le:
	djnz	Cx, lm

	; return value
	mov	A, [S-4]
	mov	Y, [S-2]
	ret	#6
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_normmaxvectorU32_I32, .-_normmaxvectorU32_I32

	end
