;
; Copyright (C) 2007-2013 Melexis N.V.
;
; MelexCM Software Platform
;

#include <mathlib.h>

	.text
	.global	_divU32_U32byU16
	.global	_divI32_I32byU16
	.global	_divI32_I32byI16

#include "math.mac"

	.macro	DIV32x8
	push	YA
	mov		A, Y          ; A = MSW(dividend_32bit)
	mov		Y, #0
	mov		X, [S-8]     ; X = divisor (from stack)
	divu	YA, X
	divu	YA, X
	mov		[S-2], A      ; MSW(result) = A (remainder in Y)
	mov		A, [S-4]      ; A = LSW(dividend_32bit); Y still keeps remainder from previous division
	divu	YA, X
	divu	YA, X
	mov		Y, [S-2]      ; Y = MSW(result); A still keeps LSW(result) 
	.endm

	.macro	DIV32
	push	A			; Save LSW of dividend
	setb	ML.7			; Prepare dividend
	mov		A, Y
	mov		Y,#0
	DIVU_X
	push	A			; Save MSW of quotient
	mov		C,AL.0		; Get back carry
	mov		A,[S-4]		; Get back LSW of dividend
	DIVU_X
	pop	Y				; Get back MSW of quotient
	pop	X				; garbage
	.endm

; -----------------------------------------------------------------------------
;	Signed integer division 32 / 16 = 32
;
; Input:
;	Y:A		dividend
;	[S-4]		divisor
;
; Output:
;	Y:A		quotient
;	

_divI32_I32byI16:

#if defined (HAS_MLX16_COPROCESSOR)
	mov		X, [S-4]	; get divisor

	;add		X,#0        ; if (divisor >= 0) ...
	jnn		d1          ; ... do divI32_I32byU16

	neg		X           ; else divisor = -divisor
	mov		[S-4], X    ; also replace saved divisor on stack

	add		Y, #0       ; if (dividend >= 0) ...
	jnn		d2          

	neg		YA          ; else dividend = -dividend

#ifdef __OPTIMIZE_SIZE__ /* compiled with -Os */
    jmp     d0
#else
	DIV32x8             ; divU32_U32byU16
	ret		#6
#endif /* __OPTIMIZE_SIZE__ */

#else
	mov		X, [S-4]	; divisor

	;add		X,#0
	jnn		d1
	neg		X

	add		Y, #0
	jnn		d2
	NEG_YA

#ifdef __OPTIMIZE_SIZE__ /* compiled with -Os */
	jmp		d0
#else
	DIV32
	ret
#endif /* __OPTIMIZE_SIZE__ */
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divI32_I32byI16, .-_divI32_I32byI16

; -----------------------------------------------------------------------------
;	Signed/Unsigned integer division 32 / 16 = 32
;
; Input:
;	Y:A		dividend (signed)
;	[S-4]		divisor	(unsigned)
;
; Output:
;	Y:A		quotient
;	

_divI32_I32byU16:

#if defined (HAS_MLX16_COPROCESSOR)
	mov		X, [S-4]   ; get divisor

d1:	add		Y, #0      ; if (dividend >= 0) ...
	jnn		d0         ; .. do divU32_U32byU16
	neg		YA         ; else dividend = -dividend

d2:
	DIV32x8            ;      divU32_U32byU16
	neg		YA         ;      result = -result

	ret		#6

#else
	mov		X, [S-4]	; divisor

d1:	add		Y, #0			; test negative or not
	jnn		d0
	NEG_YA

d2:
	DIV32
	NEG_YA

	ret
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divI32_I32byU16, .-_divI32_I32byU16

; -----------------------------------------------------------------------------
;	Unsigned integer division 32 / 16 = 32
;
; Input:
;	Y:A		dividend
;	[S-4]		divisor
;
; Output:
;	Y:A		quotient
;	

_divU32_U32byU16:

#if defined (HAS_MLX16_COPROCESSOR)

d0:	DIV32x8
	ret		#6	

#else
	mov		X, [S-4]	; divisor

d0:	DIV32
	ret

#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divU32_U32byU16, .-_divU32_U32byU16

	end
