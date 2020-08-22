;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_vecsumU32_U32

#include "../math.mac"

	;;; MLX16/MLX16-x8 versions not much gain over GCC (?)
	;; at least not wrt constant loop iteration and -funroll-loops

;
;	Unsigned integer sum (no check on overflow)
;
; Input:
;	A  	pointer to vector of unsigned 32-bit elements
;	[S-4]	size of vector (unsigned, 0 < n <= 16 (MLX16 only))
;
; Output:
;	Y:A     unsigned 32-bit result
;	

_vecsumU32_U32:	

#if defined (HAS_MLX16_COPROCESSOR)
	;; use of djnz cx limits to vectors of size 16

	; loop initialization
	mov	X, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	vs32_start
	mov	YA, #0		; return value
	ret
vs32_start:
#endif
	;mov	D, #0		; result

	mov	Y, A 		; first operand, i.e. vector, in Y

	;; first iteration
	mov	D, [Y++]	; mov D,[Y++] loads 32 bit values
	djnz	X, ls2

	; return value
	mov	YA, D
	ret

ls2:
	add	D, [Y++]	; add D,[Y++] adds 32 bit values
	djnz	X, ls2

	; return value
	mov	YA, D
	ret
	
#else
	mov	X, A		; first operand, i.e. vector, in X

	; loop initialization
	mov	Y, [S-4] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	vs32_start
	mov	A, #0 		; return value
	mov	Y, A
	ret
vs32_start:
#endif
	mov	Cx, YL

	;; first iteration
	mov	A, [X++]	; result
	mov	Y, [X++]
	djnz	Cx, ls2

	; return
	ret

ls2:
	add	A, [X++]
	adc	Y, [X++]
	djnz	Cx, ls2
	
	ret
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_vecsumU32_U32, .-_vecsumU32_U32

	end
