;
; Copyright (C) 2014 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

#if defined (HAS_MLX16_COPROCESSOR)

	.text
	.global	_mulI32_I16byI16

;
;	Signed integer multiplication
;
; Input:
;	A  	signed 16-bit multiplicand
;	[S-4]	signed 16-bit multiplier
;
; Output:
;	Y:A     signed 32-bit result
;	

_mulI32_I16byI16:

	mov	Y, [S-4]
	muls	YA, A, Y
	ret
	.size	_mulI32_I16byI16, .-_mulI32_I16byI16

#endif  /* HAS_MLX16_COPROCESSOR */

	end
