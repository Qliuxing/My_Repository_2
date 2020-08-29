; ---------------------------------------------------------------------------
;
; Description:
;  Fast Flash Loading Protocol
;
;
; Copyright (c) Melexis Digital Competence Center
;
; ---------------------------------------------------------------------------

;RAM
;LINframeflag : x x x x
;                \ \ \ \
;                 \ \ \ not used
;                  \ \ not used
;                   \ discard the frame
;                    frame data ready for transmit
;ffSTATUS : x x xx (Fast Flash Loading Status)
;            \ \  \
;             \ \  break detection step nr (3=found)
;              \ 1 : protocol setting
;               1 : response requested
;LINflashStatus : x x x x
;                  \ \ \ \
;                   \ \ \ programming mode
;                    \ \ continuous frames coming up
;                     \ not used
;                      not used
;------------------------------------------------------------------------------------

                ;------------------------------------------------------------------------------------
                ;the PLL should be started

ff_init            ;set the MSBi cell
                ; Configuration Registers : Cfg0 (0x3A) and Cfg1 (0x3B)
                mov A,#7
                mov Cfg0,A                ;Cfg0 : 0111
                                        ; NoCk = 0
                                        ; Split = 1
                                        ; RptSync = 1
                                        ; Piped = 1
                mov A,#14
                mov Cfg1,A                ;Cfg1 : 1110
                                        ; Mode 11 (PWM)
                                        ; FCan/Clkd/MArb = 1
                                        ; CanL/Diffm = 0

                ;stop the counter to stop outputting data
                dcom PlsCtrl,90h        ;1 0 01 0000
                                        ;   \  \    \
                                        ;    \  \    nop
                                        ;     \  clear Run
                                        ;      no capture

                ; Input Bloc : InpCtrl (dcom 4, msb 1), DbCpt (0x3C)
                dcom InpCtrl,0E9h        ;InpCtrl
                                        ;1 1 10 1 0 0 1
                                        ;   \ \    \ \ \
                                        ;    \ \    \ \ EnFdb=1: enable fast debounce (BusIn is debounced on a 2/3 majority vote scheme)
                                        ;     \ \    \ Fbin=0: flag reset (Bin = slow debounced BusIn))
                                        ;      \ \    AutoDb=0: flag set (DbCk devided by the 4 msb's of the Br register)
                                        ;       \ RbErr=10: flag reset (IntSplBin selected)
                                        ;        Rst=1

                ; Prescaler and Baudrate :
                ; SendBaudRate should have been called in the Mlx16 prior to switching to the fast loader mode
                ; - PrescA is already programmed
                ; - LINbr and LINbr+1 contain the baudrate values (ffBaudCntHi and ffBaudCntLo), the dmar should be called to
                ;   program the MSBi cell

                ; Prescaler Bloc : PrescA (0x3E) and PrescB (0x3F)
                mov A,#1        
                mov A,Rom:FastCstTbl shr 2[A]
                mov PrescB,A            ;PrescB

                ; Baudrate generator
                dmar #Br,LINbr
                mov B,LINbr+1
                mov A,LINbr+1            ;save the initial value
                add B,#4                ;ffIntBaudCntHi
                mov LINbr+1,B            ;LINbr doesn't change for IntBr
                dmar #IntBr,LINbr+1
                mov LINbr+1,A            ;restore the correct value of LINbr
                dcom BrCtrl,0Eh            ;00 00 1110
                                        ;      Edg[2:0]=010: Fast Falling Edge
                ;generates an ExEvt when a BrMatch occurs
                dcom CkCtrl,07h            ;00 00 0111
                                        ;  \  \    \
                                        ;   \  \    BrPls[1:0]=11: InBrCk-Halfmatch; BrCk-IntMatch; AuxCk-AuxMatch
                                        ;    \  RptCap : nop
                                        ;     Capt : nop
                ;Sync window
                dcom WindCtrl2,0Ah        ;10 00 00 1010
                                        ;  \  \  \    \
                                        ;   \  \  \    immediate load of SyncWindow with 1 - EnStrtEdg = 0
                                        ;    \  \  SiCk: nop
                                        ;     \  FlStRj: nop
                                        ;      StpCpt = 0
                ;StatCtrl - TimeOut source
                dcom StatCtrl,0Ah        ;StatCtrl
                                        ;0 0 00 1010
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=010 : LTimeOut driven only by IntBr (halfmatch)
                                        ;     \  reset Ftfr : nop
                                        ;      reset internal flags: nop

                ;initialize the periphery
                dcom Flags,0ACh            ;Hspeed = 1 (set the Bypass signal)
                ;dcom Flags,{04h or ML_SLEWFAST}    ;Phymod = ML_SLEWFAST - done on the Mlx16 or in fast2b.asm

                ;disable the timers from the periphery
                dcom Timer,16h            ;00 01 01 10
                                        ;     \  \  \
                                        ;      \  \  Sleep Timer TimeOut setting
                                        ;       \  \   0x : no effect
                                        ;        \  \  10 : disable the timeout
                                        ;         \  \ 11 : enable the timeout
                                        ;          \  Message Timer :stop the message timer
                                        ;           Sleep Timer Run Command
                                        ;            00 : no effect
                                        ;            01 : stop the sleep timer
                                        ;            10 : restart the sleep timer
                                        ;            11 : reload and restart the sleep timer - clears SleepStat

                dcom StuffCtrl,30h        ;StuffCtrl: set OutMd, ignore the stuffing part
                                        ;0 0 11 0000
                                        ;     \
                                        ;      OutMd loaded with '1'

ff_reinit        call ff_msbi_rx            ;change the registers that have been changed during TX

ff_rxinit        ;initialize the registers
                mov B,#0                ;reset the following registers
                mov ffSTATUS,B

                ;call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}

ff_newstart     ;B=0 when jumping here
                dmar #CmpRW,ClearByte    ;use the dma to clear the byte
                dmaw LINchksum,#CmpRW
                mov LINbytcnt,B
                mov LINframeflag,B
                mov X,#2
                mov LINmesslen,X        ;2 bytes : PCI + Command
                ;------------------------------------------------------------------------------------
ff_rxloopinit   mov X,#8                ;X is used as a bit counter
                ;do not start the counter until there is a falling edge
                ;here, we must issue 2 stop instructions since if a Br match appens at the same time, the counter does not stop and it results to a fail of the communication.
                dcom PlsCtrl,90h        ;1 0 01 0000
                                        ;   \  \    \
                                        ;    \  \    nop
                                        ;     \  clear run flag, stop the counter
                                        ;      no capture of Br into IntBr
                
                nop                     ;make sur the counter is stopped
                nop                     ;add 2 nops to be sure that the two stop instructions are executed at a different cnt value even if presca is 1.
                
                ;make sur the counter is stopped
                dcom PlsCtrl,90h        ;1 0 01 0000
                                        ;   \  \    \
                                        ;    \  \    nop
                                        ;     \  clear run flag, stop the counter
                                        ;      no capture of Br into IntBr
                ;set the Sync window
                dcom WindCtrl2,0Ah      ;10 00 00 1010
                                        ;  \  \  \    \
                                        ;   \  \  \    immediate load of SyncWindow with 1 - EnStrtEdg = 0
                                        ;    \  \  SiCk: nop
                                        ;     \  FlStRj: nop
                                        ;      StpCpt = 10: reset StpCpt

ff_rxloop        ;RX : this is the loop to receive the data bits
                ;X is used as a bit counter
                ;LINbytcnt is used to count the bytes received
                wait TimeOut,Event
                jtime ff_rxto          ;Half bit match is generated
                jmp ff_cmd             ;command received
ff_rxto            ;check the the bus (and reset the flags / acknowledge the event!)
                call ReadStatus            ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                mov Atmp,B
                ; Auxiliary Input bloc
                mov B,AuxIn                ;PWM=1 and NoCk=0
                                        ; x x x x
                                        ;  \ \ \ \
                                        ;   \ \ \ set at falling edge
                                        ;    \ \ on Brmatch, set with Bin
                                        ;     \ set at rising edge
                                        ;      SplBin: level on the bus
                mov A,ffSTATUS            ; x x xx : Fast Flash Loading Status
                                        ;  \ \  \
                                        ;   \ \  break detection step nr (3=found)
                                        ;    \ 1 : protocol setting
                                        ;     1 : response requested
                and A,#3
                cmp A,#3                ;test if a start of frame has already been received
                jnz ff_rxto_brk
                jmp ff_testbit

ff_rxto_brk     ;Let's search for a valid break, pattern 0 0 1 at br match
                ;Header Step 0: 0 --> B=xx0x
                ;Header Step 1: 0 --> B=0000     1 --> B=11x0 --> DONE
                ;Header Step 2: 1 --> B=11xx --> DONE
                switch A
                jmp ff_rxto_brk_st0
                jmp ff_rxto_brk_st1
                jmp ff_rxto_brk_st2
                jmp ff_rxloopinit

ff_rxto_brk_st1 cmp B,#0
                jnz ff_rxto_brk_st1b
                mov A,#2
                jmp ff_rxto_brk_end
ff_rxto_brk_st1b msk B,#8
                jnz ff_rxto_brk_err
                mov A,#1
                jmp ff_rxto_brk_end

ff_rxto_brk_st0 ;and B,#2
                ;cmp B,#0
                mov B,Atmp
                msk B,#8
                jnz ff_rxto_brk_err
                mov A,#1
                jmp ff_rxto_brk_end

ff_rxto_brk_st2 and B,#12
                cmp B,#12
                jnz ff_rxto_brk_err
                jmp ff_rxto_brk_fin

ff_rxto_brk_err mov A,#0
                mov ffSTATUS,A
                jmp ff_rxloopinit

ff_rxto_brk_end mov ffSTATUS,A
                jmp ff_rxloop

ff_rxto_brk_fin mov A,#3
                mov ffSTATUS,A
                ;do an access to the analog
                AnalogAccess
                jmp ff_rxloopinit

ff_cmd          ;check the command
                mov A,LINcmnd
                mov B,LINcmnd+1
                mov LINresp,A
            #IF coDEBUGMARK eq cvON
                mov LINdbg,A            ;save for xdma
                mov LINdbg+1,B          ;save for xdma
                xdcom #dcCOMMAND        ;mark instruction
                xdma LINdbg             ;send the command sent by the application
            #ENDIF

                switch A
                jmp ff_cmd_ack_ok       ;0 - pcNONE : No commands in the buffer
                jmp ff_cmd_stch         ;1 - pcSTCH : General command for LIN state changes
                jmp ff_cmd_ack_nok      ;2 - Command not supported
                jmp ff_cmd_ack_nok      ;3 - Command not supported
                jmp ff_cmd_ack_nok      ;4 - pcTX : Transmit the current message (Frame Processing Mode)
                jmp ff_cmd_setframe     ;5 - pcSETFRAME : Discard the current message (Frame Processing Mode) / Set DATAREADY bit / modify continous frame flag
                jmp ff_cmd_ack_nok      ;6 - Command not supported
                jmp ff_cmd_ack_nok      ;7 - Command not supported
                jmp ff_cmd_ack_nok      ;8 - Command not supported
                jmp ff_cmd_ack_nok      ;9 - Command not supported
                jmp ff_cmd_ack_nok      ;A - Command not supported
                jmp ff_cmd_ack_nok      ;B - Command not supported
                jmp ff_cmd_ack_nok      ;C - Command not supported
                jmp ff_cmd_ack_nok      ;D - Command not supported
                jmp ff_cmd_ack_nok      ;E - Command not supported
                jmp ff_cmd_rel_buf      ;F - pcRELBUF : Release frame buffer

ff_cmd_rel_buf  ;Release buffer command received ==> to set/reset programming mode bit
                msk B,#2
                jnz ff_cmd_rel_buf_end
                mov A,LINflashStatus    ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ programming mode
                                        ;   \ \ continuous frames coming up
                                        ;    \ not used
                                        ;     QR code disabled (1) or enabled (0)
                and A,#14
                msk B,#1
                jz  ff_cmd_rel_buf_wr
                or  A,#1
ff_cmd_rel_buf_wr
                mov LINflashStatus,A
ff_cmd_rel_buf_end
                jmp ff_cmd_ack_ok

ff_cmd_setframe ;--- Set Frame Command -------------------------------------------------------
                ;B=0 : discard frame
                ;B=1 : data ready
                ;B=2 : cont frame
                cmp B,#2
                jnz ff_cmd_stch_err
                mov B,LINcmnd+2
                ;set/reset continuous frame flag
                mov A,LINflashStatus    ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ programming mode
                                        ;   \ \ continuous frames coming up
                                        ;    \ not used
                                        ;     QR code disabled (1) or enabled (0)
                or A,#2
                cmp B,#1
                jz ff_cmd_setfr_end
                ;clear continuous frame flag
                and A,#13
                ;reset the status flags as we came out of cont mode which does not do this
                mov B,#0
                mov ffSTATUS,B
ff_cmd_setfr_end
                mov LINflashStatus,A
                jmp ff_cmd_ack_ok

ff_cmd_stch     cmp B,#stDISC           ;check if this is an exit command
                jnz ff_cmd_stch_err     ;invalid command
                jmp restart             ;stDISC - reinit for lin2b.asm
ff_cmd_stch_err jmp ff_cmd_ack_nok

ff_cmd_ack_ok   ;Send acknowledge event to MLX16
                mov A,#ackOK
                jmp ff_cmd_respond      ;send event

ff_cmd_ack_nok  ;Send acknowledge event to MLX16
                mov A,#ackERR
ff_cmd_respond  ;release the command buffer to be ready for the next command
                mov LINresp+1,A
                ack event
                jmp ff_rxloop

ff_testbit        ;start bit already received : test if this is a valid bit
                msk B,#1                ;B = AuxIn
                jnz ff_getbit
                ;no falling edge detected : invalid bit
                mov B,Atmp
                msk B,#8                ;test the bus level
                jnz ff_jmprxloop        ;keep trying to receive a bit
                jmp ff_rxinit           ;re-init the reception, probably we are receiving a new frame
ff_getbit        ;valid bit, store it
                rlc B                   ;get SplBin in the carry
                ;store the bit received in the byte buffer
                call shift_bytbuf       ;LINbytbuf+1 is in A, LINbytbuf in B
                ;update the bit counter (register X)
                sub X,#1
                jz ff_bytrcvd           ;check if the byte has been received completely
ff_jmprxloop    jmp ff_rxloop
ff_bytrcvd      ;a byte has been received (it is in LINbytbuf (B) / LINbytbuf+1 (A)), send a mark and update the byte counter
            #IF coDEBUGMARK eq cvON
                ;xdcom #dcFASTBYTE      ;mark instruction
                ;xdma LINbytbuf         ;send the byte received
            #ENDIF
                mov X,LINbytcnt         ;update the byte counter
                cmp X,LINmesslen        ;test if the message has been completely received (in that case the data in LINbytbuf is the CRC)
                jnz ff_storedata
                jmp ff_messrcvd         ;do not store the CRC
ff_storedata    ;store the byte received
                asl X                   ;multiply the counter by 2
                mov data:LINframe shr 4[X],B    ;store the low nibble
                add X,#1
                mov data:LINframe shr 4[X],A    ;store the high nibble
                rrc X                   ;restore the counter
                add X,#1                ;increment it
                mov LINbytcnt,X
                ;update the CRC
                call ff_crc_calc        ;do CRC calculation, the high nibble just received is already in A, low nibble in B

                cmp X,#1                ;test if this is the first byte (PCI)
                jnz ff_jmp_rxinit
                ;the PCI has been received
            #IF coDEBUGMARK eq cvON
                ;send a mark and process it
                xdcom #dcFASTPCI        ;mark instruction
                xdma LINbytbuf          ;send the byte received
            #ENDIF
                ;restore A and X : the high nibble is in A, the low nibble in X
                mov B,LINbytbuf         ;no need to restore the byte received, as if this is the PCI, LINchksum was 0
                mov A,LINbytbuf+1
                mov LINtmp,A            ;save the high part of the PCI (use LINtmp)
                msk A,#3                ;test if this is a Single Frame
                jnz ff_pci_nosf

                ;memorize if this is a protocol setting and if a response if requested
                and A,#12               ;discard the two LSBs
                or A,ffSTATUS           ; x x xx : Fast Flash Loading Status
                                        ;  \ \  \
                                        ;   \ \  break detection step nr (3=found)
                                        ;    \ 1 : protocol setting
                                        ;     1 : response requested
                mov ffSTATUS,A
ff_savelength   mov LINmesslen,B        ;save the length (for SF and last frame), length/256 for FF, or frame counter for CF
ff_jmp_rxinit   jmp ff_rxloopinit       ;go get the next byte

ff_pci_nosf     mov Btmp,B              ;save the low part of the PCI for later
                and A,#3
                cmp A,#1                ;test if this is a First Frame
                jz ff_pci_ff
                mov B,#7                ;Continuous Frame or Last Frame
                jmp ff_savelength
ff_pci_ff       mov B,#6
                jmp ff_savelength
                ;------------------------------------------------------------------------------------
ff_messrcvd        ;message received
                ;stop the counter (070830)
                dcom PlsCtrl,90h        ;1 0 01 0000
                                        ;   \  \    \
                                        ;    \  \    nop
                                        ;     \  stop the counter
                                        ;      no capture
                ;re-open the sync window for slow speed (50kbps) - bem 080221
                dcom WindCtrl0,0Ah      ;00 00 00 1010
                                        ;  \  \  \    \
                                        ;   \  \  \    immediate load of SyncWindow with 1 - EnStrtEdg = 0
                                        ;    \  \  SiCk: nop
                                        ;     \  FlStRj: nop
                                        ;      StpCpt: nop

                ;check the CRC, CRC is in LINbytbuf (B) / LINbytbuf+1 (A)
                call ff_crc_calc        ;do CRC calculation, the high nibble just received is already in A, low nibble in B
                cmpcz A,#0              ;the result should be zero if the CRC is correct (Z already contains the result of B)
                jz ff_cksumok
                ;CRC error
                mov B,#erCKSUM          ;CRC error
                call errev              ;send the event
                jmp ff_init
ff_cksumok        ;CRC valid, prepare the data to be sent to the application
                mov B,LINtmp            ;the high part of the PCI is in LINtmp
                and B,#3                ;test the two LSBs only, discard the two MSBs
                mov LINtmp,B
                switch B
                jmp ff_single           ;xx00 : single frame
                jmp ff_first            ;xx01 : first frame
                jmp ff_cont             ;xx10 : continuous frame
                jmp ff_last             ;xx11 : last frame (not in the LIN protocol)
ff_first        ;First Frame
                ;--------------------
                ;byte 0 (index 0-1):   Special PCI -> NAD
                ;byte 1 (index 2-3):   Length     -> PCI
                ;byte 2 (index 4-5):   Command     -> LEN
                ;byte 3 (index 6-7):   Data 1      -> SID
                ;byte 4 (index 8-9):   Data 2      -> Command (Data 1)
                ;byte 5 (index 10-11): Data 3      -> Data 1 (Data 2)
                ;byte 6 (index 12-13):             -> Data 2 (Data 3)
                ;byte 7 (index 14-15):             -> Data 3 (Data 4)
                ;--------------------
                ;shift the data
                mov A,#11               ;index for data 3
                mov X,#4                ;offset : 4 nibbles
                mov B,#3                ;last nibble is nibble 3
                call ff_buffloop
                ;store the SID
                mov A,#7                ;index of the SID
                call ff_sid             ;when returning, A=5
                ;store the length
                mov A,#3                ;index for length
                mov X,#2                ;offset : 2 nibbles
                mov B,#1                ;last nibble is nibble 1
                call ff_buffloop
                ;store the NAD and the PCI
                mov A,#3
                jmp ff_nadpci
ff_last         ;Last Frame - same as Continuous Frame but with less data
                ;--------------------
                ;byte 0 (index 0-1):   Special PCI -> NAD
                ;byte 1 (index 2-3):   Data 1      -> PCI
                ;byte 2 (index 4-5):   Data 2      -> Data 1
                ;byte 3 (index 6-7):   Data 3      -> Data 2
                ;byte 4 (index 8-9):   Data 4      -> Data 3
                ;byte 5 (index 10-11): Data 5      -> Data 4
                ;byte 6 (index 12-13): Data 6      -> Data 5
                ;byte 7 (index 14-15):             -> Data 6
                ;--------------------
                mov B,#2
                mov LINtmp,B            ;save the PCI as a Continuous Frame PCI (LIN compatible)
ff_cont         ;Continuous Frame
                ;--------------------
                ;byte 0 (index 0-1):   Special PCI -> NAD
                ;byte 1 (index 2-3):   Data 1      -> PCI
                ;byte 2 (index 4-5):   Data 2      -> Data 1
                ;byte 3 (index 6-7):   Data 3      -> Data 2
                ;byte 4 (index 8-9):   Data 4      -> Data 3
                ;byte 5 (index 10-11): Data 5      -> Data 4
                ;byte 6 (index 12-13): Data 6      -> Data 5
                ;byte 7 (index 14-15):             -> Data 6
                ;--------------------
                ;shift the data
                mov A,#13               ;index for data 6
                mov X,#2                ;offset : 2 nibbles
                mov B,#1                ;last nibble is nibble 1
                call ff_buffloop
                mov A,#3
                jmp ff_nadpci
ff_single        ;Single Frame
                ;--------------------
                ;byte 0 (index 0-1):   Special PCI -> NAD
                ;byte 1 (index 2-3):   Command     -> PCI
                ;byte 2 (index 4-5):   Data 1      -> SID
                ;byte 3 (index 6-7):   Data 2      -> Command (Data 1)
                ;byte 4 (index 8-9):   Data 3      -> Data 1 (Data 2)
                ;byte 5 (index 10-11): Data 4      -> Data 2 (Data 3)
                ;byte 6 (index 12-13):             -> Data 3 (Data 4)
                ;byte 7 (index 14-15):             -> Data 4 (Data 5)
                ;--------------------
                ;prepare the low part of the PCI (length)
                mov B,LINmesslen
                mov Btmp,B                ;save it for later

                ;shift the data
                mov A,#11               ;index for data 4
                mov X,#4                ;offset : 4 nibbles
                mov B,#1                ;last nibble is nibble 3 (command)
                call ff_buffloop        ;when returning, A=3
                ;store the SID
                mov A,#5
                call ff_sid             ;when returning, A=3

ff_nadpci        ;NAD and PCI are at the same location for all frames (byter 0 and 1)
                ;the NAD is a constant (ffNADhi, ffNADlo), the PCI is in (LINtmp, Btmp)
                ;store the PCI - A should be 3 when jumping to ff_nadpci
                mov B,LINtmp            ;the high part of the PCI is in LINtmp
                mov X,Btmp              ;low part of the PCI (length, length/256 or frame counter) is in Btmp
                call ff_fillbuff
                ;store the NAD
                mov X,#4
                mov X,Rom:FastCstTbl shr 2[X]
                mov B,#5
                mov B,Rom:FastCstTbl shr 2[B]
                call ff_fillbuff

                ;send the message to the application (index 10h : 0x3C)
                mov A,#1
                mov B,#0
                mov LINmess+3,A
                mov LINmess+2,B
                mov A,#evMESSrcvd
                mov LINmess,A
                mov LINmess+1,B         ;(B=0)
                set event               ;set the event and proceed while the application is responding

                ;check ffSTATUS to see if this is a protocol setting or if a response is expected
                mov B,ffSTATUS            ; x x xx : Fast Flash Loading Status
                                        ;  \ \  \
                                        ;   \ \  break detection step nr (3=found)
                                        ;    \ 1 : protocol setting
                                        ;     1 : response requested
                msk B,#8                ;if a response is requested, go to the TX routine
                jnz ff_txrqst_ini
                ;test if this is a first frame
                mov B,LINtmp            ;the high part of the PCI is in LINtmp
                cmp B,#1
                jz ff_nosof
                ;test if we have some continuous frames
                mov X,LINflashStatus    ; x x x x
                                        ;  \ \ \ \
                                        ;   \ \ \ programming mode
                                        ;    \ \ continuous frames coming up
                                        ;     \ not used
                                        ;      not used
                msk X,#2
                jz ff_waitsof           ;go wait for the next start of frame
ff_nosof        mov B,#0
                jmp ff_newstart         ;continuous frames coming up : no start of frame
ff_waitsof      jmp ff_rxinit
                ;------------------------------------------------------------------------------------
ff_txrqst_ini   ;a response is expected
                ;initialize the MSBi to detect a falling edge
                dcom BrCtrl,06h            ;00 00 0110
                                        ;      Rst[1:0]=10: reset upon Edg[2:0]
                dcom BrCtrl,0Eh            ;00 00 1110
                                        ;      Edg[2:0]=010: Fast Falling Edge
ff_txrqst       ;wait for the application to fill the buffer (8 bytes)
                ;watch the bus to avoid being stuck here
                wait Event,EvExt        ;command or event from the MSBi
                jxev ff_edge            ;external event coming from the MSBi (edge)
                call cmdrcvd            ;command from the Mlx16 : process it
                ;check if the command indicated that the data is ready
                mov A,LINframeflag      ;x x x x
                msk A,#8                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                jnz ff_tx               ;the command was a pcDTRDY
                ;the command was not a pcDTRDY, check if it is a discard TX command (ml_Discard)
                msk A,#4
                jnz ff_canceltx         ;discard command
                jmp ff_txrqst           ;wait for the next command
ff_edge         ;an edge has been detected while the data was not ready, go to the RX area
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                msk A,#4                ;test LEvExt
                jnz ff_canceltx            ;edge, cancel the TX and go back to RX
                jmp ff_txrqst           ;it wasn't an edge, keep waiting
ff_canceltx     jmp ff_init
                ;------------------------------------------------------------------------------------
ff_tx            ;the data is in the LINframe buffer, it needs to be cleaned up (NAD, RSID removed)
                ;reset the CRC buffer - if the CRC was received correctly, it should alreay be 0 (otherwise there was a checksum error)
                dmar #CmpRW,ClearByte   ;use the dma to clear the byte
                dmaw LINchksum,#CmpRW
                ;get the high nibble of the PCI to check the kind of frame that has to be sent
                mov X,#2

                ;update the CRC
                call GetByteFast        ;get the PCI (A contains LINbytbuf+1), which is the high nibble of the PCI
                call ff_crc_calc        ;do CRC calculation for the PCI, low nibble is in B, high nibble is in A
                mov B,LINbytbuf         ;restore A and B
                mov A,LINbytbuf+1
                and A,#3                ;keep only the last two bits (just in case)
                switch A                ;check the kind of frame
                jmp ff_tx_single        ;0000: Single Frame
                jmp ff_tx_first         ;0001: First Frame
                ;0010: Continuous Frame ----------
                mov B,#8                ;frame length
                mov X,#1                ;byte counter
                jmp ff_txinit
ff_tx_first     ;0001: First Frame ---------------
                ;check the RSID
                mov X,#6                ;index of the RSID
                call ff_chk_rsid
                ;jz ff_rsid_err         ;if Z is set, the RSID is 0x7F, which means there is an error
                ;move the length at the RSID place
                mov X,#4
                mov B,data:LINframe shr 4[X]    ;get the LEN (low nibble)
                add X,#1
                mov A,data:LINframe shr 4[X]
                add X,#1
                mov data:LINframe shr 4[X],B
                add X,#1
                mov data:LINframe shr 4[X],A
                mov B,#6                ;frame length: 8-2=6, the byte counter is the same as in the single frames
ff_tx_single    ;0000: Single Frame --------------
                ;check the RSID
                mov X,#4        ;index of the RSID
                call ff_chk_rsid        ;B is not modified, A and X are
                ;jz ff_rsid_err         ;if Z is set, the RSID is 0x7F, which means there is an error
                add B,#2                ;frame length offset
                mov X,#2                ;byte counter, the length is already in B

                dcom Flags,10h          ;ForceR=0 : free the bus

ff_txinit       ;initialize the registers for TX
                mov LINmesslen,B        ;save the frame length
                mov LINbytcnt,X         ;save the byte counter
                ;----------------------------
                ;stop the counter and clear all flags
                ;----------------------------
                dcom PlsCtrl,90h        ;1 0 01 0000
                                        ;   \  \    \
                                        ;    \  \    nop
                                        ;     \  clear Run
                                        ;      no capture
                ;clear F[2:0], Edg[2:0] and SEdg[2:0]
                dcom InpCtrl,0C0h       ;1 1 00 0000
                                        ;   \
                                        ;    Rst=1

                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                ;----------------------------
                ;re-initialize the LIN Module
                ;----------------------------
                dcom BrCtrl,0B7h        ;10 11 0111
                                        ;  \  \    \
                                        ;   \  \    Rst[1:0]=11 : Re-Synch mode : reset upon selected edge or BrMatch pulse
                                        ;    \  load Mdiff and Togl with 1
                                        ;     Init=10: reset Capt
                dcom OutCtrl,7Bh        ;OutCrtl: initialize the output bloc: X,Y,CpZ,NxZ,Z set - M,N cleared (recessive outputs)
                                        ;011 11 011
                                        ;  \  \   \
                                        ;   \  \   Y loaded with '1', X set
                                        ;    \  Z, Cpz and NxZ loaded with '1'
                                        ;     M and N loaded with '1'
                dcom AuxCtrl,63h        ;AuxCtrl - Auxiliary Ouput bloc
                                        ;011 00 011
                                        ;  \  \   \
                                        ;   \  \   V loaded with '1', U set - send pulses ('0' for continuous level) - (EscpB)
                                        ;    \  W, Wd unchanged
                                        ;     K and L loaded with '1'
            #IF coDEBUGMARK eq cvON
                ;mark instruction for the first byte only
                xdcom #dcFASTTX         ;mark instruction
                xdma LINbytbuf          ;send the byte just sent
            #ENDIF

                ;make sure the counter is stopped
                dcom PlsCtrl,90h        ;1 0 01 0000
                                        ;   \  \    \
                                        ;    \  \    nop
                                        ;     \  clear Run
                                        ;      no capture
ff_txnextbyte   ;reset the bit counter
                mov X,#0

                ;make sure the counter is stopped
                ;dcom PlsCtrl,90h        ;1 0 01 0000
                                        ;   \  \    \
                                        ;    \  \    nop
                                        ;     \  clear Run
                                        ;      no capture
                ;set the Sync window
                dcom WindCtrl3,0Ah      ;11 00 00 1010
                                        ;  \  \  \    \
                                        ;   \  \  \    immediate load of SyncWindow with 1 - EnStrtEdg = 0
                                        ;    \  \  SiCk: nop
                                        ;     \  FlStRj: nop
                                        ;      StpCpt = 1

                ;------------------------------------------------------------------------------------
ff_txshift      ;shift the data bits and get the bit to be sent in C
                mov A,LINbytbuf+1
                mov B,LINbytbuf
                rrc A
                rrc B
                ; THE FOLLOWING DCOM HAS TO OCCUR BEFORE THE BRCK PULSE !!!
                dcom OutCtrl,61h        ;OutCrtl
                                        ;011 00 001
                                        ;  \  \   \
                                        ;   \  \   Y loaded with CoutCpu, X set
                                        ;    \  Z, Cpz and NxZ unchanged
                                        ;     M and N loaded with '1'
                ;save the data in LINbytbuf
                mov LINbytbuf,B
                mov LINbytbuf+1,A
                ;check if we need to check the bus level
                msk X,#1
                jz ff_txwait
                ;check the bus level
                mov B,Xtmp
                msk B,#8
                jz  ff_txabort          ;the bus is low : this is a collision

ff_txwait       ;TX : this is the loop to send data on the bus
                shift X,00h             ;remove old carry bit
                shift X,084h            ;maintain current carry bit
                add X,#2                ;prepare X for next bit
                wait TimeOut            ;wake up at Br halfmatch

                ;read status registers
                mov A,Stat0             ;read status register 0 (and clear the flags): {Traffic,LEvExt,Ovf,BrEv}
                mov B,Stat1             ;read status register 1 (and clear the flags): {Bin,SplBin,SyncErr,RunErr}
                mov Xtmp,B

                mov A,AuxIn                ;PWM=1 and NoCk=0
                                        ; x x x x
                                        ;  \ \ \ \
                                        ;   \ \ \ set at falling edge
                                        ;    \ \ on Brmatch, set with Bin
                                        ;     \ set at rising edge
                                        ;      SplBin: level on the bus
                jc  ff_bytesent         ;process the next byte
                jmp ff_txshift          ;send the next bit

ff_txabort      ;abort the transmit - signal it to the application
                mov B,#erTXCOL
                call errev
                jmp ff_reinit

                ;------------------------------------------------------------------------------------
ff_bytesent     ;bit 8 : a byte has been sent
                ;get the next byte (byte 0 has already been sent)
                call GetByte            ;get the data byte
                jc ff_endtx             ;if C is set, the transmit is finished
                jnz ff_updtcrc          ;if Z is not set, this is a data byte
                ;the CRC is going to be sent next, inverse LINbytbuf (which is in B) and LINbytbuf+1 (which is in A)
                mov LINbytbuf,A
                mov LINbytbuf+1,B

ff_updtcrc      ;update the CRC
                mov B,LINbytbuf
                mov A,LINbytbuf+1
                call ff_crc_calc        ;do CRC calculation, high nibble in A, low nibble in B
                jmp ff_txnextbyte

ff_endtx        ;end of transmit
                ;stop the counter to stop outputting data
                ;dcom PlsCtrl,90h        ;1 0 01 0000
                                        ;   \  \    \
                                        ;    \  \    nop
                                        ;     \  clear Run
                                        ;      no capture
ff_endok        ;clear data ready flag
                mov X,LINframeflag      ;x x x x
                and X,#3                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                mov LINframeflag,X
                ;check if there is some continuous frames going on
                mov X,LINflashStatus    ;LINflashStatus : x x x x
                                        ;                  \ \ \ \
                                        ;                   \ \ \ programming mode
                                        ;                    \ \ continuous frames coming up
                                        ;                     \ not used
                                        ;                      not used
                msk X,#2                ;test if we have some continuous frames
                jnz ff_sendendtx
                ;go wait for another start of frame
                jmp ff_init
ff_sendendtx    ;send EndEvent to the mlx16
                mov A,#evENDtx
                call send_event            ;send the event (data is already ready)
                jmp ff_txrqst            ;keep receiving some data

;-------------------- sub functions ------------------------------------------------------------------
;Initialize the MSBi cell for a reception. Only change the registers that have been changed during TX
ff_msbi_rx      dcom BrCtrl,0B7h        ;10 11 0111
                                        ;  \  \    \
                                        ;   \  \    Rst[1:0]=11 : Re-Synch mode : reset upon selected edge (see above)
                                        ;    \  \                                           or BrMatch pulse
                                        ;     \  load Mdiff and Togl with 1
                                        ;      Init=10: reset Capt
                ; Output bloc
                dcom Flags,18h            ;ForceR=1 : force a recessive state on the bus
                dcom OutCtrl,7Bh        ;OutCrtl: initialize the output bloc: X,Y,CpZ,NxZ,Z set - M,N cleared (recessive outputs)
                                        ;011 11 011
                                        ;  \  \   \
                                        ;   \  \   Y loaded with '1', X set
                                        ;    \  Z, Cpz and NxZ loaded with '1'
                                        ;     M and N loaded with '1'
                ; Auxiliary Ouput bloc
                dcom AuxCtrl,7Bh        ;AuxCtrl: initialize the auxiliary output bloc: U,V,W,Wd set - K,L cleared
                rt 0,0

;Update the CRC - the function should be called for each byte received (or sent)
;A : high nibble just received
;B : low nibble received
;A and B are modified, X not used
ff_crc_calc     mov Atmp,A              ;low nibble calculation
                mov A,B
                xor A,LINchksum+1
                mov B,LINchksum
                xor B,Rom:ffCrcTblHi shr 2[A]
                mov LINchksum+1,B
                mov A,Rom:ffCrcTblLo shr 2[A]
                mov LINchksum,A
                mov A,Atmp              ;high nibble calculation
                xor A,LINchksum+1
                mov B,LINchksum
                xor B,Rom:ffCrcTblHi shr 2[A]
                mov LINchksum+1,B
                mov A,Rom:ffCrcTblLo shr 2[A]
                mov LINchksum,A
                rt 0,0
;--------------------------------------------------
;Check the RSID <- the reuslt is not used for now
;X : index of the RSID nibble
;B is not modified
;Z is set if the RSID is 0x7F (error)
ff_chk_rsid     mov A,data:LINframe shr 4[X]    ;get the low nibble
                add X,#1
                cmp A,#15
                mov A,data:LINframe shr 4[X]    ;get the high nibble
                cmpcz A,#7
                rt 0,0
;--------------------------------------------------
;Move data within the LINframe buffer
;A : index of the highest nibble
;X : offset in nibbles between the source and the target
;B : last nibble to move
ff_buffloop     mov Atmp,B
ff_loop         mov B,data:LINframe shr 4[A]    ;get the nibble
                add A,X
                mov data:LINframe shr 4[A],B    ;store the nibble
                sub A,X
                sub A,#1
                cmp A,Atmp
                jz ff_endsub
                jmp ff_loop
;--------------------------------------------------
;Store the SID
;A : index of the high nibble
ff_sid          mov X,#7
            #IF coFastStdAlne eq cvON
                mov B,LINflashStatus    ; x x x x
                                        ;  \ \ \ \
                                        ;   \ \ \ programming mode
                                        ;    \ \ continuous frames coming up
                                        ;     \ not used
                                        ;      not used
                msk B,#1
                jnz ff_sid_dd           ;insert the data dump SID
                ;programming mode not entered, insert the read by identifier SID
                add X,#2                ;(9)
            #ENDIF                        ;end coFastStdAlne
ff_sid_dd       mov B,Rom:FastCstTbl shr 2[X]    ;ffSID_DDhi or ffSID_RBIhi
                sub X,#1                ;(6 or 8)
                mov X,Rom:FastCstTbl shr 2[X]    ;ffSID_DDlo or ffSID_RBIlo
;Store the content of B and X in the LINframe buffer
;A : index of the high nibble
;B : content of the high nibble
;X : content of the low nibble
;A is decreased by 2 when returning from the function
ff_fillbuff     mov data:LINframe shr 4[A],B
                sub A,#1
                mov data:LINframe shr 4[A],X
                sub A,#1
ff_endsub       rt 0,0

