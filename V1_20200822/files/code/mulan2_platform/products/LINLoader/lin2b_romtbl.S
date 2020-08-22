;
; Copyright (C) 2005-2015 Melexis N.V.
;
; Software Platform
;

;--------------------------------------------------
;-------- LIN Configuration ROM tables ------------
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

_INDXtbl:   ;
    .byte 0xC0  ;FrameID 00h : unused
    .byte 0xC0  ;FrameID 01h : unused
    .byte 0xC0  ;FrameID 02h : unused
    .byte 0xC0  ;FrameID 03h : unused
    .byte 0xC0  ;FrameID 04h : unused
    .byte 0xC0  ;FrameID 05h : unused
    .byte 0xC0  ;FrameID 06h : unused
    .byte 0xC0  ;FrameID 07h : unused
    ;
    .byte 0xC0  ;FrameID 08h : unused
    .byte 0xC0  ;FrameID 09h : unused
    .byte 0xC0  ;FrameID 0Ah : unused
    .byte 0xC0  ;FrameID 0Bh : unused
    .byte 0xC0  ;FrameID 0Ch : unused
    .byte 0xC0  ;FrameID 0Dh : unused
    .byte 0xC0  ;FrameID 0Eh : unused
    .byte 0xC0  ;FrameID 0Fh : unused
    ;
    .byte 0xC0  ;FrameID 10h : unused
    .byte 0xC0  ;FrameID 11h : unused
    .byte 0xC0  ;FrameID 12h : unused
    .byte 0xC0  ;FrameID 13h : unused
    .byte 0xC0  ;FrameID 14h : unused
    .byte 0xC0  ;FrameID 15h : unused
    .byte 0xC0  ;FrameID 16h : unused
    .byte 0xC0  ;FrameID 17h : unused
    ;
    .byte 0xC0  ;FrameID 18h : unused
    .byte 0xC0  ;FrameID 19h : unused
    .byte 0xC0  ;FrameID 1Ah : unused
    .byte 0xC0  ;FrameID 1Bh : unused
    .byte 0xC0  ;FrameID 1Ch : unused
    .byte 0xC0  ;FrameID 1Dh : unused
    .byte 0xC0  ;FrameID 1Eh : unused
    .byte 0xC0  ;FrameID 1Fh : unused
    ;
    .byte 0xC0  ;FrameID 20h : unused
    .byte 0xC0  ;FrameID 21h : unused
    .byte 0xC0  ;FrameID 22h : unused
    .byte 0xC0  ;FrameID 23h : unused
    .byte 0xC0  ;FrameID 24h : unused
    .byte 0xC0  ;FrameID 25h : unused
    .byte 0xC0  ;FrameID 26h : unused
    .byte 0xC0  ;FrameID 27h : unused
    ;
    .byte 0xC0  ;FrameID 28h : unused
    .byte 0xC0  ;FrameID 29h : unused
    .byte 0xC0  ;FrameID 2Ah : unused
    .byte 0xC0  ;FrameID 2Bh : unused
    .byte 0xC0  ;FrameID 2Ch : unused
    .byte 0xC0  ;FrameID 2Dh : unused
    .byte 0xC0  ;FrameID 2Eh : unused
    .byte 0xC0  ;FrameID 2Fh : unused
    ;
    .byte 0xC0  ;FrameID 30h : unused
    .byte 0xC0  ;FrameID 31h : unused
    .byte 0xC0  ;FrameID 32h : unused
    .byte 0xC0  ;FrameID 33h : unused
    .byte 0xC0  ;FrameID 34h : unused
    .byte 0xC0  ;FrameID 35h : unused
    .byte 0xC0  ;FrameID 36h : unused
    .byte 0xC0  ;FrameID 37h : unused
    ;
    .byte 0xC0  ;FrameID 38h : unused
    .byte 0xC0  ;FrameID 39h : unused
    .byte 0xC0  ;FrameID 3Ah : unused
    .byte 0xC0  ;FrameID 3Bh : unused
    .byte 0x10  ;FrameID 3Ch: MRF (Diagnostic Master Request Frame) - THIS INDEX SHOULD NOT BE CHANGED !!!
    .byte 0x11  ;FrameID 3Dh: SRF (Diagnostic Slave Response Frame) - THIS INDEX SHOULD NOT BE CHANGED !!!
    .byte 0xC0  ;FrameID 3Eh: Reserved
    .byte 0xC0  ;FrameID 3Fh: Reserved

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
    .byte 0xA0  ;Message Idx 00h : discard
    .byte 0xA0  ;Message Idx 01h : discard
    .byte 0xA0  ;Message Idx 02h : discard
    .byte 0xA0  ;Message Idx 03h : discard
    ;
    .byte 0xA0  ;Message Idx 04h : discard
    .byte 0xA0  ;Message Idx 05h : discard
    .byte 0xA0  ;Message Idx 06h : discard
    .byte 0xA0  ;Message Idx 07h : discard
    ;
    .byte 0xA0  ;Message Idx 08h : discard
    .byte 0xA0  ;Message Idx 09h : discard
    .byte 0xA0  ;Message Idx 0Ah : discard
    .byte 0xA0  ;Message Idx 0Bh : discard
    ;
    .byte 0xA0  ;Message Idx 0Ch : discard
    .byte 0xA0  ;Message Idx 0Dh : discard
    .byte 0xA0  ;Message Idx 0Eh : discard
    .byte 0xA0  ;Message Idx 0Fh : discard
    ;
    .byte 0x07  ;Message Idx 10h : MRF (Diagnostic Master Request Frame) - RX - 8 bytes - chk13 - THIS VALUE SHOULD NOT BE CHANGED !!!
    .byte 0x27  ;Message Idx 11h : SRF (Diagnostic Slave Response Frame) - TX - 8 bytes - chk13 - THIS VALUE SHOULD NOT BE CHANGED !!!
    .byte 0xA0  ;Message Idx 12h : discard
    .byte 0xA0  ;Message Idx 13h : discard
    ;
    .byte 0xA0  ;Message Idx 14h : discard
    .byte 0xA0  ;Message Idx 15h : discard
    .byte 0xA0  ;Message Idx 16h : discard
    .byte 0xA0  ;Message Idx 17h : discard
    ;
    .byte 0xA0  ;Message Idx 18h : discard
    .byte 0xA0  ;Message Idx 19h : discard
    .byte 0xA0  ;Message Idx 1Ah : discard
    .byte 0xA0  ;Message Idx 1Bh : discard
    ;
    .byte 0xA0  ;Message Idx 1Ch : discard
    .byte 0xA0  ;Message Idx 1Dh : discard
    .byte 0xA0  ;Message Idx 1Eh : discard
    .byte 0xA0  ;Message Idx 1Fh : discard
    ;
    .byte 0xA0  ;Message Idx 20h : discard
    .byte 0xA0  ;Message Idx 21h : discard
    .byte 0xA0  ;Message Idx 22h : discard
    .byte 0xA0  ;Message Idx 23h : discard
    ;
    .byte 0xA0  ;Message Idx 24h : discard
    .byte 0xA0  ;Message Idx 25h : discard
    .byte 0xA0  ;Message Idx 26h : discard
    .byte 0xA0  ;Message Idx 27h : discard
    ;
    .byte 0xA0  ;Message Idx 28h : discard
    .byte 0xA0  ;Message Idx 29h : discard
    .byte 0xA0  ;Message Idx 2Ah : discard
    .byte 0xA0  ;Message Idx 2Bh : discard
    ;
    .byte 0xA0  ;Message Idx 2Ch : discard
    .byte 0xA0  ;Message Idx 2Dh : discard
    .byte 0xA0  ;Message Idx 2Eh : discard
    .byte 0xA0  ;Message Idx 2Fh : discard
    ;
    .byte 0xA0  ;Message Idx 30h : discard
    .byte 0xA0  ;Message Idx 31h : discard
    .byte 0xA0  ;Message Idx 32h : discard
    .byte 0xA0  ;Message Idx 33h : discard
    ;
    .byte 0xA0  ;Message Idx 34h : discard
    .byte 0xA0  ;Message Idx 35h : discard
    .byte 0xA0  ;Message Idx 36h : discard
    .byte 0xA0  ;Message Idx 37h : discard
    ;
    .byte 0xA0  ;Message Idx 38h : discard
    .byte 0xA0  ;Message Idx 39h : discard
    .byte 0xA0  ;Message Idx 3Ah : discard
    .byte 0xA0  ;Message Idx 3Bh : discard
    ;
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
