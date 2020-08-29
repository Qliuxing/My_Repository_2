;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

	.text
	.global	_divU16_U16byU16
	.global	_divI16_I16byU16
	.global	_divI16_I16byI16

#include "math.mac"

;-------------------------------------------------------------------
; Functions calculate the 16-bit integer result of the division
; (n * 2^16) / d
;
; Limitations: The dividend 'n' should be less than divisor 'd'.
; Otherwise incorrect calculation occurs due to overflow. There is
; no internal check for overflow inside the function.
;-------------------------------------------------------------------
;
; Signed integer division: (int16 * 2^16) / int16 = int16
;
; Input:
;   A       dividend
;   [S-4]   divisor
;
; Output:
;   A       quotient
;-------------------------------------------------------------------
_divI16_I16byI16:       ; int16 divI16_I16byI16(int16 n, int16 d)

#if defined (HAS_MLX16_COPROCESSOR)
	mov     X,[S-4] ; get second argument (divisor) from the stack
	mov	Y,A     ; * 2^16
	mov	A,#0

	divs	YA,X
	divs	YA,X
	dadjs	YA,X
	ret		

#else
	mov     X,[S-4]

	;add     X, #0   ; set N if negative
	jnn     d1
	neg	X

	add	A, #0
	jnn	d2		; set N if negative
	neg	A

#ifdef __OPTIMIZE_SIZE__ /* compiled with -Os */
	jmp	d0
#else
	mov	Y,A
	mov	A,#0

	setb   	ML.7
	DIVU_X
	ret
#endif /* __OPTIMIZE_SIZE__ */
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divI16_I16byI16, .-_divI16_I16byI16


;-------------------------------------------------------------------
; Signed/Unsigned integer division: (int16 * 2^16) / uint16 = int16
;
; Input:
;   A       dividend (signed)
;   [S-4]   divisor  (unsigned)
;
; Output:
;   A       quotient
;-------------------------------------------------------------------
_divI16_I16byU16:       ; int16  divI16_I16byU16(int16 n, uint16 d)

#if defined (HAS_MLX16_COPROCESSOR)
	add	A, #0
	jnn	_divU16_U16byU16

	neg	A

	mov	X,[S-4]
	mov	Y,A       ; * 2^16
	mov	A,#0

	divu	YA,X
	divu	YA,X
	neg	A
	ret		

#else
	mov	X, [S-4]

d1:	add	A,#0			; set N if negative
	jnn	d0
	neg 	A

d2:
	mov	Y,A
	mov	A,#0

	setb	ML.7
	DIVU_X
	neg	A
	ret
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divI16_I16byU16, .-_divI16_I16byU16


;-------------------------------------------------------------------
; Unsigned integer division: (uint16 * 2^16) / uint16 = uint16
;
; Input:
;   A       dividend
;   [S-4]   divisor
;
; Output:
;   A       quotient
;-------------------------------------------------------------------
_divU16_U16byU16:        ; uint16 divU16_U16byU16(uint16 n, uint16 d)

#if defined (HAS_MLX16_COPROCESSOR)
	mov	X,[S-4]

	mov	Y,A       ; * 2^16
	mov	A,#0
	divu	YA,X
	divu	YA,X
	ret		

#else
	mov    X,[S-4]

d0:
	mov	Y,A
	mov	A,#0
	setb	ML.7	
	DIVU_X
	ret
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divU16_U16byU16, .-_divU16_U16byU16

	end
