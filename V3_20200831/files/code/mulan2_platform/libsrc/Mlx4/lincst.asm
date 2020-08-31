; ---------------------------------------------------------------------------
;
; Description:
;  LIN Firmware constants for the Mlx4
;
;
; Copyright (c) Melexis Digital Competence Center
;
; ---------------------------------------------------------------------------

; lincst.asm : this file contains the constants used in the Mlx4 LIN software. Some of theses constants can be changed, depending
;on the application (particularly the time constants, which depends on the oscillator/PLL frequency).

; --- constansts are not used anymore since Phymd layout could depends on project
; Possible values for the LIN cell Slew Rate (Phymd)
;Refer to the Mlx4 periphery documentation for the values of your chip (see Phymd[1:0] in the FLASGS 1 dcom).
;Be aware that these constants need to be changed also in \libsrc\LIN\lincst.h
;SlewRate : (ML_SLEWHIGH=20kbps / ML_SLEWLOW=10kbps / ML_SLEWFAST=max (fast protocol))
;ML_SLEWHIGH equ 0
;ML_SLEWLOW equ 1
;ML_SLEWFAST equ 2

cvON equ 1
cvOFF equ 0

cvFRAME equ 1
cvIDFILT equ 2
cvFUNCT equ 3

;coFILTMODE can be cvIDFILT or cvFRAME
coFILTMODE equ cvIDFILT

; ROM tables base addresses - if these addresses have to be changed, this should be done
;by changing the ROM_TABLE constant in \setup\Chip.mk
#IFDEF ROM_TABLE
_INDXtbl equ ROM_TABLE
_PARAMtbl equ {ROM_TABLE + 64}
_AUTOADDtbl equ {ROM_TABLE + 128}
#ELSE
%FATAL "ROM_TABLE address is not defined"
#ENDIF

; Constants used in the Mlx4
; Values for coTIMEOUT
cvTO13 equ 1
cvTO2x equ 2

;---------------------------
;Auto Baudrate Constants
;---------------------------
abBRKOVF equ 25         ; Autobaudrate : overflow value to change the prescaler during break detection
abSHORTOVF equ 53       ;0x35 too long low state at worst case: 23MHz CPUCK, 800 b/s, presca 1 (higher CPUCK should use presca = 2)

abMINPRESCA equ 1
abBRSTART equ 188       ;0xBC, BR and Presca at start are chosen to have a valid wake up what ever the frequency is.
abPRESCASTART equ 3     ;0xBC, presca 3 -> 10K at 30MHz, 3K at 10 MHz -> 5Tbits are always within {250us,5000us}

; The following constants SHOULD NOT BE CHANGED, from them depends the communication between the Mlx4 and the MLX16.
;They should be consistent with \libsrc\LIN\lincst.h
tx equ 0
rx equ 1

chk13 equ 0
chk20 equ 1

TRUE equ 1
FALSE equ 0
ENABLED equ TRUE
DISABLED equ FALSE
AUTORESET equ 2
FORCE equ 3

;---------------------------
;MLX16 to MLX4 Command Codes
;---------------------------
pcNONE equ 0
pcSTCH equ 1
pcCNFBR equ 2
pcRX equ 3
pcTX equ 4
pcSETFRAME equ 5
pcCNFID equ 6
pcCNFSR equ 7
pcSFTVER equ 10
pcOPTION equ 11
pcGETST equ 12
pcCNFAUTO equ 13
pcFCNFIG equ 14
pcRELBUF equ 15

;---------------------------
;MLX16 to MLX4 Sub Command Codes
;---------------------------
pcDISCARD equ 0
pcDATARDY equ 1
pcCONTINOUSF equ 2

;---------------------------
;MLX4 to MLX16 Event Codes
;---------------------------
;evNONE equ 0
evSTCH equ 1
evERR equ 2
evMESSrcvd equ 3
evMESSrqst equ 4
evENDtx equ 5
evCOOLAUTO equ 7

;---------------------------
;MLX4 States Codes
;---------------------------
stINIT equ 0
stDISC equ 1
stACT equ 2
stSLEEP equ 4
stWKUP equ 8
stSHORT equ 14
stFAST equ 15

;---------------------------
;Command Ack
;---------------------------
ackOK equ 0
ackERR equ 1

;---------------------------
;Sleep Codes
;---------------------------
slMST equ 1
slAPP equ 2
slWKUP equ 3
slWKUPabort equ 4
slWKUPresp equ 5
slWKUPbreak equ 8
slTO equ 6
slTODOM equ 7
slREG equ 10

;---------------------------
;Error Codes
;---------------------------
erNOERR equ 0
erSHORTDONE equ 1
erCRASH equ 2
erIDPAR equ 3
erCKSUM equ 4
erTXCOL equ 5
erRX equ 6
erIDSTOP equ 7
erSYNC equ 8
erRXOVR equ 9
erSHORT equ 10
erTORESP equ 11
erBRFRM equ 12
erWKUPINIT equ 15

cnfAUTO equ 0
cnfSRATE equ 1

;---------------------------
;Error Sub Codes
;---------------------------
erSYNClo equ 1
erSYNChi equ 2
erSYNCsbhi equ 3
erSYNCbhi equ 4
erSYNChead equ 5

erCRASHIT equ 0
erCRASHPLL equ 1
erCRASHTX equ 2

erRXSTART equ 1
erRXSTOP equ 2

verLIN equ 1
verEDIT equ 2

;---------------------------
;Mark Instruction Codes
;---------------------------
;msb set if xdma is following the xdcom
;group A : errors, reset - x00x xxxx
dcRST equ 1			;01h
dcWAIT equ 2		;02h
dcERROR equ 131		;83h
dcBREAKER equ 4		;04h
dcSLEEP equ 5		;05h
dcERRORDEL equ 6    ;06h
dcVER equ 134		;86h
dcEDIT equ 135		;87h
;group B : dialog Mlx4/Mlx16, unrecognized ID - x01x xxxx
dcEVENT equ 166		;A6h
dcCOMMAND equ 167	;A7h
dcID equ 168		;A8h
;group C : break, sync, byte - x10x xxxx
dcBREAK equ 74		;4Ah
dcSYNC equ 203		;CBh
dcSTART equ 76		;4Ch
dcSTOP equ 77		;4Dh
dcDATA equ 206		;CEh
dcDATATX equ 207	;CFh
;fast protocol
dcFAST equ 80		;50h: enter fast protocol
dcFASTSOF equ 81	;51h: start of frame received
dcFASTBYTE equ 210	;D2h: byte received
dcFASTTX equ 211	;D3h: byte sent
dcFASTPCI equ 213	;D5h: PCI received
;wake-up and auto-addressing
dcWAKEUP equ 84		;54h: wake up pulse sent
dcAUTOADD equ 85	;55h: auto-addressing pulse sent
;debug
dcDEBUG equ 212		;D4h

;---------------------------
;Tword access mode
;---------------------------
#IFDEF COLIN_MODULE
regTword equ {0A0h or 01h}	; 1 for COLIN Module
#ELSE
regTword equ {0A0h or 00h}	; 0 for other cases
#ENDIF
					
;end
