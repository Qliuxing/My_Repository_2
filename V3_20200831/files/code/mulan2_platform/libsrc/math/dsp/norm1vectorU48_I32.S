;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_norm1vectorU48_I32

#include "../math.mac"

	;;; MLX16/MLX16-x8 versions not much gain over GCC (?)
	;; at least not wrt constant loop iteration and -funroll-loops

	;; no need for asm versions of norm1I16(a,b)
	;; as no overflow when adding 2 signed numbers in unsigned result

;
;	Signed integer sum of absolute values
;
; Input:
;	A  	pointer to vector of signed 32-bit elements
;	[S-4]	size of vector (unsigned, 0 < n <= 16 (all CPU targets))
;	[S-6]	pointer to store 16MSBs of 48 bit result
;
; Output:
;	Y:A     unsigned 32-bit low-part of result
;	[[S-6]]	unsigned 16-bit hi-part of result
;	

_norm1vectorU48_I32:


#if defined (HAS_MLX16_COPROCESSOR)
	;; use of djnz cx limits to vectors of size 16

	; loop initialization
	mov	Y, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	nv48_start
	mov	YA, #0		; return value
	mov	X, [S-6]	; store overflow
	mov	[X], A
	ret
nv48_start:
#endif
	inc	S, #2
	mov	Cx, YL
	;mov	D, #0

	mov	X, A 		; first operand, i.e. vector, in Y
	mov	A, #0		; A keeps track of overflow

	;; first iteration
	mov	D, [X++]	; result ; mov D,[X++] loads 32 bit values
	;cmp	D, #0
	jnn .+4			; abs
	neg	D
	;mov	A, #0		; A keeps track of overflow
	djnz	Cx, ls3

	; return value
	jmp	nv48_end

ls3:
	mov	[S-2], A	; first store overflow from prev iteration
	mov	YA, [X++]
	;cmp	YA, #0
	jnn .+4			; abs
	neg	YA
	add	D, YA
	mov	A, [S-2]	; keep track of overflow
	adc	A, #0
	djnz	Cx, ls3

nv48_end:
	; store overflow
	mov	X, [S-8]
	mov	[X], A

	; return value
	mov	YA, D
	ret	#4
	
#else
	mov	X, A		; first operand, i.e. vector, in X

	; loop initialization
	mov	Y, [S-4] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	nv48_start
	mov	A, #0 		; return value
	mov	Y, A
	mov	X, [S-6]	; store overflow
	mov	[X], A
	ret
nv48_start:
#endif
	inc	S, #6
	mov	Cx, YL

	;; first iteration
	mov	A, #0		; keep track of overflow
	mov	[S-2], A
	mov	A, [X++]	; result
	mov	Y, [X++]
	;cmp	YA, #0
	jnn abs3		; abs
	NEG_YA
abs3:
	djnz	Cx, ls3

	jmp nv48_end

ls3:
	mov	[S-6], A
	mov	[S-4], Y
	mov	A, [X++]
	mov	Y, [X++]
	;cmp	YA, #0
	jnn abs4		; abs
	NEG_YA
abs4:
	add	A, [S-6]
	adc	Y, [S-4]
	

	; keep track of overflow
	push 	X		; does not change C flag
	mov 	X, [S-4]	; does not change C flag
	adc 	X, #0
	mov 	[S-4], X
	pop 	X
	
	djnz	Cx, ls3

nv48_end:
	;; store overflow word in supplied pointer 
	mov 	X, [S-12]	; pointer where to store
	push 	A
	mov 	A, [S-4]	; overflow value
	mov 	[X], A
	pop 	A

	ret 	#8
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_norm1vectorU48_I32, .-_norm1vectorU48_I32

	end
