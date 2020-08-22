;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_vecsumI32_I16

#include "../math.mac"

	;;; MLX16/MLX16-x8 versions not much gain over GCC (?)
	;; at least not wrt constant loop iteration and -funroll-loops

;
;	Signed integer sum
;
; Input:
;	A  	pointer to vector of signed 16-bit elements
;	[S-4]	size of vector (unsigned, 0 < n <= 16 (MLX16 only))
;
; Output:
;	Y:A     signed 32-bit result
;	

_vecsumI32_I16:	

#if defined (HAS_MLX16_COPROCESSOR)
	;; use of djnz cx limits to vectors of size 16

	; loop initialization
	mov	X, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	vs16_start
	mov	YA, #0		; return value
	ret
vs16_start:
#endif
	mov	Y, A 		; first operand, i.e. vector, in Y

	;; first iteration - no savings
	movs	D, [Y++]	; result - loads 16 bit value
	djnz	X, ls

	; return value
	mov	YA, D
	ret

ls:
	adds	D, [Y++]	; add D,[Y++] adds 32 bit values
	djnz	X, ls

	; return value
	mov	YA, D
	ret
	
#else
	mov	X, A		; first operand, i.e. vector, in X

	; loop initialization
	mov	Y, [S-4] 	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	Y, #0
	jnz	vs16_start
	mov	A, #0 		; return value
	mov	Y, A
	ret
vs16_start:
#endif
	inc	S, #2
	mov	Cx, YL

	;; first iteration
	mov	A, [X++]	; result in YA
	;cmp	A, #0
	sn	Y		; sign extension to 32bit
	neg	Y
	djnz	Cx, ls

	; return
	ret	#4
ls:
	mov	[S-2], Y	; first store high word from prev iteration
	mov	Y, [X]		; sign extension needed for add
	;cmp	Y, #0
	sn	Y		
	neg	Y

	add	A, [X++]
	adc	Y, [S-2]	; add the sign extension with stored high word

	djnz	Cx, ls
	
	ret	#4
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_vecsumI32_I16, .-_vecsumI32_I16

	end
