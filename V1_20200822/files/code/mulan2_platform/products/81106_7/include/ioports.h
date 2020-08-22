/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef IOPORTS_H_
#define IOPORTS_H_

#include <mulan2_io.h>


/* =============================================================================
 *    Product specific system ports (accessed only in system mode of CPU)
 * =============================================================================
 */

/*
 * Second level interrupt controller
 */
FARPORTW    (XI0_MASK,  0x202A) /* Timer1 unit */
FARPORTW    (XI1_MASK,  0x202C) /* unused */
FARPORTW    (XI2_MASK,  0x202E) /* PWM unit */
FARPORTW    (XI3_MASK,  0x2030) /* SPI unit */
FARPORTW    (XI4_MASK,  0x2032) /* Overtemperature, overvoltage and customs */

FARPORTW    (XI0_PEND,  0x2034) /* Timer1 unit */
FARPORTW    (XI1_PEND,  0x2036) /* unused */
FARPORTW    (XI2_PEND,  0x2038) /* PWM unit */
FARPORTW    (XI3_PEND,  0x203A) /* SPI unit */
FARPORTW    (XI4_PEND,  0x203C) /* Overtemperature, overvoltage and customs */

/*
 * RC control
 */
FARPORTW    (RC_CTRL,  0x203E)

/*
 * Enables LINXCFG port
 * enter predefined value 0x5F0A here to enable LINXCFG port
 */
FARPORTW    (LIN_XKEY,  0x2042)

/*
 * Analog chip ports
 */
FARPORTW    (ANA_OUTD,  0x204A) /* ADCREF trimming */
FARPORTW    (ANA_OUTE,  0x204C) /* ADCREF trimming */
FARPORTW    (ANA_OUTF,  0x204E) /* unused */

FARPORTW    (FLASHTRIMA, 0x2050)
FARPORTW    (FLASHTRIMB, 0x2052)

FARPORTW    (CONTROL_EXT, 0x2054)
#define     DIS_FLASH       (1u << 15)
#define     DIS_REF         (1u << 14)
#define     FORCE_VMG       (1u << 13)
#define     FORCE_IREF      (1u << 12)
#define     MEAS_VMG        (1u << 11)
#define     MEAS_IREF       (1u << 10)
#define     EN_VMG          (1u << 9)
#define     EN_IREF         (1u << 8)
#define     FL_DBE_IT       (1u << 7)
#define     TM_RES2         (1u << 6)
#define     TM_RES1         (1u << 5)
#define     TM_TR           (1u << 4)
#define     RDLA            (1u << 3)
#define     RDY_OPTION      (3u << 1)   /* Flash wait-states */
#define     RDY_OPT_FL      (3u << 1)   /* Use READY from flash */
#define     RDY_OPT_2       (2u << 1)   /* Fixed 2 extra Tclk */	
#define     RDY_OPT_1       (1u << 1)   /* Fixed 1 extra Tclk */	
#define     RDY_OPT_0       (0u << 1)   /* Fixed 0 extra Tclk */	
#define     SEL_PRIO        (1u << 0)   /* MLX4/MLX16 priority: 0: MLX4 has priority, 1: MLX16 has priority */

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

PORTW       (ANA_INB,   0x281E)     /* read-only ! */

/*
 * Configuration port for external LIN phy or protocol layer
 * Used with valid LINXKEY
 */
PORTW       (LIN_XCFG,  0x2820)


/*
 * Timer 1
 */
PORTW       (TMR1_CTRL, 0x282A)     /* Control register */
PORTW       (TMR1_REGB, 0x282C)     /* Channel B */
PORTW       (TMR1_REGA, 0x282E)     /* Channel A */
PORTW       (TMR1_CNT,  0x2830)     /* read-only: Counter */

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
FARPORTW    (LV_CFG,   0x28BE)
FARPORTW    (LV_DEB,   0x28C0)     /* Debounce configuration   */
FARPORTW    (LV_OUTOD, 0x28C2)     /* Open drain outputs       */
FARPORTW    (LV_INEN,  0x28C4)     /* Threshold config / Input comparator type */
FARPORTW    (LV_TMR,   0x28C6)     /* Enables wakeup detection                 */
FARPORTW    (LV_ENWU,  0x28C8)     /* Pull-up / pull-down configuration        */
FARPORTW    (LV_IN,    0x28CA)     /* Debounced and un-debounds input from pin */

FARPORTW    (HV_IN,    0x28CC)     /* Debounced and un-debounds input from pin */
FARPORTW    (HV_CFG,   0x28CE)
FARPORTW    (HV_DEB,   0x28D0)     /* Debounce configuration   */
FARPORTW    (HV_OUTOD, 0x28D2)     /* Open drain outputs       */
FARPORTW    (HV_INEN,  0x28D4)     /* Threshold config / Input comparator type */
FARPORTW    (HV_ENWU,  0x28D6)     /* Pull-up / pull-down configuration        */
FARPORTW    (HV_TMR,   0x28D8)     /* Enables wakeup detection                 */

FARPORTW    (ANA_OUTG,  0x28DA)    /* program undervoltage detection level     */
FARPORTW    (ANA_OUTH,  0x28DC)    /* LINAA and current control                */
#define EN_LIN_AA_DAC (1u << 15)   /* */
#define DIV1          (1u << 14)   /* CM range boosting configuration bits */
#define DIV0          (1u << 13)
#define SH4           (1u << 12)   /* S&H clock 4 Current DAC */
#define SH3           (1u << 11)   /* S&H clock 3 Current DAC */
#define SH2           (1u << 10)   /* S&H clock 2 Current DAC */
#define SH1           (1u << 9)    /* S&H clock 1 Current DAC */
#define CDEN          (1u << 8)    /* Current DAC ENABLE */
#define CDOUTEN       (1u << 7)    /* connect the current DAC output to the LIN bus */
#define EN_LINAA      (1u << 6)    /* LIN AA frontend enable (logical 0 = sleep mode) */
#define RST1          (1u << 5)    /* Reset of the 1st DIDO stage */
#define RST2          (1u << 4)    /* Reset of the 2nd DIDO stage */
#define GAIN          (15u << 0)   /* Gain calibration of the 2nd DISO stage */

FARPORTW    (ANA_OUTI,  0x28DE)    /* HV differential measurement control      */
#define SWI_DAC_OUT     (1u << 8)
#define EN1V8V          (1u << 2) /* control 1.8V supply voltage for differential amplifier */
#define CP2             (1u << 1) /* CP2   */
#define CP1             (1u << 0) /* CP1   */


FARPORTW    (ANA_OUTK,  0x28E0)    /* 10Bit DAC                                */

#endif /* IOPORTS_H_ */
