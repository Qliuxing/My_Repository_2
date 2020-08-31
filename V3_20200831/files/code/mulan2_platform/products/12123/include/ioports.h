/*
 * Copyright (C) 2010-2012 Melexis N.V.
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
/* [15:5]   reserved */
#define     LOW_DEPTH_MODULATION    (1u << 4)   /* Low Supply Current Modulation depth */
#define     ADC_LOW_NOISE           (1u << 3)   /* but worse for EMC */
#define     ADC_FAST                (1u << 2)   /* (0) low-power mode, (1) fast-mode    */
#define     SFT1                    (1u << 1)   /* analog self tests configuration bit  */
#define     SFT0                    (1u << 0)   /* analog self tests configuration bit  */
#define     SFT_MASK                (3u << 0)

FARPORTB       (CAL_REG_L, 0x202E)
FARPORTB       (CAL_REG_H, 0x202F)


/* Calibration register 2, only used at initialization
 * Reset value: 0x00
 * Notes: For 12123 CAL2_REG is byte-size (see MLX12123-16 on Jira)
 */
FARPORTB       (CAL2_REG, 0x2030)               /* calibration register 2 */
#define     DIS_OUT1            (1u << 7)
#define     DIS_OUT2            (1u << 6)
#define     ADC_DIV10           (1u << 5)
#define     EN_CUSTCOM_LOWSUP   (1u << 4)
#define     PCM_DIS             (1u << 3)
#define     PDCUR_EN            (1u << 2)
#define     ADC_MODE_15_BITS    0
#define     ADC_MODE_16_BITS    1
#define     ADC_MODE_17_BITS    2
#define     ADC_MODE_18_BITS    3
#define     ADC_MODE_MASK       (3u << 0)


/* COMM (aka UART mode) interface */
FARPORTW       (COMM_DATA, 0x2034) /* r/w: COMM_DATA[15:0]; reset value: 0x0000 */
FARPORTW       (COMM_CTRL, 0x2036) /* COMM control and COMM_DATA[16]; reset value: xxxx-xxxx-xxxx-xx00    */
#define         COMM_WRITE          (1u << 15)  /* 0: read from COMM; 1: write to COMM */
/* [14:4] reserved          */
/* [ 3:1] read-only KEY_REG */
#define         COMM_RX_DATA16      (1u << 0)   /* read-only, COMM_DATA[16] */


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
#define     ISUP_MOD                (1u << 6)   /* bit used to force the current modulation on or off */
#define     OUT2LOW                 (1u << 5)   /* force OUT2 to Low Rail fault band    */
#define     OUT1LOW                 (1u << 4)   /* force OUT1 to Low Rail fault band    */
#define     OUT2HIGH                (1u << 3)   /* force OUT2 to High Rail fault band   */
#define     OUT1HIGH                (1u << 2)   /* force OUT1 to High Rail fault band   */
/* [1:0] : reserved */
#define     OUT_FB_BITS_MASK        (OUT1HIGH | OUT2HIGH | OUT1LOW | OUT2LOW)   /* fault band bits mask */

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


/* buffered outputs: 13-bits */
PORTW       (OUT1_BUF, 0x2830)  /* OUT1 output (aka P1_BUF), 13 bits, not initialized by HW: 000x-xxxx-xxxx-xxxx    */
PORTW       (OUT2_BUF, 0x2834)  /* OUT2 output (aka P2_BUF_N), 13 bits, not initialized by HW: 000x-xxxx-xxxx-xxxx  */

#endif /* IOPORTS_H_ */
