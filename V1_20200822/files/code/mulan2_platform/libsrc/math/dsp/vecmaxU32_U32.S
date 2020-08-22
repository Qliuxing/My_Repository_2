;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_vecmaxU32_U32

#include "../math.mac"

				;
;	Unsigned integer max
;
; Input:
;	A  	pointer to vector of unsigned 32-bit elements
;	[S-4]	size of vector
;
; Output:
;	Y:A 	unsigned 32-bit result
;	

_vecmaxU32_U32:

#if defined (HAS_MLX16_COPROCESSOR)
	;; use of djnz cx limits to vectors of size 16

	; loop initialization
	mov	X, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	vm32_start
	mov	YA, #0		; return value
	ret
vm32_start:
#endif
	mov	Y, A 		; first operand, i.e. vector, in Y

	;; first iteration
	mov	D, [Y++]	; result
	djnz	X, lm

	; return value
	mov	YA, D
	ret

lm:
	cmp	D, [Y]		; if (max < *vector++) { max = *(vector-1) }
	jug	le
	mov	D, [Y]
le:	
	add	Y, #4
	djnz	X, lm

	; return value
	mov	YA, D
	ret
#else
	mov	X, A		; first operand, i.e. vector, in X

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

	;; first iteration
	mov	A, [X++]	; result
	mov	Y, [X++]
	djnz	Cx, lm

	; return value
	ret

lm:
	cmp	Y, [X+2]	; if (max < *vector++) { max = *(vector-1) }
	jug	le
	jne	lm2
	cmp	A, [X]
	jug	le
lm2:
	mov	Y, [X+2]	;redundant in case of fall thru from prev line
	mov	A, [X]
le:
	add	X, #4
	djnz	Cx, lm

	; return value
	ret
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_vecmaxU32_U32, .-_vecmaxU32_U32
	
	end
