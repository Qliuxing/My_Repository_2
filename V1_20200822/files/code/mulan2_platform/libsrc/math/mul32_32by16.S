;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

	.text
	.global	_mulI32_I32byI16
	.global	_mulI32_I32byU16
	.global	_mulU32_U32byU16

#include "math.mac"


#if defined (HAS_MLX16_COPROCESSOR)
;
;	MLX16X8 Signed integer multiplication
;
; Input:
;	Y:A  	signed 32-bit multiplicand
;	[S-4]	signed 16-bit multiplier
;
; Output:
;	Y:A     signed 32-bit result
;	

_mulI32_I32byI16:

	push	A
	mov	A, [S-6]	; signed

	;add	A, #0
	jnn	m1

	muls	D, A, Y		; Y - signed, A - negative
	asl	D, #16

	pop	Y		; unsigned
	neg	A	

	mulu	YA, A, Y	; Y - unsigned, A - positive

	neg	YA

	add	D, YA
	mov	YA, D
	ret
	.size	_mulI32_I32byI16, .-_mulI32_I32byI16

;
;	MLX16X8 Signed/Unsigned integer multiplication
;
; Input:
;	Y:A  	signed 32-bit multiplicand
;	[S-4]	unsigned 16-bit multiplier
;
; Output:
;	Y:A     signed 32-bit result
;	

_mulI32_I32byU16:
	push	A
	mov	A, [S-6]
m1:
	add	Y, #0
	jnn	m0
	neg	Y

	mulu	D, A, Y		; Y - positive, A - unsigned
	neg 	D

	asl	D, #16

	pop	Y		; unsigned
	macu	D, A, Y

	mov	YA, D

	ret
	.size	_mulI32_I32byU16, .-_mulI32_I32byU16

;
;	MLX16X8 Unsigned integer multiplication
;
; Input:
;	Y:A  	unsigned 32-bit multiplicand
;	[S-4]	unsigned 16-bit multiplier
;
; Output:
;	Y:A     unsigned 32-bit result
;	

_mulU32_U32byU16:
	push	A
	mov	A, [S-6]
m0:
	mulu	D, A, Y
	asl	D, #16

	pop	Y
	macu	D, A, Y

	mov	YA, D
	ret
	.size	_mulU32_U32byU16, .-_mulU32_U32byU16

#else

;
;	MLX16 Signed integer multiplication
;
; Input:
;	Y:A  	signed 32-bit multiplicand
;	[S-4]	signed 16-bit multiplier
;
; Output:
;	Y:A     signed 32-bit result
;	

_mulI32_I32byI16:
	mov	X, [S-4]

	;add	X, #0
	jnn	m1

	neg	X

	add	Y, #0
	jnn	m2

	NEG_YA
	;jmp	m0 (no advantage to replace in case of __OPTIMIZE_SIZE__ (-Os))
	call	mul0
	ret
	.size	_mulI32_I32byI16, .-_mulI32_I32byI16

;
;	MLX16 Signed/Unsigned integer multiplication
;
; Input:
;	Y:A  	signed 32-bit multiplicand
;	[S-4]	unsigned 16-bit multiplier
;
; Output:
;	Y:A     signed 32-bit result
;	
_mulI32_I32byU16:
	mov	X, [S-4]	; load 16-bit argument

m1:	add	Y, #0
	jnn	m0

	NEG_YA
m2:
	call	mul0
	NEG_YA
	ret
	.size	_mulI32_I32byU16, .-_mulI32_I32byU16

;
;	MLX16 Unsigned integer multiplication
;
; Input:
;	Y:A  	unsigned 32-bit multiplicand
;	[S-4]	unsigned 16-bit multiplier
;
; Output:
;	Y:A     unsigned 32-bit result
;	

_mulU32_U32byU16:
	mov	X, [S-4]	; load 16-bit argument

m0:	call	mul0
	ret
	.size	_mulU32_U32byU16, .-_mulU32_U32byU16

;
;	MLX16 Unsigned integer multiplication - internal function
;
; Input:
;	Y:A  	unsigned 32-bit multiplicand
;	X	unsigned 16-bit multiplier
;
; Output:
;	Y:A     unsigned 32-bit result (low part of the 48-bit)
;	
mul0:	push	A               ; store LSB of the 32-bit argument
	mov	A, Y            ; move MSB of the 32-bit argument
	MULU_X			; YA = A * X
	push	A		; store MSB of the result
	mov	A, [S-4]	; load LSB of the 32-bit argument
	MULU_X			; YA = A * X
	add	Y, [S-2]	; add MSB to the result
	ret	#6
	.size	mul0, .-mul0
#endif  /* HAS_MLX16_COPROCESSOR */

	end
