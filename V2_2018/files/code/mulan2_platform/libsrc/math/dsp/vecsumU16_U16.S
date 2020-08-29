;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_vecsumU16_U16

#include "../math.mac"

	;;; no real improvement over GCC

				;
;	Unsigned integer sum
;
; Input:
;	A  	pointer to vector of unsigned 16-bit elements
;	[S-4]	size of vector
;
; Output:
;	A 	unsigned 16-bit result
;	

_vecsumU16_U16:	

	; loop initialization
	mov	X, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	vs16_start
	mov	A, #0		; return value
	ret
vs16_start:
#endif
	mov	Y, A 		; first operand, i.e. vector, in Y

	;; first iteration
	mov	A, [Y++]	; result
	djnz	X, ls

	; return value
	ret

ls:
	add	A, [Y++]	; add 16 bit value
	djnz	X, ls

	; return value
	ret
	.size	_vecsumU16_U16, .-_vecsumU16_U16
	
	end
