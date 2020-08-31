;
; Copyright (C) 2014 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

#if defined (HAS_MLX16_COPROCESSOR)

	.text
	.global	_mulU16_U16byU16
	.global _mulI16_I16byU16

;
;	Signed/Unsigned integer multiplication
;
; Input:
;	A  	signed 16-bit multiplicand
;	[S-4]	unsigned 16-bit multiplier
;
; Output:
;	A     	signed 16-bit result
;	

_mulI16_I16byU16:

	add	A, #0
	jnn	_mulU16_U16byU16

	neg	A
	mov	Y, [S-4]
	mulu	YA, A, Y
	neg	YA
	mov	A, Y
	ret
	.size	_mulI16_I16byU16, .-_mulI16_I16byU16

;
;	Unsigned integer multiplication
;
; Input:
;	A  	unsigned 16-bit multiplicand
;	[S-4]	unsigned 16-bit multiplier
;
; Output:
;	A     	unsigned 16-bit result
;	

_mulU16_U16byU16:

	mov	Y, [S-4]
	mulu	YA, A, Y
	mov	A, Y

	ret
	.size	_mulU16_U16byU16, .-_mulU16_U16byU16

#endif /* HAS_MLX16_COPROCESSOR */

	end
