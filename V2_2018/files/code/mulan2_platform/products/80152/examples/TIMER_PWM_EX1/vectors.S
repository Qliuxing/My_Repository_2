;------------------------------------------------------------------------------
; Copyright (C) 2010 Melexis N.V.
;
; Interrupt vector table
; Software Platform
;
; Project: MLX80152
;
; NOTE: Default vector table which is linked if application doesn't provide
;       own vectors.S file 
;------------------------------------------------------------------------------

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
#define JMPVECTOR(addr,lbl)     JVECTOR (addr), _##lbl
#define CALLVECTOR(addr,lbl,pr) CVECTOR (addr), _##lbl, (pr)

    .macro JVECTOR name, lbl
    .org    \name, 0x0
    mov UPR, #0x0
    .word   0x76DA      ; JMPF but used as word to evade relaxation
    .word   \lbl
    .endm

    .macro CVECTOR name, lbl, priority
    .org    \name, 0x0
    psup    #\priority
    .word   0x76DA      ; JMPF but used as word to evade relaxation
    .word   \lbl
    .endm

;----------------------------------------------------------------------------------------
; Interrupt vector table
    .section .vectors,"xa",@progbits
    .global __vectors

__vectors:
    JMPVECTOR  (0x0000,  start)                     ;           RST_WD_INT

    .org    0x0006  ; 0x4006: Address of external memory signature 
    .word   0xA55A  ; Signature

;              Vector    ISR        CPU prio         Allowed    Vector
;              offset    Handler    within ISR       SW prio    name
; ----------------------------------------------------------------------
    JMPVECTOR  (0x0008,  _fatal)                    ;           STACK_INT
    CALLVECTOR (0x0010,  _fatal,          0)        ; 0         PROT_ERR_INT
    CALLVECTOR (0x0018,  _fatal,          0)        ; 0         INV_AD_INT
    CALLVECTOR (0x0020,  _fatal,          0)        ; 0         PROG_ERR_INT
    CALLVECTOR (0x0028,  _fatal,          0)        ; 1         EXCHANGE_INT
    CALLVECTOR (0x0030,  _fatal,          0)        ; 1         ITC1_PWM_INT (EXT5_IT)
    CALLVECTOR (0x0038,  _fatal,          0)        ; 1         WD_ATT_INT
    CALLVECTOR (0x0040,  ITC1_TIMER1_INT, 1)        ; 2         ITC1_TIMER1_INT (EXT6_IT)
    CALLVECTOR (0x0048,  _fatal,          0)        ; 5         ITC1_TIMER2_INT (EXT7_IT)
    CALLVECTOR (0x0050,  _fatal,          0)        ; 3-6       CORE_TIMER_INT (TIMER_IT)
    CALLVECTOR (0x0058,  _fatal,          0)        ; 3-6       ADC_INT
    CALLVECTOR (0x0060,  _fatal,          0)        ; 3-6       EE_INT
    CALLVECTOR (0x0068,  _fatal,          0)        ; 3-6       COMMUT_INT (EXT0_IT)
    CALLVECTOR (0x0070,  _fatal,          0)        ; 3-6       ZCBLANK_INT (EXT1_IT)
    CALLVECTOR (0x0078,  _fatal,          0)        ; 3-6       ZC_INT (EXT2_IT)
    CALLVECTOR (0x0080,  _fatal,          0)        ; 3-6       ITC1_EXTIO_INT (EXT3_IT)
    CALLVECTOR (0x0088,  _fatal,          0)        ; 3-6       ITC1_DIAG_INT (EXT4_IT)
    CALLVECTOR (0x0090,  _fatal,          0)        ; 7         SOFT_INT

; EOF
