;
; Copyright (C) 2014 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

#if defined (HAS_MLX16_COPROCESSOR)

	.text
	.global	_mulI16_I16byI16

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

	mov	Y, [S-4]
	muls	YA, A, Y
	mov	A, Y
	ret
	.size	_mulI16_I16byI16, .-_mulI16_I16byI16

#endif /* HAS_MLX16_COPROCESSOR */

	end
