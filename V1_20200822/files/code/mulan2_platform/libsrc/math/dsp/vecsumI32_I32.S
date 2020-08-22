;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library - dsp module
;

#include <mathlib.h>

	.text
	.global	_vecsumI32_I32

	.extern _vecsumU32_U32

#include "../math.mac"

	;;; MLX16/MLX16-x8 versions not much gain over GCC (?)
	;; at least not wrt constant loop iteration and -funroll-loops

_vecsumI32_I32:	
;; = vecsumU32_U32
	jmp	_vecsumU32_U32
	.size	_vecsumI32_I32, .-_vecsumI32_I32

	end
