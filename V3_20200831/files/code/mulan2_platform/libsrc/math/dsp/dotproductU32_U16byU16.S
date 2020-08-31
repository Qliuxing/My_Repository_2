;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_dotproductU32_U16byU16

#include "../math.mac"

;
;	Unsigned integer multiplication
;
; Input:
;	A  	pointer to first vector of unsigned 16-bit elements
;	[S-4]	pointer to second vector of unsigned 16-bit elements
;	[S-6]	size of vector (unsigned, 0 < n <= 16 (MLX16-x8 only))
;
; Output:
;	Y:A     unsigned 32-bit result
;	

_dotproductU32_U16byU16:	

#if defined (HAS_MLX16_COPROCESSOR)
	;; use of djnz cx limits to vectors of size 16

	; loop initialization
	mov	Y, [S-6] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	dp32_start
	mov	YA, #0		; return value
	ret
dp32_start:
#endif
	mov	Cx, YL
	;mov	D, #0		; result

	mov	Y, A 		; first operand in y
	mov	X, [S-4]	; second operand in x

	;; first iteration
	;; mulu 2 clock cycles cheaper than macu 
	;; also saves 4 clock cycles for initialization of mov D, #0
	;; total savings: 6 clock cycles
	;; code size +5-1=+4 insn = 4 bytes (as mov D,#0 has size 6)
	mov	A, [Y++]
	mulu	D, A, [X++]	; result
	djnz	Cx, lv

	; return value
	mov	YA, D
	ret

lv:
	mov	A, [Y++]
	macu	D, A, [X++]	; only instruction that modifies CVNZ flags (no, load/stores also)
	djnz	Cx, lv

	; return value
	mov	YA, D
	ret
	
#else
	inc	S, #8

	mov	[S-6], A	; first operand
	mov	A, [S-12]	; second operand
	mov	[S-8], A		
	
	; loop initialization
	mov	X, [S-14] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	dp32_start
	mov	A, #0 		; return value
	mov	Y, A
	ret	#10
dp32_start:
#endif
	mov	A, #0 		; result
	mov	Y, A

lv:
	push	X		; keep loop counter, also when using Cx

	mov	[S-6], A	; result
	mov	[S-4], Y

	mov	Y, [S-8]	; first operand++
	mov	A, [Y++]
	mov	[S-8], Y

	mov	Y, [S-10] 	; second operand++
	mov	X, [Y++]
	mov	[S-10], Y

	;; multiply accumulate
	MULU_X
	add	A, [S-6] 	; result
	adc	Y, [S-4]

	pop	X		; restore loop counter

	djnz	X, lv
	
	; return value
 	ret	#10
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_dotproductU32_U16byU16, .-_dotproductU32_U16byU16

	end
