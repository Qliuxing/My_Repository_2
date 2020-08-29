;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library
;

#include <mathlib.h>

	.text
	.global	_divU8_U8byU8
	.global	_divI8_I8byU8
	.global	_divI8_I8byI8
;-------------------------------------------------------------------
; Functions calculate the 8-bit integer result of the division
; n / d
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
;-------------------------------------------------------------------
_divI8_I8byI8:        ; int8 divI8_I8byI8(int8 n, int8 d)
	;; to test
#if defined (HAS_MLX16_COPROCESSOR)
	;; rely on GCC
	;; ssex	A		; redundant, compiler adds SSEX in caller
	;; same as divI16_I16byI16
	ssex	YA
	mov	X, [S-4]
	;; ssex X	; redundant, done on caller side

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

	;; ssex	A		; redundant if compiler adds SSEX in caller
	add	A, #0
	jnn	d2
	neg	A

#ifdef __OPTIMIZE_SIZE__ /* compiled with -Os */
	jmp		d00
#else
	swap	A
	mov	Y, #0

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

	mov	Ah, #0		; cast to int16 (see see Jira MLXCOMP-193)
	ret
#endif /* __OPTIMIZE_SIZE__ */
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divI8_I8byI8, .-_divI8_I8byI8

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
;-------------------------------------------------------------------
_divI8_I8byU8:        ; int8 divI8_I8byU8(int8 n, uint8 d)
#if defined (HAS_MLX16_COPROCESSOR)
	;; ssex	A		; redundant if compiler adds SSEX in caller
	;; same as divI16_I16byU16
	add	A, #0
	jnn 	_divU8_U8byU8

	mov	X, [S-4]
	;; usex X ; redundant, done on caller side

	neg	A          ; A now positive
	ssex	YA

	divu	YA, X
	divu	YA, X
	neg	A
	
	ssex	A		; cast to int16 (see see Jira MLXCOMP-193)
	ret
#else
	mov	X, [S-4]
	;; usex	X		; redundant, done at caller side

d1:
	;; ssex	A		; redundant if compiler adds USEX in caller
	add	A, #0
	jnn	d0

	neg	A
d2:
	swap	A
	mov	Y, #0

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

	neg	A

	ssex	A		; cast to int16 (see see Jira MLXCOMP-193)
	ret
#endif /* HAS_MLX16_COPROCESSOR */
	.size	_divI8_I8byU8, .-_divI8_I8byU8

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
_divU8_U8byU8:        ; uint8 divU8_U8byU8(uint8 n, uint8 d)

#if defined (HAS_MLX16_COPROCESSOR)
	;; rely on GCC
	;; usex	A		; redundant if compiler adds USEX in caller
	;; same as divU16_U16byu16
	usex	YA
	mov	X, [S-4]
	;; usex X	;	redundant, done on caller side

	divu	YA, X
	divu	YA, X

	usex 	A		; cast to uint16 (see see Jira MLXCOMP-193)
	ret
#else
	;; not much gain over C compiler if keeping the safety/redundant insn
	;; usex	A		; redundant if compiler adds USEX in caller
d0:
	mov X, [S-4]
	;; usex X  ; redundant, done on caller side
d00:
	swap	A
	mov	Y, #0

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
	.size	_divU8_U8byU8, .-_divU8_U8byU8

	end
