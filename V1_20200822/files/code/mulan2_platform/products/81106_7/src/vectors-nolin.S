;------------------------------------------------------------------------------
; Copyright (C) 2009-2014 Melexis N.V.
;
; Interrupt vector table
; Software Platform
;
; Project: MLX8110x
;
; NOTE: File vectors.S linked from application directory overrides
;       this file
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
; @note:
;   - # To reduce possible errors this version of JMPVECTOR and CALLVECTOR macros
;       doesn't use compiler's include <vectors.h> and VECTORS_BASE.
;       Thus address of the vector should be specified as offset from the start
;       of the .vectors section
;   - # Interrupt handler routine invoked by JMPUSERVECTOR is executed
;       in USER mode with priority 0.
;
#define JMPVECTOR(addr,lbl)     JVECTOR  (addr), _##lbl
#define JMPUSERVECTOR(addr,lbl) JUVECTOR (addr), _##lbl
#define CALLVECTOR(addr,lbl,pr) CVECTOR  (addr), _##lbl, (pr)

#define USER_MODE   (1 << 3)

    .macro JVECTOR name, lbl
    .org    \name, 0x0
    mov UPR, #0x0       ; SYSTEM mode with priority 0
    .word   0x76DA      ; JMPF but used as word to evade relaxation
    .word   \lbl
    .endm

    .macro JUVECTOR name, lbl
    .org    \name, 0x0
    mov UPR, #(USER_MODE | 0)   ; USER mode with priority 0
    .word   0x76DA              ; JMPF but used as word to evade relaxation
    .word   \lbl
    .endm

    .macro CVECTOR name, lbl, priority
    .org    \name, 0x0
    psup    #\priority
    .word   0x76DA      ; JMPF but used as word to evade relaxation
    .word   \lbl
    .endm

;----------------------------------------------------------------------------------------------------------------------------------------
; Interrupt vector table
    .section .vectors,"xa",@progbits
    .global __vectors

;              Vector    ISR                    CPU prio        Allowed     Vector
;              offset    Handler                within ISR      SW prio     name
; ---------------------------------------------------------------------------------------------------------------------------------------
__vectors:
    JMPVECTOR  (0x0000,  start)                                 ;           RST_WD_IT
    JMPUSERVECTOR (0x0008,  STACK_IT)                           ;           STACK_IT      ; abort current instruction, return NOT possible
    JMPUSERVECTOR (0x0010,  PROT_ERR_IT)                        ; 0         PROT_ERR_IT   ; abort current instruction, return NOT possible
    JMPUSERVECTOR (0x0018,  INV_AD_IT)                          ; 0         INV_AD_IT     ; abort current instruction, return NOT possible
    JMPUSERVECTOR (0x0020,  PROG_ERR_IT)                        ; 0         PROG_ERR_IT   ; abort current instruction, return NOT possible
    CALLVECTOR (0x0028,  EXCHANGE_IT,           1)              ; 1         EXCHANGE_IT
    CALLVECTOR (0x0030,  TASK_RST_IT,           1)              ; 1         TASK_RST_IT
    CALLVECTOR (0x0038,  WD_ATT_IT,             1)              ; 1         WD_ATT_IT
    CALLVECTOR (0x0040,  M4_MUTEX_IT,           2)              ; 2         M4_MUTEX_IT
    CALLVECTOR (0x0048,  _fatal,                0)              ; 5         M4_SHE_IT
    CALLVECTOR (0x0050,  TIMER_IT,              3)              ; 3-6       TIMER_IT
    CALLVECTOR (0x0058,  ADC_IT,                3)              ; 3-6       ADC_IT
    CALLVECTOR (0x0060,  EE_IT,                 3)              ; 3-6       EE_IT
    CALLVECTOR (0x0068,  EXT0_IT,               5)              ; 3-6       EXT0_IT ; Timer 1
    CALLVECTOR (0x0070,  EXT1_IT,               5)              ; 3-6       EXT1_IT ; Timer 2
    CALLVECTOR (0x0078,  EXT2_IT,               4)              ; 3-6       EXT2_IT ; PWMs
    CALLVECTOR (0x0080,  EXT3_IT,               3)              ; 3-6       EXT3_IT ; SPI
    CALLVECTOR (0x0088,  EXT4_IT,               3)              ; 3-6       EXT4_IT ; Analog + Custom
    CALLVECTOR (0x0090,  SOFT_IT,               3)              ; 7         SOFT_IT

; EOF
