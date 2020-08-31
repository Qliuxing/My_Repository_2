;------------------------------------------------------------------------------
; Copyright (C) 2010 Melexis N.V.
;
; Interrupt vector table
; Software Platform
;
; Project: MLX90365
;
; NOTE: File vectors.S linked from application directory overrides
;       platform file
;
;
;------------------------------------------------------------------------------
;
;
; Helper macros to place vector to vector table
;
;   The format is:
;   
;      JMPVECTOR (offset, function)
;      -or-
;      CALLVECTOR (offset, function, priority)
;    
;    
;   where <offset> is an offset of the vector, the <function> is the 
;   interrupt processing function and <priority> is the absolute
;   priority at which the interrupt is to be processed.
;
; NOTE : To reduce possible errors this version of JMPVECTOR and CALLVECTOR macros
;        doesn't use compiler's include <vectors.h> and VECTORS_BASE.
;        Thus address of the vector should be specified as offset from the start
;        of the .vectors section
;
#define JMPVECTOR(addr,lbl)	    JVECTOR (addr), _##lbl
#define CALLVECTOR(addr,lbl,pr)	CVECTOR (addr), _##lbl, (pr)

	.macro JVECTOR name, lbl
	.org	\name, 0x0
	mov	UPR, #0x0
	.word	0x76DA		; JMPF but used as word to evade relaxation
	.word	\lbl
	.endm

	.macro CVECTOR name, lbl, priority
	.org	\name, 0x0
	psup	#\priority
	.word	0x76DA		; JMPF but used as word to evade relaxation
	.word	\lbl
	.endm

;---------------------------------------------------------------------------------
; Interrupt vector table
    .section .vectors,"xa",@progbits
    .global	__vectors

;              Vector    ISR                CPU prio      Allowed    Vector
;              offset    Handler            within ISR    SW prio    name
; --------------------------------------------------------------------------------
__vectors:
    JMPVECTOR  (0x0000,  start)                          ;           RST_WD_IT
    JMPVECTOR  (0x0008,  STACK_IT)                       ;           STACK_IT
    CALLVECTOR (0x0010,  PROT_ERR_IT,        0)          ; 0         PROT_ERR_IT
    CALLVECTOR (0x0018,  INV_AD_IT,          0)          ; 0         INV_AD_IT
    CALLVECTOR (0x0020,  PROG_ERR_IT,        0)          ; 0         PROG_ERR_IT
    CALLVECTOR (0x0028,  EXCHANGE_IT,        0)          ; 1         EXCHANGE_IT
    CALLVECTOR (0x0030,  NOT_CONNECTED_IT,   0)          ; 1         EXT5_IT
    CALLVECTOR (0x0038,  WD_ATT_IT,          0)          ; 1         WD_ATT_IT
    CALLVECTOR (0x0040,  NOT_CONNECTED_IT,   0)          ; 2         EXT6_IT
    CALLVECTOR (0x0048,  NOT_CONNECTED_IT,   0)          ; 5         EXT7_IT
    CALLVECTOR (0x0050,  TIMER_IT,           4)          ; 3-6       TIMER_IT
    CALLVECTOR (0x0058,  ADC_EOM_IT,         5)          ; 3-6       ADC_EOC_IT
    CALLVECTOR (0x0060,  NOT_CONNECTED_IT,   0)          ; 3-6       EE_IT
    CALLVECTOR (0x0068,  PWM_CYCLE_END_IT,   3)          ; 3-6       EXT0_IT
    CALLVECTOR (0x0070,  PWM_DC_IT,          5)          ; 3-6       EXT1_IT
    CALLVECTOR (0x0078,  PTC_SI_IT,          2)          ; 3-6       EXT2_IT
    CALLVECTOR (0x0080,  MT7V_IT,            2)          ; 3-6       EXT3_IT
    CALLVECTOR (0x0088,  MT7VB_IT,           2)          ; 3-6       EXT4_IT
    CALLVECTOR (0x0090,  SWI_IT,             5)          ; 7         SOFT_IT
    

; EOF
