;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_vecmaxU16_U16

#include "../math.mac"

				;
;	Unsigned integer max
;
; Input:
;	A  	pointer to vector of unsigned 16-bit elements
;	[S-4]	size of vector
;
; Output:
;	A 	unsigned 16-bit result
;	

_vecmaxU16_U16:	

	; loop initialization
	mov	X, [S-4]	; loop counter
#ifdef ALLOW_ZERO_SIZE_VECTOR
	;cmp	X, #0
	jnz	vm16_start
	mov	A, #0		; return value
	ret
vm16_start:
#endif
	;; use of looping vector downwards since mov A,[Y-2] does not exist
	mov	Y, X 		; first operand, i.e. &vector[n], in Y
	lsl	Y, #1		; (word address)
	add	Y, A

	;; first iteration
	mov	A, [--Y]	; result
	djnz	X, lm

	; return value
	ret

lm:
	cmp	A, [--Y]	; if (max < *vector++) { max = *(vector-1) }
	jug	le
	mov	A, [Y]
le:	
	djnz	X, lm

	; return value
	ret
	.size	_vecmaxU16_U16, .-_vecmaxU16_U16
	
	end
