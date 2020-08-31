;
; Copyright (C) 2007-2013 Melexis N.V.
;
; Math library
;

#include <mathlib.h>


	.global _rand32

#if !defined (HAS_MLX16_COPROCESSOR)
	.extern _mulU32_U16byU16	
#endif

	.text

;
;	Algorithm:
;	Marsaglia, G. et al. (1973). Random Number Package: "Super-Duper"
;	School of Computer Science, McGill University
;
;	seed = seed * 69609 + 12345 mod 2^32
;
;	seed = (4073 + 2^16) * seed + 12345
;
;	seed = (4073 + 2^16) * (seed_lo + seed_hi*2^16) + 12345
;
;	seed = 4073*seed_lo + 4073*seed_hi*2^16 + 2^16*seed_lo + 12345
;

;
; Input:
;	Y:A	seed value
;
; Output:
;	Y:A	random / next seed value
;	

_rand32:	

#if defined (HAS_MLX16_COPROCESSOR)
	; seed_lo * 2^16
	push 	Y		

	mov	Y, A
	mov	A, #0
	mov	D, YA

	; + seed_lo * 4073
	mov	A, #4073
	macu	D, A, Y

	; seed_hi * 4073
	pop	Y
	mulu	YA, A, Y

	; * 2^16
	mov	Y, A
	mov	A, #0
	add	D, YA

	add	D, #12345

	; store seed value
	mov	YA, D
	ret

#else
	inc	S, #8
	
	; seed_lo
	mov	X, A

	; seed_hi (32bit)
	mov	A, Y
	mov	Y, #0
	mov 	[S-4], A
	mov 	[S-2], Y

	; seed_lo * 2^16 (32bit)
	mov 	Y, X
	mov 	A, #0
	mov 	[S-8], A
	mov 	[S-6], Y

	; 4073 * seed_lo
	push	X
	mov 	A, #4073
	call 	_mulU32_U16byU16	

	; + seed_lo * 2^16 
	add 	A, [S-10]
	adc	Y, [S-8]
	mov	[S-10], A
	mov 	[S-8], Y

	; seed_hi * 4073
	mov 	Y, [S-6]
	push 	Y
	mov 	A, #4073
	call 	_mulU32_U16byU16	

	; * 2^16
	mov 	Y, A
	mov 	A, #0

	; + (4073 * seed_lo + seed_lo * 2^16)
	add 	A, [S-12]
	adc 	Y, [S-10]

	; + 12345
	add 	A, #12345
	adc 	Y, #0

	; store seed value
	ret 	#14
#endif  /* HAS_MLX16_COPROCESSOR */
	.size	_rand32, .-_rand32

	end
