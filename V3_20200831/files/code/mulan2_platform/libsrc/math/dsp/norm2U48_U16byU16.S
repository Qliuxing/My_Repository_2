;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_norm2U48_U16byU16

#include "../math.mac"

;
;	Unsigned integer multiplication
;
; Input:
;	A  	unsigned 16-bit first operand
;	[S-4]	unsigned 16-bit second operand
;	[S-6]	pointer to store 16MSBs of 48 bit result
;
; Output:
;	Y:A     unsigned 32-bit low-part of result
;	[[S-6]]	unsigned 16-bit hi-part of result
;	

_norm2U48_U16byU16:	

#if defined (HAS_MLX16_COPROCESSOR)
	; take first operand and square it, and store it in d
	mulu	D, A, A
	
	; take second operand and square it, and add it to d
	mov	A, [S-4]
	macu	D, A, A
	sc	X		; keep track of overflow: 1 if carry, 0 otherwise
	
	; store overflow
	mov	Y, [S-6]
	mov	[Y], X

	; return value
	mov	YA, D
 	ret
#else
	inc	S, #4
	
	; take first operand and square it, and store it on stack
	mov	X, A
	MULU_X
	mov	[S-4], A
	mov	[S-2], Y

	; take second operand and square it
	mov	A, [S-8]
	mov	X, A
	MULU_X

	; add with first result
	add	A, [S-4]
	adc	Y, [S-2]
	sc	X		; keep track of overflow: 1 if carry, 0 otherwise

	; store overflow
	push	Y
	mov	Y, [S-12]	; pointer where to store
	mov	[Y], X		; overflow value
	pop	Y
	
	; return value
 	ret	#6
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_norm2U48_U16byU16, .-_norm2U48_U16byU16

	end
