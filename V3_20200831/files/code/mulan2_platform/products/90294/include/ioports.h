/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Software Platform 90294
 *
 */

#ifndef IOPORTS_H_
#define IOPORTS_H_

#include <mmc16_io.h>


/* Pack struct's content without alignment */
#define __PACKED_ATTR   __attribute__ ((packed))

/* -----------------------------------------------------------------------------
 * System service to switch to System mode (product specific,
 * see product's linker script to define address of system_services section)
 * ( system_services defined in ..\..\..\libsrc\startup\sys_srv.S )
 * -----------------------------------------------------------------------------
 */
#if defined (__WITH_ITC_BUG_WORKAROUND__)

#define __SYS_ENTER_PROTECTED_MODE  __asm__ __volatile__ (          \
                                                    "mov R, #0\n\t" \
                                                    "call fp0:0x98" \
                                    )
#else

#define __SYS_ENTER_PROTECTED_MODE  __asm__ __volatile__ (          \
                                                    "call fp0:0x98" \
                                    )

#endif /* __WITH_ITC_BUG_WORKAROUND__ */

/*
 *  Product specific SYSTEM and USER IO ports
 */

/* ============================================================================
 *  Product specific SYSTEM ports
 * (accessed only in system mode of CPU)
 * ============================================================================
 */

/* See in <mmc16_io.h> common ports */


/* ANAOUTB */
extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint16 OUTCURRENT : 3;
    uint16 LNA_GAIN_SPEED : 3;
    uint16 LNA_GAIN_DIR : 3;
    uint16 DIR_EN : 1;
    uint16 : 6;
} AnaOutB_IO_v __attribute((nodp, addr(0x201E), aligned(2)));


/* ROM_BIST */
FARPORTW    (ROM_BIST,   0x202A)
FARPORTB    (ROM_BIST_L, 0x202A)
FARPORTB    (ROM_BIST_H, 0x202B)

FARPORTW    (SIGNATURE,   0x202A)      /* ROM_BIST [15:0], read-only: */
FARPORTB    (SIGNATURE_L, 0x202A)
FARPORTB    (SIGNATURE_H, 0x202B)

extern volatile union {

    /* write-only */
    struct __PACKED_ATTR {
        /* little end first */
        uint16 Mode      : 2;
        uint16 Xib       : 1;
        uint16           : 13;
    } w;

    /* read-only */
    const struct __PACKED_ATTR {
        uint16 Signature ;
    } r;

} RomBist_v __attribute((nodp, addr(0x202A), aligned(2)));

/* ROM BIST Mode */
enum {
    ROM_BIST_INACTIVE = 0,
    ROM_BIST_UPDOWN   = 1,
    ROM_BIST_RANDOM   = 2,
    ROM_BIST_BOTH     = 3 };

/* ROM BIST XIB */
enum {
    ROM_BIST_INTERNAL = 0,  /* Use only for internal memory */
    ROM_BIST_EXTERNAL = 1 };


/* RAM_BIST - Two words */
FARPORTW    (RAM_BIST0,   0x202C)
FARPORTB    (RAM_BIST0_L, 0x202C)
FARPORTB    (RAM_BIST0_H, 0x202D)

FARPORTW    (RAM_BIST1,   0x202E)
FARPORTB    (RAM_BIST1_L, 0x202E)
FARPORTB    (RAM_BIST1_H, 0x202F)

FARPORTB    (WRONG_DATA,      0x202D)  /* RAM_BIST1_H [7 :0], read-only: */
FARPORTB    (WRONG_ADDRESS_L, 0x202E)  /* RAM_BIST0_H [7 :0], read-only: */
FARPORTB    (WRONG_ADDRESS_H, 0x202F)  /* RAM_BIST1_L [11:8], read-only: */

/*
 * RAM BIST
 */
extern volatile struct __PACKED_ATTR {
    /* little end first */
          uint16 Mode       : 1;
    const uint16 Phase      : 3;  /* read-only */   /* New update from Samuel */
    const uint16 Failed     : 1;  /* read-only */
          uint16            : 3;
    const uint16 Data       : 8;  /* read-only */
    const uint16 Wrong_addr : 12; /* read-only */
          uint16            : 4;
} RamBist_v __attribute((nodp, addr(0x202C), aligned(2)));

/* ADC I/O and Test */
FARPORTW (ADC_SETTS_IO,   0x2030)
FARPORTB (ADC_SETTS_IO_L, 0x2030)
FARPORTB (ADC_SETTS_IO_H, 0x2031)

/* TBD - What write in this register ??? */
/* (Question to Analog Disign) */
extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint16 FT           : 4;
    uint16              : 4;
    uint16 TST_Strobe   : 1;
    uint16 TST_NegImp   : 1;
    uint16 TST_Ref      : 1;
    uint16 TST_CMPI     : 1;
    uint16 TST_OA       : 1;
    uint16 TST_BIST     : 1;
    uint16 TST_CMP      : 1;
    uint16              : 1;
} AdcSetts_IO_v __attribute((nodp, addr(0x2030), aligned(2)));

/* ADC in */
FARPORTW (ADC_IN,   0x2032) /* [14:0] ADC measurement from "adc_sa_ctrl" */
FARPORTB (ADC_IN_L, 0x2032)
FARPORTB (ADC_IN_H, 0x2033)

/* Read-only */
extern volatile const struct __PACKED_ATTR {
    /* little end first */
    uint16 Value        : 15;
    uint16              : 1;
} AdcIn_v __attribute((nodp, addr(0x2032), aligned(2)));


/* TEST_1 */
FARPORTW (TEST_1,   0x2034) /* Read-only */
FARPORTB (TEST_1_L, 0x2034)
FARPORTB (TEST_1_H, 0x2035)

FARPORTB (ADCSREF,  0x2034) /* TEST_1_L */
FARPORTB (ADCSIN,   0x2035) /* TEST_1_H */

/* Read-only */
extern volatile const struct __PACKED_ATTR {
    /* little end first */
    uint16 AdcSref            : 8;
    uint16 AdcSin             : 8;
} Test1_v __attribute((nodp, addr(0x2034), aligned(2)));


/* TEST_2 */
FARPORTW (TEST_2,   0x2036)
FARPORTB (TEST_2_L, 0x2036)
FARPORTB (TEST_2_H, 0x2037)

extern volatile struct __PACKED_ATTR { /* can be accessed if TestRESERVED2_WE=1 */
    /* little end first */
    uint16 Dig_TO_SEL_OUT        : 5;
    uint16 TESTMUXSEL_0_2_TEST   : 3;
    uint16 TESTMUXSEL_3_TEST     : 1;
    uint16                       : 4;
    uint16 OUT_DISABLE_portmap   : 1;
    uint16 TEST_ENABLE_portmap   : 1;
    uint16 VdigOnTestB_portmap   : 1;
} Test2_v __attribute((nodp, addr(0x2036), aligned(2)));

/* TEST_3 */
FARPORTW (TEST_3,   0x2038)
FARPORTB (TEST_3_L, 0x2038)
FARPORTB (TEST_3_H, 0x2039)

extern volatile struct __PACKED_ATTR { /* can be accessed if TestRESERVED3_WE=1 */
    /* little end first */
    uint16 Dig_TO_SEL_OUT          : 5;
    uint16 TESTMUXSEL_OUT          : 3;
    uint16 AnalogByfferBypass      : 1;
    uint16 Out_Test_Data           : 1;
    uint16 ADC_TEST                : 1;
    uint16 PLATESHORTED            : 1;
    uint16 OVERCLK                 : 1;
    uint16                         : 3;
} Test3_v __attribute((nodp, addr(0x2038), aligned(2)));

/* TEST_3 */
FARPORTW (TEST_4,   0x203A) /* Read - only */
FARPORTB (TEST_4_L, 0x203A)
FARPORTB (TEST_4_H, 0x203B)

extern volatile const struct __PACKED_ATTR {
    /* little end first */
    uint16 Low_Analog_Comp         : 1;
    uint16 Middle_Analog_Comp      : 1;
    uint16 High_Analog_Comp        : 1;
    uint16 Low_Analog_Comp_M       : 1;
    uint16 Middle_Analog_Comp_M    : 1;
    uint16 High_Analog_Comp_M      : 1;
    uint16 Low_Analog_Comp_FSM     : 1;
    uint16 Middle_Analog_Comp_FSM  : 1;
    uint16 High_Analog_Comp_FSM    : 1;
    uint16 dBx_Analog_Comp         : 1;
    uint16 dBx_Analog_Comp_M       : 1;
    uint16 dBx_Analog_Comp_dig     : 1;
    uint16 OCL_dBz                 : 1;
    uint16 OCL_dBz_P               : 1;
    uint16                         : 2;

} Test4_v __attribute((nodp, addr(0x203A), aligned(2)));

/* TEST_5 */
FARPORTW (TEST_5,   0x203C)
FARPORTB (TEST_5_L, 0x203C)
FARPORTB (TEST_5_H, 0x203D)

extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint16 ADC_Test_L_H_WE         : 1;
    uint16 Test2_WE                : 1;
    uint16 Test3_WE                : 1;
    uint16                         : 13;

} Test5_v __attribute((nodp, addr(0x203C), aligned(2)));


/* SpeedGDIDO_Buff */
FARPORTB (SPEED_GDIDO_BUFF, 0x203E)
/* DirectionGDIDO_Buff */
FARPORTB (DIR_GDIDO_BUFF,   0x203F)


/* ============================================================================
 *  Product specific USER ports
 * (accessed either in user or system mode of CPU)
 * ============================================================================
 */


/* XIN */
extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint16 MT3V           : 1;
    uint16 MT4V           : 1;
    uint16 CMP_A          : 2;
    uint16 CM_B           : 2;
    uint16 CM_CNT         : 1;
    uint16 HALFMMSHIFT    : 1;
} XIN_v __attribute((io, addr(0x2805), aligned(2)));


/* MMC16 (RTL) doen't support bit access */
/* This possibility was only in intermediate version of RTL */
/* #define USER_PORT_BIT_ACCESS_ENABLE */

#ifdef USER_PORT_BIT_ACCESS_ENABLE
    #define USERPORTBBIT       PORTBBIT
    #define USERPORTBIT_ACCESS bit_access
#else
    #define USERPORTBBIT       PORTB
    #define USERPORTBIT_ACCESS /* Nothing */
#endif

/* See in <mmc16_io.h> common ports */

USERPORTBBIT (ANALOG_BUFFERS_ADC_MIN_MAX_CONF, 0x281A)

extern volatile struct {
    uint8 :0; uint8 b:1;
} SpeedCopyOnTooth_v  __attribute((io, USERPORTBIT_ACCESS, addr(0x281A)));

extern volatile struct {
    uint8 :1; uint8 b:1;
} SpeedCopyOnValley_v __attribute((io, USERPORTBIT_ACCESS, addr(0x281A)));

extern volatile struct {
    uint8 :2; uint8 b:1;
} DirCopyOnTooth_v    __attribute((io, USERPORTBIT_ACCESS, addr(0x281A)));

extern volatile struct {
    uint8 :3; uint8 b:1;
} DirCopyOnValley_v   __attribute((io, USERPORTBIT_ACCESS, addr(0x281A)));

extern volatile struct {
    uint8 :4; uint8 b:1;
} ResetOnTooth_v      __attribute((io, USERPORTBIT_ACCESS, addr(0x281A)));

extern volatile struct {
    uint8 :5; uint8 b:1;
} ResetOnValley_v     __attribute((io, USERPORTBIT_ACCESS, addr(0x281A)));

extern volatile struct {
    uint8 :6; uint8 b:1;
} SaveOnTooth_v       __attribute((io, USERPORTBIT_ACCESS, addr(0x281A)));

extern volatile struct {
    uint8 :7; uint8 b:1;
} SaveOnValley_v      __attribute((io, USERPORTBIT_ACCESS, addr(0x281A)));


USERPORTBBIT (OUTCTRL__OUT_DRV_CTRL,           0x281B)

extern volatile struct {
    uint8 :0; uint8 b:1; /* Set Tooth  Event from F/W in Digital mode */
} EventTooth_v           __attribute((io, USERPORTBIT_ACCESS, addr(0x281B)));

extern volatile struct {
    uint8 :1; uint8 b:1; /* Set Valley Event from F/W in Digital mode */
} EventValley_v          __attribute((io, USERPORTBIT_ACCESS, addr(0x281B)));

extern volatile struct {
    uint8 :2; uint8 b:1; /* Level (=0) or Pulse (=1) mode */
} LevelB_Pulse_v         __attribute((io, USERPORTBIT_ACCESS, addr(0x281B)));

extern volatile struct {
    uint8 :3; uint8 b:1; /* Level (=0) or Pulse (=1) mode */
} AllowTooth_v           __attribute((io, USERPORTBIT_ACCESS, addr(0x281B)));

extern volatile struct {
    uint8 :4; uint8 b:1;
} bypassDAC_LPF_DIR_v    __attribute((io, USERPORTBIT_ACCESS, addr(0x281B)));

extern volatile struct {
    uint8 :5; uint8 b:1;
} bypassDAC_LPF_SPEED_v  __attribute((io, USERPORTBIT_ACCESS, addr(0x281B)));

extern volatile struct {
    uint8 :6; uint8 b:1;
} CAMCRANKB_v            __attribute((io, USERPORTBIT_ACCESS, addr(0x281B)));

extern volatile struct {
    uint8 :7; uint8 b:1;
} reserved_281B_7_v      __attribute((io, USERPORTBIT_ACCESS, addr(0x281B)));


USERPORTBBIT (SVT_COMP_STACKAT_UNMASK_CTRL,  0x281C)

extern volatile struct {
    uint8 :0; uint8 b:1;
} SuperVisorTimer_Enable_v            __attribute((io, USERPORTBIT_ACCESS, addr(0x281C)));

extern volatile struct {
    uint8 :1; uint8 b:1; /* Reset Capture value by Tooth  Interrupt */
} SVT_Captured_by_Tooth_v             __attribute((io, USERPORTBIT_ACCESS, addr(0x281C)));

extern volatile struct {
    uint8 :2; uint8 b:1; /* Reset Capture value by Valley Interrupt */
} SVT_Captured_by_Valley_v            __attribute((io, USERPORTBIT_ACCESS, addr(0x281C)));

extern volatile struct {
    uint8 :3; uint8 b:1; /* Tooth/Valley interrupt will be only one after Timeout */
} ToothValleyDet_Unmask_on_timeout_v  __attribute((io, USERPORTBIT_ACCESS, addr(0x281C)));

extern volatile struct {
    uint8 :4; uint8 b:3;
} reserved_281C_4_5_6_v               __attribute((io, USERPORTBIT_ACCESS, addr(0x281C)));

extern volatile struct {
    uint8 :7; uint8 b:1;
} Unmask_Valley_int_v                 __attribute((io, USERPORTBIT_ACCESS, addr(0x281C)));


USERPORTBBIT (OUT_STATUS,  0x281D)

extern volatile struct {
    uint8 :0; uint8 b:1;
} Tooth_Detected_ACK_v       __attribute((io, USERPORTBIT_ACCESS, addr(0x281D)));

extern volatile struct {
    uint8 :1; uint8 b:1;
} Valley_Detected_ACK_v      __attribute((io, USERPORTBIT_ACCESS, addr(0x281D)));

extern volatile struct {
    uint8 :2; uint8 b:1;
} FSM_Tooth_Detected_ACK_v   __attribute((io, USERPORTBIT_ACCESS, addr(0x281D)));

extern volatile struct {
    uint8 :3; uint8 b:1;
} FSM_Valley_Detected_ACK_v  __attribute((io, USERPORTBIT_ACCESS, addr(0x281D)));

extern volatile struct {
    uint8 :4; uint8 b:4;
} reserved_281D_4_5_6_7_v    __attribute((io, USERPORTBIT_ACCESS, addr(0x281D)));



USERPORTBBIT (OUT_CTRL,  0x281E)

extern volatile struct {
    uint8 :0; uint8 b:1;
} OCL_DigitalB_AnalogMode_v    __attribute((io, USERPORTBIT_ACCESS, addr(0x281E)));

extern volatile struct {
    uint8 :1; uint8 b:1;
} OCL_WithDirection_v          __attribute((io, USERPORTBIT_ACCESS, addr(0x281E)));

extern volatile struct {
    uint8 :2; uint8 b:1;
} OCL_DirectionLock_v          __attribute((io, USERPORTBIT_ACCESS, addr(0x281E)));

extern volatile struct {
    uint8 :3; uint8 b:1;
} OCL_StandStill_v             __attribute((io, USERPORTBIT_ACCESS, addr(0x281E)));

extern volatile struct {
    uint8 :4; uint8 b:1;
} OCL_dBz_DigFilter_BypassB_v  __attribute((io, USERPORTBIT_ACCESS, addr(0x281E)));

extern volatile struct {
    uint8 :5; uint8 b:1;
} OCL_dBx_DigFilter_BypassB_v  __attribute((io, USERPORTBIT_ACCESS, addr(0x281E)));

extern volatile struct {
    uint8 :6; uint8 b:1;
} OCL_dBz_DigFilter_Enable_v   __attribute((io, USERPORTBIT_ACCESS, addr(0x281E)));

extern volatile struct {
    uint8 :7; uint8 b:1;
} OCL_dBx_DigFilter_Enable_v   __attribute((io, USERPORTBIT_ACCESS, addr(0x281E)));


USERPORTBBIT (dBz_FSM_INT_CTRL,  0x281F)

extern volatile struct {
    uint8 :0; uint8 b:1;
} dBz_FSM_Enable_v                 __attribute((io, USERPORTBIT_ACCESS, addr(0x281F)));

extern volatile struct {
    uint8 :1; uint8 b:1;
} FSM_dBz_Hold25us_Buff_Enable_v   __attribute((io, USERPORTBIT_ACCESS, addr(0x281F)));

extern volatile struct {
    uint8 :2; uint8 b:3;
} reserved_281F_2_3_4_v            __attribute((io, USERPORTBIT_ACCESS, addr(0x281F)));

/*
 *  Was removed
extern volatile struct {
    uint8 :5; uint8 v:1;
} Enable_Sanity_Checker_v          __attribute((io, USERPORTBIT_ACCESS, addr(0x281F)));
*/

extern volatile struct {
    uint8 :6; uint8 v:1;
} Disable_Comp_Reassign_v          __attribute((io, USERPORTBIT_ACCESS, addr(0x281F)));

/* Was removed from RTL
extern volatile struct {
    uint8 :7; uint8 v:1;
} Slow_Fast_freq_Ctrl_WE_v         __attribute((io, USERPORTBIT_ACCESS, addr(0x281F)));
*/

/* ----- End of IO:(0x00..0x1F) bit addressing ----- */



/* ADC Max Block
 *   Reading current max value,
 * ! Byte Writing (dummy value) -> reset ADCMAX in 0x0000
 */
PORTW (ADCMAX,   0x2820) /* Read-only [14:0] */
PORTB (ADCMAX_L, 0x2820)
PORTB (ADCMAX_H, 0x2821)

/* Only read */
extern volatile const struct __PACKED_ATTR {
    /* little end first */
    uint16 Value   : 15;
    uint16         : 1;
} AdcMax_v __attribute((io, addr(0x2820), aligned(2)));

#define ADCMAX_RESET() __asm__ __volatile__ (  /* write dummy value from AL */ \
                                               "mov io:0x20, AL\n\t"           \
                                             )

/* ADC Min Block
 *   Reading current min value,
 * ! Byte Writing (dummy value) -> reset ADCMIN in 0x7FFF
 */
PORTW (ADCMIN,   0x2822) /* Read [14:0] */
PORTB (ADCMIN_L, 0x2822)
PORTB (ADCMIN_H, 0x2823)

/* Only read */
extern volatile const struct __PACKED_ATTR {
    /* little end first */
    uint16 Value   : 15;
    uint16         : 1;
} AdcMin_v __attribute((io, addr(0x2822), aligned(2)));

#define ADCMIN_RESET() __asm__ __volatile__ (  /* write dummy value from AL */ \
                                               "mov io:0x22, AL\n\t"           \
                                             )

/* ADC Max Saved Buffer */
PORTW (ADCMAX_SAVED,   0x2824) /* Read [14:0] */
PORTB (ADCMAX_SAVED_L, 0x2824)
PORTB (ADCMAX_SAVED_H, 0x2825)

/* Only read */
extern volatile const struct __PACKED_ATTR {
    /* little end first */
    uint16 Value   : 15;
    uint16         : 1;
} AdcMaxSaved_v __attribute((io, addr(0x2824), aligned(2)));

/* ADC Min Saved Buffer */
PORTW (ADCMIN_SAVED,   0x2826) /* Read [14:0] */
PORTB (ADCMIN_SAVED_L, 0x2826)
PORTB (ADCMIN_SAVED_H, 0x2827)

/* Only read */
extern volatile const struct __PACKED_ATTR {
    /* little end first */
    uint16 Value   : 15;
    uint16         : 1;
} AdcMinSaved_v __attribute((io, addr(0x2826), aligned(2)));

/* SuperVisor Timer TimeOut */
PORTW (SUPVISOR_TIMEOUT,   0x2828) /* write-only */
PORTB (SUPVISOR_TIMEOUT_L, 0x2828)
PORTB (SUPVISOR_TIMEOUT_H, 0x2829)

PORTW (SUPVISOR_COUNTER,   0x2828) /* read-only */
PORTB (SUPVISOR_COUNTER_L, 0x2828)
PORTB (SUPVISOR_COUNTER_H, 0x2829)

/* SuperVisor Timer Capture, read-only */
PORTW (SUPVISOR_CAPTURE,   0x282A)
PORTB (SUPVISOR_CAPTURE_L, 0x282A)
PORTB (SUPVISOR_CAPTURE_H, 0x282B)

#if 0
  /* NOTE! - was removed from RTL */

/* Slow/Fast (12MHz - fast, 6/3 MHz - slow) Frequency control */
PORTB (SLOW_FAST_FREQ_CTRL, 0x282C)

extern volatile struct __PACKED_ATTR {
    /* little end first */
          uint8  req_Slow_FastB  : 1;  /* Request set frequency */
    const uint8  sys_Slow_FastB  : 1;  /* Status of frequency */
          uint8                  : 6;
} SlowFastFreqCtrl_v __attribute((io, addr(0x282C)));
#endif

/* Speed Hysteresis */
PORTB (SPEED_HYST, 0x282D) /* 6 bits */
extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint8  Value  : 6;
    uint8         : 2;
} SpeedHyst_v __attribute((io, addr(0x282D)));

/* Speed Gain DIDO */
PORTW (SPEEDGDIDO,   0x282E)
PORTB (SPEEDGDIDO_L, 0x282E)
PORTB (SPEEDGDIDO_H, 0x282F)

PORTB (SPEEDGDIDO_A, 0x282E) /* SPEEDGDIDO_L: [5:0] A Speed Channel */
PORTB (SPEEDGDIDO_B, 0x282F) /* SPEEDGDIDO_H: [5:0] B Speed Channel */

extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint16 A : 6;
    uint16   : 2;
    uint16 B : 6;
    uint16   : 2;
} SpeedGDIDO_v __attribute((io, addr(0x282E), aligned(2)));

/* Thresholds */
PORTW (SPEED_THRESH,   0x2830)      /* [12:0] Speed     Threshold  Write-Only */
PORTB (SPEED_THRESH_L, 0x2830)
PORTB (SPEED_THRESH_H, 0x2831)

PORTW (SPEED_THRESH_BUFF,   0x2830) /* [12:0] Speed     Threshold  Read-Only  */
PORTB (SPEED_THRESH_BUFF_L, 0x2830)
PORTB (SPEED_THRESH_BUFF_H, 0x2831)

extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint16 Value : 13;
    uint16       : 3;
} SpeedThresh_v __attribute((io, addr(0x2830), aligned(2)));

/* Direction Gain DIDO */
PORTW (DIRGDIDO,   0x2832)
PORTB (DIRGDIDO_L, 0x2832)
PORTB (DIRGDIDO_H, 0x2833)

PORTB (DIRGDIDO_A, 0x2832) /* DIRGDIDO_L: [5:0] A Direction Channel */
PORTB (DIRGDIDO_B, 0x2833) /* DIRGDIDO_H: [5:0] B Direction Channel */

extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint16 A : 6;
    uint16   : 2;
    uint16 B : 6;
    uint16   : 2;
} DirGDIDO_v __attribute((io, addr(0x2832), aligned(2)));

PORTW (DIR_THRESH,     0x2834)      /* [12:0] Direction Threshold Write-Only */
PORTB (DIR_THRESH_L,   0x2834)
PORTB (DIR_THRESH_H,   0x2835)

PORTW (DIR_THRESH_BUFF,     0x2834) /* [12:0] Direction Threshold Read-Only */
PORTB (DIR_THRESH_BUFF_L,   0x2834)
PORTB (DIR_THRESH_BUFF_H,   0x2835)

extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint16 Value : 13;
    uint16       : 3;
} DirThresh_v __attribute((io, addr(0x2834), aligned(2)));


/* PTC Status */
PORTB (PTC_STS,  0x2836)

extern volatile struct __PACKED_ATTR {
    /* little end first */
          uint8  R_Ack    : 1; /* "Read" acknowledgment,  Read or Writing "1" to clear */
          uint8  W_Ack    : 1; /* "Write" acknowledgment, Read or Writing "1" to clear */
          uint8  IT_Ack   : 1; /* "Interrupt" ack,        Read or Writing "1" to clear */
    const uint8  PTC_MODE : 1;
          uint8           : 4;
} PTC_Sts_v __attribute((io, addr(0x2836)));


/* Polarities CTrl */
PORTB (POLARITIES_CTRL,     0x2837)

extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint8  OCL_DirectionPol       : 1;
    uint8  OCL_SpeedPol           : 1;
    uint8  DirectionPol_Analog    : 1;
    uint8  SpeedPol_Analog        : 1;
    uint8  OCL_ReversePol         : 1;
    uint8  OShap_OutPol           : 1;
    uint8                         : 2;
} Polarities_Ctrl_v __attribute((io, addr(0x2837)));

/* PTC Data */
PORTW (PTCW_DATA,    0x2838) /* Read-only*/
PORTW (PTCR_DATA,    0x2838) /* Write-only */
PORTB (PTC_DATA_L,   0x2838)
PORTB (PTC_DATA_H,   0x2839)

/* Free input */
PORTW (FREE_INPUTS,     0x283A) /* Read-only */
PORTB (FREE_INPUTS_L,   0x283A) /* Read-only */
PORTB (FREE_INPUTS_H,   0x283B) /* Read-only */

extern volatile const struct __PACKED_ATTR {
    /* little end first */
    uint16                         : 3;
    uint16  Low_Analog_Comp_M      : 1;
    uint16  Middle_Analog_Comp_M   : 1;
    uint16  High_Analog_Comp_M     : 1;
    uint16  dBx_Analog_Comp_M      : 1;
    uint16  PROB_TDI               : 1;

    uint16  PROB_TCK               : 1;
    uint16  OUT_TDI                : 1;
    uint16  OUT_TCK                : 1;
    uint16  VDD_TDI                : 1;
    uint16  VDD_TCK                : 1;
    uint16                         : 3;
} FreeInputs_v __attribute((io, addr(0x283A)));

/*********** DEBUG ***************/
FARPORTB( STDOUT, 0x283A ) /*Write-only, for debug purpose*/


/* MIXED Interrupt Status Register */
PORTB (MIXEDINT_STS,   0x283C)
/* Read or write "1" to clear */
extern volatile struct __PACKED_ATTR {
    /* little end first */
          uint8  CompInvalidStates_ACK       : 1;  /* Invalid states in Digital FSM compare */
          uint8                              : 1;
          /*uint8  Sanity_Checker_Error_ACK    : 1;*/ /* Was removed */
          uint8                              : 1;
          uint8                              : 1;
          uint8                              : 1;
    const uint8  AnalogCompInvalidStateStat  : 3;
} MixedIntSts_v __attribute((io, addr(0x283C)));

/* Reserved_2 addr(0x283D) */

/* ANALOG CONFIG */
PORTW (ADC_ANALOG_CFG,   0x283E)
extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint8  TimATimBClkMuxSel   : 1;
    uint8  AdcLowPower         : 1;
    uint8  HE_Left             : 1;
    uint8  HE_Single_config    : 1;
    uint8  TriAxis3HEB         : 1;
    uint8  PLATEALLOFF         : 1;
    uint8  PLATETEST           : 1; /* PLATE_SINGLE */
    uint8  PLATE_RIGHTPOL      : 1;
    uint8  AdcFreqRatio        : 4;
    uint8  AdcMode             : 2;
    uint8  HE_SINGLE_ctrl      : 2;

} AdcAnalogCfg_v __attribute((io, addr(0x283E)));


/* ----- End of IO:(0x00..0x3F) addressing ----- */

/* Analog Chain Settling time (ADC Start Delay) */
FARPORTW (ANA_SETTLE,   0x2840)
FARPORTB (ANA_SETTLE_L, 0x2840)
FARPORTB (ANA_SETTLE_H, 0x2841)
FARPORTB (TIMA, 0x2840) /* ANA_SETTLE_L [6:0] Time A */
FARPORTB (TIMB, 0x2841) /* ANA_SETTLE_H [6:0] Time B */

extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint16 TimA     : 7;
    uint16          : 1;
    uint16 TimB     : 7;
    uint16          : 1;
} AnaSettle_v __attribute((nodp, addr(0x2840), aligned(2)));

/* Output Shaping (OShap) Pulse width */
FARPORTW (OSHAP_P_WIDTH,   0x2842)
FARPORTB (OSHAP_P_WIDTH_L, 0x2842)
FARPORTB (OSHAP_P_WIDTH_H, 0x2843)

FARPORTB (REVERSE_P_WIDTH, 0x2842) /* OSHAP_P_WIDTH_L [6:0] */
FARPORTB (FORWARD_P_WIDTH, 0x2843) /* OSHAP_P_WIDTH_H [6:0] */

extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint16 Forward  : 7;
    uint16          : 1;
    uint16 Reverse  : 7;
    uint16          : 1;
} OShapPulseWidth_v __attribute((nodp, addr(0x2842), aligned(2)));

/* Power Safe Mode */
FARPORTB (POWERSAVE, 0x2844)

extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint8  TempNeverDis  : 1;
    uint8  TempAlwaysDis : 1;
    uint8  DidoNeverDis  : 1;
    uint8  DidoAlwaysDis : 1;
    uint8  AdcNeverDis   : 1;
    uint8  AdcAlwaysDis  : 1;
    uint8                : 2;
} PowerSave_v __attribute((nodp, addr(0x2844)));


/* NOTE! - was change in RTL */
/* Fast/Slow frequency configuration */
FARPORTB (OShap_CKTRIM,   0x2845)

extern volatile struct __PACKED_ATTR {
    /* little end first */
    uint8  v                     : 6; /* (equal to MMC16 CKTRIM) */
    uint8                        : 2;
} OShap_CKTRIM_v __attribute((nodp, addr(0x2845)));

#endif /* IOPORTS_H_ */

/* EOF */
