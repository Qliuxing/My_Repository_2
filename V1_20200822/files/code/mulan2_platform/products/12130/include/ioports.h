/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Software Platform
 *
 */
 
#ifndef IOPORTS_H_
#define IOPORTS_H_

#include <mmc16_io.h>

#if 0
/*
 * Port aliases from ZAO code (to be removed aftre integration)
 */
#warning "Port aliases to be removed after integration"

FARPORTW    (MPU_1, 0x2030)
FARPORTW	(MPU_2, 0x2032)
FARPORTW	(MPU_3, 0x2034)
FARPORTW	(U_CUST, 0x204E)

PORTW       (FAST_DATA1, 0x283A)                /* SENT fast channel nibbles 4,5 and 6 */
PORTW       (FAST_DATA2, 0x283C)                /* SENT fast channel nibbles 7 and 8 (byte)   */

FARPORTB    (SENT_STATUS, 0x2842)               /* SENT communication status bits */
/* [7:2] : reserved */
#define     SENT_STATUS_MASK        0x0Fu       /* SENT status bits mask */
#define     SENT_FAST_CH1_STATUS    (1u << 1)   /* SENT fast channel 1 status */
#define     SENT_FAST_CH0_STATUS    (1u << 0)   /* SENT fast channel 0 status */

PORTW       (SERIAL_DATA, 0x283E)               /* SENT slow channel data */

FARPORTW    (SERIAL_ID, 0x2840)                 /* SENT slow channel ID */


/* [15:11] : reserved */
#define     SENT_SERIAL_ID_SEL      (1u << 8)   /* Selection between 4bits ID (1) and 8bits ID (0) */
#define     SENT_8BITS_ID_CFG       0u          /* SENT slow channel 8bits ID configuration */
#define     SENT_4BITS_ID_CFG       (1u << 8)   /* SENT slow channel 4bits ID configuration */
#define     SENT_SERIAL_ID_MASK     0x00FFu     /* SENT slow channel ID mask */

/* SENT-communication configuration */
PORTW       (SENT_CONFIG0, 0x2822)                  /* SENT frame length and number of data nibble configuration */
/* [15] : reserved */
#define     SENT_FRAME_LENGHT_MASK      0x0FFFu     /* [11:0] SENT frame length mask */
#define     SENT_FAST_CRC_ENABLE        (1u << 12)   /* Enable fast channel CRC calculation and transmission */
#define     SENT_STATUS_IN_CRC          (1u << 13)   /* Enable the status and serial comm. nibble CRC into fast channel CRC calculation */
#define     SENT_EN                     (1u << 14)   /* SENT block enable */
#define     SENT_START_NEW_FRAME_EN     (1u << 15)   /* SENT start of new frame */

PORTW       (SENT_CONFIG1, 0x2824)                  /* SENT tick divider and serial channel configuration */
/* [15] : reserved */
#define     SENT_SERIAL_CONFIG_MASK     (3u << 14)  /* [1:0] SENT serial_config mask */
#define     SENT_TICK_DIVIDER_MASK      0x3FFFu     /* [13:0] SENT tick divider mask */
#define     SENT_TICK_DIVIDER_BASE      2u          /* base bit for SENT tick divider */

PORTW       (SENT_CONFIG2, 0x2826)                  /* SENT nibble pulse configuration and status info */
/* [15:14] : reserved */
#define     SENT_NUM_DATA_NIBBLES_BASE  12u         /* base bit for SENT number of data nibbles */
#define     SENT_NUM_DATA_NIBBLES_MASK  0xF000u     /* [14:12] SENT number of data nibbles mask*/
#define     SENT_INI_NBL_EN             (1u << 7)   /* Enable SENT serial channel */
#define     SENT_CRC_LEGACY             (1u << 6)   /* Select recommended or legacy SEND CRC calculation */
#define     SENT_NIBBLE_PULSE_CFG_BASE  4u          /* base bit for SENT nibble pulse configuration */
#define     SENT_NIBBLE_PULSE_CFG_MASK  (3u << 4)   /* SENT nibble pulse configuration mask */
#define     SENT_NIBBLE_PULSE_FIXED_MASK  0x000Fu   /* the number of tick periods for the fixed time in a nibble (low or high) */

PORTW       (SENT_CONFIG3, 0x2828)                  /* SENT nibble pulse configuration and status info */
/* [15:2] : reserved */
#define     SENT_FAIL_SAFE_HIGH          (1u << 1)   /* SENT fast channel 1 status */
#define     SENT_FAIL_SAFE_EN            (1u << 0)   /* SENT fast channel 0 status */

/*Not used anymore*/
#define     SENT_D_UFLOW_ERR            (1u << 13)  /* read-only: SENT slow or fast channel data underflow error */
#define     SENT_D_OFLOW_ERR            (1u << 12)  /* read-only: SENT slow or fast channel data overflow error */
#define     SENT_CFG_UPD_ERR            (1u << 11)  /* read-only: SENT configuration update error */
#define     SENT_SERIAL_EN              (1u << 7)   /* Enable SENT serial channel */
#define     SENT_SERIAL_COMM_MASK   (3u << 9)   /* SENT slow channel status mask    */

PORTW       (FAST_DATA0, 0x2820)                /* SENT fast channel nibbles 1,2 and 3 */


#endif /* port aliases */


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

/*
 * Product specific SYSTEM ports
 *
 * \note
 * 1. SYSTEM ports can be accessed only in system mode of CPU
 *
 */

/* ANA_OUTB calibration register, only used at initialization
 * Reset value: 0x0000
 * Notes: Address defined in mmc16_io.h
 */

/* ANA_OUTC calibration register, only used at initialization
 * Reset value: 0x0000
 * Notes: Address defined in mmc16_io.h
 */

FARPORTW    (RAM_BIST, 0x202E)

FARPORTW    (MMU_1, 0x2030)
#define MMU_EN1                 (1u << 0)
/* bits [2:1] are reserved */
#define MMU_FAULT_DETECTED      (1u << 3)  /* can be reset by FW by writing 0, connected to IT_EXT6  */

FARPORTW    (MMU_2, 0x2032)
#define MMU_EN2                 (1u << 0)

FARPORTW    (MMU_TASK_IDS, 0x2034)
#define MMU_TASK1_ID            (1u << 0)
#define MMU_TASK2_ID            (1u << 1)


FARPORTW	(OTP_BIST, 0x2026)			/* OTP BIST register */
	
/* Reserved word 0x2028 */
	
FARPORTW	(ROM_BIST, 0x202A)			/* ROM_BIST register */
/* ROM_BIST_SIGNATURE [15:0]: read only */
#define     ROM_BIST_XIB    (0x3Fu << 2)        /* [7:2] write only */
#define     ROM_BIST_MODE   (3u << 0)           /* write only: ROM BIST mode configuration */
	
FARPORTB	(RAM_BIST_CONTROL_STATUS, 0x202C)	/* RAM_BIST control/status register */
/* bits [7:5] are reserved */
#define     RAM_BIST_PHASE_MASK (7u << 1)       /* [3:1] read only: RAM BIST phase */
#define     RAM_BIST_FAIL       (1u << 4)       /* read only: RAM BIST result (1 - test fail; 0 - test pass) */
#define     RAM_BIST_MODE       (1u << 0)       /* write 1 to start RAM BIST
												 * read returns one, while RAM BIST is running												 */
FARPORTB	(RAM_BIST_FAIL_DATA, 0x202D)	    /* RAM BIST fail data (read only)  */
FARPORTW	(RAM_BIST_FAIL_ADDRESS, 0x202E)     /* read only: RAM BIST fail address (read only) */

/* =========================== */
/* Product specific USER ports */
/* =========================== */

/* ----------------------- */
/* ANA CUST Port           */
/* ----------------------- */
PORTW       (ANA_CUST0, 0x281A)            /* custom control/status (whole word) */

PORTW       (ANA_CUST1, 0x281C)            /* custom control 1 (whole word) */

PORTW       (ANA_CUST2, 0x281E)             /*  */
#define     FIX_PHASE2_CH1          (1u << 15)  /*  */
#define     FIX_PHASE1_CH1          (1u << 14)  /*  */
#define     ADC_DISABLE_CH2         (1u << 13)  /*  */
#define     ADC_DISABLE_CH1         (1u << 12)  /*  */
#define     ANA_PHASES_EN_CH2       (1u << 11)  /*  */
#define     ANA_PHASES_EN_CH1       (1u << 10)  /*  */
#define     ISUP_MOD                (1u << 9)   /*  */
#define     ADC_NORMAL_POWER        (1u << 8)   /*  */
#define     FORCE_SENT_SR_EN        (1u << 7)   /*  */
#define     EN_SENT_ANA_TEST        (1u << 6)   /*  */
#define     FORCE_SENT_HIZ          (1u << 5)   /*  */
#define     WD_ATT_OCCURED          (1u << 4)   /*  */
#define     BRG_PWM_EN              (1u << 3)   /*  */
#define     MUST_VLOW               (1u << 2)   /*  */
#define     FIX_PHASE2_CHANNEL2     (1u << 1)   /*  */
#define     FIX_PHASE1_CHANNEL2     (1u << 0)   /*  */


/* ----------------------- */
/* SENT IP Ports           */
/* ----------------------- */
PORTW       (SENT_DBASE,    0x2820)         /* SENT DMA DBASE */
PORTW       (SENT_CFG1,     0x2822)         /* SENT Config Register 1 */
PORTB       (SENT_CFG1_L,   0x2822)         /* SENT Config Register 1 Low Byte */
PORTB       (SENT_CFG1_H,   0x2823)         /* SENT Config Register 1 High Byte */
#define     FRAME_LENGTH_MASK       0x0FFF      /* 12-bit */
#define     FAST_CRC_EN             (1u << 12)  /* 1-bit */
#define     STAT_IN_CRC             (1u << 13)  /* 1-bit */
#define     SENT_EN                 (1u << 14)  /* 1-bit */
#define     START_NEW_FR            (1u << 15)  /* 1-bit */

PORTW       (SENT_CFG2,  0x2824)            /* SENT Config Register 2 */
#define     TICK_DIV_MASK           0x3FFF      /* 14-bit */
#define     SERIAL_CFG_MASK         (3u << 14)  /* 3-bit */
#define     NO_SC                   (0u << 14)
#define     SC_NIB_IO               (1u << 14)
#define     SHORT_SER               (2u << 14)
#define     ENHANCED_SER            (3u << 14)

PORTB       (SENT_CFG3,  0x2826)            /* SENT Config Register 3 */
#define     NIB_PULSE_FIX_MASK      0x000F      /* 4-bit */
#define     NIB_PULSE_CFG_MASK      (3u << 4)   /* 3-bit */
#define     NIB_PUL_50              (0u << 4)
#define     NIB_PUL_FIX_LOW         (2u << 4)
#define     NIB_PUL_FIX_HIGH        (3u << 4)
#define     CRC_RECOMMENDED         (0u << 6)
#define     CRC_LEGACY              (1u << 6)
#define     INI_NBL_EN              (1u << 7)   /* 1-bit */

PORTB       (SENT_NIB_MASK,  0x2827)        /* SENT Fast Channel Nibble Mask */
/* SPC ports not implemented */
/*PORTW       (SENT_SPC_CFG,   0x282A)*/        /* SENT SPC Config Register */
/*PORTB       (SENT_SPC_CFG1,  0x282A)*/        /* SENT SPC Config Register 1 */
/*PORTB       (SENT_SPC_CFG2,  0x282B)*/        /* SENT SPC Config Register 2 */

/* ----------------------- */
/* ANA CUST Port           */
/* ----------------------- */
PORTW       (ANA_CUST3_W, 0x282C)              /* custom control 3 (whole word) */

PORTB       (ANA_CUST3_L, 0x282C)               /*   */
#define     ANA_DIAG_EN             (1u << 7)
#define     ANA_DIAG_POL            (1u << 6)
#define     ANA_DIAG_SEL_MASK       (3u << 4)
/* [3:2] reserved */
#define     HALF_FULLB_BRIDGE       (1u << 1)
#define     SENT_ERR                (1u << 0)

#define ANA_DIAG_CHECK1_CFG         (0  << 4)
#define ANA_DIAG_CHECK2_CFG         (1u << 4)
#define ANA_DIAG_CHECK3_CFG         (2u << 4)

/* SENT/ADC synchronization */
PORTB       (SYNCH_SENT_ADC, 0x282D)            /* SENT/ADC synchronization configuration (byte)*/


/* COMM (aka UART mode) interface */
FARPORTW    (UART0, 0x2834)                 /* 16 LSBits of the UART message   */
FARPORTW    (UART1, 0x2836)                 /* 5 MSBits  of the UART message   */
#define     UART_WRITE              (1u << 15)  /* 0: read from UART; 1: write to UART */
/* [14:8] reserved */
#define     KEY_REG_BIT2_MASK       (1u << 7)   /* read-only KEY_REG[3] bit mask   */
#define     KEY_REG_BIT1_BIT0_MASK  (3u << 5)   /* read-only KEY_REG[1:0] bits mask  */
#define     UART_CRC4_BASE          1u          /* base bit UART CRC4 */
#define     UART_CRC4_MASK          0x001Eu     /* UART CRC4 mask     */
#define     UART_RX_DATA16          (1u << 0)   /* UART 16th data bit */

PORTB       (MMU_STACK_END,    0x2838)      /*   */
PORTB       (MMU_STACK_START,  0x2839)      /*   */
PORTW       (MMU_STACK_LIMITS, 0x2838)      /*   */


/* ----------------------- */
/* SENT IP Feedback Ports  */
/* ----------------------- */
PORTW       (SENT_FC0_DTA,   0x283A)        /* Current Fast Channel Data CH0 */
PORTW       (SENT_FC1_DTA,   0x283C)        /* Current Fast Channel Data CH1 */
PORTW       (SENT_SC_DTA,    0x283E)        /* Current Slow Channel Data */
FARPORTW    (SENT_SC_ID,     0x2840)        /* Current Slow Channel ID */
FARPORTB    (SENT_FC_STAT,   0x2842)        /* Current Fast Channel Status Bits */


FARPORTW	(VANA_MONITORS, 0x2848)         /* VANA_MONITORS */
#define		VPROG					(1u << 10)  /*   */

FARPORTW    (PHASE_7, 0x284A)               /* Phase 7 */
/* bite [15:12] not used */
#define PHASE7_MAX_MASK         (0x3Fu << 6)
#define PHASE7_MIN_MASK         (0x3Fu << 0)

FARPORTW    (CO_REF, 0x284C)                /* Phase 8 */
#define COREF_CHANNEL2_MASK     (7u << 13)      /* Coarse offset reference selection for channel 2 */
#define COREF_CHANNEL1_MASK     (7u << 10)      /* Coarse offset reference selection for channel 1 */
/* bite [9:6] not used */
#define PHASE8_MAX_MASK         (0x3Fu << 0)

#endif /* IOPORTS_H_ */
