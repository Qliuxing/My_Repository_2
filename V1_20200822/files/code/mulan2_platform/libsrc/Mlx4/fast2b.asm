Mlx4di

; ---------------------------------------------------------------------------
;
; Description:
;  Fast Protocol Handler for the Mlx4
;
;
; Copyright (c) Melexis Digital Competence Center
;
; ---------------------------------------------------------------------------

                intel                    ;make sure the default settings are Intel radix and words format

;LIN Software
LIN2verASM        equ 2                    ;revision code of this file
LIN2revASM        equ 7                    ; format : x xxx xxxx x xxx xxxx
LIN2dvtASM        equ 0                    ;                    \ \   \    \
                                           ;                     \ \   \    rev (0 to 15) -> for the LIN software : ver.rev [LIN2revASM]
                                           ;                      \ \   ver (0 to 7) [LIN2verASM]
                                           ;                       \ stable (1) / in dvt (0) [LIN2dvtASM]
                                           ;                        same thing for the flash loader [LIN2verFLSH, LIN2revFLSH, LIN2dvtFLSH]
;Flash Loader
LIN2verFLSH        equ 1                   ; ver 0 is in the regular LIN firmware, ver 1 is for the stand alone loader
LIN2revFLSH        equ 0                   ; LIN software stays at 2.7 because it was the last common base for fast2b.asm and lin2b.asm
LIN2dvtFLSH        equ 0

EditNbr            equ 0

;--------------- MelexCM constant definitions -----
                #include "periphery.asm"    ;dcom, dma and IO ports definitions for the Mlx4 periphery (for easier reading of the code ah ah!)

;--------------- LIN and API constant definitions -------------
            #IFDEF USE_RELATIVE_PATH
                #include "../../source/lincst.asm"
            #ELSE
                #include "lincst.asm"
            #ENDIF

;--------------- Compiler Switches ----------------
;enable or disable part of the code
                #include "switches.asm"    

;--------------------------------------------------

                words

;--------------- linker informations --------------
                segment word at 0-FFF 'all'     ;The total memory map consist of:
                contains 'reserved'             ;- reserved locations (at 0..3)
                contains 'task0vects'           ;- vectors of task 0 (at 4..7)
                contains 'task1vects'           ;- vectors of task 1 (at 8..0Bh)
                contains 'code'                 ;- normal code (no placement requirements)

;--------------- RAM mapping ----------------------  <- mapping to review!!
                bytes                    ;RAM addresses are 8 bits wide

                ;local variables of the LIN task:
                segment byte at 0-1F 'LIN private'

unused          ds.b 1    ;0x00 : unused
LINst           ds.b 1    ;0x01 : current state of the LIN task (4 bits)
LINbr           ds.b 2    ;0x02 : LIN Br register : 2 nibbles (8 bits) - even address so the lsb is 0
LINtmp          ds.b 2    ;0x04 : Temporary Byte for DMA accesses
LINbytbuf       ds.b 2    ;0x06 : Byte buffer
LINchksum       ds.b 2    ;0x08 : Message checksum temporary buffer
LINdbg          ds.b 2    ;0x0A : Debug (mark instructions)
LINmesslen      ds.b 2    ;0x0C : Message length
ClearByte       ds.b 2    ;0x0E : byte used to clear registers with a dma
LINindex        ds.b 2    ;0x10 : LIN index for ID Filtering Mode, or LIN ID for Frame Processing Mode

                        
Atmp            ds.b 1    ;0x12 : temporary buffer
Btmp            ds.b 1    ;0x13 : temporary buffer
Xtmp            ds.b 1    ;0x14 : temporary buffer
ffSTATUS        ds.b 1    ;0x15 : Fast Flash Loading Status
                          ;x x xx
                          ; \ \  \
                          ;  \ \  break detection step nr (3=found)
                          ;   \ 1 : protocol setting
                          ;    1 : response requested
LINbytcnt       ds.b 1    ;0x16 : counter used to know how many data bytes have been received
LINframeflag    ds.b 1    ;0x17 : LIN Frame Flags
                          ;x x x x
                          ; \ \ \ \
                          ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                          ;   \ \ sleep command received (ID 3C + first byte 00)
                          ;    \ discard the frame
                          ;     frame data ready for transmit
LINflashStatus  ds.b 1    ;0x18 :
                          ;x x x x
                          ; \ \ \ \
                          ;  \ \ \ programming mode
                          ;   \ \ continuous frames coming up
                          ;    \ not used
                          ;     not used

;0x20 ---------- locations used for inter-task communication -------
                segment byte at 20-2F 'LIN API'

LINresp         ds.b 4    ;0x20 : MLX4 -> MLX16 Command Response
LINcmnd         ds.b 4    ;0x24 : MLX16 -> MLX4 Command
LINmess         ds.b 6    ;0x28 : MLX4 -> MLX16 Events
LINID           ds.b 2    ;0x2E : MLX4 -> MLX16 LIN Protected Identifier

                segment byte at 30-3F 'LIN Frame'    ;for indexed access purposes, the base address has to end with 0000b
LINframe        ds.b 16   ;0x30 (0xE118) : LIN frame : up to 8 bytes (checksum calculated automaticaly)
                          ;0x34 (0xE11A)
                          ;0x38 (0xE11C)
                          ;0x3C (0xE11E)

;0x3F ---------- end of inter-task memory space ----------------------

;--------------- reserved memory locations and vector table

                words        ;ROM addresses are 16 bits wide

                segment word at 0 'reserved'    ;reserved memory locations in ROM
                dc.w 0A55Ah                     ;ROM address 0 should hold 0xA55A
                dc.w 5AA5h                      ;ROM address 1 should hold 0xA55A
                dc.w 0                          ;ROM address 2 is reserved
                dc.w 0                          ;ROM address 3 is reserved

                segment word at 4 'task0vects'  ;vector table of task 0
                jmp task0_por                   ;power-on reset vector of task 0
                jmp task0_rst                   ;task reset vector of task 0
                jmp task0_it                    ;external interrupt vector of task 0
                jmp task0_att                   ;attention interrupt vector of task 0

                segment word at 8 'task1vects'  ;vector table of task 1

;--------------- ROM tables -----------------------
                segment word at 0C 'code'       ;normal code (no calls into this code)

;--------------------------------------------------
;Mlx4 Software Version
;LIN Software
;LIN2verASM    equ 2        ;revision code of this file
;LIN2revASM    equ 7        ; format : x xxx xxxx x xxx xxxx
;LIN2dvtASM    equ 0        ;                    \ \   \    \
;            ;                     \ \   \    rev (0 to 15) -> for the LIN software : ver.rev [LIN2revASM]
;            ;                      \ \   ver (0 to 7) [LIN2verASM]
;            ;                       \ stable (1) / in dvt (0) [LIN2dvtASM]
;            ;                        same thing for the flash loader [LIN2verFLSH, LIN2revFLSH, LIN2dvtFLSH]
;Flash Loader            ;
;LIN2verFLSH    equ 1        ; ver 0 is in the regular LIN firmware, ver 1 is for the stand alone loader
;LIN2revFLSH    equ 0        ; LIN software stays at 2.7 because it was the last common base for fast2b.asm and lin2b.asm
;LIN2dvtFLSH    equ 0        ;
;            ;
;EditNbr    equ 0        ; 

    %assert LIN2revASM le 15
    %assert LIN2verASM le 7
    %assert LIN2dvtASM le 1
    %assert LIN2revFLSH le 15
    %assert LIN2verFLSH le 7
    %assert LIN2dvtFLSH le 1
SoftVersion    dc.w {{LIN2dvtFLSH shl 15} or {LIN2verFLSH shl 12} or {LIN2revFLSH shl 8} or {LIN2dvtASM shl 7} or {LIN2verASM shl 4} or LIN2revASM}
EditNumber    dc.w EditNbr
    dc.w 0000h ; alignment ..
    dc.w 0000h ; ..

;--------------------------------------------------
;Constant tables
                #include "cst_rom_tables.asm"    


;MACRO : access to analog periphery for MelexCM bug (Mlx16 stack overflow)
AnalogAccess    macro
    ;do an access to the analog
    dcom AnIo,11000011b    ;1 abc 0 hi j
            ;            \
            ;             RWB : read analog IO address 0hiabc
    mend

;--------------- parameters validity check (checked by the linker)
    %assert {LINbr and 1} eq 0          ;LINbr should be aligned at an even address
    %assert {LINtmp and 1} eq 0         ;LINtmp (temporary buffer) should be aligned at an even address
    %assert {LINbytbuf and 1} eq 0      ;LINbytbufl should be aligned at an even address
    %assert {LINchksum and 1} eq 0      ;LINchksum should be aligned at an even address

;-------------- FAST PROTOCOL -----------------------------------------------------------------------------------------
fastprotocol    ;FAST FLASH LOADING
            #IFDEF USE_RELATIVE_PATH
                #include "../../source/lin2b_fastflash.asm"
            #ELSE
                #include "lin2b_fastflash.asm"
            #ENDIF

;----------------------------------------------------------------------------------------------------------------------
;--------------- subroutines ------------------------------------------------------------------------------------------
;Get a data byte from the LINbytbuf
;X contains the updated byte counter, to be compared with LINmesslen
;LINchksum should contain the checksum or the CRC (for the fast protocol)
;Upon the procedure exit, the data byte is in A (high nibble) and B (low nibble) as well as in LINbytbuf
;C is set if all the data (and the checksum) has been sent
;Z is set if the current byte is the checksum
;The procedure can also be called from GetByteFast (with no processing done for LINbytcnt)
GetByte         mov X,LINbytcnt         ;get the byte counter
                add X,#1                ;increment it
                mov LINbytcnt,X         ;save the byte counter
                cmp X,LINmesslen        ;check if this is the last byte to send
                jc GB_DataByte
                jz GB_CheckSum
                set C                   ;set C to indicate that all the data has been sent
                jmp GB_Done
GB_CheckSum     ;get the content of LINchksum
                mov B,LINchksum
                mov A,LINchksum+1
                jmp GB_SaveDataDma      ;Z is set; C is not set
GB_DataByte     ;get a byte from the LINframe buffer
                asl X                   ;multiply the byte counter by 2 to have the pointer (we access bytes and not nibbles)
GetByteFast     mov B,data:LINframe shr 4[X]    ;get the low nibble
                add X,#1                        ;Z and C are reset
                mov A,data:LINframe shr 4[X]    ;get the high nibble
GB_SaveDataDma  ;save the data for the xdma
                mov LINdbg,B
                mov LINdbg+1,A
GB_SaveData     mov LINbytbuf,B         ;save the data in LINbytbuf
                mov LINbytbuf+1,A
GB_Done         rt 0,0                  ;return, the data is in A (high nibble) and B (low nibble)

;--------------------------------------------------
;Shift LINbytbuf to the right, the bit shifted out (or in) is in C
;at the end LINbytbuf+1 is in A, LINbytbuf in B
shift_bytbuf    mov A,LINbytbuf+1
                mov B,LINbytbuf
                rrc A
                rrc B
                jmp GB_SaveData         ;save the data in LINbytbuf

;----------------------------------------------------------------------------------------------------------------------
;Read the status registers of the MSBi cell
;Status register 0 is in A, status register 1 is in B
;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
ReadStatus      mov A,Stat0             ;read status register 0 (and clear the flags): {Traffic,LEvExt,Ovf,BrEv}
                mov B,Stat1             ;read status register 1 (and clear the flags): {Bin,SplBin,SyncErr,RunErr}
                rt 0,0

;----------------------------------------------------------------------------------------------------------------------
;Reset LINchksum and LINchksum+1
;X is changed (0)
ResetChkSum     dmar #CmpRW,ClearByte   ;use the dma to clear the byte
                dmaw LINchksum,#CmpRW
                rt 0,0

;--------------- errev : send an error to the application -------------------------------------------------------------
; parameters : error type in B
; result : same as setev
errev           ;free the data buffer
                mov A,#evERR            ;load the event code into A (the error type should already be in register B)

;--------------- setev : send an event to the application -------------------------------------------------------------
; The event will generate an interrupt on the Mlx16.
; parameters : event code in LINmessb, LINmessb+1, LINmessb+2, LINmessb+3
; result : C is set if an error occured
; A used
send_event      ;test if the event is free and signal a new one event
                jhshk send_event_ok     ;check if the previous event has been read
                jmp send_event_end

send_event_ok   ;copied the LINmess buffer into LINmess
                mov LINmess,A
                mov LINmess+1,B
                mov LINmess+2,X
                mov A,#0
                mov LINmess+3,A
                ;send the event
                set event               ;set a new event
send_event_end  ;event set or mutex freed, the acknowledge will be done in the interrupt routine N_M4_SHEM of the Mlx16
                rt 0,0

cmdrcvd         ;an event has been received, get it
                mov A,LINcmnd
                mov B,LINcmnd+1
                mov X,LINcmnd+2
                mov LINresp,A

            #IF coDEBUGMARK eq cvON
                mov LINdbg,A            ;save for xdma
                mov LINdbg+1,B          ;save for xdma    
                xdcom #dcCOMMAND        ;mark instruction
                xdma LINdbg             ;send the command sent by the application
            #ENDIF

                switch A
                jmp cmd_ack_nok         ;0 - pcNONE : No commands in the buffer
                jmp cmd_chst            ;1 - pcSTCH : General command for LIN state changes
                jmp cmd_cnfbr           ;2 - pcCNFBR : Set the target baudrate/prescaler value
                jmp cmd_ack_nok         ;3 - pcRX : Receive the current message (Frame Processing Mode)
                jmp cmd_setframe        ;4 - pcTX : Transmit the current message (Frame Processing Mode)
                jmp cmd_setframe        ;5 - pcSETFRAME : Discard the current message (Frame Processing Mode)
                jmp cmd_ack_nok         ;6 - pcCNFID : Configure a LIN ID (Identifier Filtering Mode)
                jmp cmd_cnfsr           ;7 - pcCNFSR
                jmp cmd_ack_nok         ;8 - pcREINIT : ml_ReInit command - note : pcREINIT and pcRSTART codes SHOULD NOT change (8 and 9)
                jmp cmd_ack_nok         ;9 - pcRSTART : ml_Restart command - ;note : pcREINIT and pcRSTART codes SHOULD NOT change (8 and 9)
                jmp cmd_ack_nok         ;A - pcSFTVER : Software version request
                jmp cmd_ack_nok         ;B - pcOPTION : Set the option registers
                jmp cmd_ack_nok         ;C - pcGETST : Get the state of the Mlx4
                jmp cmd_ack_nok         ;D - pcCONFIG : Program a register or an IO port with value
                jmp cmd_ack_nok         ;E - pcFCNFIG : Configure a Group (Identifier Filtring Mode)
                jmp cmd_rel_buf         ;F - pcRELBUF : Release frame buffer

cmd_cnfbr       ;pcCNFBR: set the baudrate
                ;B = caPresc (prescaler value) : PrescA
                ;{LINcmnd+3, X} = caBaud : {LINbr+1, LINbr}
                mov PrescA,B
                mov LINbr,X
                mov X,LINcmnd+3
                mov LINbr+1,X
                jmp cmd_ack_ok

cmd_cnfsr       ;pcCNFSR : Program a register or an IO port with value
                asl B                   ;A=xSSx
                and B,#6                ;0SS0
                switch B
                dcom Flags,04h          ;0000 : Phymd = 00
                jmp sr_done             ;0001
                dcom Flags,05h          ;0010 : Phymd = 01
                jmp sr_done             ;0011
                dcom Flags,06h          ;0100 : Phymd = 10
                jmp sr_done             ;0101
                dcom Flags,07h          ;0110 : Phymd = 11
sr_done         jmp cmd_ack_ok          ;0111

cmd_chst        ;pcSTCH: General command for LIN state changes - Connect and Disconnect only
                cmp B,#stFAST
                jz  cmd_chst_fenter
                cmp B,#stACT
                jnz cmd_chst_disc
cmd_chst_fenter ;enter fast mode
            #IF coDEBUGMARK eq cvON
                xdcom #dcFAST           ;mark instruction
            #ENDIF
                mov A,#ackOK
                mov LINresp+1,A
                ack event
                jmp fastprotocol
cmd_chst_disc   ;check if it is a state change to stDISC
                cmpcz B,#stDISC
                jnz cmd_chst_discno
                ;DISCONNECT: go into idle state
                jmp idle
cmd_chst_discno jmp cmd_ack_ok

cmd_setframe    ;--- Set Frame Command -------------------------------------------------------
                cmp B,#1
                mov B,LINcmnd+2         ;Move LINcmnd+2 to B for further handling in TXdataready
                jz  cmd_setfr_dtrdy     ;B=1 : data ready
                jnc cmd_setfr_cont      ;B>1 : set/reset continuous frame flag
                jmp cmd_discard        ;discard the frame
cmd_setfr_cont  ;set/reset continuous frame flag
                cmp B,#0
                mov A,LINflashStatus    ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ programming mode
                                        ;   \ \ continuous frames coming up
                                        ;    \ not used
                                        ;     QR code disabled (1) or enabled (0)
                jz cmd_setfr_rcont
                ;set continuous frame flag
                or A,#2
                mov LINflashStatus,A
                jmp cmd_ack_ok
cmd_setfr_rcont ;reset continuous frame flag
                and A,#13
                mov LINflashStatus,A
                jmp cmd_ack_ok
cmd_setfr_dtrdy ;Setting of data ready bit is needed
                mov A,LINframeflag      ;x x x x
                or  A,#8                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                mov LINframeflag,A
                jmp cmd_ack_ok
cmd_discard     ;discard the frame
                mov A,LINframeflag      ;x x x x
                or  A,#4                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                mov LINframeflag,A
                jmp cmd_ack_ok              
cmd_rel_buf     ;Clear buffer occupied flag
                msk B,#2
                jnz cmd_rel_buf_end
                mov A,LINflashStatus    ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ programming mode
                                        ;   \ \ continuous frames coming up
                                        ;    \ not used
                                        ;     QR code disabled (1) or enabled (0)
                and A,#14
                msk B,#1
                jz  cmd_rel_buf_wr
                or  A,#1
cmd_rel_buf_wr  mov LINflashStatus,A
cmd_rel_buf_end jmp cmd_ack_ok

cmd_ack_ok      ;Send acknowledge event to MLX16
                mov A,#ackOK
                jmp cmd_ack

cmd_ack_nok     ;Send acknowledge event to MLX16
                mov A,#ackERR
cmd_ack         ;release the command buffer to be ready for the next command
                mov LINresp+1,A
                ack event
                rt 0,0

;---------------- check PLL -------------------------------------------------------------------------------------------
;Read PLLCH (put it in the Mlx4 carry) to see if the PLL changed. If it did, signal an error
;If it didn't, no register is modified
pll_check        ;clear PLLCH
                dcom Int,02h            ;xxxx xx1x
                                        ;        \
                                        ;         clear PllCh
                ;read PLLOK (put it in the Mlx4 carry)
                dcom Int,80h            ;10xx xxxx
                                        ;  \
                                        ;   load Mlx4 carry with PllOk
                jnc pll_fastck            ;if the carry is not set, the PLL is not ready - wait and check again
                jmp pll_ok                ;the PLL is already ok (it was probably set by the Mlx16)
pll_fastck      ;disable the slow clock (ask for fast clock - request the PLL)
                dcom System,88h            ;1xxx 10xx
                                        ;       \
                                        ;        SlowCk = 0 (FstCkRq = 1 : request PLL)
                mov X,#0
pll_counter     ;start the counter: put the initial value in A and B
                mov A,#0
                mov A,Rom:ConstantTbl2 shr 2[A]    ;PLLcnt_lo
                mov B,#1
                mov B,Rom:ConstantTbl2 shr 2[B]    ;PLLcnt_hi
pll_loop        add A,#1                ;increment the counter
                addcz B,#0
                jc pll_cntdone          ;the counter will exit at 0xFF
                jmp pll_loop

pll_cntdone     ;read PLLCH (put it in the Mlx4 carry)
                dcom Int,0C0h           ;11xx xxxx
                                        ;  \
                                        ;   load Mlx4 carry with PllCh
                jc pll_clearch
                mov B,#13               ;index for PLLattempt
                add X,#1
                cmp X,Rom:ConstantTbl shr 2[B]    ;check how many times the counter overflowed (PLLattempt)
                jz jmp_pll_error        ;if more than 3, signal an error
                jmp pll_counter         ;else run the counter again
jmp_pll_error   ;PLL error: the PLL is still not running
                jmp pll_error
pll_clearch     ;clear PLLCH
                dcom Int,02h            ;xxxx xx1x
                                        ;        \
                                        ;         clear PllCh
pll_ok          ;the PLL has started - run a delay counter and check again: put the initial value in A and B
                mov A,#2
                mov A,Rom:ConstantTbl2 shr 2[A]     ;PLLchk_lo
                mov B,#3
                mov B,Rom:ConstantTbl2 shr 2[B]     ;PLLchk_hi
pll_okloop      add A,#1                ;increment the counter
                addcz B,#0
                jc pll_okcheck          ;the counter will exit at 0xFF
                jmp pll_okloop
pll_okcheck     ;check if the PLL changed (put it in the Mlx4 carry)
                dcom Int,0C0h           ;11xx xxxx
                                        ;  \
                                        ;   load Mlx4 carry with PllCh
                jnc pll_started
                jmp pll_check
pll_started     ;the PLL is running
                rt 0,0                  ;if the PLL did not change, return

pll_error        ;PLL error: the PLLOK has changed !
                mov X,#erCRASHPLL
                mov B,#erCRASH
                call errev              ;send an error to the application
                jmp wtinit              ;go wait for a signal

;----------------------------------------------------------------------------------------------------------------------
;--------------- interrupt and attention vectors ----------------------------------------------------------------------
;interrupts are not used in the LIN task, if this code is executed that means there was a problem
;in that case, send a {evERR,erCRASH} error to the application and reset the task

task0_it        ;interrupts should be disabled (except when terminating the reception of a frame ?)

;--------------- handler for critical errors ------
task0_att       mov B,#erCRASH
                call errev              ;LIN_errev puts #evERR in A and send it to the application
                ;test C to see if the event for correctly handled

;--------------- soft reset -----------------------
task0_rst

;--------------- power on reset -------------------
task0_por
            #IF coDEBUGMARK eq cvON
                xdcom #dcRST            ;mark instruction
            #ENDIF

                ctrl 18h                ;disable interrupts
                task 0C0h               ;disable atm

                ;clear the registers used to clear the bytes with dma
                mov A,#0
                mov ClearByte,A
                mov ClearByte+1,A
                
                wait signal             ;handshake with the Mlx16
                                        ;when the Mlx16 respond (with an event) proceed to DISCONNECT (enter the state machine)

                exit Mx                 ;- disable the Attention Interrupt
                                        ;- (don't set the event flag of the other task)
                                        ;- reset the event flag of the current task (handshake), in case it was set
                                        ;- (don't generate a Signal condition)
                                        ;- if permitted, reset the Mutex bit

                ; --- MLX4 PERIPHERY -----------
                ; Digital Bus Flags - SLEEPB
                dcom Flags,60h          ;011 x xxxx
                                        ;   \
                                        ;    sleepb = 1
    
;--------------- LIN task initialisation ----------

                mov A,#0
                mov LINflashStatus,A    ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ programming mode
                                        ;   \ \ continuous frames coming up
                                        ;    \ not used
                                        ;     not used

                ;Register initialization
                mov LINmesslen,A

                ; Timers
                ; - Watchdog timer
                ; - Sleep timer
                ; - Message timer (not used)

                ;initialize the watchdog counter (Mlx4 Periphery)
                ;the watchdog monitors the LIN bus. If a dominant level is present on the bus for mode than WatchPre_Cnt/WatchPre_Val
                ;a Mlx4 reset is generated
                mov A,#15
                mov A,Rom:ConstantTbl shr 2[A]    ;WatchPre_Cnt: watchdog timer value
                mov B,#WatchCntIdx      ;watchdog timer value index
                mov WatchCnt[B],A
                mov A,#14
                mov A,Rom:ConstantTbl shr 2[A]    ;#WatchPre_Val or 8    
                                        ;1 xxx
                                        ; \   \
                                        ;  \   prescaler (WatchPre_Val)
                                        ;   timer enable
                mov B,#WatchPreIdx      ;sleep timer prescaler index
                mov WatchCnt[B],A

                mov A,#stINIT
                mov LINst,A             ;enter INIT state

;--------------- wait for init --------------------
; Initialization done, wait for application to be ready (Handshake)
wtinit          ;wait signal            ;handshake with the Mlx16
                ;when the Mlx16 respond (with an event) proceed to DISCONNECT (enter the state machine)
restart         mov A,#0
                mov LINdbg,A            ;DEBUG
                mov LINindex,A          ;reset the LIN index register
                mov LINframeflag,A
    
                mov A,#8
                mov LINindex+1,A        ;reset the LIN index register (ID not recognized)

                call pll_check          ;request the PLL

dscnct          mov B,#stDISC           ;set the new state: disconnected
                mov LINst,B             ;signal that the state changed

;--------------- wait for connect --------------
wtcnct          waitpd Event            ;wait for a command (signaled by an event)/allow power down
                call cmdrcvd            ;process the command - commands are handled in cmdrcvd
                jmp wtcnct
    
;--------------- idle -------------------------------------------------------------------------------------------------
idle            ;acknowledge the command (correctly executed)
                mov A,#ackOK
                mov LINresp+1,A         ;return argument
                ack event               ;acknowledge the command

                ;reset the MSBi and loop to waitpd
                dcom InpCtrl,0C0h       ;InpCtrl
                                        ;1 1 00 0 0 0 0
                                        ;   \ 
                                        ;    Rst=1
                dcom BrCtrl,0C0h        ;11 00 0000
                                        ;  \ 
                                        ;   Init: reset
                dcom StatCtrl,040h      ;0 1 00 0000
                                        ;   \  
                                        ;    reset internal flags:
idle_loop       waitpd 0
                nop
                jmp idle_loop    

prog_end        end
    

