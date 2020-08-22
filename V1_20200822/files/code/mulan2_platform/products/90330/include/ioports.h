/*
 * Copyright (C) 2013 Melexis N.V.
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

	/* ----------------- */
	/* SYSTEM PORTS      */
	/* ----------------- */
    FARPORTB    (CUST_CTRL_L,    0x2032)        /* */
        #define ISUP_MOD            (1u << 0)
        #define FORCE_TX            (1u << 1)
        #define	PD_EN               (1u << 2)
        #define ADC_MODE_0          (1u << 3)
        #define ADC_MODE_1          (1u << 4)
        #define ADC_MODE            (3u << 3)
        #define IO_SELTX2OUT        (1u << 5)

    /* ----------------- */
    /* USER PORTS        */
    /* ----------------- */
    PORTW       (IO_AOUT_CTRL,    0x281A)        /* */
        #define COM1OUT_MASK        (0x0FFFu << 0)
        #define OUT_RAIL_MASK       (3u << 12)
        #define OUT_RAIL_0          (1u << 12)
        #define OUT_RAIL_1          (1u << 13)
        #define EN_LATCH            (1u << 14)
        #define IOSELTX2MUST        (1u << 15)

    PORTW       (IO_CUST_READ,   0x281C)        /* */
    extern volatile struct
    {
        unsigned FSM_ST:4;
        unsigned Reserved0:1;
        unsigned WDATToccured:1;
        unsigned SEL_SP_SN:1;
        unsigned ADC_CMP_CNT:1;
        unsigned VSUP:1;
        unsigned TX:1;
        unsigned ADC_CMP_B:2;
        unsigned ADC_CMP_A:2;
        unsigned ADC_LSB:2;
    } CUST_READ __attribute__((io, addr(0x281C)));

    PORTW       (SENT_DBASE,    0x2820)         /* SENT DMA DBASE */
    PORTW       (SENT_CFG1,     0x2822)         /* SENT Config Register 1 */
    PORTB       (SENT_CFG1_L,   0x2822)
    PORTB       (SENT_CFG1_H,   0x2823)
        #define FRAME_LENGTH_MASK   0x0FFF      /* 12-bit */
        #define FAST_CRC_EN         (1u << 12)  /* 1-bit */
        #define STAT_IN_CRC         (1u << 13)  /* 1-bit */
        #define SENT_EN             (1u << 14)  /* 1-bit */
        #define START_NEW_FR        (1u << 15)  /* 1-bit */
    PORTW       (SENT_CFG2,  0x2824)            /* SENT Config Register 2 */
        #define TICK_DIV_MASK       0x3FFF      /* 14-bit */
        #define SERIAL_CFG_MASK     (3u << 14)
        #define NO_SC               (0u << 14)
        #define SC_NIB_IO           (1u << 14)
        #define SHORT_SER           (2u << 14)
        #define ENHANCED_SER        (3u << 14)
    PORTB       (SENT_CFG3,  0x2826)            /* SENT Config Register 3 */
        #define NIB_PULSE_FIX_MASK  0x000F      /* 4-bit */
        #define NIB_PULSE_CFG_MASK  (3u << 4)
        #define NIB_PUL_50          (0u << 4)
        #define NIB_PUL_FIX_LOW     (2u << 4)
        #define NIB_PUL_FIX_HIGH    (3u << 4)
        #define CRC_RECOMMENDED     (0u << 6)
        #define CRC_LEGACY          (1u << 6)   /* 1-bit */
        #define INI_NBL_EN          (1u << 7)   /* 1-bit */
    PORTB       (SENT_NIB_MASK,  0x2827)        /* SENT Fast Channel Nibble Mask */
    PORTW       (SENT_SPC_CFG,   0x2828)        /* SENT SPC Config Register */
    PORTB       (SENT_SPC_CFG1,  0x2828)        /* SENT SPC Config Register 1 */
    PORTB       (SENT_SPC_CFG2,  0x282A)        /* SENT SPC Config Register 2 */

#endif /* __IOPORTS_H__ */
