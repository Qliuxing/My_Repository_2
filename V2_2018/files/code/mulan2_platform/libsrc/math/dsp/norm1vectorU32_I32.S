;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_norm1vectorU32_I32

#include "../math.mac"

	;;; MLX16/MLX16-x8 versions not much gain over GCC (?)
	;; at least not wrt constant loop iteration and -funroll-loops

	;; no need for asm versions of norm1I16(a,b)
	;; as no overflow when adding 2 signed numbers in unsigned result

;
;	Signed integer sum of absolute values (no check on overflow)
;
; Input:
;	A  	pointer to vector of signed 32-bit elements
;	[S-4]	size of vector (unsigned, 0 < n <= 16) (all CPU targets)
;
; Output:
;	Y:A     unsigned 32-bit result
;	

_norm1vectorU32_I32:

#if defined (HAS_MLX16_COPROCESSOR)
	;; use of djnz cx limits to vectors of size 16

	; loop initialization
	mov	Y, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	nv32_start
	mov	YA, #0		; return value
	ret
nv32_start:
#endif
	mov	Cx, YL

	;mov	D, #0		; result

	mov	X, A 		; first operand, i.e. vector, in Y

	;; first iteration
	mov	D, [X++]	; mov D,[X++] loads 32 bit values
	;cmp	D, #0
	jnn .+4			; abs
	neg	D
	djnz	Cx, ls2

	; return value
	mov	YA, D
	ret

ls2:
	mov	YA, [X++]
	;cmp	YA, #0
	jnn .+4			; abs
	neg	YA
	add	D, YA
	djnz	Cx, ls2

	; return value
	mov	YA, D
	ret
	
#else
	mov	X, A		; first operand, i.e. vector, in X

	; loop initialization
	mov	Y, [S-4] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	nv32_start
	mov	A, #0 		; return value
	mov	Y, A
	ret
nv32_start:
#endif
	inc	S, #4
	mov	Cx, YL

	;; first iteration
	mov	A, [X++]	; result
	mov	Y, [X++]
	;cmp	YA, #0
	jnn abs1		; abs
	NEG_YA
abs1:
	djnz	Cx, ls2

	; return
	ret	#6

ls2:
	mov	[S-4], A
	mov	[S-2], Y
	mov	A, [X++]
	mov	Y, [X++]
	;cmp	YA, #0
	jnn abs2		; abs
	NEG_YA
abs2:	
	add	A, [S-4]
	adc	Y, [S-2]
	djnz	Cx, ls2
	
	ret	#6
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_norm1vectorU32_I32, .-_norm1vectorU32_I32

	end
