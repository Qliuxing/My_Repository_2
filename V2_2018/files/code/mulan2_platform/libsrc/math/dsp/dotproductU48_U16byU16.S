;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_dotproductU48_U16byU16

#include "../math.mac"

;
;	Unsigned integer multiplication
;
; Input:
;	A  	pointer to first vector of unsigned 16-bit elements
;	[S-4]	pointer to second vector of unsigned 16-bit elements
;	[S-6]	size of vector (unsigned, 0 < n <= 16 (MLX16-x8 only))
;	[S-8]	pointer to store 16MSBs of 48 bit result
;
; Output:
;	Y:A     unsigned 32-bit result
;	[[S-8]]	carry/overflow word stored (16 bits)
;	

_dotproductU48_U16byU16:	

#if defined (HAS_MLX16_COPROCESSOR)
	;; use of djnz cx limits to vectors of size 16

	inc	S, #2

	; loop initialization
	mov	Y, [S-8] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	dp48_start
	mov	YA, #0		; return value
	mov	X, [S-10]	; store overflow
	mov	[X], A
	ret	#4
dp48_start:
#endif
	mov	Cx, YL
	;mov	D, #0		; result

	mov	Y, A 		; first operand in y
	mov	X, [S-6]	; second operand in x

	;; first iteration
	;; saves 10 cycles
	;; code size +6-1=+5 insn = 6 bytes (as mov D,#0 has size 6)
	mov	A, [Y++]
	mulu D, A, [X++]   ; result
	mov A, #0          ; init overflow storage
                       ; was "sc A", but doesn't work on MLX16-FX which doesn't update C-flag
                       ; after MULU (MULU itself never overflows)
	djnz	Cx, lvc

	;; return value
	jmp	dp48_end
	;; outcoding:
	;mov	X, [S-10]
	;mov	[X], A
	;mov	YA, D
	;ret	#4

lvc:
	mov	[S-2], A	; first store high word from prev iteration

	mov	A, [Y++]
	macu	D, A, [X++]	; only instruction that modifies CVNZ flags (no, load/stores also)

	; keep track of overflow
	mov	A, [S-2]	; does not change C flag
	adc 	A, #0
	
	djnz	Cx, lvc

dp48_end:
	; store overflow
	mov	X, [S-10]
	mov	[X], A

	; return value
	mov	YA, D
	ret	#4
	
#else
	inc	S, #10

	mov	[S-8], A	; first operand
	mov	A, [S-14]	; second operand
	mov	[S-10], A		
	
	; loop initialization
	mov	X, [S-16] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	dp48_start
	mov	A, #0 		; return value
	mov	Y, A
	mov	X, [S-18]	; store overflow
	mov	[X], A
	ret	#12
dp48_start:
#endif
	mov	A, #0 		; result
	mov	Y, #0
	mov	[S-2], Y	; [S-2] keeps track of overflow

lvc:
	push	X		; keep loop counter, also when using Cx

	mov	[S-8], A	; result
	mov	[S-6], Y

	mov	Y, [S-10]	; first operand++
	mov	A, [Y++]
	mov	[S-10], Y

	mov	Y, [S-12] 	; second operand++
	mov	X, [Y++]
	mov	[S-12], Y

	;; multiply accumulate
	MULU_X
	add	A, [S-8] 	; result
	adc	Y, [S-6]

	; keep track of overflow
	mov	X, [S-4]	; does not change C flag
	adc 	X, #0
	mov 	[S-4], X

	pop	X		; restore loop counter

	djnz	X, lvc
	
	; store overflow word in supplied pointer 
	mov 	X, [S-18]	; pointer where to store
	push 	A
	mov 	A, [S-4]	; overflow value
	mov 	[X], A
	pop 	A

	; return value
 	ret 	#12
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_dotproductU48_U16byU16, .-_dotproductU48_U16byU16

	end
