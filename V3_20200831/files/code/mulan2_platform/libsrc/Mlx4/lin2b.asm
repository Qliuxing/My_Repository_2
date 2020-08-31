Mlx4di

; ---------------------------------------------------------------------------
;
; Description:
;  LIN Firmware for the Mlx4
;
;
; Copyright (c) Melexis Digital Competence Center
;
; ---------------------------------------------------------------------------

                intel                   ;make sure the default settings are Intel radix and words format

;LIN Software
LIN2verASM      equ 4                   ;revision code of this file
LIN2revASM      equ 1                   ; format : xxxx xxxx x xxx xxxx
LIN2dvtASM      equ 0                   ;                  \ \   \    \
;Flash Loader                           ;                   \ \   \    rev (0 to 15) -> for the LIN software : ver.rev [LIN2revASM]
LIN2verFLSH     equ 2                   ;                    \ \   ver (0 to 7) [LIN2verASM]
LIN2revFLSH     equ 0                   ;                     \ stable (1) / in dvt (0) [LIN2dvtASM]
LIN2dvtFLSH     equ 0                   ;                      same thing for the flash loader [LIN2verFLSH, LIN2revFLSH, LIN2dvtFLSH]

EditNbr         equ 0

;--------------- MelexCM constant definitions -----
;dcom, dma and IO ports definitions for the Mlx4 periphery (for easier reading of the code)
                #include "periphery.asm"

;--------------- LIN and API constant definitions -------------
;common with mlx16.asm: contains event and command possible values and api constants
            #IFDEF USE_RELATIVE_PATH
                #include "../../source/lincst.asm"
            #ELSE
                #include "lincst.asm"
            #ENDIF

;--------------- Compiler Switches ----------------
;enable or disable part of the code
                #include "switches.asm"

;--------------- Far ROM Tables -------------------
;access : configure Txtab[5:2] (in MemTab 2)
; dcom MemTab,0FDh      ;11 1111 01
;                              \  \
;                               \  Txtab[1:0] = 01 (ored with arom[7:6])
;                               Txtab[5:2] = 1111 (replace arom[11:8])
;
; arom : DDDD DDXX XXAA
;               \    \ \
;                \    \ 2 MSBs of A - the 2 LSBs select the nibble
;                 \    X
;                  dcom MemTab (see above)

TableRom                equ 0

INDXtable               equ {_INDXtbl shr 7} or 0C0h
INDXtableX               equ {_INDXtbl shr 3} and 0Fh
    %assert {_INDXtbl and 63} eq 0     ;Make sure that INDXtbl is 6 byte address lsb aligned

PARAMtable              equ {_PARAMtbl shr 7} or 0C0h
PARAMtableX             equ {_PARAMtbl shr 3} and 0Fh
    %assert {_PARAMtbl and 63} eq 0    ;Make sure that PARAMtbl is 6 byte address lsb aligned

AUTOADDtable            equ {_AUTOADDtbl shr 7} or 0C0h
    %assert {_AUTOADDtbl and 7} eq 0   ;Make sure that AUTOADDtbl is 3 byte address lsb aligned
AUTOADDtableX           equ {_AUTOADDtbl shr 3} and 0Fh

;--------------------------------------------------

                words

;--------------- linker informations --------------
                segment word at 0-FFF 'all' ;The total memory map consist of:
                contains 'reserved'     ;- reserved locations (at 0..3)
                contains 'task0vects'   ;- vectors of task 0 (at 4..7)
                contains 'task1vects'   ;- vectors of task 1 (at 8..0Bh)
                contains 'code'         ;- normal code (no placement requirements)

;--------------- RAM mapping ----------------------  <- mapping to review!!
                bytes                   ;RAM addresses are 8 bits wide

                ;local variables of the LIN task:
                segment byte at 0-9F 'LIN private'
;----------------------- blocs of 16 nibbles
LINevendTX      ds.b 1  ;x x x x
                        ; \ \ \ \
                        ;  \ \ \ End TX event must to be sent
                        ;   \ \ unused
                        ;    \ unused
                        ;     unused
LINst           ds.b 1  ;current state of the LIN task (4 bits)
LINbr           ds.b 2  ;LIN Br register : 2 nibbles (8 bits) - even address so the lsb is 0
LINbrNom        ds.b 2  ;LIN Nominal BaudRate : like LINbr above, but should NEVER change, except during cnfBR
LINpresc        ds.b 1  ;Value for the prescaler A used with LINbr
LINprescNom     ds.b 1  ;Value for the prescaler A used with LINbrNom
LINbytbuf       ds.b 2  ;Byte buffer
LINchksum       ds.b 2  ;Message checksum temporary buffer
LINindex        ds.b 2  ;LIN index for ID Filtering Mode, or LIN ID for Frame Processing Mode
                        ;LIN index for ID Filtering Mode : 0 means the ID has been recognized in RAM; 1, 2, 3 the ID is in ROM
                        ;or LIN ID 2 msbs for Frame Processing Mode. 1xxx : indicates that the ID is not defined
LINIDtmp        ds.b 2  ;Memorized ID received/processed
;-----------------------
LIN_IDs         ds.b 12 ;User set IDs (bank 1)
LIN_IDsInit     ds.b 2  ;Index Initialized Flag (bank 1)
LINindxbk1      ds.b 1  ;Index result (bank 1)
LINindxbk2      ds.b 1  ;Index result (bank 2)
;-----------------------
LIN_IDs2        ds.b 12 ;User set IDs (bank 2)
LIN_IDsInit2    ds.b 2  ;Index Initialized Flag (bank 2)
indx2_tmp       ds.b 2  ;Index Compare 2 temporary buffer
;-----------------------
LINtocnt        ds.b 4  ;Time Out counter (low nibble) - updated at each Tbit during the databytes
LINdbg          ds.b 2  ;Debug (mark instructions)
LINtoref        ds.b 2  ;Time Out reference (low nibble) - calculated when message length is known
LINmessbuf      ds.b 1  ;Message buffer
LINbytcnt       ds.b 1  ;Counter used to know how many data bytes have been received
MessStatus      ds.b 1  ;Message status
                        ;x x x x
                        ; \ \ \ \
                        ;  \ \ \ waiting for instruction from Mlx16
                        ;   \ \ start bit already received
                        ;    \ indicates that a bit time has passed after the ID (allow immediate start of TX)
                        ;     message information received
LINerrStat      ds.b 1  ;LIN error status flags
                        ;x x x x
                        ; \ \ \ \
                        ;  \ \ \ data stop bit error detected
                        ;   \ \ wake-up pulse detected (do not handle the break)
                        ;    \ collision error detected
                        ;     ID stop bit error detected
SleepTimeOut    ds.b 2  ;Used to count the sleep timeouts (access to the analog periphery)
ClearByte       ds.b 2  ;Byte used to clear registers with a dma
;-----------------------
LINbrcpt        ds.b 5  ;Break counter
LINparity       ds.b 1  ;Store the temporary values of the ID parity calculation - used only during the ID byte reception
highsync        ds.b 3  ; 3 nibbles for high baudrate margin calculation at every sync bit, record the longest individual bit
lowsync         ds.b 3  ; 3 nibbles for low baudrate margin calculation at every sync bit, record the shortest individual bit
LINmesslen      ds.b 1  ;Message length
LINstopbit      ds.b 1  ;Stop bit counter (TX area)
LINtmp          ds.b 2  ;Temporary Byte for DMA accesses
;-----------------------
LINbrsync       ds.b 4  ;16-bits timer, used to determine the bit period from the SYNC field
LINbrhicpt      ds.b 4  ;Break delimiter counter
LINbrtmp        ds.b 3  ;Used for break length calculations
LINparam        ds.b 1  ;x x x x
                        ; \ \ \ \
                        ;  \ \ \ Not Used
                        ;   \ \ ID recognized in Index table
                        ;    \ 0=TX, 1=RX
                        ;     checksum : 0=regular (without ID field), 1=enhanced (with ID field)
LINbrflag       ds.b 1  ;Autobaudrate flag
                        ;x x x x
                        ; \ \ \ \
                        ;  \ \ \ edge to be processed
                        ;   \ \ not used
                        ;    \ baudrate detected
                        ;     auto baudrate on every frame
LINtxbytlen     ds.b 1  ;Length of the bytes (with the stop bits) (TX area)

LINstatus       ds.b 1  ;x x x x
                        ; \ \ \ \
                        ;  \ \ \ LIN bus activity
                        ;   \ \ 0 : buffer free - 1 : buffer not free
                        ;    \ not used
                        ;     event overflow occured

LINflashStatus  ds.b 1  ;x x x x
                        ; \ \ \ \
                        ;  \ \ \ programming mode
                        ;   \ \ continuous frames coming up
                        ;    \ not used
                        ;     not used

;-----------------------
ParamBuf        ds.b 2  ;ID Parameters buffer, to get data from the ROM
indx1_tmp       ds.b 2  ;Index Compare temporary buffer

LINoptions1     ds.b 1  ;Options for the Mlx4 soft
                        ;       x x xx
                        ;        \ \  \
                        ;         \ \  header stop bit length (for Slave to Master messages) : 0, 0.5, 1, 1.5 + 1 stop bits
                        ;          \ stop bits between databytes during TX : 0 or 1 + 1 stop bit -> 1 or 2
                        ;           not used
LINoptions2     ds.b 1  ;Options for the Mlx4 soft
                        ;       x x x x
                        ;        \ \ \ \
                        ;         \ \ \ state change masked (1)
                        ;          \ \ not used
                        ;           \ deep sleep (1) / light sleep (0)
                        ;            not used

AutoAddPrsc     ds.b 2
AutoAddPrCnt    ds.b 2

LINbrLSBs       ds.b 1  ;xxx 0
                        ;   \ \
                        ;    \ always 0 (that means LINbrcorr LSB should always be 0 too)
                        ;     SYNC field LSBs used for fine-tuning
LINbrcorr       ds.b 1  ;xxx x
                        ;   \ \
                        ;    \ 0 : correction not done, 1 : baudrate correction just done
                        ;     accumulated baudrate correction value, correct the baudrate if a carry is generated

AutoAddMode     ds.b 1  ;Auto adressing mode  x x x x
                                             ; \ \ \ \
                                             ;  \ \ \ within generation
                                             ;   \ \ auto addressing mode set to auto : set to disabled at the end of the procedure
                                             ;    \ auto addressing always enabled
                                             ;     not used
AutoAddEv       ds.b 1  ;Auto addressing event number
AutoAddCpt      ds.b 1  ;Auto addressing (de)-counter
LINframeflag    ds.b 1  ;x x x x
                        ; \ \ \ \
                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                        ;   \ \ sleep command received (ID 3C + first byte 00)
                        ;    \ discard the frame
                        ;     frame data ready for transmit
;-----------------------
Atmp            ds.b 1  ;Temporary buffer
Btmp            ds.b 1  ;Temporary buffer
Xtmp            ds.b 1  ;Temporary buffer
            #IF coFASTFLASH eq cvON ;Fast Flash Loading enabled
ffSTATUS        ds.b 1  ;Fast Flash Loading Status
                        ;x x xx
                        ; \ \  \
                        ;  \ \  break detection step nr (3=found)
                        ;   \ 1 : protocol setting
                        ;    1 : response requested
            #ELSE
unused          ds.b 1  ;unused nibble
            #ENDIF
unused2         ds.b 12 ;memory free, can be used if future needs
;----------------------- 
;0x7F ---------- end of access by DMA ------------------------------
;0x7F ---------- end of reserved memory space ----------------------
;0x80 ---------- locations used for inter-task communication -------
    segment byte at 80-8F 'LIN API'

LINresp         ds.b 4  ;0xA0 : MLX4 -> MLX16 Command Response
LINcmnd         ds.b 4  ;0xA4 : MLX16 -> MLX4 Command
LINmess         ds.b 6  ;0xA8 : MLX4 -> MLX16 Events
LINID           ds.b 2  ;0xAE : MLX4 -> MLX16 LIN Protected Identifier
    segment byte at 90-9F 'LIN Frame'
LINframe        ds.b 16 ;0xB0 to 0xBF (0xE158 to 0xE15F) : LIN frame : up to 8 bytes (checksum calculated automaticaly)

;0x9F ---------- end of inter-task memory space ----------------------

;--------------- reserved memory locations and vector table

                words                   ;ROM addresses are 16 bits wide

                segment word at 0 'reserved'    ;reserved memory locations in ROM

                ; 5AA5h is a signature of valid program in extrnal memory (RAM)
                ; Image with such signature can be put into both RAM and ROM
                dc.w 5AA5h              ;ROM address 0 -- signature of valid image (MLX16 reads this as 0xA55A word)
                dc.w 0A55Ah             ;ROM address 1
                dc.w 0                  ;ROM address 2 is reserved
                dc.w 0                  ;ROM address 3 is reserved

                segment word at 4 'task0vects'  ;vector table of task 0
                jmp task0_por           ;power-on reset vector of task 0
                jmp task0_por           ;task reset vector of task 0
                jmp task0_er            ;external interrupt vector of task 0, should not be possible
                jmp task0_er            ;attention interrupt vector of task 0, should not be possible

                segment word at 8 'task1vects'  ;vector table of task 1
                dc.w 0
                dc.w 0
                dc.w 0
                dc.w 0

;--------------- ROM tables -----------------------
                segment word at 0C 'code'   ;normal code (no calls into this code)

;--------------------------------------------------
;Mlx4 Software Version
;LIN Software
;revision code of this file
; SoftVersion : xxxxxxxx x xxx xxxx
;                       \ \   \    \
;                        \ \   \    rev (0 to 15) -> for the LIN software : ver.rev [LIN2revASM]
;                         \ \   ver (0 to 7) [LIN2verASM]
;                          \ stable (1) / in dvt (0) [LIN2dvtASM]
;                          [EditNbr]
; FlashVersion : 00000000 x xxx xxxx
;                         \   \    \
;                          \   \    rev (0 to 15) -> for the flash loader : ver.rev [LIN2revFLSH]
;                           \   ver (0 to 7) [LIN2verFLSH]
;                            stable (1) / in dvt (0) [LIN2dvtFLSH]


    %assert LIN2revASM le 15
    %assert LIN2verASM le 7
    %assert LIN2dvtASM le 1
    %assert EditNbr le 255
    %assert LIN2revFLSH le 15
    %assert LIN2verFLSH le 7
    %assert LIN2dvtFLSH le 1

SoftVersion     dc.w {{EditNbr shl 8} or {LIN2dvtASM shl 7} or {LIN2verASM shl 4} or LIN2revASM}
FlashVersion    dc.w {{LIN2dvtFLSH shl 7} or {LIN2verFLSH shl 4} or LIN2revFLSH}
                dc.w 00000h             ; alignment
                dc.w 00000h

    %assert {SoftVersion and 3} eq 0    ;Make sure the last 2 bits of the table are 0

;State Transition ROM table (refer to "state transition.xls")
;    to             stDISC   to stACT   to stSLEEP   to stWKUP
;       stDISC  (1)   -         ok          ok          ok
; from  stACT   (2)   ok        -           ok          -
;       stSLEEP (4)   ok        -           -           ok
;       stWKUP  (8)   ok        -           -           -
;       invalid       -         -           -           -
;                   1110(E)  0001(1)     0011(3)    0101(5)    

;access: and A,Rom:TRANStbl shr 2[X], where X is the targeted state and A the actual state, transission allowed if the result is not 0 
;X:       0  1  2  3  4  5  6  7  8  9... F
;result   0  E  1  0  3  0  0  0  5  0...
TRANStbl        dc.w 0E001h             ;@10h -> 0x0018 (Mlx16)
                dc.w 00300h             ;@11h -> 0x001A (Mlx16)
                dc.w 00500h             ;@12h -> 0x001C (Mlx16)
                dc.w 00000h             ;@13h -> 0x001E (Mlx16)

    %assert {TRANStbl and 3} eq 0       ;Make sure the last 2 bits of the table are 0

;--------------------------------------------------
;TO14tbl : table used to calculate the message maximum response in bit times (timeout)
;Tmessage_maximum = 1.4 * 10 * (Ndata + 1)
;A value of 1 is added because the timeout should be fine is the response time is exactly the maximum
;A value of 20 is added to respect the standard whatever the length of the header is
;number of byte/2 is added due to the measurment error (+/- 1/2 Tbit)
;access: mov X,Rom:TO14tbl shr 2[B] for the low nibble, where B is the message length minus 1 (0-7) shifted to the left
;        add B,#1 to get the high nibble
;        mov X,Rom:TO14tbl shr 2[B] for the high nibble
;B:            0  1  2  3  4  5  6    7
;result (dec) 29 43 57 71 85 99 113 127
;result (hex) 1D 2B 39 47 55 63  71  7F
;result + 1   1E 2C 3A 48 56 64  72  80
;result +20   32 40 4E 5C 6A 78  86  94
;result +l/2  33 42 50 5F 6D 7C  8A  99
TO14tbl         dc.w 03342h             ;@14h -> 0x0020 (Mlx16)
                dc.w 0505Fh             ;@15h -> 0x0022 (Mlx16)
                dc.w 06D7Ch             ;@16h -> 0x0024 (Mlx16)
                dc.w 08A99h             ;@17h -> 0x0026 (Mlx16)

    %assert {TO14tbl and 3} eq 0        ;Make sure the last 2 bits of the table are 0

;TO10tbl : table used to calculate the maximum response time when a message has to be sent
;Tresponse_maximum = Tmessage_maximum - Tmessage_length
;                  = 1.4 * 10 * (Ndata + 1) - 10 * (Ndata + 1) - Ndata * Additional_StopBit
;                  = 0.4 * 10 * (Ndata + 1) - Ndata * Additional_StopBit
; Ndata * Additional_StopBit will have to be substracted in the processing
; Timeout is multiplied by 2 since we are counting at every half Tbit in TX
;access: mov X,Rom:TO10tbl shr 2[B] for the low nibble, where B is the message length minus 1 (0-7) shifted to the left
;        add B,#1 to get the high nibble
;        mov X,Rom:TO10tbl shr 2[B] for the high nibble
;B:            0  1  2  3  4  5  6  7
;result (dec)  8 12 16 20 24 28 32 36
;result (hex) 08 0C 10 14 18 1C 20 24
;result - message length in case of 2 stop bit
;result - l   07 0A 0D 10 13 16 19 1C  
;result *2    0E 14 1A 20 26 2C 32 38
TO10tbl         dc.w 00E14h             ;@18h -> 0x0028 (Mlx16)
                dc.w 01A20h             ;@19h -> 0x002A (Mlx16)
                dc.w 0262Ch             ;@1Ah -> 0x002C (Mlx16)
                dc.w 03238h             ;@1Bh -> 0x002E (Mlx16)

    %assert {TO10tbl and 3} eq 0        ;Make sure the last 2 bits of the table are 0

;--------------- ROM-table constants ------------------------------------------------------------------
                #include "cst_rom_tables.asm"

;--------------- parameters validity check (checked by the linker)
    %assert {LINbr and 1} eq 0          ;LINbr should be aligned at an even address
    %assert {LINbrNom and 1} eq 0       ;LINbrNom should be aligned at an even address
    %assert {LINtmp and 1} eq 0         ;LINtmp (temporary buffer) should be aligned at an even address
    %assert {LINbytbuf and 1} eq 0      ;LINbytbufl should be aligned at an even address
    %assert {LINchksum and 1} eq 0      ;LINchksum should be aligned at an even address
    %assert {LINtocnt and 1} eq 0       ;LINtocnt should be aligned at an even address
    %assert {LINmess and 1} eq 0        ;LINmess should be aligned at an even address
    %assert {LINframe and 15} eq 0      ;LINframe should be aligned at an address that ends with 0000b
    %assert {ClearByte and 1} eq 0      ;ClearByte should be aligned at an even address
    %assert {SleepTimeOut and 1} eq 0   ;SleepTimeOut should be aligned at an even address

    %assert {LINoptions1 and 1} eq 0    ;LINoptions1 should be aligned at an even address
    %assert {MessStatus and 1} eq 0     ;MessStatus should be aligned at an even address

    %assert {indx1_tmp and 1} eq 0      ;indx1_tmp should be aligned at an even address
    %assert {LIN_IDs and 15} eq 0       ;LIN_IDs should be aligned at an address that ends with 0000b
    %assert {ParamBuf and 15} eq 0      ;ParamBuf should be aligned at an address that ends with 0000b
    %assert {LIN_IDs2 and 15} eq 0      ;LIN_IDs2 should be aligned at an address that ends with 0000b
    %assert {indx2_tmp and 1} eq 0      ;indx2_tmp should be aligned at an even address

    %assert {LINbrsync and 15} eq 0     ;LINbrsync should be aligned at an address that ends with 0000b
    %assert {LINbrcpt and 1} eq 0       ;LINbrcpt should be aligned at an even address
    %assert {LINbrhicpt and 1} eq 0     ;LINbrhicpt should be aligned at an even address
    %assert {LINbrtmp and 1} eq 0       ;LINbrtmp should be aligned at an even addresses

    %assert {AutoAddPrsc and 1} eq 0    ;AutoAddPrsc should be aligned at an even address
    %assert {AutoAddPrCnt and 1} eq 0   ;AutoAddPrCnt should be aligned at an even address

;----------------------------------------------------------------------------------------------------------------------
;--------------- code -------------------------------------------------------------------------------------------------
;----------------------------------------------------------------------------------------------------------------------

;MACRO : access to analog periphery for MelexCM bug (Mlx16 stack overflow)
AnalogAccess    macro
                ;do an access to the analog
                dcom AnIo,11000011b     ;1 abc 0 hi j
                                        ;            \
                                        ;             RWB : read analog IO address 0hiabc
                mend

;-------------- FAST PROTOCOL -----------------------------------------------------------------------------------------
fastprotocol    ;FAST FLASH LOADING
            #IF coFASTFLASH eq cvON
            #IF coDEBUGMARK eq cvON
                xdcom #dcFAST           ;mark instruction
            #ENDIF
            #IFDEF USE_RELATIVE_PATH
                #include "../../source/lin2b_fastflash.asm"
            #ELSE
                #include "lin2b_fastflash.asm"
            #ENDIF
            #ENDIF

;----------------------------------------------------------------------------------------------------------------------
;--------------- subroutines ------------------------------------------------------------------------------------------
brcpt_updt      ;add the content of LINtmp to LINbrcpt
                mov X,LINbrcpt
                add X,LINtmp
                mov LINbrcpt,X
                mov X,LINbrcpt+1
                addcz X,LINtmp+1
                mov LINbrcpt+1,X
brcpt_inc       ;increment the break counter - C should be set if LINbrcpt+2 has to be incremented by 1
                mov X,LINbrcpt+2
                addcz X,#0
                mov LINbrcpt+2,X
                mov X,LINbrcpt+3
                addcz X,#0
                mov LINbrcpt+3,X
                mov X,LINbrcpt+4
                addcz X,#0
                mov LINbrcpt+4,X
                rt 0,0

brhicpt_updtb   ;add LINtmp to LINbrhicpt, function returns with X=LINbrhicpt+3
                mov X,LINbrhicpt
                add X,LINtmp
                mov LINbrhicpt,X
                mov X,LINbrhicpt+1
                addcz X,LINtmp+1
                mov LINbrhicpt+1,X
brhicpt_inc     ;increment the break delimiter counter - C should be set if LINbrcpt+2 has to be incremented by 1
                mov X,LINbrhicpt+2
                addcz X,#0
                mov LINbrhicpt+2,X
                mov X,LINbrhicpt+3
                addcz X,#0
                mov LINbrhicpt+3,X
                rt 0,0

; Tbit value in [LINbrtmp+2,LINtmp+1,LINtmp]
; Move Tbit to lowsync if Tbit < lowsync
; Move Tbit to highsync if Tbit > highsync
low_high_sync_record
                mov A,LINtmp
                mov B,LINtmp+1
                mov X,LINbrtmp+2        ;overflow counter is in LINbrtmp+2
                cmp A,lowsync
                cmpcz B,lowsync+1
                cmpcz X,lowsync+2       ;compare [X,B,A] with low margin
                jnc low_sync_ok
                mov lowsync,A
                mov lowsync+1,B
                mov lowsync+2,X         ;if low is higher than old, replace it by the new value
low_sync_ok     cmp A,highsync
                cmpcz B,highsync+1
                cmpcz X,highsync+2      ;compare [X,B,A] with low margin
                jc high_sync_ok
                mov highsync,A
                mov highsync+1,B
                mov highsync+2,X        ;if high is lower than old, replace it by the new value
high_sync_ok    ;check LINtmp minimum length and add it to LINbrsync
                ;LINtmp is in [X,B,A]
                cmp B,#4
                cmpcz X,#0
                jc sync_updt_ok         ;Individual bit is less than 0x40, it's probably a LIN bus short, return with carry set 
                add A,LINbrsync
                addcz B,LINbrsync+1
                addcz X,LINbrsync+2
                mov LINbrsync,A
                mov LINbrsync+1,B
                mov LINbrsync+2,X
                mov A,LINbrsync+3
                addcz A,#0
                mov LINbrsync+3,A       ;carry is not set if we return from here
sync_updt_ok    rt 0,0


brhicpt_brcpt_div     ;divide the break + break delimiter value by 2 (LINbrhicpt) and divide break value by 2 (LINbrcpt) 
                mov X,LINbrhicpt+3
                shift X,10h
                mov LINbrhicpt+3,X
                mov X,LINbrhicpt+2
                rrc X
                mov LINbrhicpt+2,X
                mov X,LINbrhicpt+1
                rrc X
                mov LINbrhicpt+1,X
                mov X,LINbrhicpt
                rrc X
                mov LINbrhicpt,X

brcpt_div       ;divide the break value by 2
                mov X,LINbrcpt+4
                shift X,10h
                mov LINbrcpt+4,X
                mov X,LINbrcpt+3
                rrc X
                mov LINbrcpt+3,X
                mov X,LINbrcpt+2
                rrc X
                mov LINbrcpt+2,X
                mov X,LINbrcpt+1
                rrc X                   ;divide only the 2 lower nibbles
                mov LINbrcpt+1,X
                mov X,LINbrcpt
                rrc X
                mov LINbrcpt,X
                rt 0,0

;----------------------------------------------------------------------------------------------------------------------
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
;               jc GB_Error             ;if the pointer exceed the LINframe capacity (8 bytes), go to the extended buffer
GetByteFast     mov B,data:LINframe shr 4[X]    ;get the low nibble
                add X,#1                ;Z and C are reset
                mov A,data:LINframe shr 4[X]    ;get the high nibble
GB_SaveDataDma  ;save the data for the xdma
                mov LINdbg,B
                mov LINdbg+1,A
GB_SaveData     mov LINbytbuf,B         ;save the data in LINbytbuf
                mov LINbytbuf+1,A
GB_Done
                rt 0,0                  ;return, the data is in A (high nibble) and B (low nibble)


;this function should be call to reset the WKUP signal form the analog
dis_ana_wkup
                ;set sleepb
                dcom Flags,60h          ;011 x xxxx
                                        ;   \
                                        ;    sleepb = 1
                                        
                dcom Flags,90h          ;100 10 xxx
                                        ;      \
                                        ;       enwkup = 0
                rt 0,0

;--------------------------------------------------
;Shift LINbytbuf to the right, the bit shifted out (or in) is in C
;at the end LINbytbuf+1 is in A, LINbytbuf in B
shift_bytbuf    mov A,LINbytbuf+1
                mov B,LINbytbuf
                rrc A
                rrc B
                jmp GB_SaveData         ;save the data in LINbytbuf

;----------------------------------------------------------------------------------------------------------------------
;Update the checksum (add the content of A and B to LINchksum, add the carry if any)
;A should contain LINbytbuf+1 and B LINbytbuf
UpdateChkSum    add B,LINchksum
                addcz A,LINchksum+1
                addcz B,#0              ;add the carry
                addcz A,#0
                mov LINchksum,B         ;save the updated checksum
                mov LINchksum+1,A
                rt 0,0

;----------------------------------------------------------------------------------------------------------------------
;Read the status registers of the MSBi cell
;Status register 0 is in A, status register 1 is in B
;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
ReadStatus      mov A,Stat0             ;read status register 0 (and clear the flags): {Traffic,LEvExt,Ovf,BrEv}
                mov B,Stat1             ;read status register 1 (and clear the flags): {Bin,SplBin,SyncErr,RunErr}
                rt 0,0

;----------------------------------------------------------------------------------------------------------------------
;Increment the prescaler PrescA
;X is used
IncrPrescA      mov X,PrescA
                add X,#1
                mov PrescA,X
                rt 0,0


;----------------------------------------------------------------------------------------------------------------------
;Increment the timeout counter (LINtocnt) and check if it reached the max value (LINtoref)
;A contains the value to add to the counter
;A and B are used and modified
;C set if the timeout is not reached
CheckTimeOut    mov A,LINtocnt          ;increment the counter
                mov B,LINtocnt+1
                add A,#1
                addcz B,#0
                mov LINtocnt,A
                mov LINtocnt+1,B
                cmp A,LINtoref          ;compare with the timeout reference
                cmpcz B,LINtoref+1
                rt 0,0

;----------------------------------------------------------------------------------------------------------------------
;Increment the break counter (LINbrcpt) by Linbr when transmitting or receiving continuous
;zeros on the LIN bus. When a bit is 1, reset the counter. The bit is in C. assume that Linbrcpt+3/+4 is never used
;Use only A
UpdateZeroCpt   jc ResetZeroCnt
                mov A,LINbrcpt
                add A,LINbr
                mov LINbrcpt,A
                mov A,LINbrcpt+1
                addcz A,LINbr+1
                mov LINbrcpt+1,A
                mov A,LINbrcpt+2
                addcz A,#0
                jmp ZeroCntDone
ResetZeroCnt    ;the bit is a 1, reset the 0 counter
                mov A,#0
                mov LINbrcpt,A
                mov LINbrcpt+1,A
ZeroCntDone     mov LINbrcpt+2,A
                rt 0,0

;----------------------------------------------------------------------------------------------------------------------
;Increment the break counter (LINbrhicpt)
;A and B are used and modified,
IncrLINbrhicpt  mov A,LINbrhicpt
                mov B,LINbrhicpt+1
                add A,LINbr
                addcz B,LINbr+1
                mov LINbrhicpt,A
                mov LINbrhicpt+1,B
                mov A,LINbrhicpt+2
                mov B,LINbrhicpt+3
                addcz A,#0
                addcz B,#0
                mov LINbrhicpt+2,A
                mov LINbrhicpt+3,B
                rt 0,0

;Check if the BREAK length is not too long (LINbrhicpt < LINtocnt)
;If the BREAK is too long, the SHORT state is entered, if not, the subfunction returns
;must return Carry if LINbrhicpt < LINtocnt
CheckHeaderLen  mov A,LINbrhicpt
                mov B,LINbrhicpt+1
                cmp A,LINtocnt
                cmpcz B,LINtocnt+1
                mov A,LINbrhicpt+2
                mov B,LINbrhicpt+3
                cmpcz A,LINtocnt+2
                cmpcz B,LINtocnt+3
                rt 0,0                  ;the break length is not too long: return

;----------------------------------------------------------------------------------------------------------------------
;Check the status of the sleep timer
;If there is a timeout (SleepTimeOut overflows), return with C set (should go into sleep mode)
;If SleepTimeOut does not overflow, return with C equal 0
;A is used
CheckSleepTmr   mov A,SleepStat         ;Sleep Timer Status (read and clear)
                and A,#15               ;check the timeout flag
                clr C                   ;clear C in case there is no timeout (if there is a timeout, C will be updated with the addcz)
                jz SleepTmrDone         ;return if there is no timeout
                ;do an access to the analog
                AnalogAccess
                ;Sleep Timer timeout : update the sleep counter (SleepTimeOut) and goto sleep if there is an overflow
                mov A,#2                ;select SleepCntInc
                mov A,Rom:ConstantTbl shr 2[A]
                add A,SleepTimeOut      ;increment the counter by the SleepCntInc value
                mov SleepTimeOut,A
                mov A,SleepTimeOut+1
                addcz A,#0              ;C generated if the counter overflows
                mov SleepTimeOut+1,A
SleepTmrDone    rt 0,0                  ;return

CheckAnaTmr     ;test the sleep timer for analog access (in sleep mode)
                mov A,SleepStat         ;Sleep Timer Status (read and clear)
                msk A,#15               ;check the timeout flag
                jz AnaTmrDone
                ;do an access to the analog
                AnalogAccess
AnaTmrDone      rt 0,0                  ;return

;----------------------------------------------------------------------------------------------------------------------
wbr_init        ;this procedure is located somewhere else in the code to solve addressing problems
                jmp wbr_init_start      ;comes back to sleep_tmr

sleep_tmr       ;sleep timer initialization
                ;initialize the sleep counter (idle timeout timer) in case a Break does not appear within the required time
                ;  SleepCnt
                ;  SleepPre
                ;  Timer
                ;A and B are used, X is not used
                ;---------------------------------
                ;reset the sleep counter
                dmar #CmpRW,ClearByte   ;use the dma to clear the bytes
                dmaw SleepTimeOut,#CmpRW    ;clear SleepTimeOut and SleepTimeOut+1

                ;sleep mode enabled
                mov B,#1                ;select SleepCnt_Val
                mov A,Rom:ConstantTbl shr 2[B]
                mov B,#SleepCntIdx      ;sleep timer value index
                mov SleepCnt[B],A
                ;sleep prescaler - enable the counter
                mov B,#0                ;select SleepPre_Val
                mov A,Rom:ConstantTbl shr 2[B]
                                        ;1 xxx
                                        ; \   \
                                        ;  \   prescaler (SleepPre_Val)
                                        ;   timer enable
                mov B,#SleepPreIdx      ;sleep timer prescaler index
                mov SleepPre[B],A

                dcom Timer,33h          ;00 11 00 11
                                        ;     \  \  \
                                        ;      \  \  Sleep Timer TimeOut setting
                                        ;       \  \   0x : no effect
                                        ;        \  \  10 : disable the timeout
                                        ;         \  \ 11 : enable the timeout
                                        ;          \  Message Timer : no effect
                                        ;           Sleep Timer Run Command
                                        ;            00 : no effect
                                        ;            01 : stop the sleep timer
                                        ;            10 : restart the sleep timer
                                        ;            11 : reload and restart the sleep timer - clears SleepStat
sleep_tmr_done  rt 0,0

sleep_tmr_dis   ;disable the sleep timer
                dcom Timer,12h          ;00 01 00 10
                                        ;     \  \  \
                                        ;      \  \  Sleep Timer TimeOut setting
                                        ;       \  \   0x : no effect
                                        ;        \  \  10 : disable the timeout
                                        ;         \  \ 11 : enable the timeout
                                        ;          \  Message Timer : no effect
                                        ;           Sleep Timer Run Command
                                        ;            00 : no effect
                                        ;            01 : stop the sleep timer
                                        ;            10 : restart the sleep timer
                                        ;            11 : reload and restart the sleep timer - clears SleepStat
                mov A,SleepStat         ;reset the sleep timer status (read and clear)
                ;disable the sleep timer
                mov A,#7                ;0 111 reset the msb (timer enable)
                mov B,#SleepPreIdx      ;index
                and A,SleepPre[B]       ;get the current value of the prescaler (don't change it)
                mov SleepPre[B],A       ;disable the timer
                rt 0,0

;--------------- chstcmnd : handle a change state command from the Mlx16 ----------------------------------------------
; This is not accessed with a call but with a jump!!!
; parameters : a = pcSTCH, b = 'target' state, x is not modified
; result : jump to the 'target' state or send a 'command out of sync' before going back to the original state
; see the state transition table TRANStbl in the ROM table section for more details
chstcmnd        ;(the target state is already in B)
                ;B = 0001 -> DISCONNECT (refer to linapi_cst.asm)   -> 1110
                ;B = 0010 -> ACTIVE                                 -> 0101
                ;B = 0100 -> SLEEP                                  -> 1001
                ;B = 1000 -> WAKE-UP                                -> 0001
                ;B = 1111 -> Fast Mode
                mov A,LINst             ;get the current state
            #IF coFASTFLASH eq cvON
                ;allow transition to fast mode
                cmp B,#stFAST
                jnz chstnorm
                ;request for fast mode
                cmp A,#stDISC
                jnz chstinv
                mov A,#ackOK
                mov LINresp+1,A
                ack event
                jmp fastprotocol
            #ENDIF
chstnorm        cmp A,#stSHORT
                jz chstinv              ;we can't change the state from short
                msk A,Rom:TRANStbl shr 2[B]
                jnz chstok              ;the change state requested is possible
chstinv         ;invalid transition, report an error
                mov A,#ackERR
                mov LINresp+1,A
                ack event               ;acknowledge the command
                rt 0,0

chstok          ;acknowledge the command (correctly executed)
                mov A,#ackOK
                mov LINresp+1,A
                ack event               ;acknowledge the command

                ;valid transition, the 'target' state is still in B
                cmp B,#stSLEEP
                jnz chstnosleep
                mov X,#slAPP            ;signal that the sleep mode is entered after a command from the application
                jmp gotosleep
chstnosleep
                ;LINst is not directly updated when going to sleep (250us wait efore). update LINst only for the other states
                mov LINst,B
                cmp B,#stACT
                jnz chstnoact
                jmp active_sc
chstnoact
                cmp B,#stWKUP
                jnz chstnowkup
                jmp wakeup_sc
chstnowkup
                jmp dscnct_sc ;Last state possible: DISC. 


;--------------- errev : send an error to the application -------------------------------------------------------------
; parameters : error type in B
; result : same as setev
errev           ;free the data buffer
                mov A,LINstatus         ;x x x x
                and A,#13               ; \ \ \ \
                                        ;  \ \ \ LIN bus activity
                                        ;   \ \ 0 : buffer free - 1 : buffer not free
                                        ;    \ not used
                                        ;     event overflow occured
                mov LINstatus,A
                ;clear frame flags
                mov A,#0                ;x x x x
                mov LINframeflag,A      ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                mov A,#evERR            ;load the event code into A (the error type should already be in register B)
                jmp send_event

;--------------- setev_mess : send an event evMESSrcvd or evMESSrqst to the application -------------------------------------------------------------
; The event will generate an interrupt on the Mlx16.
; parameters : event code in A
; A,B,X used
setev_mess      ;test if the event is free and signal a new one event
                jhshk setev_mess_ok     ;check if the previous event has been read
                mov A,LINstatus         ;x x x x
                or  A,#8                ; \ \ \ \
                                        ;  \ \ \ LIN bus activity
                                        ;   \ \ 0 : buffer free - 1 : buffer not free
                                        ;    \ not used
                                        ;     event overflow occured
                mov LINstatus,A
                jmp send_event_end

setev_mess_ok   ;copy the temp registers to LINmess - dmar/dmaw is not accessible for LINmess
                mov X,LINindex          ;send the index LSB
                mov B,LINindex+1        ;send the index MSB
                mov LINmessbuf,B
                mov B,LINbr             ;send the Br LSB
                mov LINmess+4,B
                mov B,LINbr+1           ;send the Br MSB
                mov LINmess+5,B
                mov B,PrescA            ;send the PrescA
                ; A = was set before calling "setev_mess"
                ; B = PrescA
                ; X = LINindex
                ; LINmessbuf = LINindex+1
                ; LINmess+4 = LINbr
                ; LINmess+5 = LINbr+1
                jmp send_event_ok

;--------------- chst : do a state change and signal it to the application --------------------------------------------
; parameters : new state in b, old state (current state) in LINst, additional parameter in a
; registers : LINst updated
; result : same as send_event, send {A, OldState, NewState, evSTCH}
chst            mov X,LINst             ;retrieve the current state
                mov LINst,B             ;update the current state
                mov LINmessbuf,A        ;LINmessbuf can be used for additional parameter (for stACT and stSLEEP)
                mov A,LINoptions2       ;LINoptions2 : x x x x
                                        ;                     \
                                        ;                      state change masked
                and A,#1                ;test bit 0
                jz chst_send
                rt 0,0                  ;if set, return immediately without notifying the application
chst_send       mov A,#evSTCH
                ;jmp send_event         ;just for debugging purpose

;--------------- setev : send an event to the application -------------------------------------------------------------
; The event will generate an interrupt on the Mlx16.
; parameters : event code in A,B,X,LINmessbuf
; result : C is set if an error occured
; A,B used
send_event      ;test if the event is free and signal a new one event
                jhshk send_event_ok     ;check if the previous event has been read
                mov A,LINstatus         ;x x x x
                or  A,#8                ; \ \ \ \
                                        ;  \ \ \ LIN bus activity
                                        ;   \ \ 0 : buffer free - 1 : buffer not free
                                        ;    \ not used
                                        ;     event overflow occured
                mov LINstatus,A
                jmp send_event_end

send_event_ok   ;copy the temp registers to LINmess - dmar/dmaw is not accessible for LINmess
            #IF coDEBUGMARK eq cvON
                mov LINdbg,A            ;save for xdma
                mov LINdbg+1,B          ;save for xdma
            #ENDIF
                mov LINmess,A
                mov LINmess+1,B
                mov LINmess+2,X
                mov A,LINmessbuf
                mov LINmess+3,A
                ;copy the protected ID byte
                mov A,LINIDtmp          ;transfer the ID to LINID
                mov B,LINIDtmp+1
                mov LINID,A
                mov LINID+1,B
                ;send the event
                set event               ;set a new event
            #IF coDEBUGMARK eq cvON
                xdcom #dcEVENT          ;mark instruction
                xdma LINdbg             ;send the message sent to the application
            #ENDIF
send_event_end  ;event set or mutex freed, the acknowledge will be done in the interrupt routine N_M4_SHEM of the Mlx16
                rt 0,0

;--------------- cmdrcvd : process a command from the application -----------------------------------------------------
; parameters : none
; result : command code in A, sub-code in B
;          X is not used, except for cmd_cnfid
cmdrcvd         ;an event has been received, get it
                mov A,LINcmnd
                mov B,LINcmnd+1
                mov LINresp,A

            #IF coDEBUGMARK eq cvON
                mov LINdbg,A            ;save for xdma
                mov LINdbg+1,B          ;save for xdma
                xdcom #dcCOMMAND        ;mark instruction
                xdma LINdbg             ;send the command sent by the application
            #ENDIF

cmdrcvd_drok    ;not all commands are valid in all states...
                switch A
                jmp cmd_ack_nok          ;0 - pcNONE : No commands in the buffer
                jmp chstcmnd            ;1 - pcSTCH : General command for LIN state changes
                jmp cmd_cnfbr           ;2 - pcCNFBR : Set the target baudrate/prescaler value
                jmp cmd_ack_nok         ;3 - Command not supported
                jmp cmd_ack_nok         ;4 - Command not supported
                jmp cmd_setframe        ;5 - pcSETFRAME : Discard the current message (Frame Processing Mode) / Set DATAREADY bit / modify continous frame flag
                jmp cmd_cnfid           ;6 - pcCNFID : Configure a LIN ID (Identifier Filtering Mode)
                jmp cmd_cnfsr           ;7 - pcCNFSR : Configure Slew Rate
                jmp cmd_ack_nok         ;8 - Command not supported
                jmp cmd_ack_nok         ;9 - Command not supported
                jmp cmd_sftver          ;A - pcSFTVER : Software version request
                jmp cmd_option          ;B - pcOPTION : Set the option registers
                jmp cmd_getst           ;C - pcGETST : Get the state of the Mlx4
                jmp cmd_cnfauto         ;D - pcCNFAUTO : Config auto addressing pulses
                jmp cmd_ack_nok         ;E - Command not supported
                jmp cmd_rel_buf         ;F - pcRELBUF : Release frame buffer


cmd_ack_ok      ;Send acknowledge event to MLX16
                mov A,#ackOK
                jmp cmd_respond         ;send event

cmd_ack_nok     ;Send acknowledge event to MLX16
                mov A,#ackERR
cmd_respond     ;release the command buffer to be ready for the next command
                mov LINresp+1,A
                ack event
                rt 0,0

cmd_sftver      ;soft version request command-------------------------------------
                ;Request to get the software version or the flash loader version - can be sent from anywhere
                ;SoftVersion = {{EditNbr shl 8} or {LIN2dvtASM shl 7} or {LIN2verASM shl 4} or LIN2revASM}
                ;SoftVersion : xxxx xxxx x xxx xxxx
                ;                       \ \   \    \
                ;                        \ \   \    rev (0 to 15) -> for the LIN software : ver.rev [LIN2revASM]
                ;                         \ \   ver (0 to 7) [LIN2verASM]
                ;                          \ stable (1) / in dvt (0) [LIN2dvtASM]
                ;                          [EditNbr]

                cmp B,#verEDIT
                jz cmd_veredit      ;check if this is the software version or the flash loader
                ;Send immediately the software version
                
SFTVERL     equ LIN2revASM and 15
                mov B,#SFTVERL
                mov LINresp+2,B
                
SFTVERH     equ {{LIN2dvtASM shl 3} or LIN2verASM} and 15                
                mov B,#SFTVERH
                jmp cmd_sftfill
                
EDITVERL    equ EditNbr and 15                
cmd_veredit     mov B,#EDITVERL
                mov LINresp+2,B
                
EDITVERH    equ {EditNbr shr 4} and 15                
                mov B,#EDITVERH
cmd_sftfill     mov LINresp+3,B
                jmp cmd_ack_ok

;--------------- config_id : configure an ID (Identifier Filtering Mode) ----------------------------------------------
;parameters : a = pcCNFID
;             b = Index
;             {LINcmnd+3, LINcmnd+2} = 0 / 0 / LIN ID (6 bits)
;A, B, X, Btmp, Atmp modified
config_id       mov A,#0                ;initialize A
                ;B contains the index
                msk B,#8                ;check the bank
                jz idbank1
                ;bank 2 : LIN_IDs2
                mov X,#LIN_IDs2 shr 4   ;get the address for indexed addressing
                jmp idnibble
idbank1         ;bank 1 : LIN_IDs
                mov X,#LIN_IDs shr 4
idnibble        ;the bank base address is in X, the index in B
                mov Atmp,X              ;save the base address
                msk B,#4                ;check if it is in the first nibble (0 to 3 or 8 to B)...
                jz idlonibble           ;... or in the second (4 to 7 or B to F)
                ;high nibble
                add A,#1                ;offset (high/low nibble)
idlonibble      ;now push the ID bits in the index
                and B,#3                ;keep only the two lsb (to get the bit in the nibble)
                asl B                   ;B = 0xx0, where xx is the bit to modify in the nibble
                mov Btmp,B              ;save B
idgetidbitlo    ;start the processing ! (loop)
                ;bits 0 to 3
                mov B,LINcmnd+2
idrotateid      rrc B
                mov LINcmnd+2,B         ;save the temporary ID
idsetinit       mov B,Btmp              ;restore B
                mov X,Data:[X,A]
                jc idsetbit             ;bit set
                ;reset the corresponding bit
                switch B
                and X,#14               ;B = 0000 : index 0/4/8/12
                jmp idnxtbit            ;
                and X,#13               ;B = 0100 : index 1/5/9/13
                jmp idnxtbit            ;
                and X,#11               ;B = 0100 : index 2/6/10/14
                jmp idnxtbit            ;
                and X,#7                ;B = 0110 : index 3/7/11/15
                jmp idnxtbit            ;
idsetbit        ;set the corresponding bit
                switch B
                or X,#1                 ;B = 0000 : index 0/4/8/12
                jmp idnxtbit            ;
                or X,#2                 ;B = 0010 : index 1/5/9/13
                jmp idnxtbit            ;
                or X,#4                 ;B = 0100 : index 2/6/10/14
                jmp idnxtbit            ;
                or X,#8                 ;B = 0110 : index 3/7/11/15
idnxtbit        mov B,X                 ;result to save in the index
                mov X,Atmp              ;restore X
                mov Data:[X,A],B        ;save the result
                cmp A,#10               ;have all the ID bits been processed?
                jnc idindexrdy          ;yes, move on
                add A,#2                ;no, update the counter
                rrc A                   ;A = 0xxx, C = x
                msk A,#3                ;    0011, ID bit 4? (does not modify C)
                shift A,94h             ;restore A (Z unchanged)
                jz idgetidbithi
                jmp idgetidbitlo        ;get the next bit
idgetidbithi    ;bit 4
                mov B,LINcmnd+3         ;get ID bits 4 and 5
                jmp idrotateid
idindexrdy      ;check if the init flag bit is set
                cmp A,#12
                jnc idindexdone
                ;set the init flag bit
                add A,#2
                set C
                jmp idsetinit
idindexdone     ;ID index ready
                jmp cmd_ack_ok

;--------------- BRcorr : fine tuning of the baudrate -----------------------------------------------------------------
;parameters : none
;A, B modified
;registers used : LINbr/LINbr+1 (RW), LINbrcorr (RW), LINbrLSBs (R), MSBI BR register (8 bits access by DMA)
;   LINbrLSBs : xxx 0
;                  \ \
;                   \ always 0 (that means LINbrcorr LSB should always be 0 too)
;                    SYNC field LSBs used for fine-tuning
;   LINbrcorr : xxx x
;                  \ \
;                   \ 0 : correction not done, 1 : baudrate correction just done
;                    accumulated baudrate correction value, correct the baudrate if a carry is generated
BRcorrection    mov B,LINbr             ;preload the baudrate value low nibble
                mov A,LINbrcorr
                add A,LINbrLSBs
                mov LINbrcorr,A         ;update the LINbrcorr accumulator
                ;3 possibilities : overflow, A LSB not set -> LINbr should be incremented by 1, set LSB
                ;                  overflow, A LSB set -> LINbr should not change, do nothing
                ;                  no overflow, A LSB not set -> do nothing
                ;                  no overflow, A LSB set -> decrement LINbr by 1, reset LSB
                and A,#1                ;000d : decrement if d = 1
                rlc A                   ;00di : decrement if d = 1, increment if i = 1
                switch A                ;di
                jmp BRcorrdone          ;00 : nothing
                jmp BRcorrinc           ;01 : + 1, set LSB
                jmp BRcorrdec           ;10 : - 1, reset LSB
                jmp BRcorrdone          ;11 : nothing

BRcorrinc       ;update LINbr
                add B,#1
                mov LINbr,B
                mov B,LINbr+1
                addcz B,#0
                ;update LINbrcorr lsb (A is already 0001)
                or A,LINbrcorr          ;set the lsb
                
                ;we should wait before increment Linbr that the counter has actually reach the next value.
                ;if not, a new half match can be detected just after this one because the next half match will be generated at counter+1
                ;the overall proc needs 66 cpuck to update the new Linbr value but the counter is invremented after 128 cpuck if presca = 6, 256 cpuck if presca =7 or 512 cpuck if presca = 8
                ;the waiting loop is made with 10 instructions -> minimum cpuck is 10*3 = 30
                ;the the waiting time should be:
                ;62 cpuck if presca = 6 (128 - 66) (3 loops)
                ;190 cpuck if presca = 7 (256 - 66) (7 loops)
                ;446 cpuck if presca =8 (512 - 66) (15 loops)
                ;no needs to wait if presca < 6
                ;store Linbrcorr to be able to use A register
                mov LINbrcorr,A 
                
                mov A,#6                ;to make the comparison
                cmp A,PrescA
                jc BRcorrincPresca78    ;if 6 < Presca   -> no need to wait
                jz BRcorrincPresca6     ;presca = 6
                jmp BRcorrupdt
BRcorrincPresca78
                ;presca = 7 or 8
                mov A,#7
                cmp A,PrescA
                jnc BRcorrincwait ;presca = 7, we should loop 7 times. luckily, A already = 7 
                ;presca = 8
                mov A,#15
                jmp BRcorrincwait 
BRcorrincPresca6 ;presca = 6
                mov A,#3
BRcorrincwait   ;Do delay loop
                nop
                nop
                nop
                nop
                nop
                nop
                sub A,#1
                cmp A,#0
                jz BRcorrupdt
                jmp BRcorrincwait    

BRcorrdec       ;update LINbr
                sub B,#1
                mov LINbr,B
                mov B,LINbr+1
                subcz B,#0
                ;update LINbrcorr lsb
                mov A,#14               ;reset the lsb
                and A,LINbrcorr
                mov LINbrcorr,A
BRcorrupdt      mov LINbr+1,B
                ;update the Br register (dma access)
                dmar #Br,LINbr
BRcorrdone      rt 0,0

;---------------    autoadd_pls : Auto Addressing Pulse (Cooling) --------------------------------------------------------
;send a pulse and reload AutoAddCpt with the next value
;X modified - no parameter needed (values in AutoAddEv, AutoAddCpt, AutoAddMode)
autoadd_pls     ;an autoaddressing pulse has to be sent 
                mov Btmp,B              ;B must be saved since it contains MSBI Status and could be used further
                mov X,AutoAddEv         ;index of the ROM table
                add X,#1                ;increment the index
                mov AutoAddEv,X         ;update the index (the first pulse is pulse 1, the second pulse 2, and so on...)
                ;test if the event is free and signal a new one event
                jhshk autoadd_ev_sent   ;check if the previous event has been read
                mov B,LINstatus         ;x x x x
                or  B,#8                ; \ \ \ \
                                        ;  \ \ \ LIN bus activity
                                        ;   \ \ 0 : buffer free - 1 : buffer not free
                                        ;    \ not used
                                        ;     event overflow occured
                mov LINstatus,B
                jmp autoadd_ev_end
autoadd_ev_sent mov LINmess+1,X         ;pass AutoAddEv as a parameter
            #IF coDEBUGMARK eq cvON
                mov LINdbg,X            ;save AutoAddEv for xdma
                mov A,#0
                mov LINdbg+1,A          ;save for xdma
            #ENDIF
                mov A,#evCOOLAUTO
                mov LINmess,A
                set event               ;send a new event
                ;event set, the acknowledge will be done in the interrupt routine N_M4_SHEM of the Mlx16
            #IF coDEBUGMARK eq cvON
                xdcom #dcAUTOADD        ;mark instruction
                xdma LINdbg             ;send the message sent to the application
            #ENDIF
autoadd_ev_end  ;get the next value for AutoAddCpt
                ;access to AUTOADDtbl in lin2bromtbl.asm (was mov X,Rom:AutoAddTbl shr 2[X] in the local table)
                mov B,X
                mov X,#AUTOADDtableX
                ;ROM table AUTOADDtable located in F20, configure Txtab[5:2] (in MemTab 2)      <--------------
                dcom MemTab,AUTOADDtable    ;11 1111 00
                                        ;           \  \
                                        ;            \  Txtab[1:0] = 00 (ored with arom[7:6])
                                        ;             Txtab[5:2] = 1111 (replace arom[11:8])
                mov X,Rom:TableRom[X,B] ;get the new value for the counter
                mov AutoAddCpt,X        ;store the value just retrieved in AutoAddCpt
                ;pulse sent, counter reloaded
                ;if the value is 0, the auto addressing is finished
                msk X,#15               ;test if the counter value is 0
                jnz autoadd_done        ;if not, terminate the subroutine
                ;all the pulses have been sent, do not use the counter anymore
                mov X,AutoAddMode       ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ within generation
                                        ;   \ \ auto addressing mode set to auto : set to disabled at the end of the procedure
                                        ;    \ auto addressing always enabled
                                        ;     not used
                and X,#4                ;reset auto mode and generation
                mov AutoAddMode,X
autoadd_done
                mov B,Btmp
                rt 0,0

;-----------------------------------------------------------------------------------------------------
;Procedure called when a single SYNC bit is too long
;A and B used
;B should be set with Stat1
SYNCerrBLhigh   ;check if collision with a new break field, check bus level
                msk B,#8               ;check the level of the LIN bus, if high no new break
                jnz SYNCerrBLfini
                ;incorrect sync bit time, could be a start of new break
                mov B,LINerrStat        ;x x x x
                or  B,#4                ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                mov LINerrStat,B
                ;copy Low time to Linbrcpt for next break
                mov A,LINbrtmp+2        ;Linbrtmp+2 contains the number of overflows
                mov LINbrcpt+2, A        ;copy the number of overflows in Linbrcpt for next break detection
                mov A, #0
                mov LINbrcpt+0, A        ;reset other nibbles
                mov LINbrcpt+1, A
                mov LINbrcpt+3, A
                mov LINbrcpt+4, A
                ;Sync field bit length error
SYNCerrBLfini   rt 0,0

cmd_setframe    ;--- Set Frame Command -------------------------------------------------------
                cmp B,#1
                mov B,LINcmnd+2         ;Move LINcmnd+2 to B for further handling in TXdataready and continous setting
                jz  cmd_setfr_dtrdy     ;B=1 : data ready
                jc  cmd_setfr_dscrd     ;B<1 : discard frame
cmd_setfr_cont  ;set/reset continuous frame flag
                cmp B,#0
                mov A,LINflashStatus    ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ programming mode
                                        ;   \ \ continuous frames coming up
                                        ;    \ not used
                                        ;     not used
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
                mov LINevendTX,B        ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ End TX event must to be sent
                                        ;   \ \ unused
                                        ;    \ unused
                                        ;     unused
                jmp cmd_ack_ok
cmd_setfr_dscrd ;set discard frame flag
                mov A,LINframeflag      ;x x x x
                or  A,#4                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                mov LINframeflag,A
                jmp cmd_ack_ok

cmd_rel_buf     ;Clear buffer occupied flag
                mov A,LINstatus         ;x x x x
                and A,#13               ; \ \ \ \
                                        ;  \ \ \ LIN bus activity
                                        ;   \ \ 0 : buffer free - 1 : buffer not free
                                        ;    \ not used
                                        ;     event overflow occured
                mov LINstatus,A
                msk B,#2
                jnz cmd_rel_buf_end
                mov A,LINflashStatus    ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ programming mode
                                        ;   \ \ continuous frames coming up
                                        ;    \ not used
                                        ;     not used
                and A,#14
                msk B,#1
                jz  cmd_rel_buf_wr
                or  A,#1
cmd_rel_buf_wr  mov LINflashStatus,A
cmd_rel_buf_end jmp cmd_ack_ok

cmd_option      ;the command received is an option configuration - sent only in DISC state
                mov A,#stDISC
                cmp A,LINst
                jz cmd_optionok
                jmp cmd_ack_nok         ;wrong state : send an error
cmd_optionok    mov LINoptions1,B       ;set the registers
                mov B,LINcmnd+2
                mov LINoptions2,B
                jmp cmd_ack_ok

cmd_cnfsr       ;if is a Slew rate setting command
                mov A,#stDISC
                cmp A,LINst
                jz cmd_cnfok            ;we are in disconnected state
                jmp cmd_ack_nok         ;wrong state : send an error
cmd_cnfok       ;configure the slew rate
                asl B                   ;B=xSSx
                and B,#6                ;0SS0
                switch B
                dcom Flags,04h          ;0000 : Phymd = 00 -> Check Analog documentation for mode associated
                jmp cmd_srdone      ;slew rate configuration done
                dcom Flags,05h          ;0010 : Phymd = 01 -> Check Analog documentation for mode associated
                jmp cmd_srdone      ;slew rate configuration done
                dcom Flags,06h          ;0100 : Phymd = 10 -> Check Analog documentation for mode associated
                jmp cmd_srdone      ;slew rate configuration done
                dcom Flags,07h          ;0110 : Phymd = 11 -> Check Analog documentation for mode associated
cmd_srdone      jmp cmd_ack_ok          ;slew rate setting command regular end


cmd_cnfauto     ;it was a config autoaddressing
                mov B,LINcmnd+1         ;put the value in B (enable/disable/auto/force)
                cmp B,#ENABLED
                jz cmd_cnfauto_en
                cmp B,#AUTORESET
                jz cmd_cnfauto_au
cmd_cnfauto_dis ;B = disabled : immediately reset en_auto and cnf_en and cnf_auto       <- be careful : the break counter won't
                mov B,#0
                jmp cmd_cnfauto_ok
cmd_cnfauto_au  ;set a flag to indicate that en_auto will have to be disabled after the end of the first break
                mov B,#2
                jmp cmd_cnfauto_ok
cmd_cnfauto_en  ;set a flag to indicate that cnf_en is set (auto addressing mode enabled)
                mov B,#4
                                        ;          \                 is being received !!!
                                        ;           set en_auto
cmd_cnfauto_ok  mov AutoAddMode,B       ;store AutoAddMode x x x x
                                        ;                   \ \ \ \
                                        ;                    \ \ \ within generation
                                        ;                     \ \ auto addressing mode set to auto : set to disabled at the end of the procedure
                                        ;                      \ auto addressing always enabled
                                        ;                       not used
                jmp cmd_ack_ok

cmd_getst       ;get state command--------------------------------------------------
                mov A,LINst             ;get the state of the state machine, reset bits in B
                mov LINresp+2,A
                mov A,LINstatus         ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ LIN bus activity
                                        ;   \ \ 0 : buffer free - 1 : buffer not free
                                        ;    \ not used
                                        ;     event overflow occured
                mov LINresp+3,A
                or B,#6                 ; bits 1 and 2 are not used for reset
                and A,B
                mov LINstatus,A
                jmp cmd_ack_ok

cmd_cnfbr       ;-------------------------------------------------------------------
                ;set the baudrate (was setbr) - sent only in DISC state
                ;calculation of the values has to be done outside
                ;parameters : a = pcCNFBR
                ;             b != 0xE : Prescaler Value
                ;             b = 0xE : Auto baudrate enable
                ;             {LINcmnd+3, LINcmnd+2} = caBaud : {LINbr+1, LINbr} and {LINbrNom+1, LINbrNom}
                ;result : none
                mov A,#stDISC
                cmp A,LINst
                jz cmd_cnfbrok
                jmp cmd_ack_nok         ;wrong state : send an error
cmd_cnfbrok     cmp B,#14
                jz cmd_cnfbrAuto
                ;fixed baudrate mode
                mov PrescA,B
                mov LINprescNom,B
                mov LINpresc,B
                mov B,LINcmnd+2
                mov LINbr,B
                mov LINbrNom,B          ;Nominal Baudrate (never changed, LINbr is changed...
                mov B,LINcmnd+3         ;... during the sync field reception)
                mov LINbr+1,B
                mov LINbrNom+1,B
                ;set the baudrate detected flag
                mov B,LINbrflag         ;x x x x
                or  B,#4                ; \ \ \ \
                                        ;  \ \ \ edge to be processed
                                        ;   \ \ not used
                                        ;    \ baudrate detected
                                        ;     auto baudrate on every frame
                ;clear baudrate on every frame
                and B,#7
                mov LINbrflag,B
                jmp cmd_ack_ok

cmd_cnfbrAuto   ;auto baudrate mode
                mov B,LINbrflag         ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ edge to be processed
                                        ;   \ \ not used
                                        ;    \ baudrate detected
                                        ;     auto baudrate on every frame
                and B,#7                ;clear the auto baudrate on every frame flag
                mov A,LINcmnd+2
                cmp A,#0
                jz  cmd_cnfbrAutoSi
                ;set baudrate on every frame
                or  B,#8
cmd_cnfbrAutoSi ;clear baudrate detected flag
                and B,#11
                mov LINbrflag,B
                jmp cmd_ack_ok

cmd_cnfid       ;-------------------------------------------------------------------
                ;The command is a pcCNFID! This command is only accessible in DISCONNECTED state
                ;For pcCNFID and ONLY FOR THIS COMMAND, the acknowledge is not done and
                ;will have to be done later because this command need 'a lot' of processing
                mov A,#stDISC
                cmp A,LINst
                jz cmd_cnfidok
                jmp cmd_ack_nok         ;wrong state : send an error
cmd_cnfidok     mov A,LINcmnd+3         ;test if it is a SetLinId or a MessageValid command
                rlc A                   ;if C = 0, it is a SetLinId, if C = 1 it is a MessageValid
                jc cmd_enmess           ;MessageValid
                jmp config_id           ;SetLinId : go process the ID without sending an acknowledge
cmd_enmess      ;-------------------------------------------------------------------
                ;the command received is an enable/disable of an ID - sent only in DISC state
                ;get the parameters : B : index
                ;                     LINcmnd+3 : flag
                mov Btmp,B              ;save the index
                                        ;no need to save X in Xtmp, in disconnected state, X is not used
                and B,#12               ;keep the two msbs : xx00
                rr B                    ;0xx0
                switch B
                mov X,LIN_IDsInit       ;Index 0 to 3
                jmp cmd_enbkok          ;
                mov X,LIN_IDsInit+1     ;Index 4 to 7
                jmp cmd_enbkok          ;
                mov X,LIN_IDsInit2      ;Index 8 to 11
                jmp cmd_enbkok          ;
                mov X,LIN_IDsInit2+1    ;Index 12 to 15
cmd_enbkok      ;the register to modify is in X
                ;do not change X, do not change B
                mov A,LINcmnd+3         ;xFxx F is the flag for enable (1) or disable (0)
                rlc A                   ;Fxxx
                rlc A                   ;put the flag in C
                mov A,Btmp              ;get the index (again!)
                and A,#3                ;keep only the two lsbs, and does not modify C : 00xx
                rl A                    ;rl does not modify C : 0xx0
                jc cmd_ensetbit
cmd_enresetbit  ;reset the corresponding bit
                switch A
                and X,#14               ;A = 0000 : index 0/4/8/12
                jmp cmd_ensav           ;
                and X,#13               ;A = 0100 : index 1/5/9/13
                jmp cmd_ensav           ;
                and X,#11               ;A = 0100 : index 2/6/10/14
                jmp cmd_ensav           ;
                and X,#7                ;A = 0110 : index 3/7/11/15
                jmp cmd_ensav           ;
cmd_ensetbit    ;set the corresponding bit
                switch A
                or X,#1                 ;A = 0000 : index 0/4/8/12
                jmp cmd_ensav           ;
                or X,#2                 ;A = 0010 : index 1/5/9/13
                jmp cmd_ensav           ;
                or X,#4                 ;A = 0100 : index 2/6/10/14
                jmp cmd_ensav           ;
                or X,#8                 ;A = 0110 : index 3/7/11/15
cmd_ensav       switch B                ;save the result
                mov LIN_IDsInit,X       ;Index 0 to 3
                jmp cmd_ensav_done         ;
                mov LIN_IDsInit+1,X     ;Index 4 to 7
                jmp cmd_ensav_done         ;
                mov LIN_IDsInit2,X      ;Index 8 to 11
                jmp cmd_ensav_done         ;
                mov LIN_IDsInit2+1,X    ;Index 12 to 15
                ;en/dis of the message is done
cmd_ensav_done
                ; we are in dosconnected state, no need to reload X since it is not used in this state
                jmp cmd_ack_ok

;----------------------------------------------------------------------------------------------------------------------
;--------------- interrupt and attention vectors ----------------------------------------------------------------------
;interrupts are not used in the LIN task, if this code is executed that means there was a problem
;in that case, send a {evERR,erCRASH} error to the application and reset the task
task0_er        ;handler for critical errors        
                mov X,#erCRASHIT
                mov B,#erCRASH
                call errev              ;LIN_errev puts #evERR in A and send it to the application

task0_por      ;power on reset & soft reset       
                xdcom #dcRST            ;mark instruction                
                ctrl 18h                ;disable interrupts
                task 0C0h               ;disable atm

                ;clear the registers used to clear the bytes with dma
                mov A,#0
                mov ClearByte,A
                mov ClearByte+1,A

                exit Mx                 ;- if permitted, reset the Mutex bit

                ; Digital Bus Flags - SLEEPB
                dcom Flags,60h          ;011 x xxxx
                                        ;   \
                                        ;    sleepb = 1

init_lin_task   ;LIN task initialisation
                ;Configuration Registers : Cfg0 (0x3A) and Cfg1 (0x3B) - there is no reason to change these flags after initialization
                mov A,#0
                mov Cfg0,A              ;Cfg0 : 0000
                                        ; NoCk = 0
                                        ; Split = 0
                                        ; RptSync = 0
                                        ; Piped = 0 -> SplBin generated by BrCk pulses
                mov Cfg1,A              ;Cfg1 : 0000
                                        ; Mode 00 (NRZ)
                                        ; FCan/Clkd/MArb = 0
                                        ; CanL/Diffm = 0
                ; Prescaler Bloc : PrescA (0x3E) and PrescB (0x3F)
                mov PrescB,A            ;PrescB = 0 : no dividing factor
                                        ;PrescA is initialized in wbr_init

                ; Output bloc
                dcom OutCtrl,5Bh        ;OutCrtl: initialize the output bloc: X,Y,CpZ,NxZ,Z set - M,N cleared (recessive outputs)
                                        ;010 11 011
                                        ;  \  \   \
                                        ;   \  \   Y loaded with '1', X set
                                        ;    \  Z, Cpz and NxZ loaded with '1'
                                        ;     M and N loaded with '0'
                ;Auxiliary Ouput bloc
                dcom AuxCtrl,5Bh        ;AuxCtrl: initialize the auxiliary output bloc: U,V,W,Wd set - K,L cleared
                dcom StuffCtrl,30h      ;StuffCtrl: set OutMd, ignore the stuffing part
                                        ;0 0 11 0000
                                        ;     \
                                        ;      OutMd loaded with '1'
              
                ;Compare Function
                mov CmpCtrl,A           ;CmpCtrl=0 (compare function)
                                        ;FilterMsk[2:0] = 000, no mask bit enabled, OutMsk[3:0] = 1111
                
                ; Tword accessing mode configuration
                dcom MemTab,regTword	; Setup Tword accessing mode
				
                ; Timers
                ; - Watchdog timer
                ; - Sleep timer
                ; - Message timer (not used)

                ;initialize the watchdog counter (Mlx4 Periphery)
                ;the watchdog monitors the LIN bus. If a dominant level is present on the bus for mode than WatchPre_Cnt/WatchPre_Val
                ;a Mlx4 reset is generated
                mov A,#15
                mov A,Rom:ConstantTbl shr 2[A]  ;WatchPre_Cnt: watchdog timer value
                mov B,#WatchCntIdx      ;watchdog timer value index
                mov WatchCnt[B],A
                mov A,#14
                mov A,Rom:ConstantTbl shr 2[A]  ;#WatchPre_Val or 8
                                        ;1 xxx
                                        ; \   \
                                        ;  \   prescaler (WatchPre_Val)
                                        ;   timer enable
                mov B,#WatchPreIdx      ;sleep timer prescaler index
                mov WatchCnt[B],A

                mov A,#stINIT
                mov LINst,A             ;enter INIT state

restart         ;Initalize all RAM private variables
                mov A,#0
                dmar #CmpRW,ClearByte   ;use the dma to clear the bytes
                dmaw LINtmp,#CmpRW      ;clear LINtmp and LINtmp+1
                dmaw LINtocnt,#CmpRW    ;clear LINtocnt and LINtocnt+1
                dmaw MessStatus,#CmpRW  ;clear MessStatus
                dmaw LINIDtmp,#CmpRW    ;clear LINIDtmp and LINIDtmp+1
                dmaw LINoptions1,#CmpRW ;clear LINoptions1 and LINoptions2

                mov LINflashStatus,A    ;Flash status variable init
                mov AutoAddMode,A       ;reset the auto addressing status register
                mov LINbrflag,A
                mov LINmessbuf,A        ;reset the LIN message buffer
                mov LINindex,A          ;reset the LIN index register (LINindex+1 is initialized later with 8)
                mov LINstatus,A         ;reset the LIN status flags
                mov LINframeflag,A      ;reset the sleep status/special ID info

                ;The baudrate is initialized at startup
                ;to avoid a wake up generation/Auto Br detection with Br uninitiliazed
                mov X,#{abBRSTART shr 4}
                mov LINbrNom+1,X
                mov LINbr+1,X
                mov X,#{abBRSTART and 15}
                mov LINbrNom,X
                mov LINbr,X
                mov X,#{abPRESCASTART and 15}
                mov LINprescNom,X
                mov LINpresc,X
                mov B,#0
indxinit        ;initialize the index table (16 locations! reset LIN_IDsInit too)
                mov Data:LIN_IDs shr 4[B],A
                mov Data:LIN_IDs2 shr 4[B],A
                add B,#1
                jz indxready
                jmp indxinit
indxready       ;indexes have been filed with 0
                mov B,#7
                mov LINtoref,A          ;initialize the timeout reference value to max (112 = 0x70)
                mov LINtoref+1,B

                mov A,#8
                mov LINindex+1,A        ;reset the LIN index register (ID not recognized)

                mov A,#SFTVERL          ;copy software verion in LINmess+4/+5
                mov LINmess+4,A         ;api can check version compatibility at handshake
                mov A,#SFTVERH
                mov LINmess+5,A    

            #IF coDEBUGMARK eq cvON
                xdcom #dcWAIT           ;mark instruction
            #ENDIF
                wait signal             ;Initialization done, wait for application to be ready (Handshake)
                ;when the Mlx16 respond (with an event) proceed to DISCONNECT (enter the state machine)
;----------------------------------------------------------------------------------------------------------------------
;-------------- state machine -----------------------------------------------------------------------------------------
;-------------- DISCONNECTED state (stDISC) ---------------------------------------------------------------------------
; command(s) :
; power-down : allowed
; next state(s) : ACTIVE (command), WAKE-UP (command), SLEEP (command)
; action(s) :    state change (see above) or configure an ID
dscnct          ;disconnected state
                mov B,#stDISC           ;set the new state: disconnected
                call chst               ;signal that the state changed

dscnct_sc       ;entering point after state change command (don't do the call to chst)
                ;disable ANA wkup if we come from deep sleep state
                call dis_ana_wkup
                ;disable edge/timeout detection... no event should come from the MSBi cell!
                ;StatCtrl
                dcom StatCtrl,48h       ;StatCtrl
                                        ;0 1 00 1000
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=000 : LTimeOut not driven
                                        ;     \  reset Ftfr : nop
                                        ;      reset internal flags: Traffic, TraErr, flags related to CaptErr, TimeOut, EvExt

wtcnct          waitpd Event            ;wait for a command (signaled by an event)/allow power down
                ;check if this is a state change, if so, start the PLL
wtcnt_nostch    call cmdrcvd            ;process the command - commands are handled in cmdrcvd
                jmp wtcnct

;-------------- ACTIVE state (stACT) ----------------------------------------------------------------------------------
; command(s) :
; power-down : not allowed
; next state(s) : DISCONNECTED (command (Disconnect)), SLEEP (command (GotoSleep), bus timeout)
; action(s) :    Message processing happens here. If an excessively long dominant state is observed, the FSM goes to BUS SHORT.

active          ;active state
                mov A,#slREG            ;default entering point
                mov B,#stACT            ;set the new state: active
                call chst               ;signal that the state changed

active_sc       ;entering point after state change command (don't do the call to chst)
                ctrl 18h                ;disable interrupts
                task 0C0h               ;disable atm
                mov A,#0
                mov LINerrStat,A
                jmp act_init

;procedures used to go back to active from error in header
;if no data collision, signal the error if jumped in act_init_er_sig else not signal if jump in act_init_er_no_sig 
;if data collision, signal data collision in both cases
act_init_er_no_sig                      ;not signal the error met but previous break in frame 
                mov B,#erNOERR
act_init_er_sig                         ;signal an error or previous break in frame instead
                mov A,LINerrStat        ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                msk A,#1                ;check a stop bit collision
                jz act_init_er_send     ;if no collision, go to error signal signaling procedure
                and A,#14               ;data stop bit collision detected, reset it
                mov LINerrStat,A        
                mov B,LINbytcnt         ;signal collision error
                mov LINmessbuf,B
                mov X,#erRXSTOP
                mov B,#erRX
act_init_er_send                        ;signal an error if B is filled with
                cmp B,#erNOERR          ;check if an error has to be sent
                jz act_init_er          ;go to end of error procedure
                call errev              ;send the error
act_init_er     ;clear special frames and commands in case of error
                mov A,#0
                mov LINframeflag,A

;----------------------------------------------------------------------------------------------------------------------
act_init        ;Initialization of the MSBi cell for Break reception
                ;reset the special ID and discard flag
                mov X,LINframeflag      ;x x x x
                and X,#8                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                mov LINframeflag,X
                ;check the status register to see if a collision occured
                mov A,LINerrStat        ;x x x x
                msk A,#13               ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                jnz abr_init_col        ;collision detected
                jmp act_regbr           ;there wasn't any collision, go on with normal handling

abr_init_col    ;convert LINbrcpt to a prescaler value of 0
                ;get the previous message baudrate
                dmar #CmpRW,LINbr
                dmaw LINtmp,#CmpRW
abr_zero_inc    ;get the minimal prescaler
                mov X,#14
                mov X,Rom:ConstantTbl2 shr 2[X]  ; X = MinPresc
abr_zero_inc2   cmp X,PrescA                   ;check if we reached the actusal prescaler 
                jnz abr_zero_inc2ok            ;actual prescaler is not reached, continue to divide it
                jmp act_regbr                  ;actual prescaler is reached, continue
abr_zero_inc2ok ;multiply the counter by 2, divide the prescaler
                mov A,LINbrcpt
                asl A
                mov LINbrcpt,A
                mov A,LINbrcpt+1
                rlc A
                mov LINbrcpt+1,A
                mov A,LINbrcpt+2
                rlc A
                mov LINbrcpt+2,A
                mov A,LINbrcpt+3
                rlc A
                mov LINbrcpt+3,A
                mov A,LINbrcpt+4
                rlc A
                mov LINbrcpt+4,A
                add X,#1
                jmp abr_zero_inc2

act_regbr       ;regular baudrate detection
                mov X,#1
                mov AutoAddPrsc,X
                mov X,#0
                mov AutoAddPrsc+1,X
act_autoAddPr   ;Get the minimal prescaler
                mov X,#14
                mov X,Rom:ConstantTbl2 shr 2[X]  ; X = MinPresc
act_autoAddPr2  cmp X,LINpresc          ;calculate the prescaler for auto addressing
                jz  act_autoAddFin      ;prescaler is ok, continue
                mov B,AutoAddPrsc
                asl B
                mov AutoAddPrsc,B
                mov B,AutoAddPrsc+1
                rlc B
                mov AutoAddPrsc+1,B
                add X,#1
                jmp act_autoAddPr2      ;check again the prescaler
act_autoAddFin  dmar #CmpRW,AutoAddPrsc ;end of autoaddressing initialization
                dmaw AutoAddPrCnt,#CmpRW

                call wbr_init           ;no timeout from the MSBi, a slow falling edge generates an EvExt

                mov A,#0
                mov MessStatus,A        ;update Active Status

                ;check if the bus is dominant
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                msk B,#8
                jnz act_busrec          ;the bus is recessive
                ;the bus is dominant,clear the collision bit flags and jump directly to break_start
                mov A,LINerrStat
                and A,#3
                mov LINerrStat,A
                jmp break_start

act_busrec      ;the bus is recessive
                mov A,LINerrStat        ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                msk A,#13               ;test all possible collisions
                jz act_wait             ;no collision was detected go to regular wait for break
act_bus_rec_collision                   ;the bus is recessive but a collision was detected, reset collision flags
                and A,#3                ;reset all collision but stop bit (this one is to signal)
                mov LINerrStat,A
                jmp act_init_er_no_sig  ;go back to start with potential erRXSTOP error generation

                ;-----------------------------------------------------------------------------------------------------
act_wait        ;wait for a BREAK : wait for a falling edge ------------------------
                wait Event,TimeOut,EvExt
                jxev act_evext          ;external event coming from the MSBi (falling edge)
                jtime act_to            ;timeout from the sleep timer
                call cmdrcvd            ;command from the MLX16
                jmp act_wait

                ;EvExt -------------------------------------------------------------
act_evext       ;get the Status registers from the MSBi cell
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                msk B,#8                ;check the bus level
                jz break_start          ;Z is set if the bus is low (break start)
                ;if there was a timeout at the same time, ignore it and process the edge
                jmp act_wait

                ;TimeOut -----------------------------------------------------------
act_to          ;check the status of the sleep timer
                call CheckSleepTmr      ;check the status of the sleep timer (result in C)
                jc act_gts              ;if C is not set, the counter did not overflow, keep waiting
                jmp act_evext           ;sleep timer did not overflow, check the bus
act_gts         ;timeout counter overflows, goto sleep
                mov X,#slTO             ;signal that the sleep mode is entered after a sleep timer timeout
                jmp gotosleep

break_start     ;BREAK start detected : prepare to measure the BREAK length
                call sleep_tmr_dis      ;disable the sleep timer
                
                ;make sure that the counter started 
                dcom PlsCtrl,0A0h       ;10 10 0000
                
                ;set the baudrate bloc
                dcom BrCtrl,0Dh         ;00 00 1101
                                        ;      Edg[2:0]=101: Fast Rising Edge
                dcom BrCtrl,05h         ;00 00 0101
                                        ;      Rst[1:0]=01: reset by a Br Match and not by an edge
                dcom StatCtrl,79h       ;0 1 11 1001
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=001 : LTimeOut driven only by Br
                                        ;     \  reset Ftfr : fast edges set the traffic bit
                                        ;      reset internal flags: yes
                ;do not miss the first overflow
                dcom WindCtrl0,03Ah     ;00 00 11 1010
                                        ;  \  \  \    \
                                        ;   \  \  \    immediate load of SyncWindow with 1 - EnStrtEdg = 0
                                        ;    \  \  SiCk: set SiCk (19/12/05)
                                        ;     \  FlStRj: nop
                                        ;      StCpt: nop
                ;set the capture flag (to measure precisely the end of the break field)
                dcom CkCtrl,0F6h        ;11 11 0110
                                        ;  \  \    \
                                        ;   \  \    BrPls : BrMatch -> BrCk, HalfMatch -> IntbrCk
                                        ;    \  RptCap : set (if cleared clear Capt on next edge)
                                        ;     Capt : set (capture cpt in IntBr on edge)
                ;check if this is an auto addressing frame
                mov B,AutoAddMode       ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ within generation
                                        ;   \ \ auto addressing mode set to auto : set to disabled at the end of the procedure
                                        ;    \ auto addressing always enabled
                                        ;     not used
                msk B,#6                ;ENABLED or AUTO mode are set
                jz abr_checkbus         ;this is not an auto addressing frame, keep going            
                
                or B,#1                 ;set generation
                mov AutoAddMode,B
                mov B,AutoAddCpt        ;check if a pulse should be sent immediately
                msk B,#15
                jnz abr_checkbus        ;no pulse has to be sent
                call autoadd_pls        ;send a pulse now

abr_checkbus    ;check once more if the bus is low
                ;do an access to the analog
                AnalogAccess
                mov B,#8
                and B,Stat1
                jz abr_wait             ;the bus is low, go to regular wait
                ;the bus is high, that was a false start
                jmp act_init_er_no_sig  ;go back to start with potential erRXSTOP error generation

                ; BREAK and SYNC processing ----------------------------------------
                ; F10 is used to indicate the step processed:
                ; 00 : break start detected, waiting for a rising edge
                ; 01 : break low level length capture, waiting for the start bit of the sync field
                ; 10 : processing the sync field

abr_wait        ;wait for next event or ovf in BREAK-DEL-SYNC
                wait Event,TimeOut,EvExt
                jtime abr_timeout       ;timeout from the MSBi (sleep timer disabled)
                jxev abr_evext          ;external event coming from the MSBi (edge)
                call cmdrcvd            ;command from the MLX16
                jmp abr_wait

abr_evext       ;external event detected (edge)
                call ReadStatus
                jmp abr_edge

abr_timeout     ;timeout detected (counter overflow)
                call ReadStatus
                mov X,LINbrflag         ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ edge to be processed
                                        ;   \ \ not used
                                        ;    \ baudrate detected
                                        ;     auto baudrate on every frame
                
                and X,#14               ;first reset edge to be processed flag in every case
                msk B,#1                ;test if a runerr came
                jnz abr_ovf_err
                msk A,#12               ;status register 0 (and clear the flags): {Traffic,LEvExt,Ovf,BrEv}
                jz  abr_ovf_proc        ;no RunError, no traffic, no LEvExt
                ;there has been some traffic, or LEvExt bit has been set (we should not test runerr since it's likely set by IntBrck that we don't care about at this moment)
abr_ovf_err     ;an edge was detected right after the timeout event (before reading the stats)
                or  X,#1                ;set edge to be processed flag            
abr_ovf_proc    ;now process the overflow
                mov LINbrflag,X         ;store is in LINbrflag (with or without edge to process checked)
                set C                   ;increment the counter
                switch F10
                jmp abr_breakovf        ;00: break start detected, waiting for a rising edge
                jmp abr_breakhiovf      ;01: break low level length capture, waiting for the start bit of the sync field
                jmp abr_syncovf         ;10: processing the sync field
                jmp abr_wait            ;11: not implemented

abr_ovf_end     ;check the edge needs to be processed flag
                mov X,LINbrflag         ;x x x x
                msk X,#1                ; \ \ \ \
                                        ;  \ \ \ edge to be processed
                                        ;   \ \ not used
                                        ;    \ baudrate detected
                                        ;     auto baudrate on every frame
                jz jmp_abr_wait         ;no edge needs processing
                ;an edge needs processing
                and X,#14               ;clear the edge to be processed flag
                mov LINbrflag,X
abr_edge        ;now process the edge
                dmaw LINtmp+1,#IntBr    ;read IntBr (captured value of the counter)
                switch F10
                jmp abr_breakedg        ;00: break start detected, waiting for a rising edge
                jmp abr_breakhiedg      ;01: break low level length capture, waiting for the start bit of the sync field
                jmp abr_syncedg         ;10: processing the sync field
                                        ;11: not implemented
jmp_abr_wait    jmp abr_wait            ;go back to waiting loop

abr_breakovf    ;break low overflow --------
                dmaw LINtmp,#Br
                call brcpt_updt         ;Add LINtmp to LINbrcpt, function returns with X=LINbrcpt+4
                cmp X,#{abSHORTOVF shr 4} ;compare Linbrcpt+4 and Linbrcpt+3 with abSHORTOVF, jump bus_short if equal 
                jnz abr_breakovf_pls    ;proceed break overflow
                mov X,LINbrcpt+3
                cmp X,#abSHORTOVF and 15
                jnz  abr_breakovf_pls   ;proceed break overflow
                jmp BusShort            ; Line low for to long, probably a bus short to GND
abr_breakovf_pls    ;generate autoadressing pulses
                mov X,AutoAddMode       ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ within generation
                                        ;   \ \ auto addressing mode set to auto : set to disabled at the end of the procedure
                                        ;    \ auto addressing always enabled
                                        ;     not used
                msk X,#1                ;test if autoadressing must be generated
                jz abr_breakovf_fin     ;if not set, auto addressing is not used (or is finished), just divide the length
                ;we are in the auto addressing phase
                mov X,AutoAddPrCnt
                sub X,#1
                mov AutoAddPrCnt,X
                mov X,AutoAddPrCnt+1
                subcz X,#0
                mov AutoAddPrCnt+1,X
                jnz abr_breakovf_fin    ;we are not at Half Tbit pulse, d not decrement the autoaddressing counter
                ;divider reached, re-init divider number
                dmar #CmpRW,AutoAddPrsc
                dmaw AutoAddPrCnt,#CmpRW
                ;decrement the auto-addressing counter
                mov X,AutoAddCpt
                sub X,#1
                mov AutoAddCpt,X        ;store AutoAddCpt
                jnz abr_breakovf_fin    ;go do the length comparison
                ;send a pulse
                call autoadd_pls
abr_breakovf_fin    ;end of break overflow proecude
                jmp abr_ovf_end         ;finish the break overflow procedure
                ;-----------------------

abr_breakhiovf  ;break hi overflow
                ;the counter has overflowed
                call brhicpt_inc        ;Increase LINbrhicpt with 0x100, function returns with X=LINbrhicpt+3
                mov Btmp,B              ;save B
                mov B,LINbrhicpt+2
                sub B,LINbrcpt+2        ;substract the length of the break
                subcz X,LINbrcpt+3
                shift X,10h             ;divide the result by 2
                rrc B
                cmp B,LINbrcpt+2        ;do the comparison
                cmpcz X,LINbrcpt+3
                jnc abr_nobreak         ;break high level is too long, go wait for a new break
                mov B,Btmp              ;restore B
                jmp abr_ovf_end         ;check if there was a falling edge also
abr_nobreak     jmp act_init_er_no_sig  ;go wait for a new break
                ;-----------------------

abr_syncovf     ;sync overflow ---------
                mov X,LINbrtmp+2        ;...and for the individual bit
                add X,#1
                mov LINbrtmp+2,X
                cmp X,#3                ;check that there wasn't 2 overflows for a single bit
                jnc abr_sb_len_err      ;sync bit is to long, signal an error and wait for "new" break
                ;keep going (check if there was also an edge)
                jmp abr_ovf_end         ;finish the SYNC overflow procedure
abr_sb_len_err  ;sync bit measurement overflowed 2 times so the sync bit is to long
                ;signal an error and wait for a "new" break
                call SYNCerrBLhigh      ;A and B are set with Stat0 Stat1
                jmp abr_sb_sync_indiv_error  ;signal the error only if we are in fixed baudrate, a enough long break has been detected
                ;-----------------------

abr_breakedg    ;break edge ------------
                ;a rising edge just occurred (end of BREAK)
                ;- prepare to receive the start bit of the synch field
                call brcpt_updt         ;add LINtmp to LINbrcpt, function returns with X=LINbrcpt+4
                msk B,#8                ;B high bit contains the bus value, check that it's acutaly high
                jnz abr_breakedg_prescacheck    ;the bus is actualy high, enter to delimiter decoding procedure
            #IF coDEBUGMARK eq cvON
                xdcom #dcERRORDEL       ;mark instruction
            #ENDIF
                jmp act_init_er_no_sig  ;signal the error
abr_breakedg_prescacheck                ;divide LINbrcpt counter and increment the presca until LINbrcpt is under #abBRKOVF
                mov X,#0
                cmp X,LINbrcpt+4        ;Linbrcpt+4 should be 0
                jc abr_breakedg_div_counter     ;LINbrcpt+4 is not 0, LINbrcpt must be divided by 2 and presca increased 
                mov X,LINbrcpt+2        ;compare Linbrcpt+2/+3 abBRKOVF
                cmp X,#abBRKOVF and 15
                mov X,LINbrcpt+3
                cmpcz X,#{abBRKOVF shr 4} and 15
                jc abr_breakedg_prescaok ;Linbrcpt <= abBRKOVF, continue
abr_breakedg_div_counter                ;divide the counter by 2 and increment presca
                call IncrPrescA
                call brcpt_div          ;divide the break value by 2
                jmp abr_breakedg_prescacheck    ;go back to counter length check (counter can be divided several times)

abr_breakedg_prescaok                   ;If we are in fixed baudrate, check that presca is not higher than requested one
                mov A,LINbrflag         ;x x x x
                                        ; \ \ \ \ 
                                        ;  \ \ \ edge to be processed
                                        ;   \ \ not used
                                        ;    \ baudrate detected
                                        ;     auto baudrate on every frame
                
                msk A,#8                            ;check autobaudrate
                jnz abr_breakedg_prescaok_busSok    ;autobaudrate is on, skip the check
               
                msk A,#4                            ;check if baudrate already detected
                jz abr_breakedg_prescaok_busSok     ;Br not detectedyet, skip the check
                ;Br has already been detected
                mov A,LINprescNom                      
                cmp A,PrescA                        
                jnc abr_breakedg_prescaok_busSok    ;presca is not higher than requested one, continue
                ;break was too long
                mov B,#erSHORTDONE
                jmp act_init_er_sig                 ;presca is higher than requested one, it was probably a short, inform the application
abr_breakedg_prescaok_busSok                        ;end of autoadressing procedure
                ;check if the auto addressing phase is still on
                mov A,AutoAddMode       ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ within generation
                                        ;   \ \ auto addressing mode set to auto : set to disabled at the end of the procedure
                                        ;    \ auto addressing always enabled
                                        ;     not used
                msk A,#1                ;check if we are still generating
                jz abr_breakedg_fin     ;autoaddressing is not generated, bypass the following steps
                ;error : send evCOOLAUTO with index = 0
                and A,#14
                mov AutoAddMode,A       ;disable the auto addressing for this frame (in continuous mode, no change for the next frame)
                mov A,#evCOOLAUTO       ;send evCOOLAUTO with index = 0
                mov B,#0                ;signal the start pulse
                call send_event         ;send an event to the application
abr_breakedg_fin ;end of the autoaddressing processing
                ;copy LINbrcpt to LINbrhicpt
                dmar #CmpRW,LINbrcpt
                dmaw LINbrhicpt,#CmpRW
                dmar #CmpRW,LINbrcpt+2
                dmaw LINbrhicpt+2,#CmpRW;value of IntBr is in LINtmp add it to the LINbrcpt

                ;BREAK received
            #IF coDEBUGMARK eq cvON
                xdcom #dcBREAK          ;mark instruction
            #ENDIF

                ;prepare the reception of the break delimiter
                dcom BrCtrl,0Eh         ;00 00 1110
                                        ;      Edg[2:0]=110: Fast Falling Edge
                dcom BrCtrl,06h         ;00 00 0110
                                        ;      Rst[1:0]=01: reset upon an edge
                dcom StatCtrl,4Ch       ;0 1 00 1100
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=100 : LTimeOut driven by Ovf event only
                                        ;     \  reset Ftfr : fast edges set the traffic bit
                                        ;      reset internal flags: yes
                
                ;read IntBr to clear capt_flag (capt_err has been reset with internal flags at the previous instructon)
                dmaw LINtmp+1,#IntBr    ;read IntBr (captured value of the counter)
                
                mov F10,#1


                jmp abr_wait            ;end of the edge procedure, go back to waiting loop
                ;-----------------------

abr_breakhiedg  ;break hi edge ---------
                ;SYNC start detected, update the measured length of the delimiter
                ;value of IntBr is in LINtmp
                call brhicpt_updtb      ;Add LINtmp to LINbrhicpt, function returns with X=LINbrhicpt+3

                ;measure the sync length - this is the measure that will determine the baudrate
                ;8 bits should be measured, then the measurement will be divided by 8 to get the bit time

                ;prepare the reception of the sync field
                dcom BrCtrl,0Fh         ;00 00 1111
                                        ;      Edg[2:0]=111: Fast Falling or Rising Edge
                dcom BrCtrl,06h         ;00 00 0110
                                        ;      Rst[1:0]=10: reset upon Edg[2:0]
                ;sync window
                dcom WindCtrl0,0Ah      ;00 00 00 1010
                                        ;  \  \  \    \
                                        ;   \  \  \    immediate load of SyncWindow with 1 - EnStrtEdg = 0
                                        ;    \  \  SiCk: nop
                                        ;     \  FlStRj: nop
                                        ;      StCpt: nop (if set cpt is stopped at next Br match
                ;set the capture flag (to measure precisely the end of the break field)
                dcom CkCtrl,0F0h        ;11 11 0000
                                        ;  \  \    \
                                        ;   \  \    BrPls : nop
                                        ;    \  RptCap : set (if cleared clear Capt on next edge)
                                        ;     Capt : set (capture cpt in IntBr on edge)
                dcom InpCtrl,08Fh       ;1 0 00 1 1 1 1
                                        ;   \ \    \ \ \
                                        ;    \ \    \ \ EnFdb=1: enable fast debounce (BusIn is debounced on a 2/3 majority vote scheme)
                                        ;     \ \    \ Fbin=1: Bin = fast debounced BusIn
                                        ;      \ \    AutoDb=1: flag set (DbCk devided by the 4 msb's of the Br register)
                                        ;       \ RbErr=00: no change
                                        ;        Rst: no change
                mov F10,#2
                mov X,#0                ;initialize the bit counter (0 is for the start bit)
                mov Xtmp,X
                mov LINbrtmp+2,X        ;reset the overflow counter
                
                dmar #CmpRW,ClearByte   ;use the dma to clear the bytes
                dmaw highsync,#CmpRW    ;clear highsync and highsync+1
                dmaw highsync+2,#CmpRW  ;clear highsync+2 and lowsync
                mov X,#15
                mov lowsync+2,X         ;lowsync should begin with a high value
                mov lowsync+1,X         ;lowsync should begin with a high value
                
                ;LINbrcpt is not used anymore, we will divide it by 11 to make check of Tbit*11 > break faster at the end of SYNC.
                ;/11 -> 0h0.1745D...
                ;round to 0h0.18 -> we do not dvide by 11 but by 10.6
                ;*0h0.18 -> (shift by 4) + (shift by 5) -> LINbrcpt+0 is not needed for the calculation
                mov A,LINbrcpt+3
                mov B,LINbrcpt+2
                mov X,LINbrcpt+1
                ;clear +2/+3
                dmar #CmpRW,ClearByte
                dmaw LINbrcpt+2,#CmpRW
                ;shift by 4 -> mov LINbrcpt+X into LINBrcpt+X-1
                mov LINbrcpt+2,A
                mov LINbrcpt+1,B
                mov LINbrcpt,X
                ;shift by 5 -> shift by 1 then mov LINbrcpt+X into LINBrcpt+X-1
                shift A,10h
                rrc B
                rrc X
                add X,LINbrcpt
                addcz B,LINbrcpt+1
                addcz A,LINbrcpt+2
                mov LINbrcpt,X          ;save LINbrcpt/11 for chck in end of SYNC field
                mov LINbrcpt+1,B
                mov LINbrcpt+2,A
                mov LINbrsync,X         ;save it also in linbrSYNC for a minimum break first check now, if the baudrate has been detected
                mov LINbrsync+1,B
                mov LINbrsync+2,A         
                mov A,LINbrflag         ;Autobaudrate flag
                                        ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ edge to be processed
                                        ;   \ \ not used
                                        ;    \ baudrate detected
                                        ;     auto baudrate on every frame
                and A,#12
                cmp A,#4
                jz abr_min_break_do_check  ;baudrate detected check minimum break
                jmp abr_breakhiedg_end  ;baudrate not detected, don't check the minimum break length since we don't know Tbit for the moment
abr_min_break_do_check                
                mov A,PrescA            ;load the actual prescaler
abr_min_break_divPresc                
                cmp A,LINprescNom       ;compare rrquested and actual prescaler
                jnc abr_min_break_check ;both prescaler are equals, go to check counters
                add A,#1                ;increase prescaler register
                mov X,LINbrsync+2       ;divide LINbrsync by 2, it contains break length/11
                shift X,10h
                mov LINbrsync+2,X
                mov X,LINbrsync+1
                rrc X
                mov LINbrsync+1,X
                mov X,LINbrsync
                rrc X
                mov LINbrsync,X
                jmp abr_min_break_divPresc ;check again precalers
abr_min_break_check                     ;prescalers are equals, compare the counters 
                mov X,LINbrsync         ;compare requested counter with LINbrsync (break length/11)
                cmp X,LINbrNom
                mov X,LINbrsync+1
                cmpcz X,LINbrNom+1
                mov X,LINbrsync+2
                cmpcz X,#0
                jnc abr_breakhiedg_end  ;Linbrsync is higher than requested BR, break minimum length is ok
                jmp act_init_er_no_sig  ;break length is too short, go back to active without signaling the error
abr_breakhiedg_end                
                dmaw LINbrsync,#CmpRW   ;clear LINbrsync and LINbrsync+1
                dmaw LINbrsync+2,#CmpRW ;clear LINbrsync+2 and LINbrsync+3
                jmp abr_wait
                ;-----------------------

abr_syncedg     ;there was an edge, increment the counter (br counter is in LINtmp) and check indiv length
                ;add LINtmp to LINbrsync, function returns with X=LINbrsync+3
                ;and calculate Tbit +/- 50%, save it in low/high sync if highet low or lowest high. to check at the end of the sync                             
                call low_high_sync_record
                jnc abr_syncedg_ok      ;individual sync bit was enough long
                jmp abr_sb_sync_indiv_error  ;individual sync bit was very short compared with the break, go back to break detection directly
                ;reset the overflow counter
abr_syncedg_ok  mov X,#0                ;prepare next bit reception
                mov LINbrtmp+2,X       
                ;increment the bit counter
                mov X,Xtmp              ;get the bit counter
                add X,#1                ;increment the bit counter
                mov Xtmp,X              ;save the bit counter
                cmp X,#8
                jz abr_sb_meas          ;last bit, process the time
                jmp abr_wait
                ;---------------------------------------------------------------------------

abr_sb_meas     ;SYNC field processing
                ;make a division by 8 - and keep the lsbs for baudrate correction if enabled
                mov X,LINbrsync
                asl X                   ;a division by 8 is a multiplication by 2 and a division by 16
                mov LINbrLSBs,X         ;the lsb of X should always be 0
                mov X,LINbrsync+3       ;LinBrsync value before the division is not higher than 0x1800, after the division, LINbrsync+3 is always 0
                mov A,#0
                mov LINbrsync+3,A
                mov A,LINbrsync+1
                mov B,LINbrsync+2
                rlc A
                rlc B
                rlc X
                mov LINbrsync,A
                mov LINbrsync+1,B
                mov LINbrsync+2,X
abr_sb_sync_indiv_check                     ;check if individual sync bit length was ok
                shift X,10h                 ;divide Br by 2
                rrc B
                rrc A
                cmp A,lowsync               ;compare it to the shortest individual bit
                cmpcz B,lowsync+1
                cmpcz X,lowsync+2
                jnc abr_sb_sync_indiv_error ;Br divided by 2 is longer than the shortest indiv sync bit, go to error
                add A,LINbrsync             ;calculate Br*1.5
                addcz B,LINbrsync+1
                addcz X,LINbrsync+2
                cmp A,highsync              ;compare it to the longest individual bit
                cmpcz B,highsync+1
                cmpcz X,highsync+2
                jc abr_sb_sync_indiv_error  ;Br*1.5 is shorter than the longest indiv sync bit, go to error
                jmp abr_sb_sync_indiv_check_ok
abr_sb_sync_indiv_error                     ;one individual bit is not within Br margins, send an error 
                mov A,LINbrflag             ;if the baudrate is detected, the error should be sent sice the minimum break length has already been checked
                and A,#12
                cmp A,#4
                jnz abr_sb_sync_indiv_error_nsig  ;baudrate not detected don't signal the SYNC bit error
                mov X,#erSYNCbhi            ;a SYNC bit error came and the break minum length was ok, signal the error
                mov B,#erSYNC
                jmp act_init_er_sig         ;back to active with error signalment
abr_sb_sync_indiv_error_nsig
                jmp act_init_er_no_sig      ;one individual bit is not within Br margins, go to error
abr_sb_sync_indiv_check_ok                  ;individual bits length checked
                ;the bit time should be between 99 (63h) and 199 (C7h)
                mov A,PrescA            ;load the prescaler
abr_sb_div      mov B,LINbrsync+2       ;check if the measured bit time is more than 8 bits
                cmp B,#0
                mov X,LINbrsync+3
                cmpcz X,#0
                jnz abr_sb_prediv       ;LINbrsync+3 is not 0, divide the counter and increas ethe prescaler
                jmp abr_sb_presc        ;continue
abr_sb_prediv   add A,#1                ;increment the prescaler
                ;divide the measured bit time by 2
                shift X,10h             ;X contains LINbrsync+3
                mov LINbrsync+3,X
                rrc B                   ;B contains LINbrsync+2
                mov LINbrsync+2,B
                mov B,LINbrsync+1
                rrc B
                mov LINbrsync+1,B
                mov B,LINbrsync
                rrc B
                mov LINbrsync,B
                ;divide also the LSBs
                mov B,LINbrLSBs
                rrc B
                and B,#14               ;the lsb should always be 0
                mov LINbrLSBs,B  
                call brhicpt_brcpt_div  ;divide also the measured break lengths
                jmp abr_sb_div          ;loop until the higher nibble is 0
abr_sb_presc    ;LINbrsync+2 is 0, check if the bit time is more than 199 (C7h)
                mov B,LINbrsync
                mov X,LINbrsync+1
                cmp B,#7
                cmpcz X,#12
                jnc abr_sb_divagain     ;div the Br and increment the prescaler one more time
                jmp abr_sb_save         ;the bit time value is less than 199, the values are ok
abr_sb_divagain ;do a last division
                shift X,10h             ;X contains LINbrsync+1
                rrc B                   ;B contains LINbrsync
                mov Xtmp,X
                call brhicpt_brcpt_div  ;divide also the measured break lengths
                mov X,Xtmp
                add A,#1                ;increment the prescaler
abr_sb_save     mov LINbrtmp,B          ;save the baudrate to temporary register
                mov LINbrtmp+1,X
                mov X,PrescA            ;save the prescaler
                mov PrescA,A            ;update it with the correct value
                ;clear an eventual timeout
                call ReadStatus
                ;wait for the start of the sync byte stop bit
                dmaw LINtmp+1,#IntBr    ;read IntBr (captured value of the counter)
                dcom BrCtrl,0Dh         ;00 00 1101
                                        ;      Edg[2:0]=001: Fast Rising Edge
                dcom BrCtrl,06h         ;00 00 0110
                                        ; \    \
                                        ;  \ Rst[1:0]=10: reset upon Edg[2:0]
                                        ;   Nop
abr_sb_wait_div ;divide also LINtmp if presca has been increased already
                cmp X,PrescA            ;Lintmp contains the counter time we needed for the operation, will be used for bit7 length test calculation
                jz abr_sb_wait          ;if the presca hasn't been modified, continue (Intbr is saved in LINtmp)
                add X,#1                ;else divide LINtmp
                mov A,LINtmp+1
                mov B,LINtmp
                shift A,10h
                rrc B
                mov LINtmp+1,A
                mov LINtmp,B
                jmp abr_sb_wait_div

abr_sb_wait     wait Event,Timeout,EvExt    ;wait for rising edge of the stop bit
                jtime abr_sb_error_to   ;timeout from the MSBi
                jxev abr_sb_end         ;external event coming from the MSBi (edge)
                call cmdrcvd            ;command from the MLX16
                jmp abr_sb_wait

abr_sb_error_to ;a timeout occured while waiting for the stop bit
                call ReadStatus         ;read status register
                mov X,#1                ;set LINbrtmp+2 to a full overflow
                mov LINbrtmp+2,X
                call SYNCerrBLhigh      ;enter into the collision handling proc
abr_sb_error    ;an error has been detected after bit 7 recieved
                mov X,#erSYNCsbhi
sync_err_det    ;an error sync has been detected
                mov B,#erSYNC
                jmp act_init_er_sig ;signal the error

abr_sb_end      ;we are here at the stop bit of the SYNC field. this part (until wstart_init) is the most critical in term of time needed and directly
                ;influence the minimum Mlx4 frequency allowed
                ;this entire procedure must finish before the PID start bit. It does the following checks:
                ; - Bit 7 was not too long (false stop bit)
                ; - Break+Del < 28Tbit
                ; - Break > 11 Tbit
                ; - Check 15% Margin in case of fixed Br
                ; - report a frame inside frame if it is (LINerrStat == XXX1)
                ; - Prepare MSBI for next start bit
                
                ;call ReadStatus        ;we directly write the readStatus proc instead of calling it to earn 2 instructions (call, ret)
                mov A,Stat0             ;read status register 0 (and clear the flags): {Traffic,LEvExt,Ovf,BrEv}
                mov B,Stat1             ;read status register 1 (and clear the flags): {Bin,SplBin,SyncErr,RunErr}

                dcom InpCtrl,8Ah        ;1 0 00 1 0 1 0
                                        ;   \ \    \ \ \
                                        ;    \ \    \ \ EnFdb=1: enable fast debounce (BusIn is debounced on a 2/3 majority vote scheme)
                                        ;     \ \    \ Fbin=1: Bin = fast debounced BusIn
                                        ;      \ \    AutoDb=1: flag set (DbCk devided by the 4 msb's of the Br register)
                                        ;       \ RbErr=00: no change
                                        ;        Rst: no change
                ;SYNC field received correctly ------------------------------------------------------
                dmar #Br,LINbrtmp          ;generate pulses to count the break length every Tbit
                
                ;do the low limit calculation : 11 Tbits (see section 2.3.1.1 of spec 2.1)
                ;LINbrcpt contains break length divided by 11
                ;here, we should check that Tbit < LINbrcpt
                mov A,LINbrcpt+2
                cmp A,#0
                jz abr_cptlowdo         ;LINbrcpt/11 is less than 0x100, do the check
                jmp abr_brlowdone       ;LINbrcpt/11 is more than 0x100, no need to check
abr_cptlowdo    ;check if LINbrct/11 is more than Br detected (minimum length check)        
                mov A,LINbrcpt
                mov B,LINbrcpt+1
                cmp A,LINbrtmp
                cmpcz B,LINbrtmp+1
                jnc abr_brlowdone       ;LINbrcpt/11 is more than Br detected, minimum break length is ok

break_tooshort  ;false BREAK: too short
                #IF coDEBUGMARK eq cvON
                xdcom #dcBREAKER        ;mark instruction (error)
                #ENDIF
                jmp act_init_er_no_sig  ;break was too short, go back to break waiting loop with no error signal
                
abr_brlowdone
                ;bit7 maximum length check
                mov A,LINtmp
                mov B,LINtmp+1
                dmaw LINtmp+1,#IntBr    ;read IntBr (captured value of the counter)
                add A,LINtmp
                addcz B,LINtmp+1
                jnc abr_sb_end_noovf    ;bit 7 did not overflowed, continue
abr_sb_end_ovf  ;counter overflowed, this is cleary incorrect (0x63<br<0xC0 +-50%)
                jmp abr_sb_error
abr_sb_end_noovf                        ;check if the bit 7 is within Br margin
                ;Divide by 2 = 50%
                mov LINtmp,A
                mov LINtmp+1,B
                mov A,LINbrtmp
                mov B,LINbrtmp+1
                shift B,10h
                rrc A
                add A,LINbrtmp
                addcz B,LINbrtmp+1
                jc abr_sb_end_ok        ;BR*1.5 overflowed, condition is ok
                cmp A,LINtmp
                cmpcz B,LINtmp+1
                jnc abr_sb_end_ok       ;Bit7 < BR*1.5, continue 
                jmp abr_sb_error        ;bit length is not within the +50% margin


                ;Theader_max = 1.4*35 = 49
                ;Tbreak_low_max = 49-1-10-10 = 28 rounded to 32 for ease of coding and measure inconsistency
                ;the maximum length for the break is 32 Tbits, so 32*199 (because a Tbit is comprised between
                ;99 and 199), which is 18e0h
abr_sb_end_ok   ;do the high limit calculation (32 Tbit) = Tbit[<<1]<<1
                mov A,LINbrtmp
                asl A                      
                mov B,LINbrtmp+1
                rlc B
                mov X,#0
                mov LINtocnt,X
                rlc X
                mov LINtocnt+1,A
                mov LINtocnt+2,B
                mov LINtocnt+3,X

                ;do the comparison
                cmp A,LINbrhicpt+1      ;A already contains LINtocnt+1
                cmpcz B,LINbrhicpt+2    ;B already contains LINtocnt+2
                cmpcz X,LINbrhicpt+3    ;X already contains LINtocnt+3
                jnc abr_cptdone         ;the break is less than 32 Tbit, proceed
                ;the BREAK is too long, signal an error
                mov X,#erSYNChead
                jmp sync_err_det        ;the BREAK is too long, signal an error
                
abr_cptdone     ;Header was ok
                ;Check if we are in fixed baudrate or single shot auto baudrate mode
                mov X,LINbrflag         ;x x x x
                msk X,#8                ; \ \ \ \
                                        ;  \ \ \ edge to be processed
                                        ;   \ \ not used
                                        ;    \ baudrate detected
                                        ;     auto baudrate on every frame
                jnz abr_SYNC_noChk      ;We are doing auto baudrate on every frame
                ;no auto baudrate on every frame
                msk X,#4
                jnz abr_SYNC_Chk        ;Baudrate has already been detected, now check margins
abr_SYNC_noChk  ;No baudrate has been detected yet, go on without checking margins
                jmp abr_SYNC_Fin        ;Br is not fixed yet, directly go to end of SYNC procedure

abr_SYNC_Chk    ;verify that the resulting baudrate is not higher than the nominal baudrate plus a 15% margin
                ;Compute an approximation of the absolute value of a 15% margin on the nominal baudrate.
                ;This approximation is based on the fact that x/8 + x/32 yields a result very close to x * 0.15
                ;Be carreful here: we are working with time and not freq value, so the deviation formula change:
                ;Brdev = |Brnom - Br|/Brnom = |Tbitnom - Tbit|/Tbit
                ;This part (until abr_SYNC_Fin) has been optimized for minimizing the number of instruction
                ;needed for the program to check but not in term memory space needed (instructions in total).
                ;Divide by 8
                mov A,LINbrtmp+1        ;use detected baudrate for margin calculations
                mov X,LINbrtmp
                asl X
                rlc A                   ;/8 = /16 * 2
                mov B,#0
                addcz B,#0
                add A,#1                ;add 1 to high nibble since we will not make the carries addition
                addcz B,#0
                mov LINtmp,A            ;Copy the Result to LINtmp, forget low nibble, not needed
                ;Divide by 32
                mov A,LINbrtmp+1
                shift A,10h             ;/32 = /16 / 2
                add A,LINtmp
                addcz B,#0
                mov LINtmp,A            ;Copy the 15% margin to LINtmp
                mov LINtmp+1,B
                mov A,LINbrtmp          ;reload Br into A,B,X for next comparisons
                mov B,LINbrtmp+1
                mov X,PrescA                
                ;here we will compare Br recieved and requested and only one of +15% or -15% conditions
                ;depending on if bigger or lower
                ; Br_Nom > Br  ->  check only that Br+15% > Br_Nom
                ; Br_Nom < Br  ->  check only that Br-15% < Br_Nom
                cmp X,LINprescNom
                jz abr_SYNC_Chk_cmp_cnt     ;Presca are equal, compare cnt
                jc abr_SYNC_Chk_Hi          ;Presca < Presca_Nom compare only Br+15% > Br_Nom
                jmp abr_SYNC_Chk_Lo         ;Presca > Presca_Nom compare only Br-15% < Br_Nom
abr_SYNC_Chk_cmp_cnt    ;Presca are equal, compare cnt
                cmp A,LINbrNom              ;compare Br and Br_Nom counters
                cmpcz B,LINbrNom+1
                jc abr_SYNC_Chk_Hi          ;Br < Br_Nom -> compare only Br+15% > Br_Nom
                jmp abr_SYNC_Chk_Lo         ;Br > Br_Nom compare only Br-15% < Br_Nom
                
abr_SYNC_Chk_Hi     ;Compare Br+15% > Br_Nom
                add A,LINtmp                ;add 15% of Br_Nom to Br
                addcz B,LINtmp+1
                ;Check if the value is 200 or more (C8h)
                cmp A,#8
                cmpcz B,#12
                jc abr_SYNC_ChkHiDoPresc    ;it's not higher, make the test directly
                ;Margin is higher than 199
                add X,#1                    ;Increase Temp value of PrescNom
                shift B,10h                 ;Divide Br margin value by 2
                rrc A
abr_SYNC_ChkHiDoPresc                       ;Do PrescA high margin check
                cmp X,LINprescNom           ;LINprescNom should be less than or equal to PrescA
                jz abr_SYNC_ChkHiDoCnt      ;Presca are equals, go check the cnt               
                jc abr_SYNC_ChHiEr          ;Presca is still lower than Presca_Nom -> error
                jmp abr_SYNC_Fin            ;Presca is higher than Presca_Nom -> test is ok
abr_SYNC_ChkHiDoCnt                         ;Do cnt high margin check
                cmp A,LINbrNom
                cmpcz B,LINbrNom+1
                jnc jmp_abr_SYNC_Fin        ;LINbrtmp+15% is more than or equal to Br_Nom, 15% check is ok
abr_SYNC_ChHiEr mov X,#erSYNChi             ;LINbrtmp+15% is less than Br_Nom, 15% check is not ok    
                jmp sync_err_det            ;generate an error
abr_SYNC_Chk_Lo ;Compare Br-15% < Br_Nom
                sub A,LINtmp                ;sub 15% of Br_Nom to Br
                subcz B,LINtmp+1
                ;Check if the value is below 99 (63h)
                cmp A,#3
                cmpcz B,#6
                jnc abr_SYNC_ChkLoDoPresc   ;it's not lower, make the test directly    
                ;Margin is below 99
                sub X,#1                    ;Decrease Temp value of PrescNom
                asl A                       ;Multiply Br margin value by 2
                rlc B
abr_SYNC_ChkLoDoPresc                       ;Do PrescA low margin check
                cmp X,LINprescNom
                jz abr_SYNC_ChkLoDoCnt      ;Presca are equals, go check the cnt               
                jnc abr_SYNC_ChLoEr         ;Presca is still higher than Presca_Nom -> error
jmp_abr_SYNC_Fin                            ;Presca is lower than Presca_Nom -> test is ok
                jmp abr_SYNC_Fin            ;go to end of SYNC procedure
abr_SYNC_ChkLoDoCnt                         ;Do cnt low margin check
                cmp A,LINbrNom
                cmpcz B,LINbrNom+1
                jc abr_SYNC_Fin             ;LINbrtmp-15% is less than or equal to Br_Nom, 15% check is ok                
abr_SYNC_ChLoEr mov X,#erSYNClo             ;LINbrtmp-15% is more than Br_Nom, 15% check is not ok
                jmp sync_err_det            ;generate an error
abr_SYNC_Fin    ;end of SYNC check procedure, set MSBI to wait for start of ID
                ;let's update the baudrate
                dmar #CmpRW,LINbrtmp
                dmaw LINbr,#CmpRW
                mov A,PrescA
                mov LINpresc,A

                ;Br register (dma access)
                dmar #Br,LINbr              ;generate pulses to count the break length every Tbit

                ;Send SYNC+Br
            #IF coDEBUGMARK eq cvON
                xdcom #dcSYNC           ;mark instruction
                xdma LINbr              ;send the calculated baudrate
            #ENDIF

                ;update the traffic flag
                mov B,LINstatus         ;x x x x
                or B,#1                ; \ \ \ \
                                        ;  \ \ \ LIN bus activity
                                        ;   \ \ 0 : buffer free - 1 : buffer not free
                                        ;    \ not used
                                        ;     event overflow occured
                mov LINstatus,B

                ;StatCtrl
                dcom StatCtrl,4Ah       ;StatCtrl
                                        ;0 1 00 1010
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=010 : LTimeOut driven only by IntBr Flag
                                        ;     \  reset Ftfr : nop
                                        ;      reset internal flags: yes

                ;event in the middle of a bit
                dcom CkCtrl,06h         ;00 00 0110
                                        ;  \  \    \
                                        ;   \  \    immediate load of BrPls[1:0]=10 : HalfMatch -> InBrCk
                                        ;    \  RptCap : nop
                                        ;     Capt : nop
                mov B,#1
                ;Header length max check, LINbrhicpt incremented and check to be < 28 Tbit (48 -ID - SYNC)
                ;Stop bit of the sync should not increment LINbrhicpt since it's already taken into account in this 28 = 48-20.
                ;Initialize LINtmp with 1. at the wstart bit, we will decrement it and update LINbrhicpt only if = 0.
                mov LINtmp,B
                
                ;A...
                ;B is used to move the registers around
                ;X is only used to count the bits received
                ;--- the SYNC field has been received, let's proceed with the ID and data bytes...
                mov LINparity,B         ;preset the buffer used for parity verification of the ID field
                mov B,LINbytcnt         ;save Bytcnt old value, it will be sent if Break in frame
                mov Btmp,B
                mov B,#15               ;preset the data byte counter (F for the ID, 0 for the first byte...)
                mov LINbytcnt,B
                mov F10,#0              ;reset F0 and F1
                ;initialize the index compare buffers with the init flags
                dmar #CmpRW,LIN_IDsInit ;use CmpRW for 8 bits access
                dmaw indx1_tmp,#CmpRW
                dmar #CmpRW,LIN_IDsInit2
                dmaw indx2_tmp,#CmpRW
                mov B,#0
                mov LINindex+1,B        ;preset the ID page register
wstart_init     ;--------------------------------------------------
                ;wait for a Start Bit : use the false start bit rejection?          <-???

                ;no data in A,B or X
                ;Baudrate bloc Re-Initialization
                dcom BrCtrl,07h         ;00 00 0111
                                        ;  \  \    \
                                        ;   \  \    Rst[1:0]=11 : Re-Synch mode : reset upon selected edge (see next instruction)
                                        ;    \  nop
                                        ;     Init: nop
                dcom BrCtrl,0Eh         ;00 00 1110
                                        ;      Edg[2:0]=110: Fast Falling Edge
                dcom StatCtrl,30h       ;0 0 11 0000
                                        ;   \  \    \
                                        ;    \  \    nop
                                        ;     \  set Ftfr : fast edges set the traffic bit
                                        ;      nop

wstart          wait Event,TimeOut,EvExt
                ;check the source that ended the wait
                ;if RunErr is set, the mlx4 failed to process the event in time
                ;Description of what happens in case an event and a timeout occur at the exact same time:
                ;- the event (falling edge) is processed, the timeout is ignored
                jxev wstart_ev          ;if it's an external event (edge!)  <- start of the bit
                jtime wstart_to         ;if it's a timeout...       <- increment/check the response timeout counter
                call cmdrcvd            ;else check if it is a command from the Mlx16   <- command?
                jmp wstart

wstart_ev       ;external event : check the source (and reset the flags / acknowledge the event)
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                msk B,#8                ;check if the bus is actualy dominant
                jz jmp_wstart_ok        ;the bus is still recessive, false event? go back to wstart
                jmp wstart
jmp_wstart_ok
                jmp wstart_ok           ;other event

wstart_to       ;timeout : check that it comes from the baudrate counter
                ;check the status registers (and reset the flags / acknowledge the event!)
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                ;test if there was an edge at the same time (in that case ignore the timeout)
                msk B,#8                ;{Bin,X,X,X}
                jnz wstart_tstto        ;the bus is still recessive, process timeout
                jmp wstart_ok           ;the bus is dominant, process start bit immediately

wstart_tstto    msk A,#1                ;check that the timeout comes from the baudrate counter
                jnz wstart_tocnt        ;timeout comes from the MSBI
                ;check the status of the sleep timer
                call CheckSleepTmr
                jc wstart_gts          ;if C is set, the counter did overflow, we should go to sleep
                jmp wstart
wstart_gts
                ;timeout counter overflows, goto sleep
                mov X,#slTO             ;signal that the sleep mode is entered after a sleep timer timeout
                jmp gotosleep

wstart_tocnt    ;the Timeout comes from the MSBI
                ;there is a difference here if we are waiting for the ID start bit (we are still in the header)
                mov B,LINbytcnt
                cmp B,#15
                jnz wstart_data
                ;waiting for the ID
                ;BrEv : increment LINbrhicpt that measures the header length and check if it's not too long (> 28Tbit)
                ;First two timeout should be not processed since the stop bit of the SYNC is already taken in 28 = 48-20
                ;LINtmp has been intialized with 1 in SYNC. check if it's 0, if not, decrement it and do nothing
                mov B,#0
                cmp B,LINtmp
                jnz wstart_nincr_headerto
                call IncrLINbrhicpt     ;Increment Linbrhicpt with Linbr
                call CheckHeaderLen     ;check if the header length is not too long (LINbrhicpt<LINbtocnt) (LINtocnt = 28Tbit)
                jc jmp_wstart
wstart_header   ;the BREAK is too long, signal an error
                mov X,#erSYNChead
                mov B,Btmp              ;load LINbytcnt with the old byte cnt value if stopbiter must be sent
                mov LINbytcnt,B
                mov B,#erSYNC
                jmp act_init_er_sig     ;signal the error but signal BRFRM instead if header in frame                
wstart_nincr_headerto                   ;SYNC stop bit, do not increment the timeout but launch the sleep timer
                mov LINtmp,B            ;reset LINtmp in order to begin to count for maximum header length
                ;enable the sleep timer in case the start bit never comes do this operation at the first stop bit timeout (B = 1->0) to not overload Stop sync bit procedures
                call sleep_tmr
jmp_wstart      ;go back to waiting start
                jmp wstart              ;go back to waiting start

wstart_data     ;increment the bit counter for the time out and check that the timeout hasn't been reached <- note : this is not relevant in LIN spec 2.1
            #IF coTIMEOUT eq cvTO13
                call CheckTimeOut
                jnc wstart_toerr
                ;no timeout
                jmp wstart              ;no edge, go back to the waiting loop
wstart_toerr    ;timeout error : no activity on the bus for too long!
                jmp toerr
            #ELSE
                jmp wstart              ;no edge, go back to the waiting loop
            #ENDIF

wstart_ok       ;received correct start bit
                mov A,MessStatus        ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ waiting for instruction from Mlx16
                                        ;   \ \ start bit already received
                                        ;    \ indicates that a bit time has passed after the ID (allow immediate start of TX)
                                        ;    message information received
                or A,#2                 ;signal that the start bit has already been received
                mov MessStatus,A

                ;disable the sleep timer
                call sleep_tmr_dis

            #IF coDEBUGMARK eq cvON
                xdcom #dcSTART          ;mark instruction
            #ENDIF

                ;the counter has been automatically reset
                ;now wait only for timeouts to get the bits

                ;Baudrate bloc
                dcom BrCtrl,05h         ;00 00 0101
                                        ;  \  \    \
                                        ;   \  \    Rst[1:0]=01: reset with BrMatch (no edges anymore)
                                        ;    \  nop
                                        ;     Init: nop
                dcom BrCtrl,08h         ;00 00 1000
                                        ;      Edg[2:0]=000: no effect
                ;event in the middle of a bit
                dcom CkCtrl,06h         ;00 00 0110
                                        ;  \  \    \
                                        ;   \  \    immediate load of BrPls[1:0]=10 : HalfMatch -> InBrCk
                                        ;    \  RptCap : nop
                                        ;     Capt : nop


wbit_initcnt    ;--------------------------------------------------
                dmar #CmpRW,ClearByte   ;initialize LINbrcpt counter for in-frame break detection (use the dma to clear the byte)
                dmaw LINbrcpt,#CmpRW
                dmaw LINbrcpt+2,#CmpRW
                mov X,#0
                mov LINbrcpt+4,X
                ;A and B used to move things around or check registers
                ;X (de)count the bit received : from 9 to 0 - the first bit will be the start bit
                mov X,#9

                ;----------------------------------------------------------------------------------------------------------------------
wbit            ;wait for a timeout (bit) or a command from the MLX16 (no edge detection)
                ;the bit can be an ID bit (F1=0) or a regular databyte (F1=1)
                ;A and B are free, X contains the bit (de)counter
                ;------------------------------------------------------------------------------------
                wait Event,TimeOut
                ;check the source that ended the wait
                jtime wbit_to           ;if it's a timeout...
                mov Xtmp,X
                call cmdrcvd            ;else check if it is a command from the Mlx16
                mov X,Xtmp
                jmp wbit
                ;----------------------------------------------------------------------------------------------------------------------
wbit_to         ;check the source of the timeout (and reset the flags / acknowledge the event!)
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                ;get the bit received in C
                rlc B                   ;the bit just received is now in C, B={SplBin,SyncErr,RunErr,x}
                mov F0,C                ;save it into F0
                ;test for errors: if RunErr is set, the mlx4 failed to process the event in time
                ;timeout counter is increased and check at the end of each byte (during the stop bit)
                call BRcorrection

            #IF coTIMEOUT eq cvTO13     ;the timeout check is not needed any more as per LIN spec 2.1
                ;test if this is the ID (F1=0) or just a databyte (F1=1)
                jnf1 tochecked          ;ID byte
                call CheckTimeOut
                jc tochecked            ;timeout ok
                mov C,F0                ;bus level is within C
                jc toerr_nocol          ;if the bus is recessive, do not set the collision flag
                call UpdateZeroCpt
                mov B,LINerrStat        ;x x x x
                or B,#4                 ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                mov LINerrStat,B        ;set the collision flag in order to measure the break properly
toerr_nocol     jmp toerr               ;send timeout error and go back to active
tochecked
            #ENDIF   ;coTIMEOUT eq cvTO13

                ;-------------------------------------------------------------------
                ;check if this is the stop bit ------------------------------------------------------
                msk X,#15               ;does not affect C - X contains the bit (de)counter
                jz stopbit
                jmp rxbrcorrdone        ;is this the stop bit? No, jump to do the zero-bit counting thing
stopbit         ;stop bit
                ;check if the bit is high (F0 should be set)
                jnf0 stopbiter
                ;stop bit correct, prepare the msbi cell and then check the timeout
                ;Baudrate bloc Re-Initialization
                dcom BrCtrl,07h         ;00 00 0111
                                        ;  \  \    \
                                        ;   \  \    Rst[1:0]=11 : Re-Synch mode : reset upon selected edge (see next instruction)
                                        ;    \  nop
                                        ;     Init: nop
                dcom BrCtrl,0Eh         ;00 00 1110
                                        ;      Edg[2:0]=110: Fast Falling Edge
                dcom StatCtrl,30h       ;0 0 11 0000
                                        ;   \  \    \
                                        ;    \  \    nop
                                        ;     \  set Ftfr : fast edges set the traffic bit
                                        ;      nop

                jmp databytrcvd         ;stop bit received correctly
toerr           ;timeout error : send LINtocnt content
                mov B,LINbytcnt         ;get the byte number
                mov LINmessbuf,B        ;store the byte of the collision in LINmessbuf
                mov B,#erTORESP         ;signal an error : reponse timeout
                call errev
                jmp act_init

stopbiter       ;A real stop bit error has been detected, notify MLX16 of this, prepare for frame in frame reception
                ;reset the counter and change Br and IntBr
                dcom PlsCtrl,0A0h       ;1 0 10 0000
                                        ;   \  \    \
                                        ;    \  \    BrCk/OutCk pulse : nop
                                        ;     \  counter : reset and start
                                        ;      capture : nop
                mov B,LINbytcnt         ;get the byte number
                mov LINmessbuf,B        ;store the byte of the collision in LINmessbuf
                clr C                   ;carry used if cvTO13, should be cleared again here
                call UpdateZeroCpt      ;to get the correct LinBrcpt value in act_init
                ;the stop bit wasn't received : it is a zero instead. Maybe from a new break... so go wait for an end-of-break
                jnf1 idstopbiter        ;ID stop bit? (F1=0)
                ;data stop bit error
                ;set corresponding error bit
                mov B,LINerrStat        ;x x x x
                or B,#1                 ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                mov LINerrStat,B
                jmp act_init
idstopbiter     ;ID stop bit error
                ;set corresponding error bit
                mov B,LINerrStat        ;x x x x
                or B,#8                 ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                mov LINerrStat,B
                ;LINmessbuf is already filled with LINbytcnt
                mov B,#erIDSTOP
senderr         call errev              ;send an event to signal the error
                jmp act_init

                ;---------------------------------
rxbrcorrdone    ;do the zero-bit counting thing - A used, B not used
                mov C,F0                ;Bin is in F0
                call UpdateZeroCpt

                ;check if this is the start bit
                cmp X,#9
                jnz regular
                mov A,LINbytcnt         ;check if we are recieving the ID
                cmp A,#15
                jz decount              ;If yes, don't check the start bit since a stop bit error can be pending
                ;start bit, check that the bus is low
                jnf0 decount            ;bus s low, start bit ok
                mov B,#erRX             ;bus is high, start bit not recieved correctly
                mov LINmessbuf,A        ;store byte cnt into the 4st message nibble
                mov X,#erRXSTART
                call errev              ;send an event to signal the error
                jmp act_init
                
regular
                ;regular bit ------------------------------------------------------------------------
                ;this is not the stop bit, put back Bin into C
                mov C,F0
                ;this is not the stop bit, update the buffer (the bit is in C)
                call shift_bytbuf       ;LINbytbuf+1 is in A, LINbytbuf in B, the bit shifted out in C

                ;test if this is the ID (F1=0) or just a databyte (F1=1)
                jf1 decount
                jmp IDbyte              ;this is the ID

decount         ;this is not the ID field, decrement X (bit counter) and keep waiting
                sub X,#1
                jmp wbit

IDbyte          ;We are currently receiving the ID byte (but not the start bit) ---------------------
                ;The ID is 6 bits of data and followed by 2 bits of parity
                ;bits 0 to 5 - ID : compare each bit received with the RAM table, process the parity
                ;bit 6 - parity bit 0 : get the result of the above comparison, if no index is recognized, check the ROM table
                ;                       the local index (if any) is then available in LINindex.
                ;                       If the ID is not recognized (in RAM and ROM), LINindex+1 msb is set
                ;bit 7 - parity bit 1 : check the parity
                ;stop bit
                cmp X,#3                ;compare only the first 6 bits (do not compare the parity bits)
                jnc IDcomp
                ;parity bits - no compare operation
                jmp IDcheckpar

IDcomp          ;check the ID in the RAM table ---
                ;get the offset
                mov A,X
                xor A,#0Fh              ;inverse it (to get the index)
                add A,#1
                and A,#7
                asl A                   ;multiply by 2 (we are working with words and dma!)
                ;put back Bin in C
                mov C,F0                ;restore Bin : the incoming bit has to be in C

                ;Bank #1 -------------------------
                dmar #CmpRW,indx1_tmp   ;load the compare register : CmpRW = indx1_tmp (lsb=0)
                ;before receiving the first bit, indx1_tmp is preset to 0xFF
                ;CmpChk: compare the incoming bit (in C)
                dmar #CmpChk,LIN_IDs shr 4[A]

                ;IdIdx is loaded, save it before handling the next ID bank
                mov B,IdIdx             ;if IdIdx = 8, that means the ID hasn't been recognized
                mov LINindxbk1,B        ;save IdIdx for the bank 1

                ;save the temporary result (bank #1)
                dmaw indx1_tmp,#CmpRW   ;CmpRW

                ;Bank #2 -------------------------
                dmar #CmpRW,indx2_tmp   ;load the compare register : CmpRW = indx2_tmp (lsb=0)
                ;before receiving the first bit, indx2_tmp is preset to 0xFF
                ;CmpChk: compare the incoming bit (in C)
                dmar #CmpChk,LIN_IDs2 shr 4[A]

                ;IdIdx is loaded, save it before handling the next ID bank
                mov B,IdIdx             ;if IdIdx = 8, that means the ID hasn't been recognized
                mov LINindxbk2,B        ;save IdIdx for the bank 2

                ;save the temporary result (bank #1)
                dmaw indx2_tmp,#CmpRW   ;CmpRW
IDcheckpar      ;put back Bin in C
                mov B,F                 ;the incoming bit is in F0, bit 0 of register F
                and B,#1                ;B={000,Bin}

                ;--------------- calculate the parity -------------
                msk X,#1                ;check if the bit we just received is odd (0,2,4 -> Z not set) or even (1,3,5 -> Z set)
                                        ;  X    bit     Z
                                        ; 1001 (9)  start bit   0
                                        ; 1000 (8)  bit 0       0
                                        ; 0111 (7)  bit 1       1
                                        ; 0110 (6)  bit 2       0
                                        ; 0101 (5)  bit 3       1
                                        ; 0100 (4)  bit 4       0
                                        ; 0011 (3)  bit 5       1
                                        ; 0010 (2)  bit 6 (parity bit 0)
                                        ; 0001 (1)  bit 7 (parity bit 1)
                                        ; 0000 (0)  stop bit
                jnz IDbodd
                asl B                   ;if bit is odd B={00,Bin,0}, if bit is even B={000,Bin}
IDbodd          xor B,LINparity         ;LINparity=0001 B={00,Bin,1}
                mov LINparity,B         ;(bit0) LINparity={00,b0,1}
                                        ;(bit1) LINparity={00,b0,~b1}
                                        ;(bit2) LINparity={00,b0 xor b2,~b1}
                                        ;(bit3) LINparity={00,b0 xor b2,~b1 xor b3}
                                        ;(bit4) LINparity={00,b0 xor b2 xor b4,~b1 xor b3}
                                        ;(bit5) LINparity={00,b0 xor b2 xor b4,~b1 xor b3 xor b5}
                                        ;(p0)   LINparity={00,b0 xor b2 xor b4 xor p0,~b1 xor b3 xor b5}
                                        ;(p1)   LINparity={00,b0 xor b2 xor b4 xor p0,~b1 xor b3 xor b5 xor p1}
                                        ;note: ~(a xor b) = ~a xor b

                ;check if this was the last bit of the ID, if it was, check if the ID is correct and do the post-processing
                sub X,#1                ;decrease the bit counter
                switch X
                jmp IDprocess1          ;X = 0 -> go to IDprocess1 (parity ID bit 1)
                jmp IDprocess0          ;X = 1 -> go to IDprocess0 (parity ID bit 0)
                jmp IDbit5              ;X = 2 -> go to IDbit5 (ID bit 5)
                jmp wbit                ;X = 3 -> go to wbit (regular ID bit)
                jmp wbit                ;X = 4
                jmp wbit                ;X = 5
                jmp wbit                ;X = 6
                jmp tst_brfrm           ;X = 7 (first bit)
                jmp wbit                ;X = 8 (start bit)

tst_brfrm
                ;check if it was a break inside frame
                mov A,LINerrStat        ;x x x x
                msk A,#1                ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                jz  jmp_wbit
                ;it was a break inside a frame, reset the flags and signal the error
                mov A,#0
                mov LINerrStat,A

                ;a break within a response frame has been detected, signal the error
                mov B,Btmp              ;get the byte number
                mov LINmessbuf,B        ;store the byte of the collision in LINmessbuf
                mov B,#erBRFRM
                call errev
jmp_wbit
                jmp wbit  
                ;------------------------------------------------------------------------------------
IDprocess1      ;parity bit 1 received : all the bits have been received check the parity
                ;(and save the ID in LINindex)
                ;P0 = ID0 xor ID1 xor ID2 xor ID4
                ;P1 = ~(ID1 xor ID3 xor ID4 xor ID5)
                ;Perform some postprocessing to verify the parity bits. The results of a XOR on all odd and even databits is...
                ;... already available (in LINparity), now XOR bit 1 with all even bits and bit 4 with the odd ones.
                mov A,LINbytbuf         ;A = {ID3 ID2 ID1 ID0}
                mov LINIDtmp,A          ;save the protected ID
                and A,#2                ;              {0, 0, ID1, 0}
                xor A,LINparity         ;LINparity = {0, 0, ID0 xor ID2 xor ID4 xor P0, ~ID1 xor ID3 xor ID5 xor P1}
                                        ;A =         {0, 0, ID0 xor ID1 xor ID2 xor ID4 xor P0, ~ID1 xor ID3 xor ID5 xor P1}
                mov B,LINbytbuf+1       ;B = {P1, P0, ID5, ID4}
                mov LINIDtmp+1,B        ;save the protected ID
                and B,#1                ;              {0, 0, 0, ID4}
                xor A,B                 ;A = {0, 0, ID0 xor ID1 xor ID2 xor ID4 xor P0, ~ID1 xor ID3 xor ID4 xor ID5 xor P1}
                ;A is 0 if the parity is correct
                jnz IDpar_err
                ;the ID was correctly received
                jmp wbit

IDpar_err       ;ID parity error! ----------------
                mov B,#erIDPAR
                call errev              ;send an event to signal an ID parity error
                ;test if the bus is at 0, in that case it can be a collision
                mov B,Stat1             ;read status register 1 {Bin,SplBin,SyncErr,RunErr}
                msk B,#8
                jnz idparer_hi
                ;the bus is low, set the collision bit
                mov A,LINerrStat        ;x x x x
                or A,#4                 ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                mov LINerrStat,A
idparer_hi      ;the bus is high, proceed
                jmp act_init            ;go wait for the next frame

                ;------------------------------------------------------------------------------------
IDprocess0      ;parity bit 0 received - get the arguments of the frame (if it has been recognized)
                ;check if the ID was recognized
                mov A,LINindex+1        ;test if the ID has been recognized (LINindex contains a valid index)
                msk A,#8
                jz restore_indx         ;yes : restore the indexes
                mov X,#1                ;restore the bit counter for parity bit 1
                jmp wbit                ;go wait for parity bit 1
restore_indx    ;restore the indexes, which were calculated during the processing of ID bit 5
                mov A,ParamBuf
                mov X,ParamBuf+1
                or X,#PARAMtableX
                ;MemTab has been set during the reception of bit 5
                mov B,Rom:TableRom[X,A] ;  X A = 0iii iii0
                mov ParamBuf,B          ;save the data in RAM : there are 2 nibbles to get
                add A,#1
                mov B,Rom:TableRom[X,A]
                mov ParamBuf+1,B
                ;arguments saved... wait for the stop bit before processing them
                mov X,#1                ;restore the bit counter for parity bit 1
                jmp wbit                ;go wait for parity bit 1

                ;------------------------------------------------------------------------------------
IDbit5          ;bit 5 received - the index should be ready, prepare the ROM table access - check if this is a special ID (3C, 3D, 3E, 3F)
                mov B,LINbytbuf+1       ;{bit5, bit4, bit3, bit2}
                cmp B,#15
                mov A,LINframeflag
                jnz nospclid            ;jnz test with cmp B,#15, two last lines has been swapped to enable jnz to nospclid (not more than 15 lines)
                or  A,#1                ;x x x x
                                        ; \ \ \ \
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                mov LINframeflag,A
nospclid        mov A,LINbytbuf         ;{bit1, bit0, x, x}
                shift B,10h             ;insert a 0
                rrc A
                shift B,10h             ;insert a 0
                rrc A                   ;keep only bits 5 to 0, the index is ready in A and B
                mov LINindex+1,B        ;save the ID
                mov LINindex,A
                ;for ID Filtering Mode, check the result of the filtering
                ;check if the ID was recognized : if an ID was recognized in RAM, LINindxbk1 or LINindxbk2 contains its
                ;   index. If they both contain 8, the ID hasn't been recognized.
                mov A,LINindxbk1        ;check if the ID has been recognized in the bank 2 (index 0 to 7)
                msk A,#8
                jz idinram              ;id has been recognized in the bank 1
                mov A,LINindxbk2        ;check if the ID has been recognized in the bank 1 (index 8 to 7)
                msk A,#8
                jnz othrbank            ;check other places (ROM...)
                ;bank 2 ---
                add A,#8                ;add the offset to the index
                ;the index is ready in A (the high nibble should be 0)
idinram         mov LINindex,A          ;save the index (replace the ID)
                mov B,#0                ;if the ID has been recognized in RAM, the Index is between 0 and 15
                mov LINindex+1,B        ;LINindex high nibble shoulb be 0
                jmp getframearg

othrbank        ;the ID was not recognized in the RAM table ----------------------------
                ;check the ROM table (order : nibble1, nibble0, nibble3, nibble2)
                ;ROM table INDXtbl located in F00, configure Txtab[5:2] (in MemTab 2)
                dcom MemTab,INDXtable   ;11 1111 00
                                        ;       \  \
                                        ;        \  Txtab[1:0] = 00 (ored with arom[7:6])
                                        ;         Txtab[5:2] = 1111 (replace arom[11:8])
                ;Content of the ROM
                ; xx  xxxxxx
                ;   \       \
                ;    \       local index
                ;     00 (0) : ID defined in ROM - index 10h to 3Fh
                ;     01 (4) : ID defined during initialization (in RAM) - index 0 to 15
                ;     10 (8) : ID defined in EEPROM - need to be loaded in RAM during initialization
                ;     11 (C) : other / not used (default)
                mov A,LINindex          ;ID in [X,A] (without the parity bits)
                mov X,LINindex+1        ;shift the ID, the last two bits of A indicate the nibble of the ROM data
                set C                   ;set C to select the odd nibble
                rlc A
                rlc X
                or X,#INDXtableX         ;set X msb with lower adress bit of indxtable
                mov B,Rom:TableRom[X,A] ;get the high nibble : xx xx
                                        ;                       \   \
                                        ;                        \   local index
                                        ;                        00 (0) : ID defined in ROM - index 10h to 3Fh
                                        ;                        01 (4) : ID defined during initialization (in RAM) - index 0 to 15
                                        ;                        10 (8) : ID defined in EEPROM - need to be loaded in RAM during initialization
                                        ;                        11 (C) : other / not used (default)
                msk B,#0Ch
                jz idinrom              ;check if the ID has been defined in ROM
                ;the ID was not recognized anywhere
                mov B,LINindex+1
                or B,#8                 ;1xxx : indicates that the ID is not defined
                mov LINindex+1,B
                mov X,#2                ;restore the bit counter for parity bit 0
                jmp wbit                ;go wait for parity bit 0

idinrom         ;the ID is defined in ROM : get the index
                mov LINindex+1,B        ;B = 00xx
                                        ;        \
                                        ;         page (1, 2 or 3 - 0 being reserved for RAM indexes)
                                        ;B = 0000 if the ID was defined in RAM (see idinram)
                sub A,#1                ;prepare to get the low nibble of the ROM table to get the complete index
                mov A,Rom:TableRom[X,A] ;low nibble
                mov LINindex,A          ;store the index

getframearg     ;prepare the ROM table indexes (A and X) to get the arguments
                mov X,LINindex+1        ;high part of the index, the low part is in A -> X A = 00ii iiii
                asl A                   ;shift to the left
                rlc X
                ;the index is ready, there is only one table with one byte for each index
                dcom MemTab,PARAMtable  ;11 1111 01
                                        ;       \  \
                                        ;        \  Txtab[1:0] = 01 (ored with arom[7:6]) - same address as PARAMtable1
                                        ;         Txtab[5:2] = 1111 (replace arom[11:8])
                ;save A and X in ParamBuf and ParamBuf+1. These two registers are free and will be used at the next...
                ;... step (when retrieving the arguments)
                mov ParamBuf,A          ; X A = 0iii iii0
                mov ParamBuf+1,X
                mov X,#2
                jmp wbit

databytrcvd     ;--------------- all bits have been received - and the stop bit was received --------
                ;if we were receiving the ID, change F0
                ;test if it is the ID byte (F1=0) or just a databyte (F1=1)
                jnf1 IDcorrect
                jmp nextbyt

IDcorrect       ;The ID has been received correctly (with the stop bit)
                ;in case the frame was correctly processed, save the current baudrate if it hasn't been done yet
                mov A,LINbrflag         ;x x x x
                msk A,#4                ; \ \ \ \
                                        ;  \ \ \ edge to be processed
                                        ;   \ \ not used
                                        ;    \ baudrate detected
                                        ;     auto baudrate on every frame
                jnz IDbrsaved           ;if the baudrate has already been set, don't do it again
                or A,#4                 ;if not, indicates that the baudrate has been detected
                mov LINbrflag,A
                dmaw LINbrNom,#Br       ;save the baudrate to the nominal Br
                mov A,PrescA            ;get the prescaler
                mov LINprescNom,A       ;save the prescaler
                ;check if the frame should be processed
                mov A,LINframeflag      ;x x x x
                msk A,#4                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                jz IDbrsaved
                ;discard the frame
                jmp act_init

IDbrsaved       or F10,#2               ;prepare to get regular data bytes (F1=1)
                ;check if the ID was recognized
                mov X,LINindex+1        ;test if the ID has been recognized (LINindex contains a valid index)
                msk X,#8                ;1xxx : indicates that the ID is not defined
                jz arg_process          ;ID recognized, proceed with the arguments
                ;send the unrecognized ID as a mark
            #IF coDEBUGMARK eq cvON
                xdcom #dcID             ;mark instruction
                xdma LINbytbuf          ;send the ID
            #ENDIF
                jmp act_init

arg_process     ;process the argument
                ;ParamBuf+1
                ;Content: xxx x
                ;            \ \
                ;             \ not used
                ;              000 : receive
                ;              001 : transmit
                ;              010 : groupe (not implemented, discard)
                ;              011 : (error)
                ;              100 : ask the application
                ;              101 : discard
                ;              110 : hook (not implemented, discard)
                ;              111 : (error)
                mov B,ParamBuf+1
                shift B,0h              ;if needed, get the bit0 in C with the correct shift
                cmp B,#1
                jz arg_transmit         ;001 : transmit data
                jc arg_receive          ;000 : receive data
                jmp act_init            ;more than 1, it's an error, go back to active

arg_receive     mov A,#3                ;(0, 0, rx/tx, enabled) - 0 = TX, 1 = RX
                jmp get_parambuf0

arg_transmit    ;reset the data ready flag (F0)
                mov F10,#0
                ;check if we are in continous or in programing mode
                mov A,LINflashStatus    ;x x x x
                msk A,#3                ; \ \ \ \
                                        ;  \ \ \ programming mode
                                        ;   \ \ continuous frames coming up
                                        ;    \ not used
                                        ;     not used
                
                jnz tx_send_ev          ;continous or programing is enabled, do not reset data_ready bit
                ;reset data_ready and discard bits before asking for MSrqst in general case
                mov A,LINframeflag
                and A,#3
                mov LINframeflag,A
tx_send_ev      ;check if there is already some data in the frame buffer, if not send an event
                mov A,LINframeflag      ;x x x x
                msk A,#8                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                jnz txsent              ;don't generate MESSrqst event
                ;Ask immediately the application for data to allow a faster response
                mov A,#evMESSrqst       ;send a request for data
                call setev_mess
txsent          mov A,#1                ;(0, 0, rx/tx, enabled) - 0 = TX, 1 = RX

get_parambuf0   ;ParamBuf
                ;Content: x xxx     for receive and transmit messages
                ;          \   \
                ;           \   length : 0 = 1 bytes... 7 = 8 bytes) <- for receive/transmit
                ;            checksum : O = chk13, 1 = chk20 <- for receive/transmit
                mov B,ParamBuf
                ;store the checksum type (be careful: LINparam contains only the...
                ;... valid checksum type, the other arguments are wrong!)
                mov LINparam,B          ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ Not Used
                                        ;   \ \ ID recognized in Index table
                                        ;    \ 0=TX, 1=RX
                                        ;     checksum : 0=regular (without ID field), 1=enhanced (with ID field)
                and B,#7                ;get the length
                add B,#1                ;the length is coded from 0 to 7 but in reality it is from 1 to 8
                mov LINmesslen,B        ;store the length

                ;get the message timeout
                sub B,#1                ;prepare the index
                rlc B                   ;shift the length to use it as an index for the ROM table access
                add B,#1
                mov X,Rom:TO14tbl shr 2[B]  ;get the high nibble
                mov LINtoref+1,X
                sub B,#1                ;increase the index to get the low nibble
                mov X,Rom:TO14tbl shr 2[B]  ;get the low nibble
                mov LINtoref,X

                ;timeout value is ready
                ;test and set the direction flag                    <-------------------------------------
                mov B,LINparam          ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ Not Used
                                        ;   \ \ ID recognized in Index table
                                        ;    \ 0=TX, 1=RX
                                        ;     checksum : 0=regular (without ID field), 1=enhanced (with ID field)
                and B,#8                ;reset all the arguments but the checksum
                rl A                    ;(0, rx/tx, enabled, 0)
                or B,A                  ;(checksum, rx/tx, enabled, 0)
                mov LINparam,B          ;update the argument register

                ;all the arguments are updated except the master response (not supported yet)       <---

                ;check the checksum type (regular/enhanced) (LINparam is in B)
                rlc B                   ;C=1 -> enhanced checksum
                jc enhancdck            ;optional!
regularck       ;regular checksum: reset the checksum register
                dmar #CmpRW,ClearByte   ;use the dma to clear the byte
                jmp init_chksum

enhancdck       ;enhanced checksum: update the checksum register EXCEPT if the ID is between 3C and 3F, which always has a regular checksum
                dmar #CmpRW,LINbytbuf   ;use the dma to copy LINbytbuf to LINchksum
init_chksum     dmaw LINchksum,#CmpRW
                jmp updtbytcnt

nextbyt         ;a data byte has been received
                ;get the next byte!
                ;before that, get the buffer if this is the first byte and store the byte - it is currently in {LINbytbuf+1,LINbytbuf}
                ;a message is being received, get the buffer
                mov B,LINbytcnt
                msk B,#0Fh              ;check if this is the first byte (does not affect B)
                jz buffer_check
                jmp cmplenght
buffer_check    ;this is the first byte, check if the buffer is free
                ;if the buffer is not free, the Mlx16 still reading it and the Mlx4 should not modify it
                mov A,LINstatus         ; x x x x
                msk A,#2                ; \ \ \ \
                                        ;  \ \ \ LIN bus activity
                                        ;   \ \ 0 : buffer free - 1 : buffer not free
                                        ;    \ not used
                                        ;     event overflow occured
                jz buffer_free
buffer_full     ;the buffer is not free, send an error
                mov B,#erRXOVR
buffer_error    call errev
                ;
                ;check if we should overwrite the buffer or cancel the reception
                ;
                jmp act_init            ;cancel the reception

buffer_free     
                ;if auto sleep command detection is enabled, check if the ID is 3C and if the first byte is 00
                mov A,LINframeflag      ; x x x x
                msk A,#1                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                jz cmplenght            ;this is not ID 3C
                ;check the first byte
                mov X,LINbytbuf
                cmp X,#0
                mov X,LINbytbuf+1
                cmpcz X,#0
                jnz cmplenght           ;the first byte is not 00
                ;set the sleep command received flag
                or  A,#2                ; x x x x
                mov LINframeflag,A      ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit

cmplenght       cmp B,LINmesslen        ;check if the message has been received completely
                jz updtbytcnt           ;if yes, do not store the checksum
                ;clear data ready flag as we are writing to the frame buffer
                mov A,LINframeflag      ;x x x x
                and A,#7                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                mov LINframeflag,A
                ;copy the byte to the frame buffer
                ;byte number in B, data in LINbytbuf/LINbytbuf+1
                asl B                   ;get only the high nibble
                mov A,LINbytbuf         ;low nibble
                mov data:LINframe shr 4[B],A
                add B,#1                ;high nibble
                mov A,LINbytbuf+1
                mov data:LINframe shr 4[B],A

updtbytcnt      ;update the byte counter
                mov X,LINbytcnt
                add X,#1
                mov LINbytcnt,X
                ;was it the ID field?
                jnz updtchksum          ;no
                ;yes (then, depending on the checksum kind, the ID is already in memory)
                ;initialize the bit counter for the time out (LIN 2.0 spec)
                dmar #CmpRW,ClearByte   ;use the dma to clear the bytes
                dmaw LINtocnt,#CmpRW    ;clear LINtocnt and LINtocnt+1
                ;check if this is a TX
                mov A,LINparam          ;x x x x
                msk A,#4                ; \ \ \ \
                                        ;  \ \ \ Not Used
                                        ;   \ \ ID recognized in Index table
                                        ;    \ 0=TX, 1=RX
                                        ;     checksum : 0=regular (without ID field), 1=enhanced (with ID field)
                jnz ntx
                jmp TXreq               ;proceed to transmit

updtchksum      ;still receiving data bytes, update the checksum buffer
                mov B,LINbytbuf
                mov A,LINbytbuf+1

                ;the number of bytes received is already in X
                sub X,#1                ;subtract one for the checksum
                cmp X,LINmesslen        ;compare to the message length
                jz checkck              ;if X=0, the byte juste received is the checksum

                call UpdateChkSum       ;if not, update the checksum

ntx             ;send a mark and go wait for the start bit of the next byte
            #IF coDEBUGMARK eq cvON
                xdcom #dcDATA           ;mark instruction
                xdma LINbytbuf          ;send the byte received
            #ENDIF
                ;enable the sleep timer in case the start bit never comes
                call sleep_tmr
                ;the start bit wait is already initialized (for faster processing) go directly to the wait
                jmp wstart
checkck         ;the checksum has just been received, check it
                add B,LINchksum
                addcz A,LINchksum+1
                ;the checksum should be FF, check that
                add B,#1
                addcz A,#0
                jz message_ok           ;check if the checksum + 1 equals 0
message_bad     ;send an error event
                mov B,#erCKSUM          ;checksum error
                call errev              ;send the event
                jmp act_init            ;wait for the next break

message_ok      ;check if this is a sleep command from the master
                mov A,LINframeflag      ;x x x x
                msk A,#2                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                jz message_send
                ;this is a go-to-sleep command from the master : free the buffer
                mov A,LINstatus         ; x x x x
                and A,#13               ; \ \ \ \
                                        ;  \ \ \ LIN bus activity
                                        ;   \ \ 0 : buffer free - 1 : buffer not free
                                        ;    \ not used
                                        ;     event overflow occured
                mov LINstatus,A
                mov X,#slMST            ;signal that the sleep mode is entered with a go-to-sleep command from the master
                jmp gotosleep           ;if this is a sleep command, go to the sleep state
message_send    ;send a message to the application to say that a message has been received
                ;set the buffer occupied flag
                mov A,LINstatus
                or A,#2
                mov LINstatus,A
                ;send the index or the ID (which index has to be sent for special IDs ???)
                mov A,#evMESSrcvd
                call setev_mess
mess_sent       ;go back to the wait state (wait for a new break)
                jmp act_init

;----- start of the TX area --------------------------------------------------------------------------
;Variables used Content / Local         Value when entering Value at the end
;-----------------------------------------------------------------------------------------------------
;A  none
;B  none
;X  none
;F0 bit sent (for read back)    none, reset after entering
;F1 data ready (if F1 = 1)  none, reset after entering
;   half bit flag : F1 = 0 for the first half of a bit, F1 = 1 for the second half
;LINtocnt   timeout counter     none, reset after entering
;LINmesslen message length      message length (1-8)
;LINtoref   timeout value (theorical)   none, initialized   response time available to the application - in half-bits!
;LINoptions1            x x xx
;              \  \
;               \  header stop bit length (for Slave to Master messages) : 0, 0.5, 1, 1.5 + 1 stop bits
;                stop bits between databytes during TX : 0 or 1 + 1 stop bit -> 1 or 2
;LINbrcpt   0-counter, for break    none        number of successive 0s received
;LINstopbit half stop bit counter / local   none
;LINbytcnt  byte counter (used also in RX)  none
;LINchksum  checksum        0 (regular CK) or ID (enhanced checksum)
;LINtxbytlen    byte length (with the stop bits)    none
;-----------------------------------------------------------------------------------------------------
TXreq           ;a request to tranmit data bytes on the LIN bus has been received -------------------
                ;LIN param :

                ;initialize the bit and byte counters
                mov X,#15               ;initialize the pointer to the data to be sent (LINbytcnt)
                mov LINbytcnt,X         ;X will be incremented in GetByte, start with 15 (15+1=0)
                mov X,#0

                ;reset the break counter for break in frame
                dmar #CmpRW,ClearByte   ;use the dma to clear the bytes
                dmaw LINbrcpt,#CmpRW    ;clear LINbrcpt and LINbrcpt+1
                dmaw LINbrcpt+2,#CmpRW  ;clear LINbrcpt+2 and LINbrcpt+2
                mov LINbrcpt+4,X        ;clear LINbrcpt+4
                ;check the current level on the bus
                mov A,Stat1             ;read status register 1 (and clear the flags): {Bin,SplBin,SyncErr,RunErr}
                msk A,#8                ;check if the bus is high
                jnz TXncol
                ;the bus is already low -> collision! A break may have already started!

TXcol           ;signal a collision
                ; in LINmessbuf is stored the byte counter (if == 15, collision during TXreq. first byte is 0)
                ; in X is stored the bit number:
                ; 0 -> stop bit of the last byte
                ; 1 -> start bit of the current byte
                ; 2 -> first data bit
                ;...
                ; 9 -> data bit 7 
                ; A -> first stop bit of the current byte (in case 2 stop bits have been requested by the application)
                mov B,LINbytcnt         ;get the byte
                mov LINmessbuf,B        ;store the byte of the collision in LINmessbuf
                mov B,#erTXCOL
                call errev              ;send the event
                ;signal that a TXCOL error occured before entering the end of break procedure
                mov B,LINerrStat        ;x x x x
                or B,#4                 ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                mov LINerrStat,B
                ;reset the data ready flag and set the buffer free
                mov B,LINstatus         ; x x x x
                and B,#13               ; \ \ \ \
                                        ;  \ \ \ LIN bus activity
                                        ;   \ \ 0 : buffer free - 1 : buffer not free
                                        ;    \ not used
                                        ;     event overflow occured
                mov LINstatus,B
                jmp act_init_er         ;go wait for the end of the break

TXncol          ;TX initialization
                dcom BrCtrl,05h         ;00 00 0101
                                        ;  \  \    \
                                        ;   \  \    Rst[1:0]=01: reset with BrMatch
                                        ;    \  nop
                                        ;     Init: nop
                dcom BrCtrl,08h         ;00 00 1000
                                        ;      Edg[2:0]=000: no  edge
                dcom StatCtrl,6Bh       ;StatCtrl
                                        ;0 1 10 1011
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=011 : LTimeOut driven Br Flag and IntFlag <- measure half bits for stop bit length
                                        ;     \  reset Ftfr
                                        ;      reset internal flags
                dcom InpCtrl,0B0h       ;InpCtrl
                                        ;1 0 11 0000
                                        ;   \  \    \
                                        ;    \  \    no change
                                        ;     \  RbErr=11: select SplRbErr in StatReg1
                                        ;      Rst: no change
                ;reset the timeout counter
                dmaw LINtocnt,#CmpRW    ;clear LINtocnt and LINtocnt+1

                ;get the message maximum response time
                mov A,LINmesslen        ;get the length [1-8]
                sub A,#1                ;prepare the index [0-7]
                rlc A                   ;shift the length to use it as an index for the ROM table access
                mov B,Rom:TO10tbl shr 2[A]  ;get the low nibble
                mov LINtoref,B
                add A,#1                ;increase the index to get the high nibble
                mov B,Rom:TO10tbl shr 2[A]  ;get the high nibble
                mov LINtoref+1,B
                ;timeout value is ready, now calculate the real length of the message
                mov A,LINoptions1       ;x x xx
                                        ; \ \  \
                                        ;  \ \  header stop bit length (for Slave to Master messages) : 0, 0.5, 1, 1.5 + 1 stop bits
                                        ;   \ stop bits between databytes during TX : 0 or 1 + 1 stop bit -> 1 or 2
                                        ;    not used
                and A,#4                ;test the stop bits between databytes
                jz TXtocalcdone         ;one stop bit, the calculation is done
                mov X,#1                ;one additional stop bit
TXtocalcdone    add X,#10               ; X value is before 0 or 1 depending on if we want 2 or 1 stop bit
                mov LINtxbytlen,X

                ;stop bit counter
                mov X,LINoptions1       ;get the number of half stop bits
                and X,#3                ;(only the last two bits are significant)
                add X,#2                ;a half stop bit has already been received, wait for the next one and then for the additional ones (bem 070807)
                mov LINstopbit,X        ;a half stop bit has already been received, wait for the next one and then for the additional ones

                ;check if the data is ready
                mov A,LINframeflag      ;x x x x
                msk A,#8                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                mov F10,#0              ;reset data ready flag (f1)
                jz TXstart
                mov F10,#2              ;signal that the data is ready: F1 = 1, F0 = 0

TXstart         ;TX initialization done ----------
                ;enable bit error detection and set its sampling point at the end of the bit
                ;set the configuration register
                ;wait for: - the response from the application indicating that the data is ready
                ;          - the end of the stop bit
                ;          - an edge, indicating someone is transmitting (then generate an error and go check if this is a break)
                ;if RunErr is set, the mlx4 failed to process the event in time
                wait Event,EvExt,TimeOut
                ;check the source that ended the wait
                jxev TXw_ev             ;external event (edge?) : error!
                jtime TXw_to            ;timeout from the baudrate, can be BrFlag or IntFlag
                ;command from the Mlx16 ----------
                call cmdrcvd            ;command from the Mlx16 : process it
                mov X,LINframeflag      ;x x x x
                msk X,#8                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                jnz jmp_TXdatardy       ;data is ready to transmit
                ;the data is not yet available for transmit, check if message should be discarded
                msk X,#4
                jz  TXnotrdy            ;not a discard command, wait for the next command
                jmp act_init            ;message should be discarded
jmp_TXdatardy   jmp TXdatardy           ;data is ready to transmit

TXw_ev          ;external event : collision ------
                ;check the source of the event (and reset the flags / acknowledge the event!)
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                msk B,#8                ;test Bin
                jz TX_jmptxcol          ;the bus is low : somebody is driving it! Error!
                                        ;signal a collision with byte=0 / bit=0 (default values)
                ;the bus is high...
TXnotrdy        jmp TXstart             ;go back to the waiting state

TXw_to          ;timeout -------------------------
                ;count the (half) stop bits (decrease the counter) and increment the timeout counter

                ;check the source of the timeout (and reset the flags / acknowledge the event!)
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                ;check if these was some traffic on the bus (collision) and if the bus is recessive
                msk A,#8
                jnz TX_jmptxcol
                ;check if the bus is recessive
                and B,#8
                jnz TWw_tonocol
                ;collision !
TX_jmptxcol     jmp TXcol
TWw_tonocol     ;check the source of the timeout
                msk A,#1                ;BrEv set if BrFlag set and ToEn[0] set or if IntFlag set and ToEn[1] set
                                        ;here ToEn = 011 -> BrEv set by BrFlag and IntFlag
                jnz TXw_took
                jmp TXstart             ;incorrect timeout, keep waiting
TXdatardy       ;data is ready
                ;check LINstopbit, if not zero, update the data ready flag (F0)
                mov X,LINstopbit
                cmp X,#0
                jnz TXrdyflag
                jmp TXcmd               ;X = 0, start the transmit
TXrdyflag       or F10,#2               ;X not 0, update the TX flag (F1)
                jmp TXstart             ;wait for the next command

TXw_took        ;increment the bit counter for the time out and check that the timeout hasn't been reached
                call CheckTimeOut
                jc TXnto_error
                ;timeout error
                jmp toerr

TXnto_error     ;the timeout hasn't been reached
                mov X,LINstopbit        ;get the number of (half) stop bits
                ; test X to see if it is a half bit or a complete bit (X not modified)
                msk X,#1
                jnz TXwbrcorrdone
                call BRcorrection

TXwbrcorrdone   sub X,#1                ;decrease the stop bit counter
                addcz X,#0              ;if X was already 0, do not overflow
                mov LINstopbit,X        ;update the counter
                cmp X,#0
                jnz TX_jmpstart         ;minimum stop bit length not met, keep waiting
                jf1 TXcmd               ;minimum stop bit duration met, check if the data is ready (F1 = 1)
TX_jmpstart     jmp TXstart             ;data is not ready (F1 = 0) or X > 0, keep waiting

TXcmd           ;data ready and stop bit minimum length reached --------------------
                ;the data is in LINframe...
                ;restore full bit timeouts, collisions are checked automatically with RbErr, as set in InpCtrl
                ;clear data ready flag
                mov A,LINframeflag      ;x x x x
                and A,#7                ; \ \ \ \
                                        ;  \ \ \ special ID received (3C, 3D, 3E or 3F)
                                        ;   \ \ sleep command received (ID 3C + first byte 00)
                                        ;    \ discard the frame
                                        ;     frame data ready for transmit
                mov LINframeflag,A

                dcom PlsCtrl,90h        ;1 0 01 0000
                                        ;   \  \    \
                                        ;    \  \    BrCk/OutCk pulse : nop
                                        ;     \  counter : stop
                                        ;      capture : nop
                ;set the MSBi so the propagation time is measured
                dcom StatCtrl,68h       ;StatCtrl
                                        ;0 1 10 1000
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=000 : LTimeOut not driven
                                        ;     \  reset Ftfr
                                        ;      reset internal flags
                ;prepare IntBr
                mov A,LINbr             ;get the baudrate
                mov B,LINbr+1
                shift B,10h             ;divide the baudrate by 2
                rrc A
                mov LINtmp,A
                mov LINtmp+1,B
                dmar #IntBr,LINtmp+1    ;write it to IntBr
                ;send the start bit immediately and measure the propagation time (if coPROPMEAS if enabled)
                call GetByte
                call UpdateChkSum       ;C is 0
                mov X,#2
                mov Atmp,X
                mov X,#0
                ;set the MSBi so the propagation time is measured
                dmaw LINtmp+1,#IntBr    ;read IntBr to reset capt_flag
                dcom BrCtrl,0Ah         ;00 00 1010
                                        ;          Edg[2:0]=010: slow falling edge
                dcom StatCtrl,20h       ;0 0 10 0000
                                        ;   \  \    \
                                        ;    \  \    nop
                                        ;     \  reset Ftfr : slow edges set the traffic bit
                                        ;      nop
                dcom CkCtrl,65h         ;01 10 0101
                                        ;  \  \    \
                                        ;   \  \    BrPls[1:0] = 01
                                        ;    \  RptCap = 0
                                        ;     set Capt on next BrCk
                dcom OutCtrl,01h        ;000 00 001
                                        ;   \  \   \
                                        ;    \  \   X set, Y loaded with CoutCpu
                                        ;     \  Z, Cpz and NxZ unchanged
                                        ;      M and N unchanged
                dcom PlsCtrl,0A4h       ;1 0 10 0100
                                        ;   \  \    \
                                        ;    \  \    BrCk/OutCk pulse : generate a BrCK pulse
                                        ;     \  counter : reset and start
                                        ;      capture : nop
                ;set the MSBi so the propagation time is measured
                ;enable the timeout after the BrCk pulse
                dcom StatCtrl,6Bh       ;StatCtrl
                                        ;0 1 10 1011
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=011 : LTimeOut driven Br Flag - or IntBr
                                        ;     \  reset Ftfr
                                        ;      reset internal flags
                wait EvExt,TimeOut      ;wait for the falling edge - or for a timeout
                jtime TXer_delay
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                dmaw LINtmp+1,#IntBr    ;read the captured counter value (propagation delay)
                mov A,LINbr             ;get the baudrate
                mov B,LINbr+1
                shift B,10h             ;divide the baudrate by 2
                rrc A
                add A,LINtmp            ;add the propagation delay
                addcz B,LINtmp+1
                mov LINtmp,A
                mov LINtmp+1,B
                dmar #IntBr,LINtmp+1    ;write it to IntBr

                ;disable the timeout after the IntBrCk pulse
                dcom StatCtrl,09h       ;StatCtrl
                                        ;0 0 00 1001
                                        ;           \
                                        ;            load ToEn[2:0]=001 : LTimeOut driven Br Flag - or IntBr
                ;disable the edge detector
                dcom BrCtrl,08h         ;00 00 1000
                                        ;          Edg[2:0]=000: no edge detection
                jmp TXchkbus

TXer_delay      ;delay error: the propagation delay is too long
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                msk B,#8                ;check if the bus is recessive
                jnz TXer_delay_ok       ;the bus is recessive, it's actualy a delay error
                jmp TXcoldtctd          ;the bus is dominant, it's probably a collision error
TXer_delay_ok
                dcom OutCtrl,1Bh        ; X set, Y set, Z, Cpz and NxZ set, M and N unchanged
                mov X,#erCRASHTX
                mov B,#erCRASH
                call errev              ;send an error to the application
                mov B,#stDISC
                mov LINst,B             ;update the state
                jmp dscnct_sc           ;go in the disconnected state without sending and event

TXnextbyte      call GetByte            ;get the data byte (LINbytcnt is incremented (by 1)
                mov X,#0                ;reset the bit counter
                jc jmp_TXend            ;if C is set, the transmit is finished
                jnz TXdata              ;if Z is set, the checksum should be sent
                ;the checksum is in A and B, invert all the bits
                xor B,#15
                xor A,#15
            #IF coDEBUGMARK eq cvON
                mov LINdbg,B            ;save the checksum for xdma
                mov LINdbg+1,A
            #ENDIF
                call GB_SaveData        ;save the checksum in LINbytbuf
                jmp TXstartbit
jmp_TXend       jmp TXend
TXdata          ;regular data byte: update the checksum
                call UpdateChkSum
TXstartbit      clr C                   ;start bit
TXsendbit       dcom OutCtrl,01h        ;000 00 001
                                        ;   \  \   \
                                        ;    \  \   X set, Y loaded with CoutCpu
                                        ;     \  Z, Cpz and NxZ unchanged
                                        ;      M and N unchanged
TXwaitbit       wait TimeOut, Event     ;wait before sending the next bit
                jtime TXchkbus
                call cmdrcvd            ;command from the MLX16
                jmp TXwaitbit

                ;timeout : begining of a bit time, prepare the next bit
TXchkbus        call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                
                ;check if a collision occured
                msk B,#4                ;SplBin is actually RbErr
                jnz TXcoldtctd
                
                ;update the 0 counter, verify if the bit just sent was recessive or dominant
                mov A,Atmp              ;Atmp contains bit sent shifted to the left
                shift A,010h            ;shift the register two times
                shift A,010h               
                call UpdateZeroCpt      ;Carry contains the bit sent, update the zero counter

                call BRcorrection       ;X is not modified

                add X,#1                ;no collision detected, increment the bit counter
                cmp X,LINtxbytlen       ;check if the current byte has been completely sent
                jz TXstopbit
                ;if not, shift the next bit out, and fill the buffer with 1s (C is set because of the cmp instruction above)
                call shift_bytbuf       ;shift LINbytbuf+1 and LINbytbuf, the bit to send is in C
                mov A,Atmp              
                shift A,084h            ;save the bit to sent into Atmp use later for zero counting
                mov Atmp,A
                jmp TXsendbit

TXcoldtctd      ;collision!
                ;set all the outputs flip-flops
                clr C                   ;a collision has been detected necessary because a dominant state has been detected
                call UpdateZeroCpt      ;count dominant state one more time
                dcom OutCtrl,1Bh        ;000 11 011
                                        ;   \  \   \
                                        ;    \  \   X set, Y set
                                        ;     \  Z, Cpz and NxZ set
                                        ;      M and N unchanged
                jmp TXcol               ;signal the collision

TXstopbit       
                mov A,Atmp              ;we are sending the start bit (dominant), fill Atmp with it
                clr C
                shift A,084h
                mov Atmp,A
                ;the stop bit is being sent, get the next byte
            #IF coDEBUGMARK eq cvON
                xdcom #dcDATATX         ;mark instruction
                xdma LINdbg             ;send the byte sent
            #ENDIF
                jmp TXnextbyte

TXend           ;set all the outputs flip-flops to recessive (1)
                dcom OutCtrl,1Bh        ;000 11 011
                                        ;   \  \   \
                                        ;    \  \   X set, Y set
                                        ;     \  Z, Cpz and NxZ set
                                        ;      M and N unchanged
TXendwait       ;wait one more time to check the stop bit
                wait TimeOut, Event
                jtime TXendcheck
                call cmdrcvd            ;command from the MLX16
                jmp TXendwait

TXendcheck      ;check the the bus (and reset the flags / acknowledge the event!)
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                ;check if a collision occured
                msk B,#4                ;SplBin is actually RbErr
                jz TXend_ok             ;no collision
                jmp TXcoldtctd          ;collision error

TXend_ok        ;TX done
                ;data ready flag has been reset at the beginning of transmission. no need to reset it here

                ;check if the end of the transmition needs to be signaled or if continous frame are coming
                mov X,LINflashStatus    ;x x x x
                msk X,#3                ; \ \ \ \
                                        ;  \ \ \ programming mode
                                        ;   \ \ continuous frames coming up
                                        ;    \ not used
                                        ;     not used
                jnz SendEndEvent        ;continuous frames enabled
                mov X,LINevendTX        ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ End TX event must to be sent
                                        ;   \ \ unused
                                        ;    \ unused
                                        ;     unused
                msk X,#1                ;is EndEvent enabled?
                jz FrameEnd             ;no
SendEndEvent    ;send EndEvent to the mlx16
                mov A,#evENDtx
                call setev_mess
FrameEnd        jmp act_init            ;go wait for a new frame

;-------------- SHORT -------------------------------------------------------------------------------------------------
; If a dominant level is present on the LIN bus for a time too long to be a break, then it is considered to be a short.
; An error is generated.
; The state machine goes automatically to the ACTIVE state
BusShort        ;Baudrate bloc Re-Initialization : wait for a rising edge, no timeout from the MSBi cell
                dcom BrCtrl,07h         ;00 00 0111
                                        ;  \  \    \
                                        ;   \  \    Rst[1:0]=11 : Re-Synch mode : reset upon selected edge (see next instruction)
                                        ;    \  nop
                                        ;     Init: nop
                dcom BrCtrl,09h         ;00 00 1001
                                        ;      Edg[2:0]=001: Slow Rising Edge
                dcom StatCtrl,68h       ;StatCtrl
                                        ;0 1 10 1000
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=000 : LTimeOut not driven
                                        ;     \  reset Ftfr : slow edge set the traffic bit
                                        ;      reset internal flags: Traffic, TraErr, flags related to CaptErr, TimeOut, EvExt
                ;enable the sleep timer
                call sleep_tmr

                ;make sure the bus is still dominant
                mov B,Stat1             ;read status register 1 (and clear the flags): {Bin,SplBin,SyncErr,RunErr}
                msk B,#8
                jz short_enter          ;this was a false alert, don't signal it to the application
                jmp act_init_er_no_sig  ;do not signal the state change if the bus short state was not entered
short_enter     ;enter the stSHORT state
                mov X,LINst             ;retrieve the current state (it will be passed as an argument of the error erSHORT)
                mov B,#stSHORT
                mov LINst,B             ;update the current state
                ;don't signal the state change because this is done with the error erSHORT
                ;signal an error to the Mlx16
                mov B,#erSHORT
                call errev
                ;stop the counter to save power
                dcom PlsCtrl,90h        ;1 0 01 0000
                                        ;   \  \    \
                                        ;    \  \    BrCk/OutCk pulse : nop
                                        ;     \  counter : stop
                                        ;      capture : nop

                ;wait here for - a rising edge (goto ACTIVE)
                ;              - a sleep timeout (goto SLEEP with reason slTODOM)       <<<<<<<<<<<<<<<<< or slTO
                ;              - a command from the Mlx16
short_wait      wait Event,TimeOut,EvExt;wait for a next event
                jxev short_done         ;external event coming from the MSBi (edge)
                jtime short_to          ;timeout from the sleep timer
                call cmdrcvd            ;command from the MLX16
                jmp short_wait

short_done      ;go to the ACTIVE state
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                msk B,#8
                jz jmp_short_wait       ;if the bus is still low, keep waiting
                ;otherwise exit the short state and go into active state
                ;if slave was in sleep mode and issued a Short error (too long wakeup pulse), it should go back to active anyway because:
                ; - Standard not asking to the node to chek too long wakup pulse to exit sleep mode
                ; - slave already transmit a chst active to Mlx16 and then should go to it
                jmp active

short_to        ;check if this is a timeout due to the sleep timer
                call CheckSleepTmr      ;check the status of the sleep timer
                jnc jmp_short_wait      ;if C is not set, the counter did not overflow, keep waiting
                ;timeout counter overflows, goto sleep
                mov A,#slTODOM          ;signal that the sleep mode is entered after a sleep timer timeout
                mov B,#stSLEEP          ;set the new state: sleep
                call chst               ;signal that the state changed
                call sleep_tmr_dis      ;disable the sleep timer
                mov A,#0                ;we entered in sleep with a dominant state
                mov Atmp,A
                jmp sleep_entered       ;jump to sleep mode
jmp_short_wait  jmp short_wait          ;go back to event waiting

;-------------- wbr_init -----------------------------------------------------------------------------------
wbr_init_start  ;* Initialization of the MSBi cell for Break reception
                ;A and B are used, X is used
                ;use LINtmp for baud-rate calculation but do not need to keep it
                ;use LINbr/LINbr+1 (read only)
                ;MSBi cell : initialize Br and IntBr - the prescaler is already initialized in the initialization part
                ;            CkCtrl : reset Capt (do not allow a capture now)
                ;            BrCtrl : reset and start the counter upon reception of a slow falling edge
                ;            StatCtrl : disable edge/timeout detection... no event should come from the MSBi cell
                ;                       reset internal flags: Traffic, TraErr, flags related to CaptErr, TimeOut, EvExt
                ;* Registers : reset the timeout counter LINtocnt
                ;  Initialize the LINparam registers : LINparam = 0xC : enhanced checksum, RX frame, ID not recognized, n/a
                ;Reconfigure MSBii Counter
                mov F10,#0              ;clear F1 and F0
                mov X,#0                ;initialize X

                ;Reset the auto baudrate counters
                dmar #CmpRW,ClearByte   ;use the dma to clear the bytes
                dmaw LINbrhicpt,#CmpRW  ;clear LINbrhicpt and LINbrhicpt+1
                dmaw LINbrhicpt+2,#CmpRW    ;clear LINbrhicpt+2 and LINbrhicpt+3

                ;check if LINbrcpt should be cleared
                mov A,LINerrStat        ;x x x x
                msk A,#13               ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                jnz brcpt_nclr
                dmaw LINbrcpt,#CmpRW    ;clear LINbrcpt and LINbrcpt+1
                dmaw LINbrcpt+2,#CmpRW  ;clear LINbrcpt+2 and LINbrcpt+3
                mov LINbrcpt+4,X        ;clear Linbrcpt+4
brcpt_nclr      mov A,#14
                mov A,Rom:ConstantTbl2 shr 2[A]  ; A = MinPresc
                mov PrescA,A            ;init the prescaler
                mov A,#0
                mov LINbrtmp+2,A
                mov LINbrLSBs,A
                mov LINbrcorr,A

                ;Baud-rate initialization
                mov A,LINbr+1           ;divide LINbr by 2 and store it in LINtmp for dma access
                shift A,10h
                mov LINtmp+1,A
                mov A,LINbr
                rrc A
                mov LINtmp,A
                dmar #Br,LINtmp         ;generate pulses to count the break length every half bit time

                ;the baudrate is initialized, initialize the MSBi cell -----------------------------------------------
                dcom InpCtrl,0EFh       ;1 1 10 1 1 1 1
                                        ;   \ \    \ \ \
                                        ;    \ \    \ \ EnFdb=1: enable fast debounce (BusIn is debounced on a 2/3 majority vote scheme)
                                        ;     \ \    \ Fbin=1: flag set (Bin = high debounced BusIn))
                                        ;      \ \    AutoDb=1: flag set (DbCk devided by the 4 msb's of the Br register)
                                        ;       \ RbErr=10: flag reset (IntSplBin selected)
                                        ;        Rst=1
                dcom CkCtrl,85h         ;10 00 0101
                                        ;  \  \    \
                                        ;   \  \    BrPls : nop (IntMatch -> InBrCk, BrMatch -> BrCk)
                                        ;    \  RptCap : nop
                                        ;     Capt : reset (do not allow a capture now)
                ;stop the counter to save power
                dcom PlsCtrl,90h        ;1 0 01 0000
                                        ;   \  \    \
                                        ;    \  \    BrCk/OutCk pulse : nop
                                        ;     \  counter : stop
                                        ;      capture : nop

                ;Set IntBr value to half Br - use LINtmp (calculated above)
                dmar #IntBr,LINtmp+1    ;IntBr = Br / 2
                ;LINtmp is not used anymore in this routine after that
                ;read IntBr to reset capt_flag
                dmaw LINtmp+1,#IntBr

                ;baud-rate bloc : reset and start the counter upon reception of a slow falling edge
                dcom BrCtrl,06h         ;00 00 0110
                                        ;      Rst[1:0]=10: reset upon Edg[2:0] (slow falling edge)
                dcom BrCtrl,0Ah         ;00 00 1010
                                        ;      Edg[2:0]=010: Slow Falling Edge

                ;StatCtrl : disable edge/timeout detection... no event should come from the MSBi cell
                dcom StatCtrl,28h       ;0 0 10 1000
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=000 : LTimeOut not driven
                                        ;     \  set Ftfr : slow edges set the traffic bit
                                        ;      don't reset internal flags

                ;the MSBi cell is initialized
                ;initialize the registers used for auto-addressing - even if auto addressing mode is not set yet
                ;ROM table AUTOADDtable, configure Txtab[5:2] (in MemTab 2), AUTOADDtable value is set in lincst.asm
                dcom MemTab,AUTOADDtable    ;11 1111 00
                                        ;           \  \
                                        ;            \  Txtab[1:0] = 00 (ored with arom[7:6])
                                        ;             Txtab[5:2] = 1111 (replace arom[11:8])
                mov A,#0
                mov AutoAddEv,A         ;index of the ROM table
                mov X,#AUTOADDtableX
                mov B,Rom:TableRom[X,A] ;get the first value for the counter
                mov AutoAddCpt,B

                ;Check if byte counter can be cleared
                mov B,LINerrStat        ;x x x x
                msk B,#13               ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                jnz wbr_msbi_init_end

                ;reset the timeout register
                dmaw LINtocnt,#CmpRW    ;use the dma to clear the byte

                ;initialize the LINparam registers
                ;enhanced checksum, RX frame, ID not recognized, n/a
                mov A,#0Ch
                mov LINparam,A          ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ Not Used
                                        ;   \ \ ID recognized in Index table
                                        ;    \ 0=TX, 1=RX
                                        ;     checksum : 0=regular (without ID field), 1=enhanced (with ID field)
wbr_msbi_init_end
                jmp sleep_tmr

;-------------- WAKE state (stWKUP) -----------------------------------------------------------------------------------
; command(s) :
; power-down : not allowed
; next state(s) : DISCONNECTED (command), ACTIVE (bus), SLEEP (timer)
; action(s) :    In this state the task will attempt to wake up the bus by issuing a wake-up pulse and susequently monitor the bus for
;                activity. The Wake-Up request is issuing by forcing the bus to the dominant state for 250us to 5ms.
;   At  1kbps, Tbit =  1ms, 5 * Tbit =   5ms
;   At 20kbps, Tbit = 50us, 5 * Tbit = 250us -> a wake-up pulse will always be 5 bits long, regardless of the baudrate.
;   If no frame header is issued within 150ms from the wake up request (ending edge), 2 more requests can be sent. Then
;   a fourth one after 1.5 seconds.
;   UPDATED 070115 : 1.5 seconds after the 3rd pulse, an event is sent to the application. The decision to send another
;                set of pulses is left to the application.
; register(s) : A follow the wake-up cycle
;   ___     _______________________     _______________________     ________ //  _________
;      |___|         150ms         |___|          150ms        |___|       1.5s           |
;   A =  0            1              2             3             4          5             6 : send a slWKUPabort
wakeup_sc       ;entering point after state change command (don't do the call to chst)
                ;disable ANA wkup if we come from deep sleep state               
                call dis_ana_wkup
                ;initialize the MSBi cell for break detection
                call wbr_init
                ;Slow Clock Disabled (ask for fast clock)
                dcom System,88h         ;1xxx 10xx
                                        ;       \
                                        ;        SlowCk = 0 (FstCkRq = 1 : request PLL)
                ;A will now be used to follow the wake-up cycle (see schematic above)
                mov B,#0
                mov Btmp,B
wakeup_pulse    ;MSBi cell initialization
            #IF coDEBUGMARK eq cvON
                xdcom #dcWAKEUP         ;mark instruction
            #ENDIF
                ;generate pulses to count the pulse length every Tbit
                mov A,LINprescNom       ; OZH 20130129
                mov PrescA,A
                dmar #Br,LINbrNom
                ;Baudrate bloc
                dcom BrCtrl,05h         ;00 00 0101
                                        ;  \  \    \
                                        ;   \  \    Rst[1:0]=01: reset with BrMatch
                                        ;    \  nop
                                        ;     Init: nop
                dcom BrCtrl,08h         ;00 00 1000
                                        ;      Edg[2:0]=000: no edge
                ;StatCtrl
                dcom StatCtrl,49h       ;StatCtrl
                                        ;0 1 00 1001
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=011 : LTimeOut driven Br Flag
                                        ;     \  nop
                                        ;      reset internal flags
                ;CkCtrl
                dcom CkCtrl,06h         ;00 00 0110
                                        ;  \  \    \
                                        ;   \  \    BrPls : BrMatch -> BrCk
                                        ;    \  RptCap : nop
                                        ;     Capt : nop
                ;send the start bit immediately
                dcom OutCtrl,12h        ;000 10 010
                                        ;   \  \   \
                                        ;    \  \   Y reset, X unchanged
                                        ;     \  Z reset (start bit)
                                        ;      M and N unchanged
                ;reset the counter
                dcom PlsCtrl,0A0h       ;1 0 10 0000
                                        ;   \  \    \
                                        ;    \  \    BrCk/OutCk pulse : nop
                                        ;     \  counter : reset and start
                                        ;      capture : nop

                ;reset the flags so if a Timeout occured before the reset of the timer, it is discarded
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}

                ;initialize X (number of Tbit for a pulse)
                mov X,#12
                mov X,Rom:ConstantTbl shr 2[X]

wakeup_tbit     ;X contains the number of Tbit for a pulse
                wait Event,TimeOut      ;wait before sending the next bit
                ;check the source that ended the wait
                jtime wakeuppls_to      ;TimeOut from the counter
                call cmdrcvd            ;Event from the Mlx16: get the command (X not modified)
                jmp wakeup_tbit         ;keep waiting

wakeuppls_to    ;check the source of the timeout (and reset the flags / acknowledge the event!)
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                ;...
                ;check if a bit error has been detected
                ;...
                sub X,#1
                jz wakeup_done
                ;prepare the next bit
                dcom OutCtrl,02h        ;000 00 010
                                        ;   \  \   \
                                        ;    \  \   Y reset, X unchanged
                                        ;     \  Z unchanged
                                        ;      M and N unchanged
                ;keep waiting
                jmp wakeup_tbit

wakeup_done     ;put a recessive level on the bus
                dcom OutCtrl,1Bh        ;000 11 011
                                        ;   \  \   \
                                        ;    \  \   X and Y set
                                        ;     \  Z unchanged
                                        ;      M and N unchanged
                ;increment B
                mov B,Btmp
                add B,#1
                mov Btmp,B
                ;check if this is a pause (B odd) or a pulse (B even)
                msk B,#1
                jnz wakeup_pause        ;this is a pause
                ;this is a pulse, ot the sequence is finished, in that case send a slWKUPabort
                cmp B,#6
                jz wakeup_end
                jmp wakeup_pulse

wakeup_end      ;end of the wake-up sequence. No activity was detected, signal it to the application
                mov X,#slWKUPabort
                jmp gotosleep

wakeup_pause    ;wait for the rising edge of the bus, with a timeout of 1 bit time
                ;if at the end of the bit time the bus is still low, then there is someone else driving the bus, go try to detect a break
                dcom BrCtrl,09h         ;00 00 1001
                                        ;          Edg[2:0]=001: slow rising edge
                wait EvExt,TimeOut      ;wait for the rising edge, or for a timeout
                jtime wakeup_toedge
                ;if this is not a timeout, it was a rising edge, prepare to receive a break
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                jmp wakeup_nobreak

wakeup_toedge   ;check if the bus is still dominant (break)
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                msk B,#8
                jz wakeup_break
                jmp wakeup_nobreak
wakeup_break    ;the bus is dominant, there is a break ?
                mov X,#0                ;reset LINbrcpt +3/+4 
                mov LINbrcpt+3,X
                mov LINbrcpt+4,X
                mov A,LINbrNom             ;put LINbr in A/B
                mov B,LINbrNom+1
                ;put the 6 Tbit in the bit counter (6 half bits = 12)
                ;6Tbit = 4Tbit + 2Tbit
                asl A                   ;multply by 2
                rlc B
                rlc X
                mov LINbrcpt,A          ;store it in Linbrcpt
                mov LINbrcpt+1,B
                mov LINbrcpt+2,X
                asl A                   ;multply by 2 again
                rlc B
                rlc X
                add A,LINbrcpt          ;add it to LINbrcpt
                addcz B,LINbrcpt+1
                addcz X,LINbrcpt+2
                mov LINbrcpt,A          ;store it in Linbrcpt
                mov LINbrcpt+1,B
                mov LINbrcpt+2,X        ;save the value
                                        ;5 for the pulse and 1 for the timeout while waiting for an edge
                ;start the counter
                dcom PlsCtrl,0A0h       ;1 0 10 0000
                                        ;   \  \    \
                                        ;    \  \    BrCk/OutCk pulse : nop
                                        ;     \  counter : reset and start
                                        ;      capture : nop
                mov A,#slWKUPbreak
                mov B,#stACT            ;signal a change to that ACTIVE state
                call chst
                
                ;Set collision bit to make sure auto-baudrate part is handling correct
                mov A,LINerrStat        ;x x x x
                or A,#4                 ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                mov LINerrStat,A
                jmp act_init            ;go wait for the end of the break

wakeup_nobreak  ;initialize the MSBi cell for break detection
                call wbr_init

                mov B,Btmp
                cmp B,#5                ;A < 5 : C
                                        ;A = 5 : no C, Z
                                        ;A > 5 : no C
                jz wakeup_long
                mov A,#4                ;index for WuPauseCnt_Val (wake-up pause value)
                mov X,#5                ;index for WuPausePre_Val (prescaler)
                jmp wakeup_send

wakeup_long     ;long pause
                mov A,#6                ;index for WuSeqCnt_Val (wake-up long pause value)
                mov X,#7                ;index for WuSeqPre_Val (prescaler)
                ;initialise the MSBi cell (timer)
                ;Wake Up Request -> 150 ms between two pulses (pulses between 250us and 5 ms wide) then 1.5 s before another pulse sequence
                ;Pause Length : 150ms - used with the Sleep Timer : WuPauseCnt_Val, WuPausePre_Val
                ;Sequence Pause Length : 1.5s - used with the Sleep Timer : WuSeqCnt_Val, WuSeqPre_Val
wakeup_send     ;wake-up timer
                mov A,Rom:ConstantTbl2 shr 2[A] ;WuSeqCnt_Val or WuSeqCnt_Val: wake-up pause value
                mov X,Rom:ConstantTbl2 shr 2[X] ;WuPausePre_Val or WuSeqPre_Val
                                        ;1 xxx
                                        ; \   \
                                        ;  \   prescaler (WuSeqPre_Val)
                                        ;   timer enabled
                mov B,#SleepCntIdx      ;sleep timer value index
                mov SleepCnt[B],A
                mov B,#SleepPreIdx      ;sleep timer prescaler index
                mov SleepPre[B],X
                dcom Timer,33h          ;enable and restart the sleep timer timeout (060821)

wakeup_wait     ;wait...
                waitpd Event,TimeOut,EvExt
                ;Event from the Mlx16 (command)
                ;EvExt from the MSBi (edge)
                ;TimeOut from the counter
                ;check the source that ended the wait
                jxev wakeup_ev          ;EvExt from the MSBi (edge!)
                jtime wakeup_to         ;TimeOut from the counter
                call cmdrcvd            ;Event from the Mlx16: get the command
                jmp wakeup_wait         ;keep waiting

wakeup_to       ;TimeOut ! -----------------------
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                ;check that the bus is not low
                msk B,#8
                jz wakeup_ev
                ;check the status of the timers
                mov A,SleepStat         ;Sleep Timer Status (read and clear)
                and A,#15
                jz wakeup_oth           ;if it's not the Sleep Timer, check the others
                ;go to the wakeup loop
                jmp wakeup_done

wakeup_oth      ;it was the Message Timer that timed out...
                jmp wakeup_wait

wakeup_ev       ;Edge ! --------------------------
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                ;if this is really a wake-up, change state and the go measure the break...
                mov A,#slWKUPresp       ;signal that there was some activity on the bus
                mov B,#stACT            ;load the new state
                call chst               ;do the change
                jmp act_init           ;go measure the break

;-------------- SLEEP state (stSLEEP) ---------------------------------------------------------------------------------
; command(s) :
; power-down : allowed
; next state(s) : DISCONNECTED (command), ACTIVE (bus), WAKE-UP (command)
; action(s) :   allow the processor to go into low speed mode, wake-up on bus activity or Mlx16 event (WakeUp or Disconnect)
;   Sleep mode is entered on a sleep command send by the Mlx16, ona diagnostic master request frame with
;   the first byte equal to 0, or if the bus is inactive for more than 4 seconds
;   a wake-up request is a dominant pulse longer thant 150us, the cell must be ready within 100ms
;   X contains the reason why we went to sleep

gotosleep       ;enter to sleep state
                ;Xtmp is used to remember what is the source of the sleep state transition
                ;It can be slTO, slTOMESS, slMST, slAPP, slWKUP
                mov Xtmp,X
                mov A,Stat1             ; save the bus state into Atmp for later
                mov Atmp,A
                ;don't disable the sleep timer, it is used for analog access
                ;initialise the MSBi cell to wait for 250us - (Br counter at 50us)
                mov A,#7
                mov A,Rom:ConstantTbl shr 2[A]
                mov LINtmp+1,A
                mov A,#6
                mov A,Rom:ConstantTbl shr 2[A]
                mov LINtmp,A

                ;Initialize Br register on the LIN cell to measure the length of an eventual wake-up pulse
                dmar #Br,LINtmp
                ;also load IntBr
                dmar #IntBr,LINtmp+1

                ;Initialize the prescaler
                mov A,#4
                mov A,Rom:ConstantTbl shr 2[A]
                mov PrescA,A

sleep_init      ;start a 250us timer (before entering the deep sleep mode)
                dcom StatCtrl,69h       ;StatCtrl
                                        ;0 1 10 1001
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=001 : Br drives the TimeOut
                                        ;     \  reset Ftfr : slow edges set the traffic bit
                                        ;      reset internal flags: Traffic, TraErr, flags related to CaptErr, TimeOut, EvExt
                dcom BrCtrl,07h         ;00 00 0111
                                        ;  \  \    \
                                        ;   \  \    Rst[1:0]=11: reset with BrMatch or Edge (type of edge set in wbr_init)
                                        ;    \  nop
                                        ;     Init: nop
                dcom BrCtrl,0Ah         ;00 00 1010
                                        ;      Edg[2:0]=010: Slow Falling Edge
                dcom PlsCtrl,0A0h       ;1 0 10 0000
                                        ;   \  \    \
                                        ;    \  \    BrCk/OutCk pulse : nop
                                        ;     \  counter : reset and start
                                        ;      capture : nop
                ;250us is 5 Tbit@20kbps, initialize the counter - add one period to finish the stop bit
                mov X,#5
                mov X,Rom:ConstantTbl shr 2[X]  ;SleepDelay
                add X,#1

sleep_wait      ;wait for a timeout while monitoring the bus and listening to the application
                wait Event,TimeOut,EvExt
                jxev sleep_event        ;external event coming from the MSBi (edge)
                jtime sleep_to          ;timeout from the timer
                call cmdrcvd            ;command from the MLX16
                jmp sleep_wait

sleep_event     ;an edge occured, start the measurement
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
sleep_testbus   and B,#8
                jnz jmp_sleep_wait      ;if the bus is not 0, return to wait
                jmp sleep_pulse         ;else start measuring the pulse

sleep_to        ;test the analog timer
                call CheckAnaTmr
                jz sleep_tomsbi
                jmp sleep_wait
sleep_tomsbi    ;decrement the counter
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                sub X,#1
                jz sleep_took
                and A,#4                ;check if an event occured during the timeout
                jnz sleep_missev
                msk B,#1                ;check if an event occured while reading the registers
                jnz sleep_missev
                jmp sleep_testbus       ;in case an event occured, check the bus level
jmp_sleep_wait  jmp sleep_wait
sleep_missev    ;an event was missed, check the bus
                jmp sleep_testbus

sleep_took      ;the 250us have been reached, enter the sleep state (if we weren't in that state yet)
                mov X,LINst
                cmp X,#stSLEEP
                jz sleep_entered        ;already in sleep state (wrong pulse received)
                mov A,Xtmp              ;signal how the sleep mode is entered
                mov B,#stSLEEP          ;set the new state: sleep
                call chst               ;signal that the state changed

sleep_entered   ;check what is the way to get out of sleep mode:
                ; - reception of a frame on the LIN bus : soft wake-up, the Mlx4 has to monitor the bus and cannot be powered down
                ; - reception of a wake-up signal : hard wake-up, WKUP sent by the periphery, the Mlx4 can be put into deep sleep
                mov A,LINoptions2       ;x x x x
                                        ; \ \ \ \
                                        ;  \ \ \ state change masked (1)
                                        ;   \ \ not used
                                        ;    \ deep sleep (1) / light sleep (0)
                                        ;     not used
                msk A,#4
                jnz sleep_deep
                jmp sleep_light         ;go to sleep light

sleep_deep      ;In deep sleep, the wake up of the Mlx4 is done by the periphery (WKUP signal). The Mlx4 DOES NOT monitor the bus !
                dcom StatCtrl,48h       ;StatCtrl
                                        ;0 1 00 1000
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=000 : TimeOut not driven
                                        ;     \  nop
                                        ;      reset internal flags: Traffic, TraErr, flags related to CaptErr, TimeOut, EvExt
                dcom PlsCtrl,90h        ;1 0 01 0000
                                        ;   \  \    \
                                        ;    \  \    BrCk/OutCk pulse : nop
                                        ;     \  counter : stop
                                        ;      capture : nop
                dcom BrCtrl,04h         ;00 00 0100
                                        ;  \  \    \
                                        ;   \  \    Rst[1:0]=00 : no reset
                                        ;    \  nop
                                        ;     Init: nop
                ; Digital Bus Flags : reset sleepb
                dcom Flags,40h          ;010 x xxxx
                                        ;   \
                                        ;    sleepb = 0
                ; Slow Clock Enable
                dcom System,8Ch         ;1xxx 11xx
                                        ;       \
                                        ;        SlowCk = 1 (no PLL request)
                                        ;The clock will go in slow mode only if the mlx16 also request it to

sleep_chkwup    ;then check the wake-up pin from the analog before enabling the wake-up signal
                mov A,AnalogStat        ;{EXCP, WKUP, PHYSTAT[1:0]}
                and A,#4
                jz sleep_enwkup
                ;wake-up pulse detected, check the bus to see if it has changed
                ;get the Status registers from the MSBi cell
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                and A,#8                ;check the Traffic bit
                jnz jmp_dis_sleep       ;traffic bit is set : there was an edge
                jmp sleep_chkwup        ;traffic bit not set, check the wake-up signal again
jmp_dis_sleep   jmp dis_sleep

sleep_enwkup    ;no wake-up pulse, enable wake-up : wkup signal is a source ot timeout for the mlx4
                dcom Flags,98h          ;100 11 xxx
                                        ;      \
                                        ;       enwkup = 1
sleep           ;deep sleep loop
                waitpd Event,TimeOut    ;(0Bh) - power-down allowed
                jtime wu_pulse          ;check the wake-up signal coming from the periphery
                call cmdrcvd            ;a command has been received : get the command
                jmp sleep               ;loop (if this was a state change, the program goes directly to the selected state)

sleep_light     ;light sleep, keep monitoring the bus
                dcom StatCtrl,68h       ;StatCtrl
                                        ;0 1 10 1000
                                        ;   \  \    \
                                        ;    \  \    load ToEn[2:0]=000 : TimeOut not driven
                                        ;     \  reset Ftfr : slow edges set the traffic bit
                                        ;      reset internal flags: Traffic, TraErr, flags related to CaptErr, TimeOut, EvExt
                dcom PlsCtrl,90h        ;1 0 01 0000
                                        ;   \  \    \
                                        ;    \  \    BrCk/OutCk pulse : nop
                                        ;     \  counter : stop
                                        ;      capture : nop
                dcom BrCtrl,06h         ;00 00 0110
                                        ;  \  \    \
                                        ;   \  \    Rst[1:0]=10 : edge cause a reset
                                        ;    \  nop
                                        ;     Init: nop
                dcom BrCtrl,0Ah         ;00 00 1010
                                        ;      Edg[2:0]=010: Slow Falling Edge
                
                mov A,Atmp              ;reload the bus state at the beginning
                and A,#8                ;check if the bus state was recessive
                jz wait_sleep_light     ;it was dominant, drop
                mov B,Stat1             ;check the actual bus state
                and B,#8                
                jnz wait_sleep_light    ;the actual state is recessive, continue
                jmp sleep_pulse         ;the actual state is dominant while recessive at the beginning, go directly to pulse detection

                ;--- deep sleep mode -------------
wu_pulse        ;test the analog timer
                call CheckAnaTmr
                jz wu_test
                jmp sleep
wu_test         ;a wake-up pulse has been detected : check the wake-up pin from the analog
                mov A,AnalogStat        ;{EXCP, WKUP, PHYSTAT[1:0]}
                and A,#4                ;check if it was really a wake-up pulse
                jnz dis_sleep
false_wu        ;the bus is high : that was probably a glitch. Clear the flags
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
            #IF coDEBUGMARK eq cvON
                xdcom #dcSLEEP          ;mark instruction
            #ENDIF
                jmp sleep

wait_sleep_light                        ;wait for an external event of a command from the Mlx16 - power down not allowed
                wait Event,EvExt,TimeOut
                jxev sleep_light_pls    ;external event coming from the MSBi (edge)
                jtime sleep_light_to
                call cmdrcvd            ;command from the MLX16 : get the command
                jmp wait_sleep_light    ;loop (if this was a state change, the program goes directly to the selected state)

sleep_light_to  ;test the analog timer
                call CheckAnaTmr
                jmp wait_sleep_light

sleep_light_pls ;check if the bus is really low, go to the active state to measure the first pulse, which can be a break
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                and B,#8
                jz sleep_pulse          ;the bus is low, measure the pulse length
                jmp wait_sleep_light

dis_sleep       ;disable wake-up : wkup signal is not a source ot timeout for the mlx4 anymore
                call dis_ana_wkup
                jmp pulse_valid

                ;---------------------------------
sleep_pulse     ;a falling edge has been detected while waiting for the 250us before entering the sleep mode
                ;reset the timeout counter, which will be used to measure the length of the pulse
                dmar #CmpRW,ClearByte   ;use the dma to clear the byte
                dmaw LINtocnt,#CmpRW

                ;re-init the MSBi cell
                dcom StatCtrl,49h       ;StatCtrl
                                        ;0 1 00 1001
                                        ;   \       \
                                        ;    \       load ToEn[2:0]=001 : Br drives the TimeOut
                                        ;     reset internal flags
                ;wait for a rising edge
                dcom BrCtrl,09h         ;00 00 1001
                                        ;      Edg[2:0]=001: Slow Rising Edge
                ;no reset on rising edge
                dcom BrCtrl,05h         ;00 00 0101
                                        ;       Rst[1:0]=01 : reset on BrMatch
                ;reset the counter
                dcom PlsCtrl,0A0h       ;1 0 10 0000
                                        ;   \  \    \
                                        ;    \  \    BrCk/OutCk pulse : nop
                                        ;     \  counter : reset and start
                                        ;      capture : nop
                dcom CkCtrl,86h         ;10 00 0110
                                        ;  \  \    \
                                        ;   \  \    BrPls : nop (halfMatch -> InBrCk, BrMatch -> BrCk)
                                        ;    \  RptCap : nop
                                        ;     Capt : reset (do not allow a capture now)

pulse_wait      ;wait for a timeout while monitoring the bus and listening to the application
                wait Event,TimeOut,EvExt
                jxev jmp_pulse_chkedg   ;external event coming from the MSBi (edge)
                jtime pulse_inc_chk     ;timeout from the timer
                call cmdrcvd            ;command from the MLX16
                jmp pulse_wait
jmp_pulse_chkedg
                jmp pulse_chkedg

pulse_inc_chk   ;a Br event occured, increment the counter and check the length
                call CheckAnaTmr        ;check if Timeout comes from Sleep timer
                jz pulse_inc_cnt        ;wake up from MSBI, increment the counter  
                jmp pulse_wait          ;wake up from sleep timer, go back to wait
pulse_inc_cnt
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                mov A,LINtocnt
                mov B,LINtocnt+1
                add A,#1
                addcz B,#0
                mov LINtocnt,A
                mov LINtocnt+1,B

                mov X,#8
                cmp A,Rom:ConstantTbl shr 2[X]
                mov X,#9
                cmpcz B,Rom:ConstantTbl shr 2[X]
                jnz pulse_chkmax        ;check the maximum pulse length
                mov A,#2                ;x x x x
                mov LINerrStat,A        ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
pulse_chkmax    ;check if the bus hasn't been dominant for too long
                mov X,#10
                cmp A,Rom:ConstantTbl shr 2[X]
                mov X,#11
                cmpcz B,Rom:ConstantTbl shr 2[X]
                jc pulse_chkedg         ;dominant length ok, check the edge
                ;the bus has been dominant for too long, this is a short
                jmp BusShort

pulse_chkedg    ;check if there was a rising edge
                call ReadStatus         ;A = {Traffic,LEvExt,Ovf,BrEv}, B = {Bin,SplBin,SyncErr,RunErr}
                msk B,#8
                jnz pulse_end           ;the bus is high, it's actualy an edge
                jmp pulse_wait          ;it wasn't an external event... back to the waiting loop

pulse_end       ;a rising edge has been detected, check if it was long enough to be a wake-up pulse
                mov B,LINst             ;prepare state checking
                mov A,LINerrStat        ;x x x x
                msk A,#2                ; \ \ \ \
                                        ;  \ \ \ data stop bit error detected
                                        ;   \ \ wake-up pulse detected (do not handle the break)
                                        ;    \ collision error detected
                                        ;     ID stop bit error detected
                jnz pulse_valid_check   ;pulse is valid, check if we are in sleep or init
                ;the pulse has really ended but it was too short, go back to wait for the 250us delay if we were not already in the sleep state
            #IF coDEBUGMARK eq cvON
                xdcom #dcSLEEP          ;mark instruction
            #ENDIF
                cmp B,#stSLEEP
                jz jmp_sleep_light      ;we were in the sleep state, keep waiting for a pulse or a command
                jmp sleep_init          ;we were not in the sleep state, wait for 250us
jmp_sleep_light jmp sleep_light         ;go back to sleep light
pulse_valid_check                       ;the pulse is valid, check if we are in regular sleep mode           
                cmp B,#stSLEEP          ;check if we were in sleep init 
                jz pulse_valid         ;valid pulse has been received during sleep
                ;we were in sleep init, generate an wakeup error
                mov B,#erWKUPINIT
                call errev
                jmp act_init            ;go to the active state
pulse_valid     ;the wake-up pulse was valid, signal it to the application
                mov A,#slWKUP           ;signal that there was a wake-up pulse
                mov B,#stACT            ;load the new state
                call chst               ;do the change
                ;clear the wakeup flags and go wait for a new break
                mov A,#0
                mov LINerrStat,A
                jmp act_init            ;go to the active state

;--------------- program end ------------------------------------------------------------------------------------------
prog_end        end
