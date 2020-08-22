; ---------------------------------------------------------------------------
;
; Description:
;   Timing constant defined in ROM-tables (link-time configuration)
;   Fosc = 250 kHz (fixed)
;
;
; Copyright (c) Melexis Digital Competence Center
;
; ---------------------------------------------------------------------------

; Defines for CFG_SLEEP_TO
#define coLIN20_ANY     1
#define coLIN13_9600    2
#define coLIN13_19200   3

; Defines for CFG_PLL_FREQ
#define coPLL_12MHZ     1
#define coPLL_18MHZ     2
#define coPLL_20MHZ     3
#define coPLL_24MHZ     4
#define coPLL_25MHZ     5
#define coPLL_28MHZ     6
#define coPLL_30MHZ     7
#define coPLL_32MHZ     8

;Constant tables

;--------------------------------------------------------------------------------------------------
; Sleep Timeout (depends on Fosc frequency)
;
; With an oscillator at Fosc, the formula is:
;	Sleep_timeout = (1/Fosc) * 2^(9+SleepPre_Val) * (16 + SleepCnt_Val) * (256 / SleepCntInc)
;
; Notes:
;   1. Use the biggest SleepCnt_Val possible -> (4 bits = 15)
;   2. Sleep timeout is not used by fast protocol
;
;SleepPre_Val or 8 (enable bit)	=> Rom:ConstantTbl shr 2[0]
;SleepCnt_Val                   => Rom:ConstantTbl shr 2[1]
;SleepCntInc	                => Rom:ConstantTbl shr 2[2]

ConstantTbl

#IF COMPILE_STANDALONE_LOADER eq 0
#IF CFG_SLEEP_TO eq coLIN20_ANY
                dc.w 00802h     ;  [1][0][3][2] ; TO = 4.19 seconds = 4.00 + 4.9%

#ELIF CFG_SLEEP_TO eq coLIN13_9600
                dc.w 05804h     ;  [1][0][3][2] ; TO = 2.75 seconds = 2.60 + 5.7%

#ELIF CFG_SLEEP_TO eq coLIN13_19200
                dc.w 05808h     ;  [1][0][3][2] ; TO = 1.38 seconds = 1.30 + 5.7%

#ELSE
                ; by default use LIN2.0 timeout
                dc.w 00802h     ;  [1][0][3][2]
#ENDIF ; CFG_SLEEP_TO
#ELSE
    ; not used for standalone (any value)
    dc.w 00000h     ;  [1][0][3][2]

#ENDIF  ; COMPILE_STANDALONE_LOADER 


	%assert {ConstantTbl and 3} eq 0	;Make sure the last 2 bits of the table are 0

;--------------------------------------------------------------------------------------------------
; Wake-up detection (depends on F_PLL frequency)
;
; Use 50us as a base interval, i.e. set baudrate counter to 20 kbps in sleep mode
;
; ( 2^(SleepDelayPre + 1) * SleepDelayCnt) / F_PLL = 50 us
;
; SleepDelayPre is caPresc (value between 0 and 11)
; SleepDelayCnt is caBaud  (value is between 99 and 200)

;SleepDelayPre                  => Rom:ConstantTbl shr 2[4]
;SleepDelay                     => Rom:ConstantTbl shr 2[5]
;SleepDelayCnt                  => Rom:ConstantTbl shr 2[6] and Rom:ConstantTbl shr 2[7]

; Delay used before entering the sleep mode after reception of a sleep command (x50us)
; NOTE: it is possible to use 10kbps as baudrate and to double SleepDelay and WUPlsMinimum
;or to change each value idependantely
;SleepDelay		equ 5	= Rom:ConstantTbl shr 2[5]

#IF COMPILE_STANDALONE_LOADER eq 0

    #IF   CFG_PLL_FREQ eq coPLL_12MHZ
                dc.w 05196h ; [5][4][7][6] -- for 12MHz

    #ELIF CFG_PLL_FREQ eq coPLL_18MHZ
                dc.w 05270h ; [5][4][7][6] -- for 18MHz

    #ELIF CFG_PLL_FREQ eq coPLL_20MHZ
                dc.w 0527Dh ; [5][4][7][6] -- for 20MHz

    #ELIF CFG_PLL_FREQ eq coPLL_24MHZ
                dc.w 05296h ; [5][4][7][6] -- for 24MHz

    #ELIF CFG_PLL_FREQ eq coPLL_25MHZ
                dc.w 0529Ch ; [5][4][7][6] -- for 25MHz

    #ELIF CFG_PLL_FREQ eq coPLL_28MHZ
                dc.w 052AFh ; [5][4][7][6] -- for 28MHz

    #ELIF CFG_PLL_FREQ eq coPLL_30MHZ
                dc.w 052BCh ; [5][4][7][6] -- for 30MHz

    #ELIF CFG_PLL_FREQ eq coPLL_32MHZ
                dc.w 052C8h ; [5][4][7][6] -- for 32MHz

    #ELSE
                ;pll frequency is not defined, generate an error
                %FATAL "CFG_PLL_FREQ is not define"
    #ENDIF
#ELSE
    ; not used for standalone (any value)
    dc.w 00000h ; [5][4][7][6]

#ENDIF

; Minimum length of a wake-up pulse (x50us)
;WUPlsMinimum	equ 3	;150us 	= Rom:ConstantTbl shr 2[8] and Rom:ConstantTbl shr 2[9]

; Maximum length of a wake-up pulse (x50us)
;WUPlsMaximum	equ 111 = 6Fh	;5ms	= Rom:ConstantTbl shr 2[A] and Rom:ConstantTbl shr 2[B]

                dc.w 0036Fh     ;   [9][8][B][A]; not used by fast protocol

;--------------------------------------------------------------------------------------------------
; MLX4 watchdog (depends on Fosc frequency)
; Wdg_timeout = (1/Fosc) * 2^(2+WatchPre_Val) * (16 + WatchPre_Cnt)
;
;WatchPre_Val	equ 7	= Rom:ConstantTbl shr 2[E] ; Since WatchPre_Val is used to enable the timer (msb to 1), set the msb -> 15
;WatchPre_Cnt	equ 15	= Rom:ConstantTbl shr 2[F]

;--------------------------------------------------------------------------------------------------
; Wake-up pulse generation (depends on baud-rate settings)
; -> Pulses width #WuPlsCnt# is 5Tbit (between 250us and 5ms)
;Pulses width : 5Tbit
;WuPlsCnt		equ 5	= Rom:ConstantTbl shr 2[C]

;--------------------------------------------------------------------------------------------------
; PLL config (if any)
;PLL: delay to check the PLL (see coPLLCHK switch)
; -- number of retries to start the PLL
;PLLattempt		equ 3	= Rom:ConstantTbl shr 2[D]

                dc.w 035FFh     ;   [D][C][F][E]

; -- counter to wait for the PLL to lock
;PLLcnt_lo		equ 0	= Rom:ConstantTbl2 shr 2[0]
;PLLcnt_hi		equ 14	= Rom:ConstantTbl2 shr 2[1]
; -- counter to do a second check after the PLL has started
;PLLchk_lo		equ 0	= Rom:ConstantTbl2 shr 2[2]
;PLLchk_hi		equ 14	= Rom:ConstantTbl2 shr 2[3]

ConstantTbl2	dc.w 0E0E0h    ;   [1][0][3][2]

	%assert {ConstantTbl2 and 3} eq 0	;Make sure the last 2 bits of the table are 0

;--------------------------------------------------------------------------------------------------
; Wake-up request generation (depends on Fosc)
; -> Pulses width #WuPlsCnt# is 5Tbit (between 250us and 5ms)
; -> Pause 150 ms between two pulses #WuPause#
; -> Pause 1.5 s before another pulse sequence #WeSeq#
;
; WuPause = (1/Fosc) * 2^(9+WuPausePre_Val) * (16 + WuPauseCnt_Val)
;
; WeSeq   = (1/Fosc) * 2^(9+WuSeqPre_Val)   * (16 + WuSeqPre_Val)
;

;Pause Length : 150ms (exactly 180ms, i.e. +20%) - used with the Sleep Timer
;WuPauseCnt_Val	equ 6	= Rom:ConstantTbl2 shr 2[4]
;WuPausePre_Val	equ 2	= Rom:ConstantTbl2 shr 2[5] ;Since WuPausePre_Val is used to enable the timer (msb to 1), set the msb -> 10

;Sequence Pause Length : 1.5s (exactly 1.77s, i.e. +18%) - used with the Sleep Timer
;WuSeqCnt_Val	equ 11	= Rom:ConstantTbl2 shr 2[6]
;WuSeqPre_Val	equ 5	= Rom:ConstantTbl2 shr 2[7] ;Since WuSeqPre_Val is used to enable the timer (msb to 1), set the msb -> 14

                dc.w 0A6DBh     ;   [5][4][7][6]; not used by fast protocol

                dc.w 0          ;   [9][8][B][A]

;not used                       => Rom:ConstantTbl2 shr 2[C]
;not used                       => Rom:ConstantTbl2 shr 2[D]
;MinPresc                       => Rom:ConstantTbl2 shr 2[E]
;not used                       => Rom:ConstantTbl2 shr 2[F]

#IF COMPILE_STANDALONE_LOADER eq 0
    #IF   CFG_PLL_FREQ eq coPLL_12MHZ
                dc.w 00001h     ;   [D][C][F][E]

    #ELIF CFG_PLL_FREQ eq coPLL_18MHZ
                dc.w 00001h     ;   [D][C][F][E]

    #ELIF CFG_PLL_FREQ eq coPLL_20MHZ
                dc.w 00002h     ;   [D][C][F][E]

    #ELIF CFG_PLL_FREQ eq coPLL_24MHZ
                dc.w 00002h     ;   [D][C][F][E]

    #ELIF CFG_PLL_FREQ eq coPLL_25MHZ
                dc.w 00002h     ;   [D][C][F][E]

    #ELIF CFG_PLL_FREQ eq coPLL_28MHZ
                dc.w 00002h     ;   [D][C][F][E]

    #ELIF CFG_PLL_FREQ eq coPLL_30MHZ
                dc.w 00002h     ;   [D][C][F][E]

    #ELIF CFG_PLL_FREQ eq coPLL_32MHZ
                dc.w 00002h     ;   [D][C][F][E]

    #ELSE
    ; by default use settings for coPLL_20MHZ
                %FATAL "CFG_PLL_FREQ is not define"
    #ENDIF
#ELSE
                ; not used for standalone (any value)
                dc.w 00002h ; [5][4][7][6]

#ENDIF


#IF coFASTFLASH eq cvON
;--------------------------------------------------
;CRC Tables for the Fast Protocol
;index is 1032
ffCrcTblHi		dc.w 0A07Dh
				dc.w 00AD7h
				dc.w 0E439h
				dc.w 04E93h
ffCrcTblLo		dc.w 0E0D3h
				dc.w 097A4h
				dc.w 01F2Ch
				dc.w 0685Bh

	%assert {ffCrcTblHi and 3} eq 0	;Make sure the last 2 bits of the table are 0
	%assert {ffCrcTblLo and 3} eq 0

	;Constants
;ffPrescA		equ 15	= Rom:FastCstTbl shr 2[0]
;ffPrescB		equ 1	= Rom:FastCstTbl shr 2[1]

;Baudrate counter
;
; Note: fast protocol baud rate (values defined by ffPrescA, ffBaudCntLo and ffBaudCntHi)
;       is set at run time now. Values ffPrescA, ffBaudCntLo and ffBaudCntHi are not used anymore
;
;When the Split signal is set, the baudrate counter is divided in 2 parts : Cpt[5:0] and Cpt[7:6] (see MSBi spec)
;At 125kbps the baudrate value is 120, which is 3x40. We put 40 in Cpt[5:0] and 2 in Cpt[7:6] (so from 0 to 2 there are 3 increments)
;Br = 10 10 1000
;Br = 0xA8
;For IntBr, the higher part of the counter is 1 more than for Br. IntBr = 11 101000 = 0xE8
;ffBaudCntLo	equ  8	= Rom:FastCstTbl shr 2[2]
;ffBaudCntHi	equ 10	= Rom:FastCstTbl shr 2[3]
;ffIntBaudCntHi	equ ffBaudCntHi + 4	calculated

FastCstTbl		dc.w 01FA8h		;[1][0][3][2]; 125kpbs @ 30MHz


	%assert {FastCstTbl and 3} eq 0	;Make sure the last 2 bits of the table are 0

;NAD : broadcast (0x7F)
;ffNADhi		equ 7	= Rom:FastCstTbl shr 2[5]
;ffNADlo		equ 15	= Rom:FastCstTbl shr 2[4]

;SID : B4 (Data Dump)
;ffSID_DDhi		equ 11	= Rom:FastCstTbl shr 2[7]
;ffSID_DDlo		equ 4	= Rom:FastCstTbl shr 2[6]

				dc.w 07FB4h	; [5][4][7][6]

;SID : B2 (Read by Identifier)
;ffSID_RBIhi	equ 11	= Rom:FastCstTbl shr 2[9]
;ffSID_RBIlo	equ 2	= Rom:FastCstTbl shr 2[8]

				dc.w 0B200h	; [9][8][B][A]
#ENDIF

;--- EOF
