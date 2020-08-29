;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_dotproductI48_I16byI16

	.extern	_mulI32_I16byI16

#include "../math.mac"

;
;	Signed integer multiplication
;
; Input:
;	A  	pointer to first vector of signed 16-bit elements
;	[S-4]	pointer to second vector of signed 16-bit elements
;	[S-6]	size of vector (unsigned, 0 < n <= 16 (all CPU targets))
;	[S-8]	pointer to store 16MSBs of 48 bit result
;
; Output:
;	Y:A     un(!)signed 32-bit result
;	[[S-8]]	carry/overflow word stored (16 bits)
;	

_dotproductI48_I16byI16:	

#if defined (HAS_MLX16_COPROCESSOR)
	;; use of djnz cx limits to vectors of size 16

	inc	S, #6
				; [S-2] high word	
				; [S-4] copy of second operand
				; [S-6] copy of first operand
				; D result
				; X high word

	; loop initialization
	mov	Y, [S-12] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	dp48_start
	mov	YA, #0		; return value
	mov	X, [S-14]	; store overflow
	mov	[X], A
	ret	#8
dp48_start:
#endif
	mov	Cx, YL
	;mov	D, #0		; result

	mov	Y, A 		; first operand in Y
	mov	X, [S-10]	; second operand in X

	;; first iteration
	mov	A, [Y++]
	muls	D, A, [X++]	; result
	; keep track of overflow
	;cmp	D, #0
	sn	A		; sign extension
	neg	A
	mov	[S-6], Y	; store first operand
	mov	[S-4], X	; store second operand
	mov	X, A		; next loop iterations need high word in X
	djnz	Cx, lvc

	;; return value
	jmp	dp48_end
	;; outcoding:
	;mov	Y, [S-10]
	;mov	[Y], X
	;mov	YA, D
	;ret	#4

lvc:
	mov	[S-2], X	; first store high word from prev iteration

	mov	X, [S-6]	; reload first operand
	mov	A, [X++]
	mov	[S-6], X
	mov	Y, [S-4]	; reload second operand
	mov	X, [Y++]
	mov	[S-4], Y

	muls	YA, A, X	; multiply
	sn	X		; sign extension
	neg	X
	add	D, YA		; accumulate

	; keep track of overflow
	adc 	X, [S-2]	; add the sign extension with stored high word
	
	djnz	Cx, lvc

dp48_end:
	; store overflow
	mov	Y, [S-14]
	mov	[Y], X

	; return value
	mov	YA, D
	ret	#8
	
#else
	inc	S, #10
				; [S-2] high word
				; [S-4] result (hi)
				; [S-6] result (lo)
				; [S-8] copy of first operand
				; [S-10] copy of second operand
				; X high word, YA result

	mov	[S-8], A	; first operand
	mov	A, [S-14]	; second operand
	mov	[S-10], A		
	
	; loop initialization
	mov	Y, [S-16] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	dp48_start
	mov	A, #0 		; return value
	mov	Y, A
	mov	X, [S-18]	; store overflow
	mov	[X], A
	ret	#12
dp48_start:
#endif
	mov	Cx, YL
	
	mov	A, #0 		; result
	mov	Y, #0
	mov	X, #0		; high word

lvc:
	push	M		; keep loop counter

	mov	[S-8], A	; first store result from prev iteration
	mov	[S-6], Y
	mov	[S-4], X	; high word

	mov	Y, [S-10]	; first operand
	mov	A, [Y++]
	mov	[S-10], Y

	mov	Y, [S-12]	; second operand
	mov	X, [Y++]
	mov	[S-12], Y

	;; multiply
	;MULS_X: YA = A * X, clobbers Cx
	push	X
	call	_mulI32_I16byI16; first arg in A, second arg on stack,
				; result in YA
	dec	S, #2


	cmp	Y, #0		; sign extension
	sn	X
	neg	X

	;; accumulate
	add	A, [S-8]
	adc	Y, [S-6]
	adc	X, [S-4]

	pop	M		; restore loop counter

	djnz	Cx, lvc
	
	; store overflow word in supplied pointer
	push	Y
	mov 	Y, [S-20]	; pointer where to store
	mov 	[Y], X		; overflow value
	pop 	Y

	; return value
 	ret 	#12
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_dotproductI48_I16byI16, .-_dotproductI48_I16byI16

	end
