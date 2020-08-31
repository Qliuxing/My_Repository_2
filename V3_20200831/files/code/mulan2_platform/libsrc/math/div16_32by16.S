;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

    .text
	.global	_divI16_I32byI16
	.global	_divI16_I32byU16
	.global	_divU16_U32byU16

#include "math.mac"

;
;	Signed integer division 32 / 16 = 16
;
; Input:
;	Y:A	dividend
;	[S-4]	divisor
;
; Output:
;	A	quotient
;	

_divI16_I32byI16:

#if defined (HAS_MLX16_COPROCESSOR)
    mov     X,[S-4]
	divs	YA,X
	divs	YA,X
	dadjs	YA,X
	ret		

#else
	mov		X,[S-4]

	;add		X,#0
	jnn		d1
	neg		X

	add		Y,#0
	jnn		d2
	NEG_YA

#ifdef __OPTIMIZE_SIZE__ /* compiled with -Os */
	jmp		d0
#else
	setb	ML.7
	DIVU_X
	ret
#endif /* __OPTIMIZE_SIZE__ */
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divI16_I32byI16, .-_divI16_I32byI16

;
;	Signed/Unsigned integer division 32 / 16 = 16
;
; Input:
;	Y:A	dividend (signed)
;	[S-4]	divisor	 (unsigned)
;
; Output:
;	A	quotient
;	

_divI16_I32byU16:

#if defined (HAS_MLX16_COPROCESSOR)
	add		Y,#0
	jnn		_divU16_U32byU16
	neg		YA

	mov		X,	[S-4]

	divu	YA,X
	divu	YA,X
	neg		A

	ret		

#else
	mov		X,[S-4]

d1:	add		Y, #0			; test negative or not
	jnn		d0
	NEG_YA

d2:	setb	ML.7
	DIVU_X
	neg		A
	ret
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divI16_I32byU16, .-_divI16_I32byU16

;
;	Unsigned integer division 32 / 16 = 16
;
; Input:
;	Y:A	dividend
;	[S-4]	divisor
;
; Output:
;	A	quotient
;	

_divU16_U32byU16:

#if defined (HAS_MLX16_COPROCESSOR)
	mov		X,[S-4]
	divu	YA,X
	divu	YA,X
	ret

#else
	mov		X,[S-4]

d0:	setb	ML.7
	DIVU_X
	ret
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divU16_U32byU16, .-_divU16_U32byU16

	end
