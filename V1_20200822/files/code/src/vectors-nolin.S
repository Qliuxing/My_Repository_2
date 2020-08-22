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
#define JMPVECTOR(addr,lbl)				JVECTOR (addr), _##lbl
#define CALLVECTOR(addr,lbl,pr)			CVECTOR (addr), _##lbl, (pr)
#define CALLVECTORID(addr,lbl,pr,id)	CVECTORID (addr), _##lbl, (pr), (id)


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

    .macro CVECTORID name, lbl, priority, id
    .org    \name, 0x0
    psup    #\priority
	mov		YL, #\id
    .word   0x76DA      ; JMPF but used as word to evade relaxation
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
    JMPVECTOR    (0x0000,  start)                 ;           RST_WD_IT
	.word	0xA55A	; Flash marker
    JMPVECTOR    (0x0008,  _fatal)                ;           STACK_IT      ; abort current instruction, return NOT possible
; bit <15:14> = 00 : Fixed configuration (no EEPROM)
;				01*: EEPROM User Configuration (@ 0x1000)
;				10 : EEPROM shadow User Configuration (@ 0x01AA)
;				11 : Reserved
; bit <13:12> = 00*: Reserved
; bit <11> = 0 : Motor Scope not supported
;			 1 : Motor Scope supported
; bit <10:00> = Motor Scope buffer-address (@ _adcMotorRun)
;	.word	0x4000								  ; UniROM support
;extern _adcMotorRun;
;	.word	0x4800 + _adcMotorRun				  ; UniROM & Motor-scope (IVIT)
	.word	0x0000								  ; -
    CALLVECTORID (0x0010,  _fatal,    0, 2)       ; 0         PROT_ERR_IT   ; abort current instruction, return NOT possible
    CALLVECTORID (0x0018,  _fatal,    0, 3)       ; 0         INV_AD_IT     ; abort current instruction, return NOT possible
    CALLVECTORID (0x0020,  _fatal,    0, 4)       ; 0         PROG_ERR_IT   ; abort current instruction, return NOT possible
    CALLVECTORID (0x0028,  _fatal,    0, 5)       ; 1         EXCHANGE_IT
    CALLVECTORID (0x0030,  _fatal,    0, 6)       ; 1         TASK_RST_IT
    CALLVECTORID (0x0038,  _fatal,    0, 7)       ; 1         WD_ATT_IT
    CALLVECTORID (0x0040,  _fatal,    0, 8)       ; 2         M4_MUTEX_IT
    CALLVECTORID (0x0048,  _fatal,	  0, 9)		  ; 5		  M4_SHE_IT		; NOTE: ml_LinInterruptHandler should NOT be called directly, but through MLX4 trampoline (linit)
    CALLVECTOR   (0x0050,  TIMER_IT,  5)          ; 3-6 (6)   TIMER_IT		; See Timer.c - Timer_Init()
    CALLVECTOR   (0x0058,  ADC_IT,    4)          ; 3-6 (5)   ADC_IT		; See ADC.c - ADC_Start()
    CALLVECTORID (0x0060,  _fatal,    0, 12)      ; 3-6 (-)   EE_IT
;   CALLVECTOR   (0x0060,  EE_IT,     2)          ; 3-6 (6)   EE_IT
    CALLVECTOR   (0x0068,  EXT0_IT,   3)		  ; 3-6 (4)   EXT0_IT		; Timer 1; See MotorDriver - MotorDriverInit()
    CALLVECTOR   (0x0070,  EXT1_IT,	  4)		  ; 3-6 (5)   EXT1_IT		; Timer 2; See PWM_Communication
    CALLVECTORID (0x0078,  _fatal,    0, 15)      ; 3-6 (-)   EXT2_IT		; PWMs
    CALLVECTORID (0x0080,  _fatal,    0, 16)      ; 3-6 (-)   EXT3_IT		; SPI
;   CALLVECTOR   (0x0080,  EXT3_IT,   5)          ; 3-6 (6)   EXT3_IT		; SPI; See SPI.c - SPI_initialize()
    CALLVECTOR   (0x0088,  EXT4_IT,   2)          ; 3-6 (3)   EXT4_IT		; Analog + Custom; See Diagnostic.c - DiagnosticsInit()
    CALLVECTORID (0x0090,  _fatal,    0, 18)      ; 7         SOFT_IT

; EOF
