/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */
 
#ifndef IOPORTS_H_
#define IOPORTS_H_

#include <mmc16_io.h>

/* -------------------------------------------------------------------------
 * System service to switch to System mode (product specific,
 * see product's linker script to define the address of
 * system_services section)
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


/* ----------------------------------------------------------------------------
 *  Product specific SYSTEM and USER IO ports
 */

/* ============================================================================
 *  Product specific SYSTEM ports
 * (accessed only in system mode of CPU)
 * ============================================================================
 */

/* Not Present */
FARPORTW       (ANA2_TEST, 0x202A)      /* analog test register, not used */
FARPORTB       (ANA2_TEST_L, 0x202A) 
FARPORTB       (ANA2_TEST_H, 0x202B) 

FARPORTW       (ANA3_TEST, 0x202C)      /* analog test register, not used, not initialized by HW (x) */
FARPORTB       (ANA3_TEST_L, 0x202C)
FARPORTB       (ANA3_TEST_H, 0x202D)


/* Calibration register, only used at initialization
 * Reset value: 0x0000
 */
FARPORTW       (CAL_REG, 0x202E)                /* calibration register */
/* [15:10]   reserved */
#define     STOP_ADC12_CHOP         (1u << 9)   /* stop chopping of ADC (in analog)*/
/* [8:6]   reserved */
#define     OSPI_XMIT_OVERLAP       (1u << 5)   /* OSPI transmitter overlap allowed */
#define     LOW_DEPTH_MODULATION    (1u << 4)   /* Low Supply Current Modulation depth */
#define     ADC_LOW_NOISE           (1u << 3)   /* but worse for EMC */
#define     ADC_FAST                (1u << 2)   /* (0) low-power mode @1.33MHZ, (1) fast-mode @ 2.66 MHz */
#define     SFT1                    (1u << 1)   /* analog self tests configuration bit  */
#define     SFT0                    (1u << 0)   /* analog self tests configuration bit  */
#define     SFT_MASK                (3u << 0)

FARPORTB       (CAL_REG_L, 0x202E)
FARPORTB       (CAL_REG_H, 0x202F)


/* Calibration register 2 and 3,  only used at initialization
 * Reset value: 0x00
 */
FARPORTW       (CAL2_CAL3_REG, 0x2030)           /* calibration register 2 */
/* [15:11]  reserved */
#define     OSPI_TRISTATE_MODE2     (1u << 10)
#define     OSPI_RX_THRESHOLD_MASK  (3u << 8)
/* [7]      reserved */
#define     OSPI_CLK_EN             (1u << 6)
/* [5:4]    reserved */
#define     DMA_TEST                (1u << 3)
#define     PDCUR_EN                (1u << 2)
#define     ADC_MODE_15_BITS        0
#define     ADC_MODE_16_BITS        1
#define     ADC_MODE_17_BITS        2
#define     ADC_MODE_18_BITS        3
#define     ADC_MODE_MASK           (3u << 0)

FARPORTB       (CAL2_REG, 0x2030)           /* calibration register 2 */
FARPORTB       (CAL3_REG, 0x2031)           /* calibration register 3 */



/* COMM (aka UART mode) interface */
FARPORTW       (COMM_DATA, 0x2034) /* r/w: COMM_DATA[15:0]; reset value: 0x0000 */
FARPORTW       (COMM_CTRL, 0x2036) /* COMM control and COMM_DATA[16]; reset value: xxxx-xxxx-xxxx-0000    */
#define         COMM_WRITE          (1u << 15)  /* 0: read from COMM; 1: write to COMM */
/* [14:4] reserved          */
#define         KEY_REG_BIT2        (1u << 3)   /* [ 3:1] read-only KEY_REG */
#define         KEY_REG_BIT1_BIT0   (3u << 1)   /* [ 3:1] read-only KEY_REG */
#define         COMM_RX_DATA16      (1u << 0)   /* read-only, COMM_DATA[16] */


/* OSPI interface logic mode register
 *  Reset value: xx11-1111-xx00-0000
 */
FARPORTW       (OIL_MODE,   0x2038)             /* OSPI interface logic mode register */
#define        P2_COPY_ERROR_N      (1u << 15)	/* r/w: indicates that HW copying of pressure data from P2_BUF_N to P2_OUT_N failed (P2_OUT_N != P2_BUF_N)
                                                 * set by HW; cleared by SW; not initialized by HW (x)
                                                 */
#define        T2_COPY_ERROR_N      (1u << 14)	/* r/w: indicates that HW copying of temperature data from T2_BUF_N to T2_OUT_N failed (T2_OUT_N != T2_BUF_N)
                                                 * set by HW; cleared by SW; not initialized by HW (x)
                                                 */
#define        STATR_CLR_N          (1u << 13)  /* read-only: request to clear status register (inverted)   */
#define        SRST_N               (1u << 12)  /* read-only: soft reset request (inverted)                 */
#define        TSSM_N               (1u << 11)  /* r/w: temperature sample mode (inverted), 0: single-shot mode, 1: continuous mode   */
#define        PSSM_N               (1u << 10)  /* r/w: pressure sample mode (inverted),    0: single-shot mode, 1: continuous mode   */
#define        WPDIS_N              (1u << 9)   /* read-only: EEPROM write protection disabled (inverted)   */
#define        TMEN_N               (1u << 8)   /* read-only: Test mode enabled (inverted)                  */
#define        P1_COPY_ERROR        (1u << 7)	/* r/w: indicates that HW copying of pressure data from P1_BUF to P1_OUT failed (P1_OUT != P1_BUF)
                                                 * set by HW; cleared by SW; not initialized by HW (x)
                                                 */
#define        T1_COPY_ERROR        (1u << 6)	/* r/w: indicates that HW copying of temperature data from T1_BUF to T1_OUT failed (T1_OUT != T1_BUF)
                                                 * set by HW; cleared by SW; not initialized by HW (x)
                                                 */
#define        STATR_CLR            (1u << 5)   /* read-only: request to clear status register  */
#define        SRST                 (1u << 4)   /* read-only: soft reset request                */
#define        TSSM                 (1u << 3)   /* r/w: temperature sample mode, 1: single-shot mode, 0: continuous mode */
#define        PSSM                 (1u << 2)   /* r/w: pressure sample mode,    1: single-shot mode, 0: continuous mode */
#define        WPDIS                (1u << 1)   /* read-only: EEPROM write protection disabled  */
#define        TMEN                 (1u << 0)   /* read-only: Test mode enabled                 */


/* OSPI interface logic status register mask
 *  Normally only the OIL is writing to these bits, but the MCU should initialize
 *  them to the values written in the EEPROM registers SMSKx_INIT (+ read back)
 *
 *  Not initialized by HW
 *  Reset value: xxxx-xxxx-xxxx-xxxx
 */
FARPORTW       (OIL_MSK, 0x203A)    /* OSPI interface logic status register mask */
/* [15] reserved */
/* [14] SMSK3[6] */
/* [13] reserved */
/* [12] SMSK3[4] */
/* [11] reserved */
/* [10] SMSK3[2] */
/* [9]  reserved */
/* [8]  SMSK3[0] */
/* [7]  SMSK2[7] */
/* [6]  SMSK1[6] */
/* [5]  SMSK2[5] */
/* [4]  SMSK1[4] */
/* [3]  SMSK2[3] */
/* [2]  SMSK1[2] */
/* [1]  SMSK2[1] */
/* [0]  SMSK1[0] */

/* OSPI interface logic status register mask, inverted (dual rail logic)
 *
 * Not initialized by HW
 * Reset value: xxxx-xxxx-xxxx-xxxx
 */
FARPORTW       (OIL_MSK_N, 0x203C)  /* OSPI interface logic status register mask, inverted */
/* [15] SMSK3[7] */
/* [14] reserved */
/* [13] SMSK3[5] */
/* [12] reserved */
/* [11] SMSK3[3] */
/* [10] reserved */
/* [9]  SMSK3[1] */
/* [8]  reserved */
/* [7]  SMSK1[7] */
/* [6]  SMSK2[6] */
/* [5]  SMSK1[5] */
/* [4]  SMSK2[4] */
/* [3]  SMSK1[3] */
/* [2]  SMSK2[2] */
/* [1]  SMSK1[1] */
/* [0]  SMSK2[0] */


/* Pressure update counter:
 *  - incremented by HW every 1us, 8 bits (was 7 bits in 12126AA)
 *  - reset by SW by setting P_UPDT=1
 *
 *  Not initialized by HW
 *  Reset value: xxxx-xxxx-xxxx-xxxx
 */
FARPORTW       (P_UCNT, 0x203E)                     /* Pressure update counter */
#define        P_UPDATE_ERROR       (     1u << 15) /* r/w, pressure update error flag; set by HW when P_UCNT=120; cleared by SW */
#define        P_UCNT_MASK          (0x00FFu <<  0) /* read-only, 8 bits */


/* Temperature update counter:
 *  - incremented by HW every 1us, 12 bits
 *  - reset by SW by setting T_UPDT=1
 *
 *  Not initialized by HW
 *  Reset value: xxxx-xxxx-xxxx-xxxx
 */
FARPORTW       (T_UCNT, 0x2040)                     /* Temperature update counter */
#define        T_UPDATE_ERROR       (     1u << 15) /* r/w, temperature update error flag; set by HW when T_UCNT=2750; cleared by SW */
#define        T_UCNT_MASK          (0x0FFFu <<  0) /* read-only, 12 bits */


/* ============================================================================
 *  Product specific USER ports
 * (accessed either in user or system mode of CPU)
 * ============================================================================
 */
PORTW       (CUST_CTRL_W, 0x281A)               /* custom control/status (whole word)   */
PORTB       (CUST_CTRL_L, 0x281A)               /* custom control/status (low byte)     */
#define     WDATT_OCCURRED          (1u << 7)   /* read-only, WD attention interrupt occurred; normally
                                                 * WD attention should happen only once (during BIST);
                                                 * second WD attention indicates a failure
                                                 */
#define     ISUP_MOD                (1u << 6)   /* bit used to force the current modulation on or off       */
/*  [5:2] : reserved */
#define     DISABLE_RSM             (1u << 1)   /* bit used to disable M half bridge voltage  (new in 12126BA) */
#define     DISABLE_RSP             (1u << 0)   /* bit used to disable P half bridge voltage  (new in 12126BA) */

PORTB       (CUST_CTRL_H, 0x281B)                /* custom control/status (high byte) */
/* [7:6] : reserved */
/* [5:0] : read-only, 6-bit counter in ADC Timer Control Logic unit */
#define     ANA_CNT_MASK            0x3Fu       /* 6 bits  */
#define     ANA_CNT_NO_LSBIT_MASK   0x3Eu       /* take only 5 MSBits and mask LSBit of the counter */


/*
 * ADC control (for debugging), read-only
 * not initialized by HW
 * Reset value: xxxx-xxxx-xxxx-xxxx
 */
PORTW       (ADC_CTRL_DBG, 0x2826)
PORTB       (ADC1_CTRL_DBG, 0x2826) /* read-only: ADC1 control (for debugging) */
#define     ADC1_2LSBS_MASK         (3u << 6) /* 2 lsbs of 17/18 bit ADC result ADC1[1:0], normally not used */
#define     ADC1_CMPA_MASK          (3u << 4) /* compare A */
#define     ADC1_CMPB_MASK          (3u << 2) /* compare B */
#define     ADC1_CMP_CNT            (1u << 1) /* compare count */
#define     ADC1_OVF                (1u << 0) /* overflow */

PORTB       (ADC2_CTRL_DBG, 0x2827) /* read-only: ADC2 control (for debugging) */
#define     ADC2_2LSBS_MASK         (3u << 6) /* 2 lsbs of 17/18 bit ADC result ADC2[1:0], normally not used */
#define     ADC2_CMPA_MASK          (3u << 4) /* compare A */
#define     ADC2_CMPB_MASK          (3u << 2) /* compare B */
#define     ADC2_CMP_CNT            (1u << 1) /* compare count */
#define     ADC2_OVF                (1u << 0) /* overflow */

/* ADC result (for debugging)
 * not initialized by HW
 * Reset value: xxxx-xxxx-xxxx-xxxx
 */
PORTW       (ADC1, 0x2828)      /* read-only: ADC1 result [17:2], 16 MSBs */
PORTW       (ADC2, 0x282A)      /* read-only: ADC2 result [17:2], 16 MSBs */

/* pressure write buffer control
 *
 * not initialized by HW
 * Reset value: 0000-000x-0000-01xx
 */
PORTW       (P_CTRL, 0x282C)                    /* pressure write buffer control, not initialized by HW */
/* bits [15:9] reserved */
#define     P_SOFT_ERROR_N          (1u << 8)   /* r/w by SW; complementary for P_SOFT_ERROR; not initialized by HW             */
/* bits [7:3] reserved */
#define     P_COPY_MASK             (1u << 2)   /* r/w by SW; if (1) blocks HW coping from P1_BUF/P2_BUF_N to P1_OUT/P2_OUT_N   */
#define     P_UPDT                  (1u << 1)   /* rt0 : if (1) clears pressure update counter; bit is automatically returns-to-0 after 1 clock pulse */
#define     P_SOFT_ERROR            (1u << 0)   /* r/w: indicates that writing to P1_BUF/P2_BUF_N failed;
                                                 * set/cleared by SW based on read-back/compare of P1_BUF/P2_BUF_N with original values
                                                 * not initialized by HW
                                                 */
PORTB       (P_CTRL_L, 0x282C) 
PORTB       (P_CTRL_H, 0x282D) /* pressure write buffer control */


/* temperature write buffer control
 * not initialized by HW
 * Reset value: 0000-000x-0000-01xx
 */
PORTW       (T_CTRL, 0x282E)                    /* temperature write buffer control, not initialized by HW */
/* bits [15:9] reserved */
#define     T_SOFT_ERROR_N          (1u << 8)   /* r/w by SW; complementary for T_SOFT_ERROR; not initialized by HW             */
/* bits [7:2] reserved */
/* Bit 2 (T_COPY_MASK) was removed in 12126BA */
#define     T_UPDT                  (1u << 1)   /* rt0 : if (1) clears temperature update counter; bit is automatically returns-to-0 after 1 clock pulse */
#define     T_SOFT_ERROR            (1u << 0)   /* r/w: indicates that writing to T1_BUF/T2_BUF_N failed;
                                                 * set/cleared by SW based on read-back/compare of T1_BUF/T2_BUF_N with original values
                                                 * not initialized by HW
                                                 */
PORTB       (T_CTRL_L, 0x282E) 
PORTB       (T_CTRL_H, 0x282F) /* pressure write buffer control */


/* buffered outputs */
PORTW       (P1_BUF,   0x2830)      /* Pressure ch. 1 output buffer, 12 bits, not initialized by HW: 0000-xxxx-xxxx-xxxx */
PORTW       (P1_OUT,   0x2832)      /* read-only: Pressure ch.1 communicated output, 12 bits; reset value: 0x0000 */
PORTW       (P2_BUF_N, 0x2834)      /* Pressure ch. 2 output, 12 bits, not initialized by HW: 0000-xxxx-xxxx-xxxx */
PORTW       (P2_OUT_N, 0x2836)      /* read-only: Pressure ch. 2 communicated output, 12 bits; reset value: 0x0FFF */


PORTW       (T1_OUT_BUF, 0x2838)    /* Combined T1_OUT and T1_BUF */
PORTB       (T1_BUF,     0x2838)    /* Temperature ch. 1 output buffer, 8 bits, not initialized by HW: xxxx-xxxx    */
PORTB       (T1_OUT,     0x2839)    /* read-only: Temperature ch. 1 communicated output, 8 bits; reset value: 0x00  */

PORTW       (T2_OUT_BUF_N, 0x283A)  /* Combined T2_OUT_N and T2_BUF_N */
PORTB       (T2_BUF_N, 0x283A)      /* Temperature channel 2 output buffer, 8 bits, not initialized by HW: xxxx-xxxx */
PORTB       (T2_OUT_N, 0x283B)      /* read-only: Temperature ch. 2 communicated output, 8 bits; reset value: 0xFF   */


PORTW       (STATUS_MSG_CNT, 0x283C)            /* Combined: Status and Message Counter */
PORTB       (STATUS, 0x283C)                    /* DSP status, not initialized by HW: xxxx-xxxx */
#define     TCLP                    (1u << 7)   /* Temperature clipping           */
#define     PCLP                    (1u << 6)   /* Pressure clipping              */
#define     UV                      (1u << 5)   /* under voltage                  */
#define     RST                     (1u << 4)   /* reset (cleared by OSPI master) */
#define     TDI                     (1u << 3)   /* read-only: Temperature Data Invalid; combined indicator of T1_COPY_ERROR | T_SOFT_ERROR | T_UPDATE_ERROR   */
#define     PDI                     (1u << 2)   /* read-only: Pressure Data Invalid; combined indicator of P1_COPY_ERROR | P_SOFT_ERROR | P_UPDATE_ERROR      */
#define     FAULT                   (1u << 1)   /* indicates not recoverable fault      */
#define     INIT                    (1u << 0)   /* indicates init state                 */

PORTB       (MSG_CNT, 0x283D)                   /* read only: message counter; MSG_CNT is incremented and cleared under control of the OIL */

PORTW       (STATUS_ERR_N, 0x283E)              /* Combined: Status (inverted) and Error registers          */
PORTB       (STATUS_N, 0x283E)                  /* DSP status, inverted, not initialized by HW: xxxx-xxxx   */
#define     TCLP_N                  (1u << 7)   /* Temperature clipping           */
#define     PCLP_N                  (1u << 6)   /* Pressure clipping              */
#define     UV_N                    (1u << 5)   /* under voltage                  */
#define     RST_N                   (1u << 4)   /* reset (cleared by OSPI master) */
#define     TDI_N                   (1u << 3)   /* read-only: Temperature Data Invalid; combined indicator of T2_COPY_ERROR_N | T_SOFT_ERROR_N | ~T_UPDATE_ERROR  */
#define     PDI_N                   (1u << 2)   /* read-only: Pressure Data Invalid; combined indicator of P2_COPY_ERROR_N | P_SOFT_ERROR_N | ~P_UPDATE_ERROR     */
#define     FAULT_N                 (1u << 1)   /* indicates not recoverable fault    */
#define     INIT_N                  (1u << 0)   /* indicates init state               */

PORTB       (ERROR, 0x283F)                     /* read-only; not initialized by HW: xxxx-xxxx */
#define     S_ERR_N                 (1u << 1)
#define     S_ERR                   (1u << 0)

FARPORTW    (OIL_ADDR_DAT, 0x2840)  /* read-only: OIL data and address bus value, not initialized by HW: xxxx-xxxx-xxxx-xxxx; not used */

FARPORTB    (OIL_DAT, 0x2840)       /* read-only: OIL data bus value (8 bits), not initialized by HW: xxxx-xxxx, not used */

FARPORTB    (OIL_ADDR, 0x2841)      /* read-only: OIL address bus value, not initialized by HW: xxxx-xxxx, not used */
#define     OIL_ADDRESS_MASK        (0x7Fu << 0)    /* address bus value */
#define     OIL_RWB                 (   1u << 7)    /* r/w access indicator: reading (1) or writing (0) */

#endif /* IOPORTS_H_ */
