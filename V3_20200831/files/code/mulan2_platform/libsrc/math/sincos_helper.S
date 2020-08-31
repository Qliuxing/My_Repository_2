;
; Copyright (C) 2008-2013 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

	.text
#if defined (HAS_MLX16_COPROCESSOR)
	.global	_sin3rad_helper
	.global	_sin5rad_helper
	.global	_sin7rad_helper
	.global	_cos2rad_helper
	.global	_cos4rad_helper
	.global	_cos6rad_helper
#else
	; no support for MLX16(-8)
	; use lookup table
#endif  /* HAS_MLX16_COPROCESSOR */

;
;	Algorithm:
;	Taylor series expansion
;	sin(x)	= x - x^3/3! + x^5/5! - x^7/7! ...
;		= x (1 - x^2/(2*3) (1 - x^2/(4*5) (1 - x^2/(6*7) ( 1 - ... ))))
;	cos(x)	= 1 - x^2/2! + x^4/4! - x^6/6! ...
;		= 1 - x^2/2 (1 - x^2/(3*4) (1 - x^2/(5*6) (1 - ... )))
;

;	all multiplications are single precision: 16bit * 16bit -> 16MSBs:
;		mulu	YA, A, value
;		mov	A, Y
;	1-x^2/n is implemented as: negate of x^2/n before scaling to 16MSBs:
;		neg	YA
;		mov	A, Y
;	this caters for the case in which 16MSBs of x^2/n are 0, which would
;	nullify the final result

;	constraints: see above
;		sin3:	x > sqrt(2^16) -> 256
;		sin5:	x > sqrt(2 * 2^16) -> 362
;		sin7:	x > sqrt(2 * 2^16) -> 362
;		cos2:	x > sqrt(2) -> 2
;		cos4:	x > sqrt(2^16) -> 256
;		cos6:	x > sqrt(2 * 2^16) -> 362
;		all:	x < 65536 -> 57 deg
;
;	final term effective from (theoretical lower bound):
;		sin3:	x > (6 * 2^32)^(1/3) -> 2953 (2.5 deg)
;		sin5:	x > (120 * 2^64)^(1/5) = 18578 (16 deg)
;		sin7:	x > (5040 * 2^96)^(1/7) = 45428 (40 deg)
;		cos2:	x > (2 * 2^16)^(1/2) -> 362 (<1 deg)
;		cos4:	x > (24 * 2^48)^(1/4) = 9065 (8 deg)
;		cos6:	x > (720 * 2^80)^(1/6) = 30899 (27 deg)
;
;	effective results (measured: > 2LSBs):
;		sin1:	x upto 3 deg (2950)
;		sin3:	x upto < 18 deg
;		sin5:	x upto 44 deg
;		sin7:	full range (<57deg)
;		cos0:	x upto 443 (512TBC?)
;		cos2:	x upto 8-9 deg
;		cos4:	x upto 28-29 deg
;		cos6:	full range (<57 deg)
;	can be slightly further improved (i.e. minimize least squares) by cosrad - 1 TBC
;	slightly larger ranges when for signed (2^15) results, 
;	but still insufficient to use cos4 for 35-45 degrees
;

	;; can add scaling factors for the different angle representations,
	;; e.g. *(2pi)^2. Will not overflow in range of 0..pi/4.
	;; currently only x^2/2 implemented as a special case wo multiplication
	;; therefore need a multiplication anyway, so can do *2 pi in appl
	;; also sin requires multiplication with x
;#define U16(f) ((uint16) (f * 65536))
#define U16_6 10922
#define U16_12 5461
#define U16_20 3276
#define U16_30 2184
#define U16_42 1560
#define S16_1_2 16384
#define S16_1_24 1365
#define S16_1_720 45

#if defined (HAS_MLX16_COPROCESSOR)
;
;	nth order series approximation of trigonometric function sin(x)
;
; Input:
;	A  	unsigned 16-bit angle 
;		unit: radians
;		range [0..1), i.e. 16 fractional bits
;
; Output:
;	A	unsigned 16-bit result
;	

_sin3rad_helper:
	;; x - x^3 / 6 = x (1 - x^2/6)
	mov	X, A		; store x

	mulu	YA, A, A	; = x^2
	mov	A, Y
	mulu	YA, A, #U16_6	; * 1/6

	neg	YA		; = 1 - x^2/6
	mov	A, Y

	mulu	YA, A, X 	; * x
	mov	A, Y

	ret
	.size	_sin3rad_helper, .-_sin3rad_helper

_sin5rad_helper:
	;; x - x^3 / 6 + x^5/ 120= x (1 - x^2/6 (1 - x^2/20))
	mov	X, A		; store x
	
	mulu	YA, A, A	; = x^2
	mov	A, Y

	movu	D, A 		; store x^2 in Dl instead of push/pop
	
	mulu	YA, A, #U16_20	; * 1/20

	neg	YA		; = 1 - x^2/20
	mov	A, Y

	mov	Y, Dl		; *x^2
	mulu	YA, A, Y 	
	mov	A, Y
	mulu	YA, A, #U16_6	; *1/6

	neg	YA		; = 1 - ..
	mov	A, Y

	mulu	YA, A, X 	; *x
	mov	A, Y

	ret
	.size	_sin5rad_helper, .-_sin5rad_helper

_sin7rad_helper:
	;; x - x^3 / 6 + x^5/ 120 - x^7 /5040 = x (1 - x^2/6 (1 - x^2/20 (1 - x^2/42)))
	mov	X, A		; store x
	
	mulu	YA, A, A	; = x^2
	mov	A, Y

	movu	D, A 		; store x^2 in Dl instead of push/pop

	mulu	YA, A, #U16_42	; * 1/42

	neg	YA		; = 1 - x^2/42
	mov	A, Y

	mov	Y, Dl		; *x^2
	mulu	YA, A, Y 	
	mov	A, Y
	mulu	YA, A, #U16_20	; *1/20

	neg	YA		; = 1 - ..
	mov	A, Y

	mov	Y, Dl		; *x^2
	mulu	YA, A, Y 	
	mov	A, Y
	mulu	YA, A, #U16_6	; *1/6

	neg	YA		; = 1 - ..
	mov	A, Y

	mulu	YA, A, X 	; *x
	mov	A, Y

	ret
	.size	_sin7rad_helper, .-_sin7rad_helper

;
;	nth order series approximation of trigonometric function cos(x)
;
; Input:
;	A  	unsigned 16-bit angle 
;		unit: radians
;		range [0..1), i.e. 16 fractional bits
;
;		constraint: x > sqrt(2 * 2^16) -> 362
;
; Output:
;	A	unsigned 16-bit result
;	

_cos2rad_helper:
	;; 1 - x^2/2

	mulu	YA, A, A	; = x^2
	lsr	YA, #1		; /2, unsigned
	neg	YA
	mov	A, Y

	ret
	.size	_cos2rad_helper, .-_cos2rad_helper

_cos4rad_helper:
	;; 1 - x^2/2 + x^4/24 = 1 - (x^2/2 (1 - x^2/12))

	mulu	YA, A, A	; = x^2
	mov	A, Y
	mov	X, A		; store x^2
	
	mulu	YA, A, #U16_12	; * 1/12

	neg	YA		; = 1 - x^2/12
	mov	A, Y

	mulu	YA, A, X	; * x^2
	lsr	YA, #1		; /2, unsigned

	neg	YA		; = 1 - ..
	mov	A, Y

	ret
	.size	_cos4rad_helper, .-_cos4rad_helper


_cos6rad_helper:
	;; 1 - x^2/2 + x^4/24 - x^6/720 = 1 - (x^2/2 (1 - x^2/12 (1 - x^2/30)))

	mulu	YA, A, A	; = x^2
	mov	A, Y
	mov	X, A		; store x^2
	
	mulu	YA, A, #U16_30	; * 1/30

	neg	YA		; = 1 - x^2/30
	mov	A, Y

	mulu	YA, A, X	; * x^2
	mov	A, Y
	mulu	YA, A, #U16_12	; * 1/12

	neg	YA		; = 1 - ..
	mov	A, Y

	mulu	YA, A, X	; *x^2
	lsr	YA, #1		; /2, unsigned

	neg	YA		; = 1 - ..
	mov	A, Y

	ret
	.size	_cos6rad_helper, .-_cos6rad_helper

#else
	; no support for MLX16(-8)
	; use lookup table
#endif  /* HAS_MLX16_COPROCESSOR */

	end
