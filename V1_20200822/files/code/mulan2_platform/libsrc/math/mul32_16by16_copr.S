;
; Copyright (C) 2014 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

#if defined (HAS_MLX16_COPROCESSOR)

	.text
	.global	_mulU32_U16byU16
	.global	_mulI32_I16byU16

;
;	Signed/Unsigned integer multiplication
;
; Input:
;	A  	signed 16-bit multiplicand
;	[S-4]	unsigned 16-bit multiplier
;
; Output:
;	Y:A     signed 32-bit result
;	

_mulI32_I16byU16:

	add	A, #0
	jnn	_mulU32_U16byU16

	neg	A
	mov	Y, [S-4]
	mulu	YA, A, Y
	neg	YA
	ret
	.size	_mulI32_I16byU16, .-_mulI32_I16byU16

;
;	Unsigned integer multiplication
;
; Input:
;	A  	unsigned 16-bit multiplicand
;	[S-4]	unsigned 16-bit multiplier
;
; Output:
;	Y:A     unsigned 32-bit result
;	

_mulU32_U16byU16:

	mov	Y, [S-4]
	mulu	YA, A, Y

	ret
	.size	_mulU32_U16byU16, .-_mulU32_U16byU16

#endif  /* HAS_MLX16_COPROCESSOR */

	end
