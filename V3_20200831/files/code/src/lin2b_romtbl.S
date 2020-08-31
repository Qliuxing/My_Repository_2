;--------------------------------------------------
;--------------- LIN API ROM tables ---------------
;--------------------------------------------------

	.global _INDXtbl
	.global _PARAMtbl
	.global _AUTOADDtbl

	.sect .mlx4_table,"xa",@progbits

; LIN 2.0 SAE J2602, GM Water-valve 1.0
#define LIN2J_VALVE_GM				3
#define LIN2X_VALVE_NEXT			4
; Choose one of the above protocol's
#define LINPROT			LIN2X_VALVE_NEXT


;Index table (ROM) : 32 bytes (32 x 8 x 2 = 64 ID)
;Access : Rom:INDXtbl[X,A] with ID in [X,A] shifted once to the left
;Index : LIN ID
;(order : nibble1, nibble0, nibble3, nibble2)
;         arg_b    arg_a    arg_d    arg_c
;
;         xx  xxxxxx
;           \       \
;            \       local index
;             00 (0) : ID defined in ROM - index 10h to 3Fh
;             01 (4) : ID defined during initialization (in RAM) or part of a group - index 0 to 15 <------------------------
;             10 (8) : ID defined in EEPROM - need to be loaded in RAM during initialization - not available yet
;             11 (C) : other / not used (default)

_INDXtbl:	;00h
#if LINPROT == LIN2J_VALVE_GM
	.byte 0xC0	;00h : unused
	.byte 0xC0	;01h : unused
	.byte 0xC0	;02h : unused
	.byte 0xC0	;03h : unused
	.byte 0xC0	;04h : unused
	.byte 0xC0  ;05h : unused
	.byte 0xC0	;06h : unused
	.byte 0xC0	;07h : unused
	;04h
	.byte 0xC0	;08h : unused
	.byte 0xC0	;09h : unused
	.byte 0xC0	;0Ah : unused
	.byte 0xC0	;0Bh : unused
	.byte 0xC0	;0Ch : unused
	.byte 0xC0	;0Dh : unused
	.byte 0xC0	;0Eh : unused
	.byte 0xC0	;0Fh : unused
	;08h
	.byte 0xC0	;10h : unused
	.byte 0xC0	;11h : unused
	.byte 0xC0	;12h : unused
	.byte 0xC0	;13h : unused
	.byte 0xC0	;14h : unused
	.byte 0xC0  ;15h : unused
	.byte 0xC0	;16h : unused
	.byte 0xC0	;17h : unused
	;0Ch
	.byte 0xC0	;18h : unused
	.byte 0xC0	;19h : unused
	.byte 0xC0	;1Ah : unused
	.byte 0xC0	;1Bh : unused
	.byte 0xC0	;1Ch : unused
	.byte 0xC0	;1Dh : unused
	.byte 0xC0	;1Eh : unused
	.byte 0xC0	;1Fh : unused
	;10h
	.byte 0xC0	;20h : unused
	.byte 0xC0	;21h : unused
	.byte 0xC0	;22h : unused
	.byte 0xC0	;23h : unused
	.byte 0xC0	;24h : unused
	.byte 0xC0  ;25h : unused
	.byte 0xC0	;26h : unused
	.byte 0xC0	;27h : unused
	;14h
	.byte 0xC0	;28h : unused
	.byte 0xC0	;29h : unused
	.byte 0xC0	;2Ah : unused
	.byte 0xC0	;2Bh : unused
	.byte 0xC0	;2Ch : unused
	.byte 0xC0	;2Dh : unused
	.byte 0xC0	;2Eh : unused
	.byte 0xC0	;2Fh : unused
	;18h
	.byte 0xC0	;30h : unused
	.byte 0xC0	;31h : unused
	.byte 0xC0	;32h : unused
	.byte 0xC0	;33h : unused
	.byte 0xC0	;34h : unused
	.byte 0xC0	;35h : unused
	.byte 0xC0	;36h : unused
	.byte 0xC0	;37h : unused
	;1Ch
	.byte 0xC0	;38h : unused
	.byte 0xC0	;39h : unused
	.byte 0xC0	;3Ah : unused
	.byte 0xC0	;3Bh : unused
#endif

#if LINPROT == LIN2X_VALVE_NEXT
  	.byte 0xC0	;00h : ACT_Control
	.byte 0xC0	;01h : ACT_Status
	.byte 0xC0	;02h : unused
	.byte 0xC0	;03h : unused
	.byte 0xC0	;04h : unused
	.byte 0xC0  ;05h : unused
	.byte 0xC0	;06h : unused
	.byte 0xC0	;07h : unused
	;04h
	.byte 0xC0	;08h : unused
	.byte 0xC0	;09h : unused
	.byte 0xC0	;0Ah : unused
	.byte 0xC0	;0Bh : unused
	.byte 0xC0	;0Ch : unused
	.byte 0xC0	;0Dh : unused
	.byte 0xC0	;0Eh : unused
	.byte 0xC0	;0Fh : unused
	;08h
	.byte 0xC0	;10h : unused
	.byte 0xC0	;11h : unused
	.byte 0xC0	;12h : unused
	.byte 0xC0	;13h : unused
	.byte 0xC0	;14h : unused
	.byte 0xC0  ;15h : unused
	.byte 0xC0	;16h : unused
	.byte 0xC0	;17h : unused
	;0Ch
	.byte 0xC0	;18h : unused
	.byte 0xC0	;19h : unused
	.byte 0xC0	;1Ah : unused
	.byte 0xC0	;1Bh : unused
	.byte 0xC0	;1Ch : unused
	.byte 0xC0	;1Dh : unused
	.byte 0xC0	;1Eh : unused
	.byte 0xC0	;1Fh : unused
	;10h
	.byte 0xC0	;20h : unused
	.byte 0xC0	;21h : unused
	.byte 0xC0	;22h : unused
	.byte 0xC0	;23h : unused
	.byte 0xC0	;24h : unused
	.byte 0xC0  ;25h : unused
	.byte 0xC0	;26h : unused
	.byte 0xC0	;27h : unused
	;14h
	.byte 0xC0	;28h : unused
	.byte 0xC0	;29h : unused
	.byte 0xC0	;2Ah : unused
	.byte 0xC0	;2Bh : unused
	.byte 0xC0	;2Ch : unused
	.byte 0xC0	;2Dh : unused
	.byte 0xC0	;2Eh : unused
	.byte 0xC0	;2Fh : unused
	;18h
	.byte 0xC0	;30h : unused
	.byte 0xC0	;31h : unused
	.byte 0xC0	;32h : unused
	.byte 0xC0	;33h : unused
	.byte 0xC0	;34h : unused
	.byte 0xC0	;35h : unused
	.byte 0xC0	;36h : unused
	.byte 0xC0	;37h : unused
	;1Ch
	.byte 0xC0	;38h : unused
	.byte 0xC0	;39h : unused
	.byte 0xC0	;3Ah : unused
	.byte 0xC0	;3Bh : unused
#endif
	.byte 0x10	;3Ch: ACT_DFR_DIA (Master Request Command Frame) - THIS INDEX SHOULD NOT BE CHANGED !!!
	.byte 0x11	;3Dh: ACT_RFR_DIA (Slave Response Command Frame) - THIS INDEX SHOULD NOT BE CHANGED !!!
	.byte 0xC0	;3Eh: User-Defined Extended Frame
	.byte 0xC0	;3Fh: Reserved
		
;--------------------------------------------------
;Frame configuration table

;The 16 first Index are reserved for Configurable IDs (in RAM)

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
;                     100 : ask the application
;                     101 : discard
;                     110 : reserved (hook)
;                     111 : reserved (error)
;

_PARAMtbl1:	;Page 0 and 1 - The ID configured in RAM are in page 0
#if LINPROT == LIN2J_VALVE_GM
; LIN SAE J2602 Actuator: LIN Msg-ID's 0x00-0x01
	.byte 0x09	;Index 00h : CONTROL_ACTx - RX - 2 bytes - chk20 (Message-ID: 0x0000)
	.byte 0x2A	;Index 01h : STATUS_ACTx - TX - 3 bytes - chk20 (Message-ID: 0x0001)
	.byte 0xA0	;Index 02h : discard
#endif
#if LINPROT == LIN2X_VALVE_NEXT
; LIN 2.x, Actuator 2.2: LIN Msg-ID's 0x00-0x02
	.byte 0x2B	;Index 00h : STATUS_ACTx - TX - 8 bytes - chk20 (Message-ID: 0x0000)
	.byte 0x09	;Index 01h : CONTROL_ACTx - RX - 8 bytes - chk20 (Message-ID: 0x0001)
	.byte 0xA0	;Index 02h : discard
#endif
; LIN Msg-ID's 0x03-0x0F
	.byte 0xA0	;Index 03h : discard
	.byte 0xA0	;Index 04h : discard
	.byte 0xA0	;Index 05h : discard
	.byte 0xA0	;Index 06h : discard
	.byte 0xA0	;Index 07h : discard
	.byte 0xA0	;Index 08h : discard
	.byte 0xA0	;Index 09h : discard
	.byte 0xA0	;Index 0Ah : discard
	.byte 0xA0	;Index 0Bh : discard
	.byte 0xA0	;Index 0Ch : discard
	.byte 0xA0	;Index 0Dh : discard
	.byte 0xA0	;Index 0Eh : discard
	.byte 0xA0	;Index 0Fh : discard
	;Page 1 ---------
	.byte 0x07	;Index 10h : ACT_DFR_DIA (Master Request Command Frame) - RX - 8 bytes - chk13 - THIS INDEX SHOULD NOT BE CHANGED !!!
	.byte 0x27	;Index 11h : ACT_RFR_DIA (Slave Response Command Frame) - TX - 8 bytes - chk13 - THIS INDEX SHOULD NOT BE CHANGED !!!
	.byte 0xA0	;Index 12h : discard
	.byte 0xA0	;Index 13h : discard
	.byte 0xA0	;Index 14h : discard
	.byte 0xA0	;Index 15h : discard
	.byte 0xA0	;Index 16h : discard
	.byte 0xA0	;Index 17h : discard
	.byte 0xA0	;Index 18h : discard
	.byte 0xA0	;Index 19h : discard
	.byte 0xA0	;Index 1Ah : discard
	.byte 0xA0	;Index 1Bh : discard
	.byte 0xA0	;Index 1Ch : discard
	.byte 0xA0	;Index 1Dh : discard
	.byte 0xA0	;Index 1Eh : discard
	.byte 0xA0	;Index 1Fh : discard

_PARAMtbl2:	;Page 2 and 3 ---
	.byte 0xA0	;Index 20h : discard
	.byte 0xA0	;Index 21h : discard
	.byte 0xA0	;Index 22h : discard
	.byte 0xA0	;Index 23h : discard
	.byte 0xA0	;Index 24h : discard
	.byte 0xA0	;Index 25h : discard
	.byte 0xA0	;Index 26h : discard
	.byte 0xA0	;Index 27h : discard
	.byte 0xA0	;Index 28h : discard
	.byte 0xA0	;Index 29h : discard
	.byte 0xA0	;Index 2Ah : discard
	.byte 0xA0	;Index 2Bh : discard
	.byte 0xA0	;Index 2Ch : discard
	.byte 0xA0	;Index 2Dh : discard
	.byte 0xA0	;Index 2Eh : discard
	.byte 0xA0	;Index 2Fh : discard
	;Page 3 ---------
; LIN Msg-ID's 0x30-0x3F
	.byte 0xA0	;Index 30h : discard
	.byte 0xA0	;Index 31h : discard
	.byte 0xA0	;Index 32h : discard
	.byte 0xA0	;Index 33h : discard
	.byte 0xA0	;Index 34h : discard
	.byte 0xA0	;Index 35h : discard
	.byte 0xA0	;Index 36h : discard
	.byte 0xA0	;Index 37h : discard
	.byte 0xA0	;Index 38h : discard
	.byte 0xA0	;Index 39h : discard
	.byte 0xA0	;Index 3Ah : discard
	.byte 0xA0	;Index 3Bh : discard
	.byte 0xA0	;Index 3Ch : discard
	.byte 0xA0	;Index 3Dh : discard
	.byte 0xA0	;Index 3Eh : discard
	.byte 0xA0	;Index 3Fh : discard

;--------------------------------------------------
;Cooling Auto Addressing Pulse Table
;Store the values in half bit times between two pulses
;Access : Rom:AUTOADDtbl[X,A] with X = 8

_AUTOADDtbl:	;20h	; Pulse Number
;* 13T AA-LIN			; Step 0, 2 & 4: Setup Measurement, Step 1, 3 & 5: Perform Measurement
    .byte 0x10	; 2,  1	; Step 0 directly (code delay: 0.5T), Step 1 after 0.5T
    .byte 0x26	; 4,  3	; Step 2 after 3.0T, Step 3 after 1.0T
    .byte 0x26	; 6,  5	; Step 4 after 3.0T, Step 5 after 1.0T
    .byte 0x16	; 8,  7	; Step 6 after 3.0T, Step 7 after 0.5T
    .byte 0x00	;10,  9	; Total: 0.5(delay) + 0.5 + 3.0 + 1.0 + 3.0 + 1.0 + 3.0 + 0.5 = 12.5T
    .byte 0x00	;12, 11
    .byte 0x00	;14, 13
    .byte 0x00	;16, 15

;--------------------------------------------------
	end
