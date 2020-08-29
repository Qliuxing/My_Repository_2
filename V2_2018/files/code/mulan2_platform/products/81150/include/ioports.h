/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef IOPORTS_H_
#define IOPORTS_H_

#include <mulan2_io.h>

/*
 * ADC configuration
 *
 *  ADC_CFG[15:8]  -- channel selection
 *
 *  ADC_CFG[7:0]
 *      HW trigger source [7:4]
 *      unused [3:2]
 *      ADC reference [1:0]
 */

/* HW trigger source (in LSByte) */
#define ADC_HW_TRIGGER_PWM0_CNT     ( 0u << 4)
#define ADC_HW_TRIGGER_PWM1_CNT     ( 1u << 4)
#define ADC_HW_TRIGGER_PWM2_CNT     ( 2u << 4)
#define ADC_HW_TRIGGER_PWM3_CNT     ( 3u << 4)

#define ADC_HW_TRIGGER_PWM0_CMP     ( 4u << 4)
#define ADC_HW_TRIGGER_PWM1_CMP     ( 5u << 4)
#define ADC_HW_TRIGGER_PWM2_CMP     ( 6u << 4)
#define ADC_HW_TRIGGER_PWM3_CMP     ( 7u << 4)

#define ADC_HW_TRIGGER_TMR1_CMPB    ( 8u << 4)
#define ADC_HW_TRIGGER_TMR1_CMPA    ( 9u << 4)
#define ADC_HW_TRIGGER_TMR1_CAPB    (10u << 4)
#define ADC_HW_TRIGGER_TMR1_CAPA    (11u << 4)

#define ADC_HW_TRIGGER_TMR2_CMPB    (12u << 4)
#define ADC_HW_TRIGGER_TMR2_CMPA    (13u << 4)
#define ADC_HW_TRIGGER_TMR2_CAPB    (14u << 4)
#define ADC_HW_TRIGGER_TMR2_CAPA    (15u << 4)

/* Reference voltage (LSByte) */
#define ADC_REF_2_50_V              3
#define ADC_REF_1_50_V              2
#define ADC_REF_0_75_V              1
#define ADC_REF_OFF                 0

/* Channels (MSByte) */
#define ADC_CH0     ( 0u << 8)
#define ADC_CH1     ( 1u << 8)
#define ADC_CH2     ( 2u << 8)
#define ADC_CH3     ( 3u << 8)
#define ADC_CH4     ( 4u << 8)
#define ADC_CH5     ( 5u << 8)
#define ADC_CH6     ( 6u << 8)
#define ADC_CH7     ( 7u << 8)
#define ADC_CH8     ( 8u << 8)
#define ADC_CH9     ( 9u << 8)
#define ADC_CH10    (10u << 8)
#define ADC_CH11    (11u << 8)
#define ADC_CH12    (12u << 8)
#define ADC_CH13    (13u << 8)
#define ADC_CH14    (14u << 8)
#define ADC_CH15    (15u << 8)


/* =============================================================================
 *    Product specific system ports (accessed only in system mode of CPU)
 * =============================================================================
 */

/*
 * Second level interrupt controller
 */
FARPORTW    (XI0_MASK,  0x202A) /* Timer1 and Timer 3 units */
FARPORTW    (XI1_MASK,  0x202C) /* Timer2 and Timer 4 units */
FARPORTW    (XI2_MASK,  0x202E) /* PWM unit */
FARPORTW    (XI3_MASK,  0x2030) /* SPI and UART units */
FARPORTW    (XI4_MASK,  0x2032) /* Overtemperature, overvoltage, PLL moniotr and customs */

FARPORTW    (XI0_PEND,  0x2034) /* Timer1 and Timer 3 units */
FARPORTW    (XI1_PEND,  0x2036) /* Timer2 and Timer 4 units */
FARPORTW    (XI2_PEND,  0x2038) /* PWM unit */
FARPORTW    (XI3_PEND,  0x203A) /* SPI and UART units */
FARPORTW    (XI4_PEND,  0x203C) /* Overtemperature, overvoltage, PLL monitor and customs */

/*
 * PLL control bits
 */
FARPORTW    (PLL_CTRL,  0x203E)
/* PLL_CTRL[15:8] == PLL_FBDIV[7:0] feedback divider setting for PLL: f(PLL) = 250kHz * (PLL_FBDIV+1) */
/* PLL_CTRL[7:4]  == PLL_CTLCK[3:0] lock control bits, default locking=0000, max relaxed locking =1101, most critical=0111 */
#define     PLL_SELXTAL (1u << 3)   /* select crystal oscillator as PLL source (if any) */
#define     PLL_XTALON  (1u << 2)   /* start crystal oscillator (if any)    */
/* bit 1: reserved */
#define     PLL_EN      (1u << 0)   /* activate PLL */

/*
 * PLL status bits
 */
FARPORTW    (PLL_STAT,  0x2040)
/* bits 15 - 2 are reserved */
#define     PLL_CM      (1u << 1)   /* Clock Monitor error bit; write 1 to clear the clock monitor error flag */
#define     PLL_LOCKED  (1u << 0)   /* read only: PLL has locked at high frequency */

/*
 * Enables LIN_XCFG port
 * enter predefined value 0x5F0A here to enable LIN_XCFG port
 */
FARPORTW    (LIN_XKEY,  0x2042)
#define     LIN_XKEY_VALID  (1u << 0)   /* read only: 1=LIN_XKEY is valid */

/*
 * Analog chip ports
 */
FARPORTW    (ANA_TESTX, 0x2044)
FARPORTW    (ANA_TESTY, 0x2046)
FARPORTW    (ANA_TESTZ, 0x2048)
FARPORTW    (ANA_OUTD,  0x204A)
FARPORTW    (ANA_OUTE,  0x204C)
FARPORTW    (ANA_OUTF,  0x204E)

FARPORTW    (FLASHTRIMA, 0x2050)
FARPORTW    (FLASHTRIMB, 0x2052)

/*
 * Note:
 *  1. Can only be launched through RAM program and if Mlx4
 *     and ADC DMA are stopped (if not, this could block the
 *     arbitration and crash the IC)
 */

FARPORTW	(CONTROL_EXT, 0x2054)
#define		DIS_FLASH	(1u << 15)
#define		DIS_REF		(1u << 14)
#define		FORCE_VMG	(1u << 13)
#define		FORCE_IREF	(1u << 12)
#define		MEAS_VMG	(1u << 11)
#define		MEAS_IREF	(1u << 10)
#define		EN_VMG		(1u << 9)
#define		EN_IREF		(1u << 8)
#define		FL_DBE_IT	(1u << 7)
#define		TM_RES2		(1u << 6)
#define		TM_RES1		(1u << 5)
#define		TM_TR		(1u << 4)
#define		RDLA		(1u << 3)
#define		RDY_OPTION	(3u << 1)	/* Flash wait-states */
#define		RDY_OPT_FL	(3u << 1)	/* Use READY from flash */
#define		RDY_OPT_2	(2u << 1)	/* Fixed 2 extra Tclk */	
#define		RDY_OPT_1	(1u << 1)	/* Fixed 1 extra Tclk */	
#define		RDY_OPT_0	(0u << 1)	/* Fixed 0 extra Tclk */	
#define		SEL_PRIO	(1u << 0)	/* MLX4/MLX16 priority: 0: MLX4 has priority, 1: MLX16 has priority */

FARPORTW    (FLASH_BIST,      0x2056)
FARPORTW    (FLASHCUR_SEL_LW, 0x2058)       /* low word:  FLASHCUR_SEL[15:0] */
FARPORTW    (FLASHCUR_SEL_HW, 0x205A)       /* high word: FLASHCUR_SEL[16:31] */
FARPORTW    (FLASHTRIMC,      0x205C)



/* =============================================================================
 *    Product specific user ports
 * =============================================================================
 */

/*
 * Analog Watchdog (AWD) interface
 */
PORTW       (AWD_CTRL,  0x281A)
#define     AWD_RST         (1u << 15)  /* read-only: AWD Reset Flag (cleared by POR or WAKE UP)            */
#define     AWD_ATT         (1u << 14)  /* AWD attention flag(cleared by POR or WAKE UP or write with 1)    */
#define     AWD_WRITE_FAIL  (1u << 13)  /* AWD_CKDIV and AWD_TIMER are updated too fast (less than 100 us) ..
                                         * .. cleared by POR or WAKE UP or write with 1
                                         */
/* bits [12:10] are reserved    */
/* bits [9:8] are AWD_CKDIV:00=by 64 ; 01=by 16; 10=by 4; 11=by 1  */
/* bits [7:0] are AWD_TIMER     */

/*
 *
 */
PORTW       (ANA_INA,   0x281C)     /* read-only ! */
#define     OVT             (1u << 15)  /* overtemp interrupt flag  */
#define     UV_VS           (1u << 14)  /* undervoltage signal      */
#define     OV_VS           (1u << 13)  /* overvoltage signal       */
#define     PLL_ERR         (1u << 12)  /* PLL error interrupt      */
#define     SHPS            (1u << 11)  /* sensor supply (PS) short detection */
#define     RELCON1         (1u << 10)  /* relay contact pin SENSE2 */
#define     RELCON0         (1u <<  9)  /* relay contact pin SENSE1 */
#define     OC_SENSE        (1u <<  8)  /* over currents sense detection */
#define     KEYINT7         (1u <<  7)
#define     KEYINT6         (1u <<  6)
#define     KEYINT5         (1u <<  5)
#define     KEYINT4         (1u <<  4)
#define     KEYINT3         (1u <<  3)
#define     KEYINT2         (1u <<  2)
#define     KEYINT1         (1u <<  1)
#define     KEYINT0         (1u <<  0)

PORTW       (ANA_INB,   0x281E)
/* bits [15:11] are reserved */
#define     INTERNAL_WU     (1u << 10)
#define     LOCAL_WU        (1u <<  9)
#define     LIN_WU          (1u <<  8)
#define     XTDIN1          (1u <<  7)
#define     XTDIN0          (1u <<  6)
#define     PWMDIN1         (1u <<  5)
#define     PWMDIN0         (1u <<  4)
#define     TC1_UNDEBOUNCED (1u <<  3)
#define     TC0_UNDEBOUNCED (1u <<  2)
#define     TC1_DEBOUNCED   (1u <<  1)
#define     TC0_DEBOUNCED   (1u <<  0)

/*
 * Configuration port for external LIN phy or protocol layer
 * Used with valid LIN_XKEY
 */
PORTW       (LIN_XCFG,  0x2820)

#define     LIN_XPHY_ACTIVE (1u << 9)   /* read-only: LIN extern physical layer activated, LIN_XKEY was ok */
#define     LIN_XPRO_ACTIVE (1u << 8)   /* read-only: LIN extern protocol activated, LIN_XKEY was ok */
#define     SLEEPB_LIN      (1u << 7)   /* used in XPRO mode */
#define     LSM             (1u << 6)   /* used in XPRO mode */
#define     HSM             (1u << 5)   /* used in XPRO mode */
#define     BYPASS          (1u << 4)   /* used in XPRO mode */
#define     LIN_XOUTINV     (1u << 3)   /* 1: invert LIN output, e.g. when open drain IO is used; 0: non-inverted */
#define     DISTERM         (1u << 2)
#define     LIN_EN_XPHY     (1u << 1)   /* write-only: enable extern physical layer for LIN */
#define     LIN_EN_XPRO     (1u << 0)   /* write-only: enable extern protocol layer for LIN */


PORTW       (CHIP_REVISION,  0x2822)    /* read-only: CHIP_VER[7:0] */


/*
 * Timer 1
 */
PORTW       (TMR1_CTRL, 0x282A)     /* Control register */
PORTW       (TMR1_REGB, 0x282C)     /* Channel B */
PORTW       (TMR1_REGA, 0x282E)     /* Channel A */
PORTW       (TMR1_CNT,  0x2830)     /* read-only: Counter */

/*
 * Timer 2
 */
PORTW       (TMR2_CTRL, 0x2832)     /* Control register */
PORTW       (TMR2_REGB, 0x2834)     /* Channel B */
PORTW       (TMR2_REGA, 0x2836)     /* Channel A */
PORTW       (TMR2_CNT,  0x2838)     /* read-only: Counter */

/*
 * PWM 1
 */
FARPORTB    (PWM1_CTRL, 0x284A)     /* Control register   */
FARPORTB    (PWM1_PSCL, 0x284B)     /* Prescaler register */
FARPORTW    (PWM1_PER,  0x284C)     /* Period duration    */
FARPORTW    (PWM1_LT,   0x284E)     /* Low threshold register  */
FARPORTW    (PWM1_HT,   0x2850)     /* High threshold register */
FARPORTW    (PWM1_CMP,  0x2852)     /* Compare interrupt threshold register */

/*
 * PWM 2
 */
FARPORTB    (PWM2_CTRL, 0x2854)     /* Control register   */
FARPORTB    (PWM2_PSCL, 0x2855)     /* Prescaler register */
FARPORTW    (PWM2_PER,  0x2856)     /* Period duration    */
FARPORTW    (PWM2_LT,   0x2858)     /* Low threshold register  */
FARPORTW    (PWM2_HT,   0x285A)     /* High threshold register */
FARPORTW    (PWM2_CMP,  0x285C)     /* Compare interrupt threshold register */

/*
 * PWM 3
 */
FARPORTB    (PWM3_CTRL, 0x285E)     /* Control register   */
FARPORTB    (PWM3_PSCL, 0x285F)     /* Prescaler register */
FARPORTW    (PWM3_PER,  0x2860)     /* Period duration    */
FARPORTW    (PWM3_LT,   0x2862)     /* Low threshold register  */
FARPORTW    (PWM3_HT,   0x2864)     /* High threshold register */
FARPORTW    (PWM3_CMP,  0x2866)     /* Compare interrupt threshold register */

/*
 * PWM 4
 */
FARPORTB    (PWM4_CTRL, 0x2868)     /* Control register   */
FARPORTB    (PWM4_PSCL, 0x2869)     /* Prescaler register */
FARPORTW    (PWM4_PER,  0x286A)     /* Period duration    */
FARPORTW    (PWM4_LT,   0x286C)     /* Low threshold register  */
FARPORTW    (PWM4_HT,   0x286E)     /* High threshold register */
FARPORTW    (PWM4_CMP,  0x2870)     /* Compare interrupt threshold register */


/*
 * SPI 1
 */
FARPORTW    (SPI1_CTRL, 0x289A)     /* Control register (word access) */
FARPORTB    (SPI1_PCR,  0x289A)     /* Control register */
FARPORTB    (SPI1_PSCR, 0x289B)     /* Status and Control register */
FARPORTW    (SPI1_BRR,  0x289C)     /* BRR register */
FARPORTW    (SPI1_DR,   0x289E)     /* Data register */


/*
 * Multi-purpose high voltage I/O (KEY[7:0])
 */
FARPORTW    (KEY_CFG,   0x28BE)
FARPORTW    (KEY_DEB,   0x28C0)     /* Debounce configuration   */
FARPORTW    (KEY_OD,    0x28C2)     /* Open drain outputs       */
FARPORTW    (KEY_EN,    0x28C4)     /* Threshold config / Input comparator type */
FARPORTW    (KEY_PULL,  0x28C6)     /* Pull-up / pull-down configuration        */
FARPORTW    (KEY_WU,    0x28C8)     /* Enables wakeup detection                 */
FARPORTW    (KEY_IN,    0x28CA)     /* Debounced and un-debounds input from pin */

FARPORTW    (ANA_OUTG,  0x28CC)
#define     V1V8REG_OCPROT_OFF  (1u << 15)  /* switch off overcurrent protection V1V8 regulator */
#define     XTENST              (1u << 14)  /* enable schmitt triggers for XTAL digital inputs  */
#define     TCENPU1             (1u << 13)  /* enable pull up's for TC inputs */
#define     TCENPU0             (1u << 12)
#define     PWMENST1            (1u << 11)  /* enable schmitt triggers for PWM inputs */
#define     PWMENST0            (1u << 10)
#define     PRUV1               (1u <<  9)  /* program undervoltage detection level */
#define     PRUV0               (1u <<  8)
#define     INACTIVE_OVT        (1u <<  7)  /* disable OVT (for HTOL) */
#define     WUI1                (1u <<  6)  /* set delay for internal wake up */
#define     WUI0                (1u <<  5)
#define     REL_DISFW1          (1u <<  4)  /* disable freewheel diode for relay drivers */
#define     REL_DISFW0          (1u <<  3)
#define     PWMEN1              (1u <<  2)  /* PWM outputs enable bits */
#define     PWMEN0              (1u <<  1)
#define     ENPS                (1u <<  0)  /* enable extern hall power supply PS (also required for PWM outputs) */

FARPORTW    (ANA_OUTH,  0x28CE)
FARPORTW    (ANA_OUTI,  0x28D0)
FARPORTW    (ANA_OUTK,  0x28D2)
#define     OREL1       (1u << 15)  /* relay driver port outputs */
#define     OREL0       (1u << 14)
#define     RELMODE     (1u << 13)  /* 0: REL1 and REL2 outputs are from PWM4, PWM3 units   */
                                    /* 1: REL1 and REL2 outputs are from port OREL[1:0]     */
#define     OPWM1       (1u << 12)  /* PWM output driver port */
#define     OPWM0       (1u << 11)
#define     PWMMODE     (1u << 10)  /* 0: PWM1 and PWM2 outputs are from PWM1, PWM2 units   */
                                    /* 1: PWM1 and PWM2 outputs are from port OPWM[1:0]     */
/* bits [9:0] are reserved */

#endif /* IOPORTS_H_ */
