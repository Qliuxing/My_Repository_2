;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_vecsumU16_U8

#include "../math.mac"

	;;; GCC considers a local byte variable typically as an unsigned int
	;; hence not good optimizations

;
;	Unsigned integer sum
;
; Input:
;	A  	pointer to vector of unsigned 8-bit elements
;	[S-4]	size of vector
;
; Output:
;	A 	unsigned 16-bit result
;	

_vecsumU16_U8:	

	; loop initialization
	mov	X, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	vs8_start
	mov	A, #0		; return value
	ret
vs8_start:
#endif
	mov	Y, A 		; first operand, i.e. vector, in Y

	;; first iteration
	mov	AL, [Y++]	; result
	usex	A
	djnz	X, ls

	; return value
	ret

ls:
	add	AL, [Y++]	; add 8 bit value
	adc	AH, #0		; carry over to AH 
	djnz	X, ls

	; return value
	ret
	.size	_vecsumU16_U8, .-_vecsumU16_U8
	
	end
