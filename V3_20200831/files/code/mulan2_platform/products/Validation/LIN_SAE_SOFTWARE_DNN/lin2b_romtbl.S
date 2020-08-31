;--------------------------------------------------
;   LIN Configuration ROM tables
;   Template for SAE J2602
;--------------------------------------------------

    .global _INDXtbl
    .global _PARAMtbl
    .global _AUTOADDtbl

    .sect .mlx4_table,"xa",@progbits

;Index table (ROM) : 32 bytes (32 x 8 x 2 = 64 ID)
;Access : Rom:INDXtbl[X,A] with ID in [X,A] shifted once to the left
;Index : Frame ID
;(order : nibble1, nibble0, nibble3, nibble2)
;         arg_b    arg_a    arg_d    arg_c
;
;         xx  xxxxxx
;           \       \
;            \       local index
;             00 (0) : Message Index defined in ROM - index 10h to 3Fh
;             01 (4) : Message Index defined during initialization (in RAM) or part of a group - index 00h to 0Fh <------------------
;             10 (8) : Message Index defined in EEPROM - need to be loaded in RAM during initialization - not available yet
;             11 (C) : other / not used (default)

_INDXtbl:
    ;------------------------------
    ;       Message Idx  ; FrameID
    ;------------------------------
    ; NAD 0x60
    .byte       0x40     ; 00h
    .byte       0x40     ; 01h
    .byte       0x40     ; 02h
    .byte       0x40     ; 03h

    ; NAD 0x61
    .byte       0x40     ; 04h
    .byte       0x40     ; 05h
    .byte       0x40     ; 06h
    .byte       0x40     ; 07h

    ; NAD 0x62
    .byte       0x40     ; 08h
    .byte       0x40     ; 09h
    .byte       0x40     ; 0Ah
    .byte       0x40     ; 0Bh

    ; NAD 0x63
    .byte       0x40     ; 0Ch
    .byte       0x40     ; 0Dh
    .byte       0x40     ; 0Eh
    .byte       0x40     ; 0Fh

    ; NAD 0x64  
    .byte       0x40     ; 10h
    .byte       0x40     ; 11h
    .byte       0x40     ; 12h
    .byte       0x40     ; 13h

    ; NAD 0x65
    .byte       0x40     ; 14h
    .byte       0x40     ; 15h
    .byte       0x40     ; 16h
    .byte       0x40     ; 17h

    ; NAD 0x66  
    .byte       0x40     ; 18h
    .byte       0x40     ; 19h
    .byte       0x40     ; 1Ah
    .byte       0x40     ; 1Bh

    ; NAD 0x67
    .byte       0x40     ; 1Ch
    .byte       0x40     ; 1Dh
    .byte       0x40     ; 1Eh
    .byte       0x40     ; 1Fh

    ; NAD 0x68  
    .byte       0x40     ; 20h
    .byte       0x40     ; 21h
    .byte       0x40     ; 22h
    .byte       0x40     ; 23h

    ; NAD 0x69
    .byte       0x40     ; 24h
    .byte       0x40     ; 25h
    .byte       0x40     ; 26h
    .byte       0x40     ; 27h

    ; NAD 0x6A
    .byte       0x40     ; 28h
    .byte       0x40     ; 29h
    .byte       0x40     ; 2Ah
    .byte       0x40     ; 2Bh

    ; NAD 0x6B
    .byte       0x40     ; 2Ch
    .byte       0x40     ; 2Dh
    .byte       0x40     ; 2Eh
    .byte       0x40     ; 2Fh

    ; NAD 0x6C
    .byte       0x40     ; 30h
    .byte       0x40     ; 31h
    .byte       0x40     ; 32h
    .byte       0x40     ; 33h

    ; NAD 0x6D
    .byte       0x40     ; 34h
    .byte       0x40     ; 35h
    .byte       0x40     ; 36h
    .byte       0x40     ; 37h

    ; DNN Based broadcast messages  (not used)
    .byte       0x40     ; 38h
    .byte       0x40     ; 39h
    .byte       0x40     ; 3Ah
    .byte       0x40     ; 3Bh

    ;--------------------------------------------------------------------
    ;  NB: The following values shall NOT be changed by the user !!!
    ;--------------------------------------------------------------------
    ; Diagnostic messages
    .byte       0x10     ; 3Ch: MRF (Diagnostic Master Request Frame)
    .byte       0x11     ; 3Dh: SRF (Diagnostic Slave Response Frame)

    ; Reserved messages
    .byte       0xC0     ; 3Eh: Reserved LIN2.1 / User-Defined Extended Frame LIN2.0 / Configuaration SAE J2602
    .byte       0xC0     ; 3Fh: Reserved
        
;--------------------------------------------------

;Frame configuration table
;Access : Rom:PARAMtbl[X,A] with index in [X,A] shifted once to the left

;The 16 first Index are reserved for Configurable Message Indices (in RAM)

;Content for Transmit and Receive frames
;   byte 0 : xxx x   x xxx
;               \ \   \   \
;                \ \   \   \
;                 \ \   \   \
;                  \ \   \   length : 0 = 1 bytes... 7 = 8 bytes)
;                   \ \   checksum : 0 = chk13, 1 = chk20
;                    \ not used yet
;                     000 : receive
;                     001 : transmit
;                     010 : reserved
;                     011 : reserved (error)
;                     100 : reserved (ask the application)
;                     101 : discard
;                     110 : reserved (hook)
;                     111 : reserved (error)
;

_PARAMtbl:
    .byte 0x29  ;Message Idx 00h : msg_PUB1 - Slave TX - 2 bytes - chk20
    .byte 0x0F  ;Message Idx 01h : msg_SUB1 - Slave RX - 8 bytes - chk20
    .byte 0x29  ;Message Idx 02h : msg_PUB2 - Slave TX - 2 bytes - chk20
    .byte 0x09  ;Message Idx 03h : msg_SUB2 - Slave RX - 2 bytes - chk20
    .byte 0xA0  ;Message Idx 04h : discard
    .byte 0xA0  ;Message Idx 05h : discard
    .byte 0xA0  ;Message Idx 06h : discard
    .byte 0xA0  ;Message Idx 07h : discard
    .byte 0xA0  ;Message Idx 08h : discard
    .byte 0xA0  ;Message Idx 09h : discard
    .byte 0xA0  ;Message Idx 0Ah : discard
    .byte 0xA0  ;Message Idx 0Bh : discard
    .byte 0xA0  ;Message Idx 0Ch : discard
    .byte 0xA0  ;Message Idx 0Dh : discard
    .byte 0xA0  ;Message Idx 0Eh : discard
    .byte 0xA0  ;Message Idx 0Fh : discard
    .byte 0x07  ;Message Idx 10h : MRF (Diagnostic Master Request Frame) - RX - 8 bytes - chk13 - THIS VALUE SHOULD NOT BE CHANGED !!!
    .byte 0x27  ;Message Idx 11h : SRF (Diagnostic Slave Response Frame) - TX - 8 bytes - chk13 - THIS VALUE SHOULD NOT BE CHANGED !!!
    .byte 0xA0  ;Message Idx 12h : discard
    .byte 0xA0  ;Message Idx 13h : discard
    .byte 0xA0  ;Message Idx 14h : discard
    .byte 0xA0  ;Message Idx 15h : discard
    .byte 0xA0  ;Message Idx 16h : discard
    .byte 0xA0  ;Message Idx 17h : discard
    .byte 0xA0  ;Message Idx 18h : discard
    .byte 0xA0  ;Message Idx 19h : discard
    .byte 0xA0  ;Message Idx 1Ah : discard
    .byte 0xA0  ;Message Idx 1Bh : discard
    .byte 0xA0  ;Message Idx 1Ch : discard
    .byte 0xA0  ;Message Idx 1Dh : discard
    .byte 0xA0  ;Message Idx 1Eh : discard
    .byte 0xA0  ;Message Idx 1Fh : discard
    .byte 0xA0  ;Message Idx 20h : discard
    .byte 0xA0  ;Message Idx 21h : discard
    .byte 0xA0  ;Message Idx 22h : discard
    .byte 0xA0  ;Message Idx 23h : discard
    .byte 0xA0  ;Message Idx 24h : discard
    .byte 0xA0  ;Message Idx 25h : discard
    .byte 0xA0  ;Message Idx 26h : discard
    .byte 0xA0  ;Message Idx 27h : discard
    .byte 0xA0  ;Message Idx 28h : discard
    .byte 0xA0  ;Message Idx 29h : discard
    .byte 0xA0  ;Message Idx 2Ah : discard
    .byte 0xA0  ;Message Idx 2Bh : discard
    .byte 0xA0  ;Message Idx 2Ch : discard
    .byte 0xA0  ;Message Idx 2Dh : discard
    .byte 0xA0  ;Message Idx 2Eh : discard
    .byte 0xA0  ;Message Idx 2Fh : discard
    .byte 0xA0  ;Message Idx 30h : discard
    .byte 0xA0  ;Message Idx 31h : discard
    .byte 0xA0  ;Message Idx 32h : discard
    .byte 0xA0  ;Message Idx 33h : discard
    .byte 0xA0  ;Message Idx 34h : discard
    .byte 0xA0  ;Message Idx 35h : discard
    .byte 0xA0  ;Message Idx 36h : discard
    .byte 0xA0  ;Message Idx 37h : discard
    .byte 0xA0  ;Message Idx 38h : discard
    .byte 0xA0  ;Message Idx 39h : discard
    .byte 0xA0  ;Message Idx 3Ah : discard
    .byte 0xA0  ;Message Idx 3Bh : discard
    .byte 0xA0  ;Message Idx 3Ch : discard
    .byte 0xA0  ;Message Idx 3Dh : discard
    .byte 0xA0  ;Message Idx 3Eh : discard
    .byte 0xA0  ;Message Idx 3Fh : discard

;--------------------------------------------------

;Cooling Auto Addressing Pulse Table
;Store the values in half bit times between two pulses
;Access : Rom:AUTOADDtbl[X,A] with X = 8

_AUTOADDtbl:    ;   ; Pulse Number
    .byte 0x00  ; 2,  1
    .byte 0x00  ; 4,  3
    .byte 0x00  ; 6,  5
    .byte 0x00  ; 8,  7
    .byte 0x00  ;10,  9
    .byte 0x00  ;12, 11
    .byte 0x00  ;14, 13
    .byte 0x00  ;16, 15

;--------------------------------------------------
    end
