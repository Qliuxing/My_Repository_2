;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

	;; truncation behavior for negative may be different from C/gcc_math
	;; not needed for MLX16-x8
	;; improves MLX16 stdlib as only 8x mul1 vs 16x in case of signed
	;; but effectively no real change due to longer prologues/epilogues
	;; with U/SSEX safety

	.text
	.global	_mulI24_I16byI8
	.global _mulI24_I16byU8
	.global	_mulU24_U16byU8

#include "math.mac"

	;;YA = A * X, 
	;;where A unsigned 16 bit, X unsigned 8-bit integers (zero extended)
	;;YA:	24 bit result with highbyte 0
	.macro	MULU24_16_8_X
	MULU248_16_8_X		;YA now contains 24bit result << 8
	mov	AL, YL
	swap	A
	mov	YL, YH
	mov	YH, #0
	.endm


;
;	Signed integer multiplication
;
; Input:
;	A  	signed 16-bit multiplicand
;	[S-4]	signed 8-bit multiplier (sign extended?)
;
; Output:
;	Y:A    	signed 24-bit result (most significant byte 0)
;	

_mulI24_I16byI8:

#if defined (HAS_MLX16_COPROCESSOR)
	; GCC same result
	mov	YL, [S-4]
	ssex	Y
	muls	YA, A, Y
	ret
#else
	mov	YL, [S-4]
	ssex	Y		; alt: mov Y, [S-4] if known sign extended
	mov	X, Y		; usex X not supported

	;add	X, #0
	jnn	m1

	neg	X

	add	A, #0
	jnn	m2

	neg	A
#ifdef __OPTIMIZE_SIZE__ /* compiled with -Os */
	jmp	m0
#else
	MULU24_16_8_X

	ret
#endif /* __OPTIMIZE_SIZE__ */
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_mulI24_I16byI8, .-_mulI24_I16byI8

;
;	Signed/Unsigned integer multiplication
;
; Input:
;	A  	signed 16-bit multiplicand
;	[S-4]	unsigned 8-bit multiplier (zero extended?)
;
; Output:
;	Y:A    	signed 24-bit result (most significant byte 0)
;	

_mulI24_I16byU8:

#if defined (HAS_MLX16_COPROCESSOR)
	add	A, #0
	jnn	_mulU24_U16byU8

	neg	A

	mov	Y, [S-4]
	usex	Y
	
	mulu	YA, A, Y
	neg	YA
	ret
#else
	mov	YL, [S-4]
	usex	Y		; alt: mov Y, [S-4] if known zero extended
	mov	X, Y		; usex X not supported

m1:	add	A, #0
	jnn	m0

	neg	A
m2:
	MULU24_16_8_X
	NEG_YA
	
	ret
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_mulI24_I16byU8, .-_mulI24_I16byU8

;
;	Unsigned integer multiplication
;
; Input:
;	A  	unsigned 16-bit multiplicand
;	[S-4]	unsigned 8-bit multiplier (zero extended?)
;
; Output:
;	Y:A    	unsigned 24-bit result (most significant byte 0)
;	

_mulU24_U16byU8:

#if defined (HAS_MLX16_COPROCESSOR)
	; GCC same result
	mov	YL, [S-4]
	usex	Y
	mulu	YA, A, Y
	ret
#else
	mov	YL, [S-4]
	usex	Y		; alt: mov Y, [S-4] if known zero extended
	mov	X, Y		; usex X not supported

m0:	MULU24_16_8_X

	ret
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_mulU24_U16byU8, .-_mulU24_U16byU8

	end
