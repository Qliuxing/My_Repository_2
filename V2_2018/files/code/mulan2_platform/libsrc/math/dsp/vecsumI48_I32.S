;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_vecsumI48_I32

#include "../math.mac"

	;;; MLX16/MLX16-x8 versions not much gain over GCC (?)
	;; at least not wrt constant loop iteration and -funroll-loops

;
;	Signed integer sum
;
; Input:
;	A  	pointer to vector of signed 32-bit elements
;	[S-4]	size of vector (unsigned, 0 < n <= 16 (MLX16 only))
;	[S-6]	pointer to store 16MSBs of 48 bit result
;
; Output:
;	Y:A     un(!)signed 32-bit low-part of result
;	[[S-6]]	signed 16-bit hi-part of result
;	

_vecsumI48_I32:	

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
	inc	S, #2
	;mov	D, #0

	mov	Y, A 		; first operand, i.e. vector, in Y

	;; first iteration
	mov	D, [Y++]	; result ; mov D,[Y++] loads 32 bit values
	;cmp	D, #0
	sn	A		; A keeps track of overflow, sign extension
	neg	A
	djnz	X, ls3

	; return value
	jmp	vs48_end

ls3:
	mov	[S-2], A	; first store high word from prev iteration
	mov	A, [Y+2]	; sign extension needed for add,
				; hi-word of vector element sufficient
	;cmp	A, #0
	sn	A
	neg	A

	add	D, [Y++]	; add D,[Y++] adds 32 bit values
	adc	A, [S-2]	; add the sign extension with stored high word
	
	djnz	X, ls3

vs48_end:
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
	jnz	vs48_start
	mov	A, #0 		; return value
	mov	Y, A
	mov	X, [S-6]	; store overflow
	mov	[X], A
	ret
vs48_start:
#endif
	inc	S, #6
	mov	Cx, YL

	;; first iteration
	mov	A, [X+2]	; keep track of overflow
				; done here to avoid using stack and saves insn
	;cmp	A, #0
	sn	A		; sign extension of first value (Y below)
	neg	A
	mov	[S-2], A
	mov	A, [X++]	; result
	mov	Y, [X++]
	djnz	Cx, ls3

	jmp vs48_end

ls3:
	mov	[S-4], Y	; first store high(mid) word from prev iteration

	mov	Y, [X+2]	; sign extension needed for add
				; hi-word of vector element sufficient
	;cmp	Y, #0
	sn	Y
	neg	Y
	mov	[S-6], Y  	; store the sign extension
	mov	Y, [S-4]	; restore high(mid) word from prev iteration
	
	add	A, [X++]
	adc	Y, [X++]

	; keep track of overflow
	push 	X		; does not change C flag
	mov 	X, [S-4]	; does not change C flag
	adc 	X, [S-8]	; add the sign extension with stored high word
	mov 	[S-4], X
	pop 	X
	
	djnz	Cx, ls3

vs48_end:
	;; store overflow word in supplied pointer 
	mov 	X, [S-12]	; pointer where to store
	push 	A
	mov 	A, [S-4]	; overflow value
	mov 	[X], A
	pop 	A

	ret 	#8
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_vecsumI48_I32, .-_vecsumI48_I32

	end
