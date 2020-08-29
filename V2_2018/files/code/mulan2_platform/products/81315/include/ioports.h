/*
 * Copyright (C) 2009-2015 Melexis N.V.
 *
 * Software Platform
 *
 * IO ports header file
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
#define ADC_HW_TRIGGER_PWM1_CNT     ( 0u << 4)
#define ADC_HW_TRIGGER_PWM2_CNT     ( 1u << 4)
#define ADC_HW_TRIGGER_PWM3_CNT     ( 2u << 4)
#define ADC_HW_TRIGGER_PWM4_CNT     ( 3u << 4)
#define ADC_HW_TRIGGER_PWM5_CNT     (( 0u << 4) | 8u)

#define ADC_HW_TRIGGER_PWM1_CMP     ( 4u << 4)
#define ADC_HW_TRIGGER_PWM2_CMP     ( 5u << 4)
#define ADC_HW_TRIGGER_PWM3_CMP     ( 6u << 4)
#define ADC_HW_TRIGGER_PWM4_CMP     ( 7u << 4)
#define ADC_HW_TRIGGER_PWM5_CMP     (( 4u << 4) | 8u)

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
#define ADC_CH0     ( 0u << 8)	/* VS, Internal high ohmic divider 1:14 */
#define ADC_CH1     ( 1u << 8)	/* TEMP, Internal temperature sensor */
#define ADC_CH2     ( 2u << 8)	/* VDDD, Digital supply voltage */
#define ADC_CH3     ( 3u << 8)	/* VDDA/2, Analogue supply voltage */
#define ADC_CH4     ( 4u << 8)	/* VSM_FILT, Output of the VSM Motor-driver Sensor (=VSM/14) after filtering */
#define ADC_CH5     ( 5u << 8)	/* IO0 (0-ADCref, with ADCref = 0.75V or 1.5V or 2.5V) */
#define ADC_CH6     ( 6u << 8)	/* IO1 (0-ADCref, with ADCref = 0.75V or 1.5V or 2.5V) */
#define ADC_CH7     ( 7u << 8)	/* IO2 (0-ADCref, with ADCref = 0.75V or 1.5V or 2.5V) */
#define ADC_CH8     ( 8u << 8)	/* IO3 (0-ADCref, with ADCref = 0.75V or 1.5V or 2.5V) */
#define ADC_CH9     ( 9u << 8)	/* U, Voltage on U Driver output divided by 14 */
#define ADC_CH10    (10u << 8)	/* V, Voltage on V Driver output divided by 14 */
#define ADC_CH11    (11u << 8)	/* W, Voltage on W Driver output divided by 14 */
#define ADC_CH12    (12u << 8)	/* LIN SHUNT Current (chopping mode 1) */
#define ADC_CH13    (13u << 8)	/* SENSE, Driver Current before filter */
#define ADC_CH14    (14u << 8)	/* VSM/14, VSM voltage divided by 14 */
#define ADC_CH15    (15u << 8)	/* VAUX/2, Internal VAUX2 voltage divided by 2 */
#define ADC_CH17    (17u << 8)	/* TEMP_DRV, Internal motor-driver temperature sensor */
#define ADC_CH21    (21u << 8)	/* IO4 (0-ADCref, with ADCref = 0.75V or 1.5V or 2.5V) */
#define ADC_CH22    (22u << 8)	/* IO5 (0-ADCref, with ADCref = 0.75V or 1.5V or 2.5V) */
#define ADC_CH24    (24u << 8)	/* IO3 voltage divided by 14 */
#define ADC_CH25    (25u << 8)	/* T, Voltage on T Driver output divided by 14; Req: PLTF-617 */
#define ADC_CH26    (26u << 8)	/* TC1 (0-ADCref, with ADCref = 0.75V or 1.5V or 2.5V) */
#define ADC_CH28    (28u << 8)	/* LIN SHUNT Current (chopping mode 2) */
#define ADC_CH29	(29u << 8)	/* SENSE, If = ‘1’ -> ADC input = Driver Current after filter */


/* =============================================================================
 *    Product specific system ports (accessed only in system mode of CPU)
 * =============================================================================
 */

/*
 * Second level interrupt controller
 */
FARPORTW    (XI0_MASK,  0x202A) /* Timer1 and Timer 3 units */
#define EN_T1_INT3	(1u << 5)	/* TMR1_Overflow_Interrupt */
#define EN_T1_INT4	(1u << 6)	/* TMR1_Compare_B_Interrupt */
#define EN_T1_INT2	(1u << 7)	/* TMR1_Compare_A_Interrupt */
#define EN_T1_INT5	(1u << 8)	/* TMR1_Capture_B_Interrupt */
#define EN_T1_INT1	(1u << 9)	/* TMR1_Capture_A_Interrupt */

FARPORTW    (XI1_MASK,  0x202C) /* Timer2 and Timer 4 units */
#define EN_T2_INT3	(1u << 5)	/* TMR2_Overflow_Interrupt */
#define EN_T2_INT4	(1u << 6)	/* TMR2_Compare_B_Interrupt */
#define EN_T2_INT2	(1u << 7)	/* TMR2_Compare_A_Interrupt */
#define EN_T2_INT5	(1u << 8)	/* TMR2_Capture_B_Interrupt */
#define EN_T2_INT1	(1u << 9)	/* TMR2_Capture_A_Interrupt */

FARPORTW    (XI2_MASK,  0x202E) /* PWM unit */

FARPORTW    (XI3_MASK,  0x2030) /* SPI and UART units */

FARPORTW    (XI4_MASK,  0x2032) /* Overtemperature, overvoltage, PLL monitor and customs */
#define XI4_OVT		(1u << 15)	/* Overtemperature */
#define XI4_UV		(1u << 14)	/* Under-voltage */
#define XI4_OV		(1u << 13)  /* Over-voltage */
#define XI4_PLL_INT	(1u << 12)	/* PLL interrupt */
#define XI4_OC_DRV	(1u <<  8)	/* Over-current driver */
#define XI4_IO5		(1u <<  5)	/* IO[5] interrupt */
#define XI4_IO4		(1u <<  4)	/* IO[4] interrupt */
#define XI4_IO3		(1u <<  3)	/* IO[3] interrupt */
#define XI4_IO2		(1u <<  2)	/* IO[2] interrupt */
#define XI4_IO1		(1u <<  1)	/* IO[1] interrupt */
#define XI4_IO0		(1u <<  0)	/* IO[0] interrupt */

FARPORTW    (XI0_PEND,  0x2034) /* Timer1 and Timer 3 units */
#define CLR_T1_INT3	(1u << 5)	/* TMR1_Overflow_Interrupt */
#define CLR_T1_INT4	(1u << 6)	/* TMR1_Compare_B_Interrupt */
#define CLR_T1_INT2	(1u << 7)	/* TMR1_Compare_A_Interrupt */
#define CLR_T1_INT5	(1u << 8)	/* TMR1_Capture_B_Interrupt */
#define CLR_T1_INT1	(1u << 9)	/* TMR1_Capture_A_Interrupt */

FARPORTW    (XI1_PEND,  0x2036) /* Timer2 and Timer 4 units */
#define CLR_T2_INT3	(1u << 5)	/* TMR2_Overflow_Interrupt */
#define CLR_T2_INT4	(1u << 6)	/* TMR2_Compare_B_Interrupt */
#define CLR_T2_INT2	(1u << 7)	/* TMR2_Compare_A_Interrupt */
#define CLR_T2_INT5	(1u << 8)	/* TMR2_Capture_B_Interrupt */
#define CLR_T2_INT1	(1u << 9)	/* TMR2_Capture_A_Interrupt */

FARPORTW    (XI2_PEND,  0x2038) /* PWM unit */

FARPORTW    (XI3_PEND,  0x203A) /* SPI and UART units */

FARPORTW    (XI4_PEND,  0x203C) /* Over-temperature, over-voltage, PLL monitor and customs */

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
#define		IO5_DIS_PMOS	(1u << 13)
#define		IO4_DIS_PMOS	(1u << 12)
#define		IO3_DIS_PMOS	(1u << 11)
#define		IO2_DIS_PMOS	(1u << 10)
#define		IO1_DIS_PMOS	(1u << 9)
#define		IO0_DIS_PMOS	(1u << 8)
#define		IO5_ENA			(1u << 5)
#define		IO4_ENA			(1u << 4)
#define		IO3_ENA			(1u << 3)
#define		IO2_ENA			(1u << 2)
#define		IO1_ENA			(1u << 1)
#define		IO0_ENA			(1u << 0)

FARPORTW    (FLASHTRIMA, 0x2050)

FARPORTW	(FLASHTRIMB, 0x2052)
#define		SW_VMG		(3u << 0)

/*
 * Note:
 *  1. Access to CONTROL_EXT can only be launched through RAM program and if Mlx4
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

FARPORTW	(FLASH_BIST, 0x2056)

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
PORTW       (ANA_INA,   0x281C)
#define		OVT				(1u << 15)	/* Read-only: over temp flag. interrupt source */
#define		UV_VS			(1u << 14)	/* Read-only: under voltage at supply flag; interrupt source */
#define		OV_VS			(1u << 13)	/* Read-only: under voltage at supply flag */
#define		CUSTINT_12		(1u << 12)	/* MMP091019-6 */
#define		CUSTINT_11		(1u << 11)	/* MMP091019-6 */
#define		CUSTINT_10		(1u << 10)	/* MMP091019-6 */
#define		CUSTINT_9		(1u << 9)	/* MMP091019-6 */
#define		OC_DRV			(1u << 8)	/* Read-only: driver over-current */
#define		CUSTINT_7		(1u << 7)	/* MMP091019-6 */
#define		CUSTINT_6		(1u << 6)	/* MMP091019-6 */
#define		IOINT_5			(1u << 5)	/* Read-only: I/O-5 */
#define		IOINT_4			(1u << 4)	/* Read-only: I/O-4 */
#define		IOINT_3			(1u << 3)	/* Read-only: I/O-3 */
#define		IOINT_2			(1u << 2)	/* Read-only: I/O-2 */
#define		IOINT_1			(1u << 1)	/* Read-only: I/O-1 */
#define		IOINT_0			(1u << 0)	/* Read-only: I/O-0 */

PORTW       (ANA_INB,   0x281E)
#define		WAKEUP_LIN		(1u << 8)	/* Wake-up by LIN */
#define		WAKEUP_IO3		(1u << 9)	/* Wake-up by IO3 */
#define		WAKEUP_TMR		(1u << 10)	/* Wake-up by wake-up timer */
#define		TC2_UNDEB		(1u << 3)	/* TC2 undebounced */
#define		TC1_UNDEB		(1u << 2)	/* TC1 undebounced */
#define		TC2_DEB			(1u << 1)	/* TC2 debounced */
#define		TC1_DEB			(1u << 0)	/* TC1 debounced */

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
PORTW       (TMR1_CTRL, 0x282A)		/* Control register */
#define		TMRx_DIV1		(1u << 15)	/* Prescaler ratio */
#define		TMRx_DIV0 		(1u << 14)	/* Prescaler ratio */
#define		TMRx_MODE2		(1u << 13)	/* Timer mode selector */
#define		TMRx_MODE1		(1u << 12)	/* Timer mode selector */
#define		TMRx_MODE0		(1u << 11)	/* Timer mode selector */
#define		TMRx_ENCMP		(1u << 10)	/* Compare enable */
#define		TMRx_OVRB		(1u << 9)	/* Over-run register B */
#define		TMRx_OVRA		(1u << 8)	/* Over-run register A */
#define		TMRx_DIN1		(1u << 7)	/* De-bounce method */
#define		TMRx_DIN0		(1u << 6)
#define		TMRx_EDG2_1		(1u << 5)	/* Edge selector for channel B */
#define		TMRx_EDG2_0		(1u << 4)
#define		TMRx_EDG1_1		(1u << 3)	/* Edge selector for channel A */
#define		TMRx_EDG1_0		(1u << 2)
#define		TMRx_START		(1u << 1)	/* Disable the reset */
#define		TMRx_T_EBLK		(1u << 0)	/* Enable the timer module */

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
#define		ECI				0x10	/* Enable the comparator interrupt signal */
#define		EPI				0x08	/* Enable the PWM counter interrupt signal */
#define		MODE			0x04	/* Select the PWM Mode (Mirror or independent) */
#define		EXT				0x02	/* Select the internal/external counter */
#define		EBLK			0x01	/* Enable module  */

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
 * PWM 5
 */
FARPORTB    (PWM5_CTRL, 0x2872)     /* Control register   */
FARPORTB    (PWM5_PSCL, 0x2873)     /* Prescaler register */
FARPORTW    (PWM5_PER,  0x2874)     /* Period duration    */
FARPORTW    (PWM5_LT,   0x2876)     /* Low threshold register  */
FARPORTW    (PWM5_HT,   0x2878)     /* High threshold register */
FARPORTW    (PWM5_CMP,  0x287A)     /* Compare interrupt threshold register */

/*
 * SPI 1
 */
FARPORTW    (SPI1_CTRL, 0x289A)     /* Control register (word access) */

FARPORTB    (SPI1_PCR,  0x289A)     /* Control register */
#define SPI_CKEN			(1u << 0)	/* Enable bit for SPI clock */
#define SPI_EN				(1u << 1)	/* Enable SPI module */
#define SPI_CPHA			(1u << 2)	/* Clock phase selector; defines the timing relation ship between output clock and output data (1 = transmission starts on falling edge of SPISIB, 0 = transmission starts on first SPSCK edge) */
#define SPI_CPOL			(1u << 3)	/* Clock polarity selector. Logic state of SPI clock output (1 = SPSCK _OUT is high during transmission, 0 = SPSCK _OUT is low during transmission) */
#define SPI_MSTR			(1u << 4)	/* Master bit mode (1 = Master mode selected, 0 = Slave mode selected) */
#define SPI_BYTEMOD			(1u << 5)	/* Frame length selector. (1 = receive/transmit an 8 bits frame, 0 = receive/transmit an 16 bits frame) */
#define SPI_TFIE			(1u << 6)	/* Enable bit for the interrupt request SPI_TF. SPI_TF is the source of interrupt for SPI_TI. (1 = SPI_TF interrupt enabled, 0 = SPI_TF interrupt disabled) */
#define SPI_RFIE			(1u << 7)	/* Enable bit for the interrupt request SPI_RF. SPI_RF is a source of interrupt for SPI_RI.(1 = SPI_RF interrupt enabled, 0 = SPI_RF interrupt disabled) */

FARPORTB    (SPI1_PSCR, 0x289B)     /* Status and Control register */
#define SPI_ERRIE			(1u << 0)	/* Error Interrupt Enable Bit. (1 = SPI_MODF and SPI_OVRF interrupt enabled, 0 = Interrupt disabled; To enable the SPI_MODF interrupt, bit should be combined with SPI_MODFEN.) */
#define SPI_MSTRONLY		(1u << 1)	/* Master transmit mode; only valid in master mode. (1 = Transmit mode selected, 0 = Full-duplex mode selected) */
#define SPI_MODFEN			(1u << 2)	/* Mode Fault enable bit. Should be combined with SPI_ERRIE to enable interrupt on SPI_MODF. (1 = SPI_MODF update enabled, 0 = SPI_MODF forced to 0) */
#define SPI_FRSSOEN			(1u << 3)	/* Frame Slave Select Output Enable Bit; used in Master mode only to drive SSOB signal is drives to the active state (to low) every time when frame is being transmitted or when Master is selected and enabled. (1 = SSOB low when frame is being transmitted, 0 = SSOB low when Master is configured and enabled) */
#define SPI_MODF			(1u << 4)	/* Mode Fault bit; used in master mode only to detect the status of input SPISIB during a transmission. (1 = SPISIB switch high during transmission, 0 = SPISIB stays at low level) */
										/* This bit generates interrupt request UART_MODF if SPI_MODFEN and SPI_OVRF are high. UART_MODF is  a source of interrupt for SPI_RI. */
#define SPI_OVRF			(1u << 5)	/* Overflow flag. SPI_OVRF is a source of interrupt for SPI_RI. (1 = Receive port full, receive data are lost, 0 = data are read before next transmission) */
#define SPI_TF				(1u << 6)	/* Transmit Full. (1 = Transmit port SPI_DR is full, 0 = Transmit port SPIR_DR is empty) */
#define SPI_RF				(1u << 7)	/* Receiver Full. (1 = Receive port SPI_DR is full, 0 = Receive port SPIR_DR is empty) */

FARPORTW    (SPI1_BRR,  0x289C)     /* BRR register */

FARPORTW    (SPI1_DR,   0x289E)     /* Data register */


/*
 * Product specific ports
 */
FARPORTW    (IO_CFG,    0x28BE)
#define FRB_IO5				(1u <<  5)	/* IO[5] interrupt Rising (0)/Falling (1) */
#define FRB_IO4				(1u <<  4)	/* IO[4] interrupt Rising (0)/Falling (1) */
#define FRB_IO3				(1u <<  3)	/* IO[3] interrupt Rising (0)/Falling (1) */
#define FRB_IO2				(1u <<  2)	/* IO[2] interrupt Rising (0)/Falling (1) */
#define FRB_IO1				(1u <<  1)	/* IO[1] interrupt Rising (0)/Falling (1) */
#define FRB_IO0				(1u <<  0)	/* IO[0] interrupt Rising (0)/Falling (1) */

FARPORTW    (IO_DEB,    0x28C0)
#define IO5_DEBOUNCE_8ms	(3u << 10)
#define IO5_DEBOUNCE_4ms	(2u << 10)
#define IO5_DEBOUNCE_1ms	(1u << 10)
#define IO5_DEBOUNCE_OFF	(0u << 10)
#define IO4_DEBOUNCE_8ms	(3u << 8)
#define IO4_DEBOUNCE_4ms	(2u << 8)
#define IO4_DEBOUNCE_1ms	(1u << 8)
#define IO4_DEBOUNCE_OFF	(0u << 8)
#define IO3_DEBOUNCE_8ms	(3u << 6)
#define IO3_DEBOUNCE_4ms	(2u << 6)
#define IO3_DEBOUNCE_1ms	(1u << 6)
#define IO3_DEBOUNCE_OFF	(0u << 6)
#define IO2_DEBOUNCE_8ms	(3u << 4)
#define IO2_DEBOUNCE_4ms	(2u << 4)
#define IO2_DEBOUNCE_1ms	(1u << 4)
#define IO2_DEBOUNCE_OFF	(0u << 4)
#define IO1_DEBOUNCE_8ms	(3u << 2)
#define IO1_DEBOUNCE_4ms	(2u << 2)
#define IO1_DEBOUNCE_1ms	(1u << 2)
#define IO1_DEBOUNCE_OFF	(0u << 2)
#define IO0_DEBOUNCE_8ms	(3u << 0)
#define IO0_DEBOUNCE_4ms	(2u << 0)
#define IO0_DEBOUNCE_1ms	(1u << 0)
#define IO0_DEBOUNCE_OFF	(0u << 0)

FARPORTW    (IO_EXTIO,  0x28C2)
/* bits [15:7] are reserved    */
#define IO5_PMOS_DISABLE    (1u << 6)   /* disable PMOS on IO5  */
#define IO5_OUT             (1u << 5)
#define IO5_ENABLE          (1u << 4)   /* enable IO5 buffer    */
/* bit 3 : reserved */
#define IO4_PMOS_DISABLE    (1u << 2)   /* disable PMOS on IO4  */
#define IO4_OUT             (1u << 1)
#define IO4_ENABLE          (1u << 0)   /* enable IO4 buffer    */

/* 0x28C4: reserved             */
FARPORTW    (DRVCFG,    0x28C6)
#define DIS_OT				(1u << 15)  /* Disable (1)/Enable (0) Over Temperature protection */
#define DIS_OC		        (1u << 14)  /* Disable (1)/Enable (0) Over Current protection */
#define DIS_UV			    (1u << 13)  /* Disable (1)/Enable (0) Under Voltage protection */
#define DIS_OV				(1u << 12)  /* Disable (1)/Enable (0) Over Voltage protection */
#define OVTPM				(1u << 11)	/* Driver configuration at OVT: 0 = Tri-state, 1 = To Ground */
#define DIS_SHOA	        (1u << 10)  /* Disable (1)/Enable (0) OpAmp for ADC measurement of shunt current */
#define OCPM			    (1u << 9)   /* Driver configuration at OC: 0 = To Ground, 1 = Tri-state */
#define DIS_DRV				(1u << 8)   /* Disable (1)/Enable (0) Driver */
#define DRV_CFG_T			(3u << 6)   /* Driver configuration T, see spec. */
#define DRV_CFG_T_TRISTATE  (0u << 6)	/* Tri-state T */
#define DRV_CFG_T_PWM       (1u << 6)	/* PWM T */
#define DRV_CFG_T_0         (2u << 6)	/* '0' T */
#define DRV_CFG_T_1         (3u << 6)	/* '1' T */
#define DRV_CFG_W			(3u << 4)   /* Driver configuration W, see spec. */
#define DRV_CFG_W_TRISTATE  (0u << 4)	/* Tri-state W */
#define DRV_CFG_W_PWM       (1u << 4)	/* PWM W */
#define DRV_CFG_W_0         (2u << 4)	/* '0' W */
#define DRV_CFG_W_1         (3u << 4)	/* '1' W */
#define DRV_CFG_V			(3u << 2)   /* Driver configuration V, see spec. */
#define DRV_CFG_V_TRISTATE  (0u << 2)	/* Tri-state V */
#define DRV_CFG_V_PWM       (1u << 2)	/* PWM V */
#define DRV_CFG_V_0         (2u << 2)	/* '0' V */
#define DRV_CFG_V_1         (3u << 2)	/* '1' V */
#define DRV_CFG_U			(3u << 0)   /* Driver configuration U, see spec. */
#define DRV_CFG_U_TRISTATE  (0u << 0)	/* Tri-state U */
#define DRV_CFG_U_PWM       (1u << 0)	/* PWM U */
#define DRV_CFG_U_0         (2u << 0)	/* '0' U */
#define DRV_CFG_U_1         (3u << 0)	/* '1' U */

FARPORTW    (IO_WU,     0x28C8)

FARPORTW    (IO_IN,     0x28CA)
#define IO_DEB5				(1u << 5)	/* Input (debounced) I/O 5 */
#define IO_DEB4				(1u << 4)	/* Input (debounced) I/O 4 */
#define IO_DEB3				(1u << 3)	/* Input (debounced) I/O 3 */
#define IO_DEB2				(1u << 2)	/* Input (debounced) I/O 2 */
#define IO_DEB1				(1u << 1)	/* Input (debounced) I/O 1 */
#define IO_DEB0				(1u << 0)	/* Input (debounced) I/O 0 */

FARPORTW    (ANA_OUTG,  0x28CC)
#define TC2_ENPU			(1u << 13)	/* TC2 Enable pull-up */
#define TC1_ENPU			(1u << 12)	/* TC1 Enable pull-up */
#define PRUV                (3u << 8)   /* Program under-voltage detection level */
#define PRUV_9V             (3u << 8)   /* Vs=9+-0.5V */
#define PRUV_8V             (2u << 8)   /* Vs=8+-0.5V */
#define PRUV_7V             (1u << 8)   /* Vs=7+-0.5V */
#define PRUV_6V             (0u << 8)   /* Vs=6+-0.5V */
#define INT_WU				(3u << 5)	/* Internal Wake-up timer */
#define INT_WU_DIS			(0u << 5)
#define INT_WU_4k			(1u << 5)	/*  4096 x 1/10kHz = 0.41sec */
#define INT_WU_8k			(2u << 5)	/*  8192 x 1/10kHz = 0.82sec */
#define INT_WU_16k			(3u << 5)	/* 16384 x 1/10kHz = 1.64sec */
#define INACTIVE_OVT        (1u << 7)   /* Disable OVT */

FARPORTW    (ANA_OUTH,  0x28CE)
#define TEST_MODE_DET		(1u << 7)	/* Test-mode entering occurred (detected) */
#define EN_LIN_AGC			(1u << 6)	/* Enable LIN current sources */
#define TRIM_LIN_CS			(7u << 3)	/* Trim bits for output current of LIN ACFG cell */
#define SEL_LIN_CS			(7u << 0)	/* Selection bits for output current of LIN ACFG cell */

FARPORTW    (ANA_OUTI,  0x28D0)
#define SEL_UV_VS			(1u << 12)	/* Switch UV_VS to intern */
#define TC2_DEBOUNCE_400u	(3u << 10)	/* TC2 debounce time 400 us */
#define TC2_DEBOUNCE_200u	(2u << 10)	/* TC2 debounce time 200 us */
#define TC2_DEBOUNCE_100u	(1u << 10)	/* TC2 debounce time 100 us */
#define TC2_DEBOUNCE_OFF	(0u << 10)	/* TC2 debounce time OFF */
#define TC1_DEBOUNCE_400u	(3u << 8)	/* TC1 debounce time 400 us */
#define TC1_DEBOUNCE_200u	(2u << 8)	/* TC1 debounce time 200 us */
#define TC1_DEBOUNCE_100u	(1u << 8)	/* TC1 debounce time 100 us */
#define TC1_DEBOUNCE_OFF	(0u << 8)	/* TC1 debounce time OFF */
#define TMRCFG				0x00FF
#define T2_INB_KEY2			(0u << 6)	/* Timer2 Input-B: KEY1 */
#define T2_INB_KEY1			(1u << 6)	/* Timer2 Input-B: KEY0 */
#define T2_INB_TC2			(2u << 6)	/* Timer2 Input-B: TC2 */
#define T2_INB_TC1			(3u << 6)	/* Timer2 Input-B: TC1 */
#define T2_INA_KEY2			(1u << 4)	/* Timer2 Input-A: KEY1 */
#define T2_INA_KEY1			(0u << 4)	/* Timer2 Input-A: KEY0 */
#define T2_INA_TC2			(3u << 4)	/* Timer2 Input-A: TC2 */
#define T2_INA_TC1			(2u << 4)	/* Timer2 Input-A: TC1 */
#define T1_INB_KEY2			(2u << 2)	/* Timer1 Input-B: KEY1 */
#define T1_INB_KEY1			(3u << 2)	/* Timer1 Input-B: KEY0 */
#define T1_INB_TC2			(0u << 2)	/* Timer1 Input-B: TC2 */
#define T1_INB_TC1			(1u << 2)	/* Timer1 Input-B: TC1 */
#define T1_INA_KEY2			(3u << 0)	/* Timer1 Input-A: KEY1 */
#define T1_INA_KEY1			(2u << 0)	/* Timer1 Input-A: KEY0 */
#define T1_INA_TC2			(1u << 0)	/* Timer1 Input-A: TC2 */
#define T1_INA_TC1			(0u << 0)	/* Timer1 Input-A: TC1 */

FARPORTW    (ANA_OUTK,  0x28D2)
#define SEL_TX_OUT			(15u << 8)
#define SEL_TX_OUT_IO5		(12u << 8)	/* IO5 undebounced */
#define SEL_TX_OUT_IO4		(11u << 8)	/* IO4 undebounced */
#define SEL_TX_OUT_IO3		(10u << 8)	/* IO3 undebounced */
#define SEL_TX_OUT_IO2		(9u << 8)	/* IO2 undebounced */
#define SEL_TX_OUT_IO1		(8u << 8)	/* IO1 undebounced */
#define SEL_TX_OUT_IO0		(7u << 8)	/* IO0 undebounced */
#define SEL_TX_OUT_SOFT		(6u << 8)	/* ANA_OUTN:SOFT_TX */
#define SEL_TX_OUT_T2OUT	(5u << 8)
#define SEL_TX_OUT_T1OUT	(4u << 8)
#define SEL_TX_OUT_PWMI		(3u << 8)
#define SEL_TX_OUT_PWM3		(2u << 8)
#define SEL_TX_OUT_PWM2		(1u << 8)
#define SEL_TX_OUT_PWM1		(0u << 8)
#define _SLEEPB_LIN			(1u << 3)
#define _LSM				(1u << 2)
#define _HSM				(1u << 1)
#define _BYPASS				(1u << 0)

FARPORTW    (ANA_OUTL,  0x28D4)
#define ASSP_LIN_KEY		0x4300		/* ASSP LIN key */
#define	ASSP				(1u << 0)	/* ASSP-mode: 0 = Off, 1 = On */

FARPORTW	(ANA_OUTM,	0x28D6)
#define IO5_OUTCFG_SOFT		(3u << 14)	/* Software */
#define IO5_OUTCFG_T2OUT	(2u << 14)	/* Timer 2 output */
#define IO5_OUTCFG_T1OUT	(1u << 14)	/* Timer 1 output */
#define IO4_OUTCFG_SOFT		(3u << 12)	/* Software */
#define IO4_OUTCFG_T2OUT	(2u << 12)	/* Timer 2 output */
#define IO4_OUTCFG_T1OUT	(1u << 12)	/* Timer 1 output */
#define IO3_OUTCFG_SOFT		(7u << 9)	/* Software */
#define IO3_OUTCFG_PWMI		(6u << 9)	/* PWM I */
#define IO3_OUTCFG_PWM3		(5u << 9)	/* PWM 3 */
#define IO3_OUTCFG_PWM2		(4u << 9)	/* PWM 2 */
#define IO3_OUTCFG_PWM1		(3u << 9)	/* PWM 1 */
#define IO3_OUTCFG_T2OUT	(2u << 9)	/* Timer 2 output */
#define IO3_OUTCFG_T1OUT	(1u << 9)	/* Timer 1 output */
#define IO3_OUTCFG_SPI		(0u << 9)	/* SPI interface */
#define IO2_OUTCFG_SOFT		(7u << 6)	/* Software */
#define IO2_OUTCFG_PWMI		(6u << 6)	/* PWM I */
#define IO2_OUTCFG_PWM3		(5u << 6)	/* PWM 3 */
#define IO2_OUTCFG_PWM2		(4u << 6)	/* PWM 2 */
#define IO2_OUTCFG_PWM1		(3u << 6)	/* PWM 1 */
#define IO2_OUTCFG_T2OUT	(2u << 6)	/* Timer 2 output */
#define IO2_OUTCFG_T1OUT	(1u << 6)	/* Timer 1 output */
#define IO2_OUTCFG_SPI		(0u << 6)	/* SPI interface */
#define IO1_OUTCFG_SOFT		(7u << 3)	/* Software */
#define IO1_OUTCFG_PWMI		(6u << 3)	/* PWM I */
#define IO1_OUTCFG_PWM3		(5u << 3)	/* PWM 3 */
#define IO1_OUTCFG_PWM2		(4u << 3)	/* PWM 2 */
#define IO1_OUTCFG_PWM1		(3u << 3)	/* PWM 1 */
#define IO1_OUTCFG_T2OUT	(2u << 3)	/* Timer 2 output */
#define IO1_OUTCFG_T1OUT	(1u << 3)	/* Timer 1 output */
#define IO1_OUTCFG_SPI		(0u << 3)	/* SPI interface */
#define IO0_OUTCFG_SOFT		(7u << 0)	/* Software */
#define IO0_OUTCFG_PWMI		(6u << 0)	/* PWM I */
#define IO0_OUTCFG_PWM3		(5u << 0)	/* PWM 3 */
#define IO0_OUTCFG_PWM2		(4u << 0)	/* PWM 2 */
#define IO0_OUTCFG_PWM1		(3u << 0)	/* PWM 1 */
#define IO0_OUTCFG_T2OUT	(2u << 0)	/* Timer 2 output */
#define IO0_OUTCFG_T1OUT	(1u << 0)	/* Timer 1 output */
#define IO0_OUTCFG_SPI		(0u << 0)	/* SPI interface */

FARPORTW	(ANA_OUTN,	0x28D8)
#define TX_SOFT_OUT			(1u << 8)	/* TX software output signal */
#define IO5_SOFT_OUT		(1u << 5)	/* IO5 software output signal */
#define IO4_SOFT_OUT		(1u << 4)	/* IO4 software output signal */
#define IO3_SOFT_OUT		(1u << 3)	/* IO3 software output signal */
#define IO2_SOFT_OUT		(1u << 2)	/* IO2 software output signal */
#define IO1_SOFT_OUT		(1u << 1)	/* IO1 software output signal */
#define IO0_SOFT_OUT		(1u << 0)	/* IO0 software output signal */

FARPORTW	(ANA_OUTO,	0x28DA)
#define TMRCFG_T2_INB_RX1	(8u << 12)	/* RX-input */
#define TMRCFG_T2_INB_TC2	(7u << 12)	/* TC2 debounced input */
#define TMRCFG_T2_INB_TC1	(6u << 12)	/* TC1 debounced input */
#define TMRCFG_T2_INB_IO5	(5u << 12)	/* IO5 debounced input */
#define TMRCFG_T2_INB_IO4	(4u << 12)	/* IO4 debounced input */
#define TMRCFG_T2_INB_IO3	(3u << 12)	/* IO3 debounced input */
#define TMRCFG_T2_INB_IO2	(2u << 12)	/* IO2 debounced input */
#define TMRCFG_T2_INB_IO1	(1u << 12)	/* IO1 debounced input */
#define TMRCFG_T2_INB_IO0	(0u << 12)	/* IO0 debounced input */
#define TMRCFG_T2_INA_RX1	(8u << 8)	/* RX-input */
#define TMRCFG_T2_INA_TC2	(7u << 8)	/* TC2 debounced input */
#define TMRCFG_T2_INA_TC1	(6u << 8)	/* TC1 debounced input */
#define TMRCFG_T2_INA_IO5	(5u << 8)	/* IO5 debounced input */
#define TMRCFG_T2_INA_IO4	(4u << 8)	/* IO4 debounced input */
#define TMRCFG_T2_INA_IO3	(3u << 8)	/* IO3 debounced input */
#define TMRCFG_T2_INA_IO2	(2u << 8)	/* IO2 debounced input */
#define TMRCFG_T2_INA_IO1	(1u << 8)	/* IO1 debounced input */
#define TMRCFG_T2_INA_IO0	(0u << 8)	/* IO0 debounced input */
#define TMRCFG_T1_INB_RX1	(8u << 4)	/* RX-input */
#define TMRCFG_T1_INB_TC2	(7u << 4)	/* TC2 debounced input */
#define TMRCFG_T1_INB_TC1	(6u << 4)	/* TC1 debounced input */
#define TMRCFG_T1_INB_IO5	(5u << 4)	/* IO5 debounced input */
#define TMRCFG_T1_INB_IO4	(4u << 4)	/* IO4 debounced input */
#define TMRCFG_T1_INB_IO3	(3u << 4)	/* IO3 debounced input */
#define TMRCFG_T1_INB_IO2	(2u << 4)	/* IO2 debounced input */
#define TMRCFG_T1_INB_IO1	(1u << 4)	/* IO1 debounced input */
#define TMRCFG_T1_INB_IO0	(0u << 4)	/* IO0 debounced input */
#define TMRCFG_T1_INA_RX1	(8u << 0)	/* RX-input */
#define TMRCFG_T1_INA_TC2	(7u << 0)	/* TC2 debounced input */
#define TMRCFG_T1_INA_TC1	(6u << 0)	/* TC1 debounced input */
#define TMRCFG_T1_INA_IO5	(5u << 0)	/* IO5 debounced input */
#define TMRCFG_T1_INA_IO4	(4u << 0)	/* IO4 debounced input */
#define TMRCFG_T1_INA_IO3	(3u << 0)	/* IO3 debounced input */
#define TMRCFG_T1_INA_IO2	(2u << 0)	/* IO2 debounced input */
#define TMRCFG_T1_INA_IO1	(1u << 0)	/* IO1 debounced input */
#define TMRCFG_T1_INA_IO0	(0u << 0)	/* IO0 debounced input */

#endif /* IOPORTS_H_ */
