;------------------------------------------------------------------------------
; Copyright (C) 2012 Melexis N.V.
;
; Interrupt vector table
; Software Platform
;
; Project: MLX81300
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
; NOTE : To reduce possible errors this version of JMPVECTOR and CALLVECTOR macros
;        doesn't use compiler's include <vectors.h> and VECTORS_BASE.
;        Thus address of the vector should be specified as offset from the start
;        of the .vectors section
;
#define JMPVECTOR(addr,lbl,pr)			JVECTOR (addr), _##lbl, (pr)
#define JMPFATALVCTR(addr,id,pr)		JVECTORID (addr), _##_fatal, (pr), (id)
#define CALLVECTOR(addr,lbl,pr)			CVECTOR (addr), _##lbl, (pr)
#define USER_MODE						(1 << 3)


    .macro JVECTOR name, lbl, priority
    .org    \name, 0x0
    mov     UPR, #\priority
    .word   0x76DA                          ; JMPF but used as word to evade relaxation
    .word   \lbl
    .endm

    .macro JVECTORID name, lbl, priority, id
    .org    \name, 0x0
    mov     UPR, #\priority                 ; Enter user-mode
    mov     YL, #\id
    .word   0x76DA                          ; JMPF but used as word to evade relaxation
    .word   \lbl
    .endm

    .macro CVECTOR name, lbl, priority
    .org    \name, 0x0
    psup    #\priority
    .word   0x76DA                          ; JMPF but used as word to evade relaxation
    .word   \lbl
    .endm

; ----------------------------------------------------------------------
; Interrupt vector table
; MMP110504-2: Use absolute priority-mask (not relative) for EXTn_IT's
    .section .vectors,"xa",@progbits
    .global __vectors

;                Vector    ISR        Prio within   Allowed   Vector
;                offset    Handler    ISR           SW prio   name
; ----------------------------------------------------------------------
__vectors:
#if defined (USE_PRESTART)
    JMPVECTOR    (0x0000, _prestart, 0)           ;           RST_WD_IT
#else
    JMPVECTOR    (0x0000, start, 0)               ;           RST_WD_IT
#endif /* USE_PRESTART */
	.word	0xA55A	; Flash marker
    JMPVECTOR    (0x0008, STACK_IT, USER_MODE+0)  ;           STACK_IT      ; abort current instruction, return NOT possible
; Unsupported interrupts are handled by non-returnable _fatal-handler
    JMPFATALVCTR (0x0010, 2, USER_MODE+0)         ; 0         PROT_ERR_IT   ; abort current instruction, return NOT possible (MMP141023-1)
    JMPFATALVCTR (0x0018, 3, USER_MODE+0)         ; 0         INV_AD_IT     ; abort current instruction, return NOT possible (MMP141023-1)
    JMPFATALVCTR (0x0020, 4, USER_MODE+0)         ; 0         PROG_ERR_IT   ; abort current instruction, return NOT possible (MMP141023-1)
    JMPFATALVCTR (0x0028, 5,           0)         ; 1         EXCHANGE_IT
    JMPFATALVCTR (0x0030, 6,           0)         ; 1         TASK_RST_IT
    JMPFATALVCTR (0x0038, 7,           0)         ; 1         WD_ATT_IT
    JMPFATALVCTR (0x0040, 8,           0)         ; 2         M4_MUTEX_IT
    CALLVECTOR   (0x0048, ml_LinInterruptHandler, 4) ; 5      M4_SHE_IT		; NOTE: ml_LinInterruptHandler should NOT be called directly, but through MLX4 trampoline (linit)
    CALLVECTOR   (0x0050, TIMER_IT,    5)         ; 3-6 (6)   TIMER_IT		; See Timer.c - Timer_Init()
    CALLVECTOR   (0x0058, ADC_IT,      4)         ; 3-6 (5)   ADC_IT		; See ADC.c - ADC_Start()
    JMPFATALVCTR (0x0060, 12,          0)         ; 3-6 (-)   EE_IT
    CALLVECTOR   (0x0068, EXT0_IT,     3)         ; 3-6 (4)   EXT0_IT		; Timer 1; See MotorDriver - MotorDriverInit()
    JMPFATALVCTR (0x0070, 14,          0)         ; 3-6 (-)   EXT1_IT		; Timer 2;
    JMPFATALVCTR (0x0078, 15,          0)         ; 3-6 (-)   EXT2_IT		; PWMs
    JMPFATALVCTR (0x0080, 16,          0)         ; 3-6 (-)   EXT3_IT		; SPI
    CALLVECTOR   (0x0088, EXT4_IT,     2)         ; 3-6 (3)   EXT4_IT		; Analog + Custom; See Diagnostic.c - DiagnosticsInit()
    JMPFATALVCTR (0x0090, 18,          0)         ; 7         SOFT_IT

; EOF
