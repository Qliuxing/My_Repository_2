;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library
;


#include <mathlib.h>

	.text
	.global	_divU8hi_U8byU8

#if 0
/* deprecated functions */
	.global	_divI8hi_I8byU8
	.global	_divI8hi_I8byI8

;-------------------------------------------------------------------
; Functions calculate the 8-bit integer result of the division
; n * (2^8) / d, where (n<d)
;
;-------------------------------------------------------------------
;
; Signed integer division: int8 / int8 = int8
;
; Input:
;   A       dividend
;   [S-4]   divisor
;
; Output:
;   A       quotient
;
; Notes:
;   This function is deprecated and will be removed in next release
;-------------------------------------------------------------------
_divI8hi_I8byI8:        ; int8 divI8hi_I8byI8(int8 n, int8 d)
	;; to test
#if defined (HAS_MLX16_COPROCESSOR)
	;; rely on GCC
	;; ssex	A		; redundant, compiler adds SSEX in caller
	;; same as divI16_I16byI16
	ssex	YA
	swap	A		; * 256
	mov	Al, #0
	mov	X, [S-4]

	divs	YA, X
	divs	YA, X
	dadjs	YA, X

	ssex	A		; cast to int16 (see see Jira MLXCOMP-193)
	ret
#else
	;; C compiler also pretty complex for signed ..
	mov	X, [S-4]
	;; ssex	X		; redundant, done on caller side

	;add	X, #0
	jnn	d1
	neg	X

	;;ssex	A		; redundant if compiler adds U/SSEX in caller
	add	A, #0
	jnn	d2
	neg	A

#ifdef __OPTIMIZE_SIZE__ /* compiled with -Os */
	jmp		pp
#else
	mov	Y, A
	mov	A, #0

	setb	ML.7
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	rlc	A

	ssex A			; cast to int16 (see see Jira MLXCOMP-193)
	ret
#endif /* __OPTIMIZE_SIZE__ */
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divI8hi_I8byI8, .-_divI8hi_I8byI8

;-------------------------------------------------------------------
;
; Signed/Unsigned integer division: int8 / uint8 = int8
;
; Input:
;   A       dividend
;   [S-4]   divisor
;
; Output:
;   A       quotient
;
; Notes:
;   This function is deprecated and will be removed in next release
;-------------------------------------------------------------------
_divI8hi_I8byU8:        ; int8 divI8hi_I8byI8(int8 n, uint8 d)
#if defined (HAS_MLX16_COPROCESSOR)

	mov	X, [S-4]
	;; usex X ; redundant, done on caller side

	add	A, #0
	jnn 	pp		; if A >= 0

	neg	A          	; A now positive
	ssex	YA

	swap	A		; * 256
	mov	Al, #0

	divu	YA, X
	divu	YA, X
	neg	A
	
	ssex	A		; cast to int16 (see see Jira MLXCOMP-193)
	ret

pp:
	swap	A
	mov	Y, #0

	divu	YA, X
	divu	YA, X

	ssex 	A		; cast to int16 (see see Jira MLXCOMP-193)
	ret
#else
	mov	X, [S-4]
	;; usex	X		; redundant, done at caller side

d1:
	;; ssex	A		; redundant if compiler adds USEX in caller
	add	A, #0
	jnn	pp			; if A >= 0

	neg	A
d2:
	mov	Y, A
	mov	A, #0

	setb	ML.7
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	rlc	A

	;; to check
	neg	A

	ssex	A		; cast to int16 (see see Jira MLXCOMP-193)
	ret
pp:
	mov	Y, A
	mov	A, #0

	setb	ML.7
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	rlc	A

	ssex A			; cast to int16/uint16 (see see Jira MLXCOMP-193)
	ret
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divI8hi_I8byU8, .-_divI8hi_I8byU8

#endif /* 0 */

;-------------------------------------------------------------------
; Unsigned integer division: uint8 / uint8 = uint8
;
; Input:
;   A       dividend
;   [S-4]   divisor
;
; Output:
;   A       quotient
;-------------------------------------------------------------------
_divU8hi_U8byU8:        ; uint8 divU8hi_U8byU8(uint8 n, uint8 d)

#if defined (HAS_MLX16_COPROCESSOR)
	;; rely on GCC
	;; TBD
	;; usex	A		; redundant if compiler adds USEX in caller
	swap	A
	;; same as divU16_U16byu16
	mov	Y, #0
	mov	X, [S-4]
	;; usex X
	
	divu	YA, X
	divu	YA, X

	usex 	A		; cast to uint16 (see see Jira MLXCOMP-193)
	ret
#else
	;; not much gain over C compiler if keeping the safety/redundant insn
	;; usex	A		; redundant if compiler adds USEX in caller

	mov X, [S-4]
	;; usex X  ; redundant, done on caller side
d0:
	mov	Y, A
	mov	A, #0

	setb	ML.7
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	div1	X
	rlc	A

	mov	Ah, #0		; cast to int16/uint16 (see see Jira MLXCOMP-193)
	ret
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divU8hi_U8byU8, .-_divU8hi_U8byU8

	end
