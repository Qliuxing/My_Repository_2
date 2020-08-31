;
; Copyright (C) 2007-2014 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

#if !defined (HAS_MLX16_COPROCESSOR)

	.text
	.global	_mulI16_I16byI16
	.global _mulI16_I16byU16
	.global	_mulU16_U16byU16

#include "math.mac"

;
;	Signed integer multiplication
;
; Input:
;	A  	signed 16-bit multiplicand
;	[S-4]	signed 16-bit multiplier
;
; Output:
;	A     	signed 16-bit result
;	

_mulI16_I16byI16:

	mov	X, [S-4]

	;add	X, #0
	jnn	m1

	neg	X
	add	A, #0
	jnn	m2

	neg	A
#ifdef __OPTIMIZE_SIZE__ /* compiled with -Os */
	jmp	m0
#else
	MULU_X
	mov	A, Y
	ret
#endif /* __OPTIMIZE_SIZE__ */
	.size	_mulI16_I16byI16, .-_mulI16_I16byI16

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

	mov	X, [S-4]

m1:	add	A, #0
	jnn	m0

	neg	A
m2:	
	MULU_X
	NEG_YA
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

	mov	X, [S-4]

m0:	MULU_X
	mov	A, Y
	ret
	.size	_mulU16_U16byU16, .-_mulU16_U16byU16

#endif /* HAS_MLX16_COPROCESSOR */

	end
