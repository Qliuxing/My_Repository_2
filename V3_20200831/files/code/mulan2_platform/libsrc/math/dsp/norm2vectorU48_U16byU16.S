;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_norm2vectorU48_U16byU16

#include "../math.mac"

;
;	Unsigned integer multiplication
;
; Input:
;	A  	pointer to vector of unsigned 16-bit elements
;	[S-4]	size of vector (unsigned, 0 < n <= 16)
;	[S-6]	pointer to store 16MSBs of 48 bit result
;
; Output:
;	Y:A     unsigned 32-bit low-part of result
;	[[S-6]]	unsigned 16-bit hi-part of result
;	

_norm2vectorU48_U16byU16:	

#if defined (HAS_MLX16_COPROCESSOR)
	;; use of djnz cx limits to vectors of size 16

	; loop initialization
	mov	Y, [S-4] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	nv48_start
	mov	YA, #0		; return value
	mov	X, [S-6]	; store overflow
	mov	[X], A
	ret
nv48_start:
#endif
	mov	Cx, YL
	;mov	D, #0		; sum
	;mov	X, #0		; X keeps track of overflow

	mov	Y, A 		; vector, first operand

	;; first iteration
	;; saves 10 cycles
	;; code size +5-2=+3 insn = 2 bytes (as mov D,#0 has size 6)
	mov	A, [Y++]
	mulu	D, A, A
	mov X, #0          ; init overflow storage
                       ; was "sc X", but doesn't work on MLX16-FX which doesn't update C-flag
                       ; after MULU (MULU itself never overflows)
	djnz	Cx, lnc

	; return value
	jmp	nv48_end
	;; outcoding:
	;mov	Y, [S-6]
	;mov	[Y], X
	;mov	YA, D
	;ret

lnc:
	; take next element, square it, and add it to d
	mov	A, [Y++]	; vector++
	macu	D, A, A
	adc	X, #0		; keep track of overflow
	djnz	Cx, lnc

nv48_end:
	; store overflow
	mov	Y, [S-6]
	mov	[Y], X

	; return value
	mov	YA, D
	ret
	
#else
	inc	S, #8

	mov	[S-8], A	; vector, first operand
	
	; loop initialization
	mov	Y, [S-12] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	nv48_start
	mov	A, #0 		; return value
	mov	Y, A
	mov	X, [S-14]	; store overflow
	mov	[X], A
	ret	#10
nv48_start:
#endif
	mov	Cx, YL
	mov	A, #0 		; result
	mov	Y, A
	mov	[S-2], Y	; overflow

loopc:
	mov	[S-6], A	; result
	mov	[S-4], Y

	mov	Y, [S-8]	; vector++
	mov	A, [Y++]
	mov	[S-8], Y

	;; multiply accumulate
	push	M		; keep loop counter, also when using Cx, as used by MULU_X, Ml sufficient
	mov	X, A
	MULU_X
	pop	M		; restore loop counter
	add	A, [S-6] 	; result
	adc	Y, [S-4]

	; keep track of overflow
	; alt: make following conditional on carry/overflow flag
	; to fast track normal case
	mov	X, [S-2]	; does not change C flag
	adc	X, #0
	mov	[S-2], X
	
	djnz	Cx, loopc	; could use X here anyway,
				; if deferring the pop, but it would be odd
				; that MLX16-x8 would constrain vector length
				; and MLX16 not
	
	;; store overflow word in supplied pointer 
	mov	X, [S-14]	; pointer where to store
	push	A
	mov	A, [S-4]	; overflow value
	mov	[X], A
	pop	A

	; return value
 	ret	#10
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_norm2vectorU48_U16byU16, .-_norm2vectorU48_U16byU16

	end
