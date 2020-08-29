; ---------------------------------------------------------------------------
;
; Description:
;  MelexCM Constant Definitions
;  DCOM, DMA and IO ports definitions for the Mlx4 periphery
;
;
; Copyright (c) Melexis Digital Competence Center
;
; ---------------------------------------------------------------------------

;--------------- MSBi DCOM ------------------------
#define	WindCtrl0 0
#define	WindCtrl1 1
#define	WindCtrl2 2
#define	WindCtrl3 3
#define	StatCtrl 4
#define	InpCtrl 4
#define	OutCtrl 5
#define	AuxCtrl 6
#define	StuffCtrl 7
#define	PlsCtrl 7
#define	BrCtrl 8
#define	CkCtrl 9
;--------------- MSBi I/O -------------------------
#define	CmpCtrl io:16h
#define	AuxIn io:17h
#define	Stat1 io:18h
#define	Stat0 io:19h
#define	Cfg0 io:1Ah
#define	Cfg1 io:1Bh
#define	DbCnt io:1Ch
#define	IdIdx io:1Dh
#define	PrescA io:1Eh
#define	PrescB io:1Fh
;--------------- MSBi DMA -------------------------
#define	Br 0
#define	IntBr 0
#define	CmpRW 1
#define	CmpSetId 1
#define	CmpChk 2
#define	CmpTst 2
;--------------- Mlx4 Dig I/0 ---------------------
#define	XcgStat io:0h
#define	XcgDr1 io:1h
#define	XcgDr3 io:2h
#define	AnalogStat io:3h
#define	ComDout io:4h
#define	RamRxt io:5h
#define	SleepStat io:6h
#define	MessStat io:7h
;indexed I/O
#define	WatchPre io:7h
#define	WatchPreIdx 0
#define	WatchCnt io:7h
#define	WatchCntIdx 1
#define	SleepPre io:7h
#define	SleepPreIdx 2
#define	SleepCnt io:7h
#define	SleepCntIdx 3
#define	MessPre io:7h
#define	MessPreIdx 4
#define	MessCnt io:7h
#define	MessCntIdx 5
#define	EeW0 io:7h
#define	EeW0Idx 6
#define	EeW1 io:7h
#define	EeW1Idx 7
#define	EeW2 io:7h
#define	EeW2Idx 8
#define	EeW3 io:7h
#define	EeW3Idx 9
;--------------- Mlx4 Dig DCOM --------------------
#define	MemRam 10
#define	Timer 11
#define	MemTab 11
#define	AnIo 12
#define	Int 13
#define	Flags 14
#define	System 15
#define	FSafe 15
;--------------------------------------------------
