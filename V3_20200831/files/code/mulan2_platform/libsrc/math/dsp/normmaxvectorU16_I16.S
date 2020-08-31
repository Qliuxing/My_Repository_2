;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_normmaxvectorU16_I16
;	.global _normmaxvectorU16_I162

#include "../math.mac"

				;
;	Signed integer max of absolute values
;
; Input:
;	A  	pointer to vector of signed 16-bit elements
;	[S-4]	size of vector
;
; Output:
;	A 	unsigned 16-bit result
;	

_normmaxvectorU16_I16:	

	; loop initialization
	mov	X, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	vm16_start
	mov	A, #0		; return value
	ret
vm16_start:
#endif
	mov	Y, A 		; first operand, i.e. vector, in Y

	;; first iteration
	mov	A, [Y++]	; result, max
	jnn .+4			; abs
	neg	A
	djnz	X, lms

	; return value
	ret
#if defined (HAS_MLX16_COPROCESSOR)
lms:
	MOVU	D, A
lm:
	mov	A, [Y++]	; abs(*vector++)
	jnn .+4			; abs
	neg	A
	CMPU	D, A		; if (max < abs) { max = abs; }
	juge	le
	MOVU	D, A
le:
	djnz	X, lm

	; return value
	mov	A, Dl
	ret
#else
lms:
	inc	S, #2
	mov	[S-2], A	; result, max
lm:
	mov	A, [Y++]	; abs(*vector++)
	jnn .+4			; abs
	neg	A
	cmp	A, [S-2]	; if (abs > max) { max = abs; }
	jule	le
	mov	[S-2], A
le:	
	djnz	X, lm

	; return value
	mov	A, [S-2]
	ret	#4
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_normmaxvectorU16_I16, .-_normmaxvectorU16_I16

#if 0
	;; Cx used as loop counter, so max vector size 17
	;; for mlx16-x8, can use D(l) as extra reg, such that X can be kept as loop counter
_normmaxvectorU16_I162:	

	; loop initialization
	mov	Y, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	vm16_start
	mov	A, #0		; return value
	ret
vm16_start:
#endif
	mov	Cx, YL
	
	mov	X, A 		; first operand, i.e. vector, in X

	;; first iteration
	mov	A, [X++]	; result, max
	jnn .+4			; abs
	neg	A
	djnz	Cx, lm2

	; return value
	ret

lm2:
	mov	Y, [X++]	; abs(*vector++)
	jnn .+4			; abs
	neg	Y
	cmp	Y, A		; if (abs > max) { max = abs; }
	jule	le2
	mov	A, Y
le2:	
	djnz	Cx, lm2

	; return value
	ret
	.size	_normmaxvectorU16_I162, .-_normmaxvectorU16_I162
#endif
	
	end
