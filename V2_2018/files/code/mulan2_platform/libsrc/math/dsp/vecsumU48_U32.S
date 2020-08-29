;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_vecsumU48_U32

#include "../math.mac"

	;;; MLX16/MLX16-x8 versions not much gain over GCC (?)
	;; at least not wrt constant loop iteration and -funroll-loops

;
;	Unsigned integer sum
;
; Input:
;	A  	pointer to vector of unsigned 32-bit elements
;	[S-4]	size of vector (unsigned, 0 < n <= 16 (MLX16 only))
;	[S-6]	pointer to store 16MSBs of 48 bit result
;
; Output:
;	Y:A     unsigned 32-bit low-part of result
;	[[S-6]]	unsigned 16-bit hi-part of result
;	

_vecsumU48_U32:	

#if defined (HAS_MLX16_COPROCESSOR)
	;; use of djnz cx limits to vectors of size 16

	; loop initialization
	mov	X, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	vs48_start
	mov	YA, #0		; return value
	mov	X, [S-6]	; store overflow
	mov	[X], A
	ret
vs48_start:
#endif
	;mov	D, #0

	mov	Y, A 		; first operand, i.e. vector, in Y
	mov	A, #0		; A keeps track of overflow

	;; first iteration
	mov	D, [Y++]	; result ; mov D,[Y++] loads 32 bit values
	;mov	A, #0		; A keeps track of overflow
	djnz	X, ls3

	; return value
	jmp	vs48_end

ls3:
	add	D, [Y++]	; add D,[Y++] adds 32 bit values
	adc	A, #0		; keep track of overflow
	djnz	X, ls3

vs48_end:
	; store overflow
	mov	X, [S-6]
	mov	[X], A

	; return value
	mov	YA, D
	ret
	
#else
	inc	S, #2
	mov	X, A		; first operand, i.e. vector, in X

	; loop initialization
	mov	Y, [S-6] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	vs48_start
	mov	A, #0 		; return value
	mov	Y, A
	mov	X, [S-8]	; store overflow
	mov	[X], A
	ret	#4
vs48_start:
#endif
	mov	Cx, YL

	;; first iteration
	mov	A, #0		; keep track of overflow
	mov	[S-2], A
	mov	A, [X++]	; result
	mov	Y, [X++]
	djnz	Cx, ls3

	jmp vs48_end

ls3:
	add	A, [X++]
	adc	Y, [X++]

	; keep track of overflow
	; alt:
	; use R (sub)register (4bits) of M register,
	; but not guaranteed to be available/implemented
	; alt: make following conditional on carry/overflow flag
	; to fast track normal case
	push 	X		; does not change C flag
	mov 	X, [S-4]	; does not change C flag
	adc 	X, #0
	mov 	[S-4], X
	pop 	X
	
	djnz	Cx, ls3

vs48_end:
	;; store overflow word in supplied pointer 
	mov 	X, [S-8]	; pointer where to store
	push 	A
	mov 	A, [S-4]	; overflow value
	mov 	[X], A
	pop 	A

	ret 	#4
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_vecsumU48_U32, .-_vecsumU48_U32

	end
