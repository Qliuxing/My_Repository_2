;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_vecsumI8_I8

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
;	A 	signed 8-bit result
;	

;int8  vecsumI8_I8  (const  int8  *a, uint16 n);
_vecsumI8_I8:	

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
	djnz	X, ls

	; return value
	ssex	A		; safety
	ret

ls:
	add	AL, [Y++]	; add 8 bit value
	djnz	X, ls

	; return value
	ssex	A		; safety
	ret
	.size	_vecsumI8_I8, .-_vecsumI8_I8
	
	end
