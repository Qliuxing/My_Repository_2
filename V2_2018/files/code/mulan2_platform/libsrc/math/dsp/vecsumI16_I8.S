;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_vecsumI16_I8

#include "../math.mac"

	;;; GCC considers a local byte variable typically as an unsigned int
	;; hence not good optimizations

;
;	Signed integer sum
;
; Input:
;	A  	pointer to vector of signed 8-bit elements
;	[S-4]	size of vector
;
; Output:
;	A 	signed 16-bit result
;	

_vecsumI16_I8:
	inc	S, #2

	; loop initialization
	mov	X, [S-6]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	vs8_start
	mov	A, #0		; return value
	ret	#4
vs8_start:
#endif
	mov	Y, A 		; first operand, i.e. vector, in Y

	;; first iteration
	mov	AL, [Y++]	; result
	ssex	A
	djnz	X, ls

	; return value
	ret	#4

ls:
	mov	[S-2], A	; save accumulated value
	mov	AL, [Y++]	; get 8 bit value
	ssex	A
	add	A, [S-2]	; add with stored accumulated value
	djnz	X, ls

	; return value
	ret	#4
	.size	_vecsumI16_I8, .-_vecsumI16_I8
	
	end
