;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_norm2U32_I16byI16

	.extern _mulI32_I16byI16

#include "../math.mac"

;
;	Signed integer multiplication
;
; Input:
;	A  	signed 16-bit first operand
;	[S-4]	signed 16-bit second operand
;
; Output:
;	Y:A     signed 32-bit result
;	

_norm2U32_I16byI16:

#if defined (HAS_MLX16_COPROCESSOR)
	; take first operand and square it, and store it in d
	muls	D, A, A
	
	; take second operand and square it, and add it to d
	mov	A, [S-4]
	macs	D, A, A
	
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
	
	; return value
 	ret 	#6
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_norm2U32_I16byI16, .-_norm2U32_I16byI16

	end
