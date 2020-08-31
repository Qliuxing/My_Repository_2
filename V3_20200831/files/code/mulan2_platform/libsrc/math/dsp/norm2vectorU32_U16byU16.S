;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_norm2vectorU32_U16byU16

#include "../math.mac"

;
;	Unsigned integer multiplication
;
; Input:
;	A  	pointer to vector of unsigned 16-bit elements
;	[S-4]	size of vector (unsigned, >0)
;
; Output:
;	Y:A     unsigned 32-bit result
;	

_norm2vectorU32_U16byU16:	

#if defined (HAS_MLX16_COPROCESSOR)
	; loop initialization
	mov	X, [S-4] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp 	X, #0
	jnz	nv32_start
	mov	YA, #0		; return value
	ret
nv32_start:
#endif
	;mov	D, #0		; sum

	mov	Y, A 		; vector, first operand

	;; first iteration
	;; mulu 2 clock cycles cheaper than macu 
	;; also saves 4 clock cycles for initialization of mov D, #0
	;; total savings: 6 clock cycles
	;; code size +5-1=+4 insn = 4 bytes (as mov D,#0 has size 6)
	mov	A, [Y++]
	mulu	D, A, A		; result
	djnz	X, ln

	; return value
	mov	YA, D
	ret

ln:
	; take next element, square it, and add it to d
	mov	A, [Y++]	; vector++
	macu	D, A, A		
	djnz	X, ln

	; return value
	mov	YA, D
	ret
	
#else
	inc	S, #6

	mov	[S-6], A	; vector, first operand
	
	; loop initialization
	mov	X, [S-10] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	nv32_start
	mov	A, #0 		; return value
	mov	Y, A 		
	ret	#8
nv32_start:
#endif
	
	mov	A, #0 		; result
	mov	Y, A 		

loop:
	mov	[S-4], A	; result
	mov	[S-2], Y

	mov	Y, [S-6]	; vector++
	mov	A, [Y++]
	mov	[S-6], Y

	;; multiply accumulate
	push	X		; keep loop counter, also when using Cx
	mov	X, A
	MULU_X
	pop	X		; restore loop counter
	add	A, [S-4] 	; result
	adc	Y, [S-2]

	djnz	X, loop
	
	; return value
 	ret	#8
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_norm2vectorU32_U16byU16, .-_norm2vectorU32_U16byU16

	end
