;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

	;; not needed for MLX16-x8
	;; performance comparable with stdlib for MLX16 with U/SSEX safety
	
	.text
	.global	_mulI16_I8byI8
	.global	_mulI16_I8byU8
	.global	_mulU16_U8byU8

#include "math.mac"

;
;	Signed integer multiplication
;
; Input:
;	A  	signed 8-bit multiplicand (sign extended)
;	[S-4]	signed 8-bit multiplier (sign extended?)
;
; Output:
;	A       signed 16-bit result
;	

_mulI16_I8byI8:

#if defined (HAS_MLX16_COPROCESSOR)
	; GCC same result
	mov	YL, [S-4]
	ssex	Y 		; alt: mov Y, [S-4] if known sign extended
	muls	A, AL, YL
	ret
#else
	mov	YL, [S-4]
	ssex	Y		; alt: mov Y, [S-4] if known zero extended
	mov	X, Y		; ssex X not supported

	;add	X, #0
	jnn	m1

	neg	X

	add	A, #0
	jnn	m2

	neg	A

#ifdef __OPTIMIZE_SIZE__ /* compiled with -Os */
	jmp	m0
#else
	MULU8_X

	ret
#endif /* __OPTIMIZE_SIZE__ */
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_mulI16_I8byI8, .-_mulI16_I8byI8

;
;	Signed/Unsigned integer multiplication
;
; Input:
;	A  	signed 8-bit multiplicand (sign extended)
;	[S-4]	unsigned 8-bit multiplier (zero extended?)
;
; Output:
;	A       signed 16-bit result

_mulI16_I8byU8:

#if defined (HAS_MLX16_COPROCESSOR)
	; GCC same result
	mov	YL, [S-4]
	usex	Y		; alt: mov Y, [S-4] if known zero extended
	MULS	A, A, Y		; muls AL,YL does not suffice as YL unsigned
	ret
#else
	mov	YL, [S-4]
	usex	Y		; alt: mov Y, [S-4] if known zero extended
	mov	X, Y		; usex X not supported

m1:	add	A, #0
	jnn	m0

	neg	A
m2:
	MULU8_X
	neg	A

	ret
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_mulI16_I8byU8, .-_mulI16_I8byU8

;
;	Unsigned integer multiplication
;
; Input:
;	A  	unsigned 8-bit multiplicand (zero extended)
;	[S-4]	unsigned 8-bit multiplier (zero extended?)
;
; Output:
;	A       signed 16-bit result

_mulU16_U8byU8:

#if defined (HAS_MLX16_COPROCESSOR)
	; GCC same result
	mov	YL, [S-4]
	usex	Y 		; alt: mov Y, [S-4] if known zero extended
	mulu	A, AL, YL
	ret
#else
	mov	YL, [S-4]
	usex	Y		; alt: mov Y, [S-4] if known zero extended
	mov	X, Y		; usex X not supported

m0:	MULU8_X

	ret
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_mulU16_U8byU8, .-_mulU16_U8byU8

	end
