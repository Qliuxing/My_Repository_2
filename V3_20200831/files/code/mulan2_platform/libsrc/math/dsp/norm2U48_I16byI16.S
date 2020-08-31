;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_norm2U48_I16byI16

	.extern _mulI32_I16byI16

#include "../math.mac"

;
;	Signed integer multiplication
;
; Input:
;	A  	signed 16-bit first operand
;	[S-4]	signed 16-bit second operand
;	[S-6]	pointer to store 16MSBs of 48 bit result
;
; Output:
;	Y:A     signed 32-bit low-part of result
;	[[S-6]]	signed 16-bit hi-part of result
;	

	;; not effective
	;; squaring always positive number, and only 30 significant bits
	;; adding 2 30 bit numbers -> 31 bit, so no overflow
_norm2U48_I16byI16:	

#if defined (HAS_MLX16_COPROCESSOR)
	; take first operand and square it, and store it in d
	muls	D, A, A
	
	; take second operand and square it, and add it to d
	mov	A, [S-4]
	macs	D, A, A
	sc	X		; keep track of overflow: 1 if carry, 0 otherwise

	;; squaring always positive number, and only 30 significant bits
	;; adding 2 30 bit numbers -> 31 bit, so no overflow
	
	; store overflow
	mov	Y, [S-6]
	mov	[Y], X

	; return value
	mov	YA, D
 	ret
#else
	inc	S, #4
	
	; take first operand and square it, and store it on stack
	push	A
	call	_mulI32_I16byI16; first arg in A, second arg on stack,
				; result in YA
	dec	S, #2
	mov	[S-4], A
	mov	[S-2], Y

	; take second operand and square it
	mov	A, [S-8]
	push	A
	call	_mulI32_I16byI16; first arg in A, second arg on stack,
				; result in YA
	dec	S, #2

	; add with first result
	add	A, [S-4]
	adc	Y, [S-2]
	sc	X		; keep track of overflow: 1 if carry, 0 otherwise

	;; squaring always positive number, and only 30 significant bits
	;; adding 2 30 bit numbers -> 31 bit, so no overflow
	
	; store overflow
	push	Y
	mov	Y, [S-12]	; pointer where to store
	mov	[Y], X		; overflow value
	pop	Y
	
	; return value
 	ret	#6
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_norm2U48_I16byI16, .-_norm2U48_I16byI16

	end
