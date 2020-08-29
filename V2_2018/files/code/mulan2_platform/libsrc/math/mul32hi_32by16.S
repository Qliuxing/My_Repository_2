;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

	.text
	.global	_mulI32hi_I32byI16
	.global	_mulI32hi_I32byU16
	.global	_mulU32hi_U32byU16
	
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
;	Y:A     signed 32-bit result (high part of the 48-bit)
;	

_mulI32hi_I32byI16:

	mov	X, [S-4]	; signed

	;add	X, #0
	jnn	m1

	neg	X		; negate X for unsigned arithmetic
				; beware of X = 0x8000!?
	mulu	D, A,X		; 32_low multiplicand * 16 multiplier
				; A - unsigned, X - positive (negated)
	mov	A, Dl 		; store "carry" as should truncate, not round
	lsr	D, #16		; keep high part
 				; D negated result

	swap	YA		; muls requires A register

	add 	X, #0 		; X = 0x8000 :	-(
	jn	m2
	macs	D, A,X		; + 32_high multiplicand * 16 multiplier
				; 'Y' - signed, X - positive (negated)
 				; YA negated result
	
	jmp	m3
m2:
	;; no msubs (d - a*x)
	neg 	D
	macs 	D, A, X
	neg	D
m3:
	;; truncate result instead of round, as needed
	add	Y, #0
	jz	m44
	addu	D, #1 		; add carry (YA swapped)
m44:	
	mov	YA, D
	neg	YA		; as used negated X

	ret
	.size	_mulI32hi_I32byI16, .-_mulI32hi_I32byI16

;
;	MLX16X8 Signed/Unsigned integer multiplication
;
; Input:
;	Y:A  	signed 32-bit multiplicand
;	[S-4]	unsigned 16-bit multiplier
;
; Output:
;	Y:A     signed 32-bit result (high part of the 48-bit)

;	

_mulI32hi_I32byU16:
	mov	X, [S-4]
m1:
	add	Y, #0
	jnn	m0
	neg	YA		; problematic for Y=8000 or YA=80000000?

	mulu	D, A, X		; YA - positive (negated), X - unsigned
	mov	A, Dl 		; store "carry" as should truncate, not round
	lsr	D, #16		; keep high part
				; D negated result

	swap	YA		; macu requires A register
	macu	D, A, X		; YA - positive (negated), A - unsigned

	;; truncate result instead of round, as needed
	add	Y,#0
	jz	m4
	addu	D, #1 		; add carry (YA swapped)
m4:		
	mov	YA, D
	neg	YA
	
	ret
	.size	_mulI32hi_I32byU16, .-_mulI32hi_I32byU16

;
;	MLX16X8 Unsigned integer multiplication
;
; Input:
;	Y:A  	unsigned 32-bit multiplicand
;	[S-4]	unsigned 16-bit multiplier
;
; Output:
;	Y:A     unsigned 32-bit result (high part of the 48-bit)

;	

_mulU32hi_U32byU16:
	mov	X, [S-4]
m0:
	mulu	D, A, X		; 32_low multiplicand * 16 multiplier
	lsr	D, #16		; / 2^16

	swap	YA		; macu requires A register
	macu	D, A, X		; + 32_high multiplicand * 16 multiplier

	mov	YA, D		; store result
	ret
	.size	_mulU32hi_U32byU16, .-_mulU32hi_U32byU16

#else

;
;	MLX16 Signed integer multiplication
;
; Input:
;	Y:A  	signed 32-bit multiplicand
;	[S-4]	signed 16-bit multiplier
;
; Output:
;	Y:A     signed 32-bit result (high part of the 48-bit)

;	

_mulI32hi_I32byI16:
	mov	X, [S-4]

	;add	X, #0
	jnn	m1

	neg	X
	
	add	Y, #0
	jnn	m2

	;!= NEG_YA + jmp m0
	call	mul0shi		; X positive (negated), YA neg
	NEG_YA			; as used negated X

	ret
	.size	_mulI32hi_I32byI16, .-_mulI32hi_I32byI16

;
;	MLX16 Signed/Unsigned integer multiplication
;
; Input:
;	Y:A  	signed 32-bit multiplicand
;	[S-4]	unsigned 16-bit multiplier
;
; Output:
;	Y:A     signed 32-bit result (high part of the 48-bit)
;	
_mulI32hi_I32byU16:
	mov	X, [S-4]	; load 16-bit argument

m1:	add	Y, #0
	jnn	m0

	NEG_YA
m2:
	call	mul0hi_truncate	; X positive (negated), YA non-neg 
	NEG_YA			; as used negated X
	ret
	.size	_mulI32hi_I32byU16, .-_mulI32hi_I32byU16

;
;	MLX16 Unsigned integer multiplication
;
; Input:
;	Y:A  	unsigned 32-bit multiplicand
;	[S-4]	unsigned 16-bit multiplier
;
; Output:
;	Y:A     unsigned 32-bit result (high part of the 48-bit)
;	

_mulU32hi_U32byU16:
	mov	X, [S-4]	; load 16-bit argument

m0:	call	mul0hi
	ret
	.size	_mulU32hi_U32byU16, .-_mulU32hi_U32byU16

;
;	MLX16 Unsigned integer multiplication - internal function
;
; Input:
;	Y:A  	unsigned 32-bit multiplicand
;	X	unsigned 16-bit multiplier
;
; Output:
;	Y:A     unsigned 32-bit result (high part of the 48-bit)
;	
mul0hi:	push	Y               ; store MSB of multiplicant
	MULU_X			; YA = A * X 
				; = 32_low multiplicant * 16 multiplier
	push	Y		; store high part
				; = 32_low multiplicant * 16 multiplier / 2^16
	mov	A, [S-4]	; retrieve MSB of 32 multiplicant
	MULU_X			; YA = A * X
				; = 32_high multiplicant * 16 multiplier
	add	A, [S-2]	; + 32_low multiplicant * 16 multiplier / 2^16
	adc	Y, #0
	ret	#6		; result = YA
	.size	mul0hi, .-mul0hi

;
;	MLX16 Unsigned integer multiplication - internal function
;
; Input:
;	Y:A  	unsigned 32-bit multiplicand
;	X	unsigned 16-bit multiplier
;
; Output:
;	Y:A     unsigned 32-bit result (high part of the 48-bit) with "carry"
;	
mul0hi_truncate:	push	Y               ; store MSB of multiplicant
	MULU_X			; YA = A * X 
				; = 32_low multiplicant * 16 multiplier
	push	Y		; store high part
				; = 32_low multiplicant * 16 multiplier / 2^16
	push	A 		; store "carry" as should truncate, not round
	mov	A, [S-6]	; retrieve MSB of 32 multiplicant
	MULU_X			; YA = A * X
				; = 32_high multiplicant * 16 multiplier

	;; truncate result instead of round, as needed
	mov 	X, [S-2]
	jz 	mxx
	add 	A, #1
	adc 	Y, #0
mxx:	
	add	A, [S-4]	; + 32_low multiplicant * 16 multiplier / 2^16
	adc	Y, #0
	ret	#8		; result = YA
	.size	mul0hi_truncate, .-mul0hi_truncate

;
;	MLX16 Signed integer multiplication - internal function
;
; Input:
;	Y:A  	signed 32-bit multiplicand
;	X	unsigned 16-bit multiplier
;
; Output:
;	Y:A     signed 32-bit result (high part of the 48-bit) - with truncate
;	
mul0shi:push	Y               ; store MSB of multiplicant
	MULU_X			; YA = A * X 
				; = 32_low multiplicant * 16 multiplier
	push	Y		; store high part
				; = 32_low multiplicant * 16 multiplier / 2^16
	push	A 		; store "carry" as should truncate, not round
	mov	A, [S-6]	; retrieve MSB of 32 multiplicant
	call	muls_x		; YA = A * X
				; = 32_high multiplicant * 16 multiplier

	;; truncate result instead of round, as needed
	mov 	X, [S-2]
	jz 	myy
	add 	A, #1
	adc 	Y, #0
myy:	
	add	A, [S-4]	; + 32_low multiplicant * 16 multiplier / 2^16
	adc	Y, #0
	ret	#8		; result = YA
	.size	mul0shi, .-mul0shi

;
;	MLX16 signed integer multiplication - internal function
;
; Input:
;	A  	signed 16-bit multiplicand
;	X	unsigned 16-bit multiplier
;
; Output:
;	Y:A     signed 32-bit result
;	
muls_x:	
	add	A, #0		; redundant, but safety
	jnn	mulsx0
	neg	A
	MULU_X
	NEG_YA
	ret
mulsx0:				; redundant, i.e. not exercised
				; path:
				; 	mulI32hi_I32byI16 ->
				; 	(Y neg) mul0shi ->
				;	(A = Y) muls_x ->
				; 	(A non-neg) mulsx0
	MULU_X
	ret
	.size	muls_x, .-muls_x
#endif /* HAS_MLX16_COPROCESSOR */

	end
