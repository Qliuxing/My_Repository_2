;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library
;
; Revision $Name: PLTF_MULAN2_MMC16_RELEASE_3_3_0 $
;
; File $RCSfile: mul32_16by16.S,v $
;

#include <mathlib.h>

	.text

#if defined (HAS_MLX16_COPROCESSOR)
	.global	_muldivI16_I16byI16byI16
	.global	_muldivU16_U16byU16byU16

#include "../../../libsrc/math/math.mac"

;
;	Signed integer multiplication and division
;
; Input:
;	A  	signed 16-bit multiplicand
;	[S-4]	signed 16-bit multiplier
;	[S-6]	signed 16-bit divider
;
; Output:
;	A   signed 16-bit result
;

_muldivI16_I16byI16byI16:
	mov		Y, [S-4]
	muls	YA, A, Y
  mov   X, [S-6]
	divs	YA, X
	divs	YA, X
	dadjs	YA, X
	ret

;
;	Unsigned integer multiplication and division
;
; Input:
;	A  	unsigned 16-bit multiplicand
;	[S-4]	unsigned 16-bit multiplier
;	[S-6]	unsigned 16-bit divider
;
; Output:
;	A   unsigned 16-bit result
;

_muldivU16_U16byU16byU16:
	mov		Y, [S-4]
	mulu	YA, A, Y
  mov   X, [S-6]
	divu	YA, X
	divu	YA, X
	ret
#endif

	end
