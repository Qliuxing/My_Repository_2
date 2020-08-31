; ---------------------------------------------------------------------------
;
; Description:
;  LIN Firmware Compiler Switches for the Mlx4
;
;
; Copyright (c) Melexis Digital Competence Center
;
; ---------------------------------------------------------------------------


; ---------------------------------------------------------------------------
;  These switches values can be configured
; ---------------------------------------------------------------------------
;coFastStdAlne           equ cvON\cvOFF              ;Enable/Disable Stand Alone 'Enter Programming Mode' Command in Fast Mode
;CFG_PLL_FREQ            equ 12\20\22\24[MHz]        ;Mlx4 clock frequency
;coTIMEOUT               equ TO13\TO2X               ;Decide to use 1.3 or 2.X standard for RX timeout detection
;CFG_SLEEP_TO            equ 13_9600\13_19200\LIN20  ;Define idle bus time before to fall asleep, the switch decide on the standard to follow
;coFASTFLASH             equ cvON\cvOFF              ;Enable the fast protocol for flash loading (include lin2b_fastflash.asm)
;coDEBUGMARK             equ cvON\cvOFF              ;Enable debugging by mark instructions

coDEBUGMARK              equ cvON                    ;Enable debugging by mark instructions
; Other switches values defined into Makefile


; ---------------------------------------------------------------------------
;  These switches values are hard coded for MLX4 and can't be configured
; ---------------------------------------------------------------------------
;coFILTMODE              equ IDFILT  ;FRAME : no filtering, the application receives all the LIN ID and decides what to do
;                                    ;IDFILT: mapping between LIN ID and INDEX, parameters of each INDEX and LIN ID are defined
;                                    ;in rom tables and can be partially changed into RAM."
;coREDUCERAM             equ cvOFF   ;Amount of space alocated to private Mlx4 RAM.
;                                    ;The final value will depends on the amount of switches removed and final value of each of them
;coBRCOOL                equ cvON    ;Enable AutoAddressing feature: cvON, cvOFF
;coSLEEPMODE             equ cvON    ;Sleep Mode enabled: cvON, cvOFF
;coSLEEPAUTO             equ cvON    ;Sleep Mode Command Auto-detect (3C-00): cvON, cvOFF
;coWAKEUP                equ cvON    ;Enable Wake-Up pulse generation capabilities: cvON, cvOFF
;coWAKEUPSHORT           equ cvON    ;Enable the detection of a short if a wake-up pulse is too long: cvON, cvOFF
;coBRCORR                equ cvON    ;Enable baudrate correction: cvON, cvOFF
;coIDBANK2               equ cvON    ;Enable the second set of configurable IDs in Identifier Filtering Mode: cvON, cvOFF
;                                    ;if coIDBANK2 is off, the configurable IDs are limited to 8, if it is on, to 16.
;coFUNCFILT              equ cvOFF   ;Enable Functional Filtering : Enable/Disable Flag and/or QR Messages: cvON, cvOFF
;coFILTQR                equ cvOFF   ;Enable QR Messages: cvON, cvOFF (assumes coFUNCFILT eq cvON)
;coPARAMLT               equ cvON    ;Enable light parameters: no filtering (assumes coFUNCFILT eq cvOFF)
;coIDINIT                equ cvOFF   ;ID Table Initialization for ROM Table: cvON, cvOFF
;coBRFRM                 equ cvON    ;Signal a Break in Frame (this is always detected, even if the switch is off): cvON, cvOFF
;coFLSHLD                equ cvON    ;Enable Flash Loading Protocol detection: cvON, cvOFF
;coPROPMEAS              equ cvON    ;Enable the measurement of the propagation time during TX: cvON, cvOFF
;coSTOPBREAK             equ cvON    ;Enable to skip the stop bit error signaling in case a valid break is detected: cvON, cvOFF
;
;coRSVD5AA5              equ cvON    ;First code word is 5AA5 (cvON) or A55A (cvOFF)
;
;coPLLCHK                equ cvOFF   ;Enable the check of the PLL status: cvON, cvOFF
;coCMD_OPTION            equ cvON    ;Enable the pcOPTION command support
;coCMD_SFTVER            equ cvON    ;Enable the pcSFTVER command support
;coCMD_GETST             equ cvON    ;Enable the pcGETST command support
;coCMD_ACK               equ cvON    ;Do an acknowledge of the commands from the Mlx16 (cvON) or use the erCSYNC message (cvOFF)
;
;coSPECIALID             equ cvOFF   ;Enable processing of special ID (3D, 3D, 3E, 3F) without accessing the rom tables: cvON, cvOFF
;coQR0_USED_BY_3D_FRAME  equ cvOFF   ;Enable use for QR code for special IF 3D
;
;coFASTFLASHCK           equ cvON    ;Checksum of the Fast Protocol checked or not - DEBUG, default is cvON
;coFASTFLASHCKTX         equ cvOFF   ;Checksum of the Fast Protocol checked or not for TX messages - DEBUG, default is cvOFF
;coFASTFLASHDBG          equ cvOFF   ;Flash Loading Fast Protocol - DEBUG, default is cvOFF
;coFastDb                equ cvON    ;Enable/Disable Fast Edges
;coFastCmd               equ cvON    ;Enable/Disable the reception of commands during the RX loop
