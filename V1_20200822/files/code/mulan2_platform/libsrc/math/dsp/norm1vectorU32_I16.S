;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_norm1vectorU32_I16

#include "../math.mac"

	;;; MLX16/MLX16-x8 versions not much gain over GCC (?)
	;; at least not wrt constant loop iteration and -funroll-loops

	;; no need for asm versions of norm1I16(a,b)
	;; as no overflow when adding 2 signed numbers in unsigned result

;
;	Signed integer sum of absolute values
;
; Input:
;	A  	pointer to vector of signed 16-bit elements
;	[S-4]	size of vector (unsigned, 0 < n <= 16 (MLX16 only))
;
; Output:
;	Y:A     unsigned 32-bit result
;	

_norm1vectorU32_I16:	

#if defined (HAS_MLX16_COPROCESSOR)
	;; use of djnz cx limits to vectors of size 16

	; loop initialization
	mov	X, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	nv16_start
	mov	YA, #0		; return value
	ret
nv16_start:
#endif
	mov	Y, A 		; first operand, i.e. vector, in Y

	;; first iteration - does not save
	mov	A, [Y++]
	;cmp	A, #0
	jnn .+4			; abs
	neg	A
	movu	D, A		; result
	djnz	X, ls

	; return value
	mov	YA, D
	ret

ls:
	mov	A, [Y++]
	;cmp	A, #0
	jnn .+4			; abs
	neg	A
	addu	D, A
	djnz	X, ls

	; return value
	mov	YA, D
	ret
	
#else
	mov	X, A		; first operand, i.e. vector, in X

	; loop initialization
	mov	Y, [S-4] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	nv16_start
	mov	A, #0 		; return value
	mov	Y, A
	ret
nv16_start:
#endif
	mov	Cx, YL

	;; first iteration
	mov	A, [X++]	; result in YA
	;cmp	A, #0
	jnn .+4			; abs
	neg	A
	mov	Y, #0		; zero extension
	djnz	Cx, ls

	; return
	ret
ls:
	push	Y		; temp store hi-part of result
	mov	Y, [X++]
	;cmp	Y, #0
	jnn .+4			; abs
	neg	Y
	
	add	A, Y
	pop	Y		; does not change C flag
	adc	Y, #0

	djnz	Cx, ls
	
	ret
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_norm1vectorU32_I16, .-_norm1vectorU32_I16

	end
