;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_vecsumI16_I16

	.extern _vecsumU16_U16

#include "../math.mac"

	;;; no real improvement over GCC

				;
;	Unsigned integer sum
;
; Input:
;	A  	pointer to vector of signed 16-bit elements
;	[S-4]	size of vector
;
; Output:
;	A 	signed 16-bit result
;	

_vecsumI16_I16:	
;; = vecsumU16_U16
	jmp	_vecsumU16_U16
	.size	_vecsumI16_I16, .-_vecsumI16_I16

	end
