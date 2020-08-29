;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_vecsumU32_U16

#include "../math.mac"

	;;; MLX16/MLX16-x8 versions not much gain over GCC (?)
	;; at least not wrt constant loop iteration and -funroll-loops

;
;	Unsigned integer sum
;
; Input:
;	A  	pointer to vector of unsigned 16-bit elements
;	[S-4]	size of vector (unsigned, 0 < n <= 16 (MLX16 only))
;
; Output:
;	Y:A     unsigned 32-bit result
;	

_vecsumU32_U16:	

#if defined (HAS_MLX16_COPROCESSOR)
	;; use of djnz cx limits to vectors of size 16

	; loop initialization
	mov	X, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	vs16_start
	mov	YA, #0		; return value
	ret
vs16_start:
#endif
	mov	Y, A 		; first operand, i.e. vector, in Y

	;; first iteration
	movu	D, [Y++]	; result - loads 16 bit value
	djnz	X, ls

	; return value
	mov	YA, D
	ret

ls:
	addu	D, [Y++]	; add D,[Y++] adds 32 bit values
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
	jnz	vs16_start
	mov	A, #0 		; return value
	mov	Y, A
	ret
vs16_start:
#endif
	mov	Cx, YL

	;; first iteration
	mov	A, [X++]	; result
	mov	Y, #0
	djnz	Cx, ls

	; return
	ret
ls:
	add	A, [X++]
	adc	Y, #0
	djnz	Cx, ls
	
	ret
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_vecsumU32_U16, .-_vecsumU32_U16

	end
