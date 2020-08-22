;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_norm2vectorU32_I16byI16

	.extern _mulI32_I16byI16

#include "../math.mac"

;
;	Signed integer multiplication
;
; Input:
;	A  	pointer to vector of signed 16-bit elements
;	[S-4]	size of vector (unsigned, >0)
;
; Output:
;	Y:A     signed 32-bit result
;	

_norm2vectorU32_I16byI16:

#if defined (HAS_MLX16_COPROCESSOR)
	; loop initialization
	mov	X, [S-4] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp 	X, #0
	jnz	nv32_start
	mov	YA, #0		; return value
	ret
nv32_start:
#endif
	;mov	D, #0		; sum

	mov	Y, A 		; vector, first operand

	;; first iteration - no savings
	mov	A, [Y++]
	muls	D, A, A		; result
	djnz	X, ln

	; return value
	mov	YA, D
	ret

ln:
	; take next element, square it, and add it to d
	mov	A, [Y++]	; vector++
	macs	D, A, A		
	djnz	X, ln

	; return value
	mov	YA, D
	ret
	
#else
	inc	S, #6

	mov	[S-6], A	; vector, first operand
	
	; loop initialization
	mov	X, [S-10] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	nv32_start
	mov	A, #0 		; return value
	mov	Y, A 		
	ret	#8
nv32_start:
#endif
	
	mov	A, #0 		; result
	mov	Y, A 		

loop:
	mov	[S-4], A	; result
	mov	[S-2], Y

	mov	Y, [S-6]	; vector++
	mov	A, [Y++]
	mov	[S-6], Y

	;; multiply accumulate
	push	X		; keep loop counter, also when using Cx

	push	A
	call	_mulI32_I16byI16; first arg in A, second arg on stack,
				; result in YA
	dec	S, #2

	pop	X		; restore loop counter
	add	A, [S-4] 	; result
	adc	Y, [S-2]

	djnz	X, loop
	
	; return value
 	ret	#8
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_norm2vectorU32_I16byI16, .-_norm2vectorU32_I16byI16

	end
