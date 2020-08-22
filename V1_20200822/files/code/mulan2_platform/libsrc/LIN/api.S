;
; Copyright (C) 2005-2015 Melexis N.V.
;
; Software Platform
;

; Include the file that contains all the constants, used by both the mlx16 and the mlx4
#include <ioports.h>
#include "lincst.h"

    .global _LinResp
    .global _LinCmnd
    .global _LinMess
    .global _LinMess2
    .global _LinProtectedID
    .global _LinFrame

    .extern _LinStatus

    .section .special
    .org 0x0

_LinResp:           .word   0
_LinCmnd:           .word   0
_LinMess:           .word   0
_LinMess2:          .byte   0
_LinProtectedID:    .byte   0
_LinFrame:          .byte   0,0,0,0,0,0,0,0


#if defined (HAS_MLX4_CMD_ACK_TIMEOUT)
#define MLX4_CMD_ACK_TIMEOUT    1000       /* command acknowledgement timeout */
#endif /* HAS_MLX4_CMD_ACK_TIMEOUT */

;------------------------------------------------------------------------------
    .section .mlx_text,"xa",@progbits

/* Software and Flash Loader version */
    .global _ml_LinModuleVersion

/* Bus Timing */
    .global _ml_SetBaudRate                ;was ml_SendBaudRate
    .global _ml_SetAutoBaudRateMode

/* Software and Hardware options */
    .global _ml_SetSlewRate
    .global _ml_SetOptions                 ;was ml_Options, now includes ml_SlewRate

/* Task Control and Task Status*/
    .global _ml_Connect
    .global _ml_Disconnect

#if (STANDALONE_LOADER == 0)
    .global _ml_WakeUp
    .global _ml_GotoSleep
#if (LIN_PIN_LOADER != 0)
    .global _ml_SwitchToFast               ;requires switch "coFASTFLASH equ cvON" in MLX4
#endif /* LIN_PIN_LOADER */
    .global _ml_GetState

/* Message Management */
    .global _ml_AssignFrameToMessageID     ;was ml_SetLinId
    .global _ml_EnableMessage              ;was ml_MessageValid
    .global _ml_DisableMessage             ;was ml_MessageValid

/* Auto-addressing */
#if defined (HAS_LIN_AUTOADDRESSING)
    .global _ml_AutoAddressingConfig       ;was ml_AutoAddCfg
#endif /* HAS_LIN_AUTOADDRESSING */

#endif /* !STANDALONE_LOADER */

/* Data Transfer */
    .global _ml_DiscardFrame               ;was ml_Discard
    .global _ml_ContFrame
    .global _ml_ReleaseBuffer
    .global _ml_ReleaseBufferProg
    .global _ml_DataReady


;-----------------------------------------------------------------------------------------------------
; PUBLIC FUNCTIONS
;-----------------------------------------------------------------------------------------------------
_ml_ReleaseBuffer:  ;ml_Status ml_ReleaseBuffer(void)
    ;------------------------------------------------------------------------------------
    ;parameters : ml_bool blProgMode
    ;return : ml_Status
    mov al,#(0x20 | pcRELBUF)               ; a = 0000 0000 0010 1111
    jmp SendCmnd

_ml_ReleaseBufferProg:  ;ml_Status ml_ReleaseBuffer(ml_bool blProgMode)
    ;------------------------------------------------------------------------------------
    ;parameters : ml_bool blProgMode
    ;return : ml_Status
    and al,#1                               ; a = 0000 0000 0000 000t   ; t: blProgMode argument
    asl al,#2                               ; a = 0000 0000 0000 0t00
    asl al,#2                               ; a = 0000 0000 000t 0000
    or  al,#pcRELBUF                        ; a = 0000 0000 000t 1111
    jmp SendCmnd


;-----------------------------------------------------------------------------------------------------
_ml_DataReady:  ;ml_Status ml_DataReady(ml_bool DataTransmittedEvent)
    ;------------------------------------------------------------------------------------
    ;Transfer the data from LinMessBuffer (8 bytes) into the common area of the
    ;RAM so it can be handled by the LIN task
    ;Signal that the data buffer is full
    ;Update
    ;parameters: DataTransmittedEvent in a, if enabled an evENDtx is sent to the application
    ;after the data has been transmitted on the LIN bus
    ;no return value (always 0)

                                            ; a = 0000 0000 0000 000t   ; t: DataTransmittedEvent argument
    mov ah,al                               ; a = 0000 000t 0000 0000
    mov al,#(0x10 | pcSETFRAME)             ; a = 0000 000t 0001 0101   ; 1: DataReady

    ;push the data into the shared ram area and signal that the buffer is full
    ;always push 8 bytes, it's simpler that way and LinMessBuffer is dedicated to that
    mov x,#_LinFrame
    mov y,#_LinFrameDataBuffer
    movsw [x++],[y++]
    movsw [x++],[y++]
    movsw [x++],[y++]
    movsw [x++],[y++]
    jmp SendCmnd

#if (STANDALONE_LOADER == 0)
;-----------------------------------------------------------------------------------------------------
_ml_AssignFrameToMessageID:  ;ml_Status ml_AssignFrameToMessageID(l_u8 Index, l_u8 LinIdentifier)
    ;------------------------------------------------------------------------------------
    ;configure the index IDs
    ;parameters: (1) Index    in a
    ;            (2) LinIdentifier   in [s-4]

    ;format
    ; LinCmnd 00 xx xxxx xxxx xxxx
    ;           \  \    \    \    \
    ;            \  \    \    \    pcCNFID (command)
    ;             \  \    \    index (4 bits)
    ;              \  \    LIN ID (4 lsbs)
    ;               \  LIN ID (2 msbs)
    ;                not used, reserved for ml_EnableMessage and ml_DisableMessage

    mov ah,[s-4]         ;LIN ID
    and ah,#0x3F         ;discard eventual parity bits

    jmp SendCnfId        ;load the index and the command

;-----------------------------------------------------------------------------------------------------
_ml_DisableMessage:  ; ml_DisableMessage(ml_MessageID MessageIndex);
    ;------------------------------------------------------------------------------------
    ;disable an index ID
    ;parameters: (1) MessageIndex in a

    mov ah,#2            ; ah = 0000 0010
    jmp ShiftCnfId

;-----------------------------------------------------------------------------------------------------
_ml_EnableMessage:  ; ml_EnableMessage(ml_MessageID MessageIndex);
    ;------------------------------------------------------------------------------------
    ;enable an index ID
    ;parameters: (1) MessageIndex in a

    ;format
    ; LinCmnd 1 m 00 0000 xxxx 0110
    ;          \ \  \    \    \    \
    ;           \ \  \    \    \    pcCNFID (command)
    ;            \ \  \    \    index (4 bits)
    ;             \ \  \    not used
    ;              \ \  not used
    ;               \ enabled (1) / disabled (0)
    ;                1 to indicate that this is an ml_EnableMessage/ml_DisableMessage (was ml_MessageValid) command

    mov ah,#3           ; ah = 0000 0011
ShiftCnfId:
    rr ah,#2            ; ah = 1m00 0000        ; shift the enable/disable bit into required position

SendCnfId:              ;index and command
    asl al,#2           ;shift the index (4 times)
    asl al,#2
    or al,#pcCNFID      ;command: configure an ID
    jmp SendCmnd
#endif /* !STANDALONE_LOADER */

#if defined (HAS_LIN_AUTOADDRESSING) && (STANDALONE_LOADER == 0)
;-----------------------------------------------------------------------------------------------------
_ml_AutoAddressingConfig:  ; ml_Status ml_AutoAddressingConfig(ml_AutoAddressingMode AutoAddressingMode);
    ;------------------------------------------------------------------------------------
    ;enable or disable the autoconfig switch
    ;parameters: (1) AutoAddressingMode in a
    ;command to send (in a): xxxx xxxx AAAA 1101
    ;                            \    \    \    \
    ;                             \    \    \    pcCNFAUTO
    ;                              \    \    AutoAddressingMode
    ;                               \    not used
    ;                                not used

                                            ; a = 0000 0000 0000 AAAA
    asl al,#2                               ; a = 0000 0000 00AA AA00
    asl al,#2                               ; a = 0000 0000 AAAA 0000
    or  al, #pcCNFAUTO                      ; a = 0000 0000 AAAA 1101
    jmp SendCmnd        ;send the command
#endif /* HAS_LIN_AUTOADDRESSING */

;-----------------------------------------------------------------------------------------------------
_ml_SetSlewRate:  ; ml_Status ml_SetSlewRate(ml_uint16 SlewRate);
    ;------------------------------------------------------------------------------------
    ;set the LIN cell slew rate (20kbps, 10kbps, max)
    ;parameters: (1) SlewRate in a
    ;command to send (in a): xxxx xxxx x0SS 0111
    ;                            \    \    \    \
    ;                             \    \    \    pcCONFIG
    ;                              \    \    SlewRate
    ;                               \    not used
    ;                                not used

                                                    ; a = 0000 0000 0000 00SS
    asl a, #2                                       ; a = 0000 0000 0000 SS00
    asl a, #2                                       ; a = 0000 0000 00SS 0000
    or  a, #pcCNFSR                                 ; a = 0000 0000 00SS 0111
    jmp SendCmnd        ;send the command

;-----------------------------------------------------------------------------------------------------
_ml_SetOptions: ;ml_Status ml_SetOptions( ml_uint8 IDStopBitLength, 
    ;                                     ml_uint8 TXStopBitLength,
    ;                                     ml_bool  EnableStateChangeEvent,
    ;                                     ml_bool  SleepMode);
    ;------------------------------------------------------------------------------------
    ;configure the option register of the Mlx4 software
    ;parameters: (1) IDStopBitLength in a
    ;            (2) TXStopBitLength in [s-4]
    ;            (3) EnableStateChangeEvent in [s-6]
    ;            (4) Deep Sleep Mode in [s-8]
    ;this command configures the options register on the mlx4
    ;                   _
    ; command: 0000  xSxC  xTII  1011
    ;                  \ \   \ \     \
    ;                   \ \   \ \     pcOPTION
    ;                    \ \   \ IDStopBitLength
    ;                     \ \   TXStopBitLength
    ;                      \ EnableStateChangeEvent
    ;                       deep sleep (1) / light sleep (0)

                                                    ; a = 0000 0000 0000 00II
    mov ah,[s-4]                                    ; a = 0000 000T 0000 00II
    asl ah,#2                                       ; a = 0000 0T00 0000 00II
    or al,ah                                        ; a = 0000 0T00 0000 0TII
    asl al,#2                                       ; a = 0000 0T00 000T II00
    asl al,#2                                       ; a = 0000 0T00 0TII 0000
    or al,#pcOPTION                                 ; a = 0000 0T00 0TII 1011
    or ah,[s-8]                                     ; a = 0000 000S 0TII 1011
    asl ah,#2                                       ; a = 0000 0S00 0TII 1011
    or ah,[s-6]                                     ; a = 0000 0S0C 0TII 1011
    xor ah,#0x01       ;inverse EnableStateChangeEvent
    jmp SendCmnd       ;send the command

;-----------------------------------------------------------------------------------------------------
_ml_DiscardFrame:  ;ml_Status ml_DiscardFrame(void)
    ;parameters : none
    mov al,#pcSETFRAME                              ; a = 0000 0000 0000 0101
    ;ah still contains the ID (if needed)
    jmp SendCmnd

;-----------------------------------------------------------------------------------------------------
_ml_ContFrame:      ;ml_Status _ml_ContFrame(ml_bool blEnable)
                                                    ; a = 0000 0000 0000 000E
    mov ah,al                                       ; a = 0000 000E 0000 0000
    mov al,#(0x20 | pcSETFRAME)                     ; a = 0000 000E 0010 0101
    jmp SendCmnd

;-----------------------------------------------------------------------------------------------------
SendCmnd:  ;send a command to the Mlx4
    ;the command has to be in the register a
#if defined (HAS_MLX4_SEND_CMD_RETRY)
    mov Cx, #2              ; one additional retry is allowed
#else /* HAS_MLX4_CMD_RETRY */
    mov Cx, #1              ; no retry is allowed
#endif

SendCmndRetry:
    mov _LinCmnd, a         ; load the command in the RAM
    mov y, a                ; save _LinCmnd to Y, as SendCmnd can be interrupted with another _LinCmnd
    mov al,#0x41
    mov SLVCMD,al           ; send an event

#if defined (HAS_MLX4_CMD_ACK_TIMEOUT)
    mov x, #MLX4_CMD_ACK_TIMEOUT
WaitAck:
    mov al,SLVCMD           ; check that the event is acknowledged
    and al,#0x02
    jnz AckReceived         ; exit if acknowledged
    djnz x, WaitAck
    mov a, #2               ; return timeout failure
    ret
#else
WaitAck:
    mov al,SLVCMD           ; check that the event is acknowledged
    and al,#0x02
    jz WaitAck              ; wait forever (until watchdog reset)
#endif
AckReceived:
    ; test the command resp
    mov x,y             ; restore initial _LinCmnd
    and x,#15
    mov a,_LinResp      ;a = DDDD DDDD FFFF CCCC
                        ;             \    \    \
                        ;              \    \    command: pcXXXX
                        ;               \    ack status 0 = OK, 1 = ERROR (data invalid)
                        ;                D = data
    mov ah,#0
    cmp a,x
    jne AckCmdErr       ;error
    mov a, #0           ; ok
    ret
AckCmdErr:
    mov a, y            ; use initial _LinCmnd for retry
    djnz Cx,SendCmndRetry
    mov a, #1
    ret

;-----------------------------------------------------------------------------------------------------
_ml_LinModuleVersion:  ;ml_uint16 ml_LinModuleVersion(void);
    ;------------------------------------------------------------------------------------
    ;Return the Mlx4 LIN firmware version
    ;parameters : none
    ;return parameter : Mlx4 LIN firmware version

    ;Return the Mlx4 LIN firmware edition number
    mov al, #((verEDIT << 4) | pcSFTVER)    ;send the command
    call SendCmnd
    cmp a, #0              ; if command sending failed ..
    jne SoftVerEnd         ; .. signal error
    ;get the response (the response is sent immediately)
    mov a,_LinResp    ;a = VVVV VVVV xxxx CCCC
            ;             \         \
            ;              \         command: evSFTVER
            ;               firmware edition number (2 nibbles, 8 bits)
    and al,#15        ;keep only the last 4 digits and check the message
    cmp al,#pcSFTVER
    jne SoftVerEnd
                    ;the firmware edition number is in ah
    mov yl,ah       ;save it in yl

    ;Return the Mlx4 firmware version
    mov al, #((verLIN << 4) | pcSFTVER)    ;send the command
    call SendCmnd
    cmp a, #0              ; if command sending failed ..
    jne SoftVerEnd         ; .. signal error
    ;get the response (the response is sent immediately)
    mov a,_LinResp      ;a = VVVV VVVV xxxx CCCC
                        ;             \         \
                        ;              \         command: evSFTVER
                        ;               firmware version (2 nibbles, 8 bits)
    and al,#15          ;keep only the last 4 digits and check the message
    cmp al,#pcSFTVER
    jne SoftVerEnd
                        ; the firmware version is in ah (msbyte)
    mov al,yl           ; get the firmware edition number in al (lsbyte)
    ret                 ; return

SoftVerEnd:
    mov a, #0           ; signal an error
    ret


#if (STANDALONE_LOADER == 0)
;-----------------------------------------------------------------------------------------------------
_ml_GetState:  ;ml_LinState ml_GetState(uint8 bits_to_reset);
    ;------------------------------------------------------------------------------------
    ;Return the Mlx4 state
    ;parameters : which bits need to be reset after reading
    ;return parameter : current state of the Mlx4
    ;clear bits in mlx4, as we don't have a direct access to them

                                                    ; a = 0000 0000 0000 CCCC
    asl a ,#2                                       ; a = 0000 0000 00CC CC00
    asl a ,#2                                       ; a = 0000 0000 CCCC 0000
    or  a,#pcGETST                                  ; a = 0000 0000 CCCC 1100
    call SendCmnd

    cmp a,#0          ; if SendCmnd result = 0 - error occured
    jne GetStateError

    ;else - get command result
    mov a,_LinResp      ;a = SSSS LLLL SSSS CCCC
                        ;        \    \    \    \
                        ;         \    \    \    command
                        ;          \    \    status
                        ;           \     LINstate
                        ;            LINstatus

    cmp al, #pcGETST    ; (status_OK << 4) | pcGETST
    jne GetStateError

    mov al,ah
    asr ah,#2
    asr ah,#2
    mov _LinStatus,ah
    and a,#0x000F
    ret

GetStateError:
    mov a,#(-1)           ; signal an error
    ret                 ;return      LINstatus

#endif /* STANDALONE_LOADER */


;-----------------------------------------------------------------------------------------------------
_ml_SetBaudRate:  ;ml_Status ml_SetBaudRate(int Prescaler, int BaudRate)
    ;------------------------------------------------------------------------------------
    ;Send the baudrate parameters to the Mlx4
    ;parameters : - Prescaler     in a
    ;             - BaudRate     in [s-4]

                                                    ; a = 0000 0000 0000 PPPP
    asl al,#2                                       ; a = 0000 0000 00PP PP00
    asl al,#2                                       ; a = 0000 0000 PPPP 0000
    or al,#pcCNFBR                                  ; a = 0000 0000 PPPP 0010
    mov y,[s-4]
    mov ah,yl                                       ; a = BBBB BBBB PPPP 0010
    jmp SendCmnd

;-----------------------------------------------------------------------------------------------------
_ml_SetAutoBaudRateMode:    ;ml_Status _ml_SetAutoBaudRateMode(int iMode)
    ;------------------------------------------------------------------------------------
    ;Send the baudrate parameters to the Mlx4
    ;parameters : - iMode in a
    ; pcCNFBR 0xF ... 0x0

                                                    ; a = 0000 0000 0000 MMMM
    mov ah,al                                       ; a = 0000 MMMM 0000 0000
    mov al,#pcCNFBR                                 ; a = 0000 MMMM 0000 0010
    or  al,#(0x0E << 4)                             ; a = 0000 MMMM 1110 0010
    jmp SendCmnd

;-----------------------------------------------------------------------------------------------------
_ml_Connect:  ;ml_Status ml_Connect(void);
    ;------------------------------------------------------------------------------------
    ;Go to ACTIVE state
    ;parameters: none
    ;variables used : none
    ;shared ram location : LinCmnd
    ;register used : al

    ;send a command (State Change) -> ACTIVE
    mov al,#stACT
    jmp StateChange

;-----------------------------------------------------------------------------------------------------
; \note
;   1. In Standalone loader mode command ml_Disconnect stops communication processing,
;      releases LIN pin and puts MLX4 in wait power down state. Thus reconnection with ml_Connect
;      is not possible and reset should be applied to reinitialize LIN module.
;
_ml_Disconnect:  ;ml_Status ml_Disconnect(void);
    ;------------------------------------------------------------------------------------
    ;Go to DISCONNECTED state
    ;parameters: none
    ;variables used : none
    ;shared ram location : LinCmnd
    ;register used : al

    ;send a command (State Change) -> DISCONNECTED
    mov al,#stDISC        ;parameter
    jmp StateChange

#if (LIN_PIN_LOADER != 0) && (STANDALONE_LOADER == 0)
;-----------------------------------------------------------------------------------------------------
_ml_SwitchToFast:  ;ml_Status ml_SwitchToFast(void);
    ;------------------------------------------------------------------------------------
    ;Enter the fast mode
    ;parameters: none
    ;variables used : none
    ;shared ram location : LinCmnd
    ;register used : al

    ;send a command (State Change) -> FAST
    mov al,#stFAST
    jmp StateChange
#endif /* LIN_PIN_LOADER */


#if (STANDALONE_LOADER == 0)
;-----------------------------------------------------------------------------------------------------
_ml_WakeUp:  ;ml_Status ml_WakeUp(void);
    ;------------------------------------------------------------------------------------
    ;Go to WAKE-UP state
    ;parameters: none
    ;variables used : none
    ;shared ram location : LinCmnd
    ;register used : al

    ;send a command (State Change) -> WAKE-UP
    mov al,#stWKUP
    jmp StateChange

;-----------------------------------------------------------------------------------------------------
_ml_GotoSleep:  ;ml_Status ml_GotoSleep(void);
    ;------------------------------------------------------------------------------------
    ;Go to SLEEP state
    ;parameters: none
    ;variables used : none
    ;shared ram location : LinCmnd
    ;register used : al

    ;send a command (State Change) -> SLEEP
    mov al,#stSLEEP
    jmp StateChange
#endif /* !STANDALONE_LOADER */

;-----------------------------------------------------------------------------------------------------
; PRIVATE FUNCTIONS
;-----------------------------------------------------------------------------------------------------
StateChange:  ;send a command (State Change)
    ;the new state has to be in al - al is the only register that is modified
    ;return upon reception of an acknoledge from the Mlx4

                                                    ; a = 0000 0000 0000 SSSS
    asl al,#2                                       ; a = 0000 0000 00SS SS00
    asl al,#2                                       ; a = 0000 0000 SSSS 0000
    or al,#pcSTCH                                   ; a = 0000 0000 SSSS 0001
    jmp SendCmnd

    end
