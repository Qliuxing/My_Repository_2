;
; Copyright (C) 2008-2013 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

	.text
#if defined (HAS_MLX16_COPROCESSOR)
	.global	_tan3rad_helper
	.global	_tan5rad_helper
	.global	_tan7rad_helper
	.global	_tan9rad_helper
#else
	; no support for MLX16(-8)
	; use lookup table
#endif  /* HAS_MLX16_COPROCESSOR */

;
;	Algorithm:
;	Taylor series expansion
;	tan(x)	= x + x^3/3 + 2*x^5/15 + 17*x^7/315 ...

;#define U16(f) ((uint16) (f * 65536))
#define U16_3 21845
#define U16_2_15 8738
#define U16_17_315 3536
#define U16_62_2835 1433

#if defined (HAS_MLX16_COPROCESSOR)
;
;	nth order series approximation of trigonometric function tan(x)
;
; Input:
;	A  	unsigned 16-bit angle 
;		unit: radians
;		range [0..1), i.e. 16 fractional bits
;
; Output:
;	A	unsigned 16-bit result
;	

_tan3rad_helper:
	;; x + x^3 / 3 = x (1 + x^2/3)
	mov	X, A		; store x

	mulu	YA, A, A	; = x^2
	mov	A, Y
	mulu	YA, A, #U16_3	; * 1/3

	mov	A, Y

	mulu	YA, A, X 	; * x -> x^3/3
	mov	A, Y

	add	A, X		; + x

	ret
	.size	_tan3rad_helper, .-_tan3rad_helper

_tan5rad_helper:
	;; x + x^3 / 3 + 2 x^5/ 15 = x (1 + x^2/3 + 2 x^4/15)
	push	A		; store x
	;movu	D, #0
	
	mulu	YA, A, A	; = x^2
	mov	A, Y
	mov	X, A		; store x^2

	mulu	D, A, #U16_3	; = * 1/3

	mulu	YA, A, X	; = x^4
	mov	A, Y
	macu	D, A, #U16_2_15	; + * 2/15

	mov	YA, D
	mov	A, Y

	pop	X
	mulu	YA, A, X 	; *x
	mov	A, Y

	add	A, X		; +x

	ret			; ret #4?
	.size	_tan5rad_helper, .-_tan5rad_helper

_tan7rad_helper:
	;; x + x^3 / 3 + 2 x^5/ 15 + 17 x^7 /315
	;; = x (1 + x^2/3 + 2 x^4/15 + 17 x^6/315)
	push	A		; store x
	;movu	D, #0
	
	mulu	YA, A, A	; = x^2
	mov	A, Y
	mov	X, A		; store x^2

	mulu	D, A, #U16_3	; = * 1/3

	mulu	YA, A, X	; = x^4
	mov	A, Y
	macu	D, A, #U16_2_15	; + * 2/15

	mulu	YA, A, X	; = x^6
	mov	A, Y
	macu	D, A, #U16_17_315	; + * 17/315

	mov	YA, D
	mov	A, Y

	pop	X
	mulu	YA, A, X 	; *x
	mov	A, Y

	add	A, X		; + x

	ret			; ret #4?
	.size	_tan7rad_helper, .-_tan7rad_helper

_tan9rad_helper:
	;; x + x^3 / 3 + 2 x^5/ 15 + 17 x^7 /315 + 62 x^9/2835 
	;; = x (1 + x^2/3 + 2 x^4/15 + 17 x^6/315 + 62 x^8/2835)
	push	A		; store x
	;movu	D, #0
	
	mulu	YA, A, A	; = x^2
	mov	A, Y
	mov	X, A		; store x^2

	mulu	D, A, #U16_3	; = * 1/3

	mulu	YA, A, X	; = x^4
	mov	A, Y
	macu	D, A, #U16_2_15	; + * 2/15

	mulu	YA, A, X	; = x^6
	mov	A, Y
	macu	D, A, #U16_17_315	; + * 17/315

	mulu	YA, A, X	; = x^8
	mov	A, Y
	macu	D, A, #U16_62_2835	; + * 62/2835

	mov	YA, D
	mov	A, Y

	pop	X
	mulu	YA, A, X 	; *x
	mov	A, Y

	add	A, X		; +x

	ret			; ret #4?
	.size	_tan9rad_helper, .-_tan9rad_helper

#else
	; no support for MLX16(-8)
	; use lookup table for sin and cos and then calculate sin/cos
#endif  /* HAS_MLX16_COPROCESSOR */

	end
