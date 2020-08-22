/*
 * Copyright (C) 2010-2012 Melexis N.V.
 *
 * Software Platform 90365
 *
 */

#ifndef IOPORTS_H_
#define IOPORTS_H_

#include <mmc16_io.h>

/* -------------------------------------------------------------------------
 * System service to switch to System mode (product specific,
 * see product's linker script to define address of system_services section)
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

/* Not Present */

/* ============================================================================
 *  Product specific USER ports
 * (accessed either in user or system mode of CPU)
 * ============================================================================
 */

PORTW (ANA_SETTLE, 0x281A)
PORTB (TIMA, 0x281A)
PORTB (TIMB, 0x281B)

/* Create struct for all ports */
extern volatile struct {
	/* little end first */
	unsigned int TimeA : 7; /* Analog Settling Time A */
	unsigned int Free1_1 : 1;
	unsigned int TimeB : 7; /* Analog Settling Time B */
	unsigned int Free1_2 : 1;
} AnaSettle_v __attribute__((nodp, addr(0x281A)));

PORTB (ADCFREQRATIO, 0x281C)
#define M_ADCFREQRATIO                 (15U << 0U)
#define M_NULL4_4_0                    (15U << 4U)

PORTB (OUTPUT, 0x281D)
#define M_DIGOUT_CONTROL		(1U << 7U)
#define M_FOLLOWER			(1U << 6U)
#define M_NULL2_4			(3U << 4U)
#define M_IO_DIGOUT			(1U << 3U)
#define M_OUTMODE			(7U << 0U)
#define DIGOUT_CONTROL_0                (0U << 7)
#define DIGOUT_CONTROL_1                (1U << 7)
#define FOLLOWER_0                      (0U << 6)
#define FOLLOWER_1                      (1U << 6)
#define IO_DIGOUT_0                     (0U << 3)
#define IO_DIGOUT_1                     (1U << 3)
#define OUTMODE_DISABLE                 (0U << 0)
#define OUTMODE_DIRECTACCESS            (5U << 0)

/* Analog Gain - Coarse & Fine Gain */
PORTW (ANAGAIN, 0x281E)
PORTB (ANAFINEGAIN, 0x281E)
#define M_NULL2	_6			(3U << 6U)
#define M_ANAFINEGAIN			(63U << 0U)

PORTB (COARSEGAIN, 0x281F)
#define M_NULL4_4_1			(15U << 4U)
#define M_COARSEGAIN			(15U << 0U)

/* Misc. */
PORTB (MISC, 0x2820)
#define M_LINE_UP			(1U << 7U)
#define M_HE_SINGLE                    (1U << 6U)
#define M_HE_TOPLEFT                   (1U << 5U)
#define M_BG2TRIM			(7U << 2U)
#define M_OVERCLK			(1U << 1U)
#define M_PLATESHORTED			(1U << 0U)

PORTB (ADC_SETTS1, 0x2821)
#define M_NULL4_4_2			(15U << 4U)
#define M_ADCLOWPOWER			(1U << 3U)
#define M_NULL1_2			(1U << 2U)
#define M_ADCMODE			(3U << 0U)

PORTW(ADC_SETTS2_TST, 0x2822)
PORTB (ADC_SETTS2, 0x2822)
#define M_NULL4_4_3			(15U << 4U)
#define M_FT				(15U << 0U)

PORTB (ADC_SETTS_TST, 0x2823)
#define M_NULL1_7			(1U << 7U)
#define M_TST_CMP			(1U << 6U)
#define M_TST_DAC                      (63U << 0U)
#define M_TST_BIAS			(1U << 5U)
#define M_TST_OA			(1U << 4U)
#define M_TST_CMPI			(1U << 3U)
#define M_TST_REF			(1U << 2U)
#define M_TST_NEGIMP			(1U << 1U)
#define M_TST_STROBE			(1U << 0U)

PORTB (POWERSAVE, 0x2824)
#define M_NULL2_6			(3U << 6U)
#define M_ADC_ALWAYS_DIS		(1U << 5U)
#define M_ADC_NEVER_DIS			(1U << 4U)
#define M_DIDO_ALWAYS_DIS		(1U << 3U)
#define M_DIDO_NEVER_DIS		(1U << 2U)
#define M_TEMP_ALWAYS_DIS		(1U << 1U)
#define M_TEMP_NEVER_DIS		(1U << 0U)
#define M_TEST_DAC			(63U << 0U)

PORTB (GAINSTRESS, 0x2825)
#define M_NULL5_3                      (32U << 3U)
#define M_BG2TRIM_PORT                 (7U << 0U)

PORTW (PWM_CONTROL, 0x2826)
#define	M_PCTRL 		        (255U << 0U)
#define	M_PPSCL				(255U << 8U)

PORTB (PCTRL, 0x2826) /* PWM control status */
#define M_NULL1_7_0                    (1U << 7U)
#define M_CNT_RES                      (1U << 5U)
/* #define M_PEOC_IT_SEL                  (1U << 5U) */
#define M_PEOC_IT_EN                   (1U << 4U)
#define M_PDCE_IT_SEL                  (1U << 3U)
#define M_PDCE_IT_EN                   (1U << 2U)
#define M_PWM_INV                      (1U << 1U)
#define M_PWM_EN                       (1U << 0U)

PORTB (PPSCL, 0x2827) /* PWM pre-scaler */
#define M_PWM_PRDV1                    (15U << 4U)
#define M_PWM_PRDV0                    (15U << 0U)

PORTW (PPER, 0x2828) /* Period value for PWM */

PORTW (PHT, 0x282A) /* High threshold level for PWM output HS */

PORTW (PCMP, 0x282C) /* PWM threshold for Duty-Cycle int */
PORTW (FREEINPUT, 0x282E) /* Read only */
#define M_MT4V                         (1U << 0U)

/* ANA_OUTA */
/*ANA_OUTA_L*/
#define M_OSCTRIM			(127u << 0u)
#define M_NULL1_7_2                      (1u << 7u)
/*ANA_OUTA_H*/
#define M_IN2GAIN			(15u << 12u)
#define M_VANATRIM			(15u << 0u)

/*ANA_OUTB*/
#define M_DAC				(4095u << 0u)
/*ANA_OUTB_L*/
#define M_DAC_L            		(255u << 0u)
/*ANA_OUTB_H*/
#define M_DAC_H            		(15U << 0U)

/*ANA_OUTC_L*/
#define M_FREE1_7                       (1u << 7u)
#define M_VDIGMODE                      (3U << 5U)
#define M_I_TRIM                        (31U << 0U)

/*ANA_TEST_L*/
#define M_IDDQ_DIG            		(1u << 2U)
#define M_IDDQ_CK             		(1u << 1U)
#define M_IDDQ_CURREF        		(1u << 0U)
/*ANA_TEST_H*/
#define M_TESTOUT_DISABLE		(1U << 7U)
#define M_OUTMUXSEL			(15U << 3U)
#define M_TEST_LSB_DAC			(1U << 2U)
#define M_ENVCG               		(1U << 1U)
#define M_ENVS                		(1U << 0U)

/*XIN*/
#define M_PTC_SI			(1U << 0U)
#define M_MT7V				(1U << 1U)

#define M_CMP				(31U << 2U)
#define M_CMP_CNT_DIG		        (1U << 2U)
#define M_CMP_B_DIG			(3U << 3U)
#define M_CMP_A_DIG			(3U << 5U)
#define M_MT3V				(1U << 7U)

/*MASK*/
/* Debug and watch ports */
#define DEB_TIME		0

FARPORTB (TIMER_IT_TGL, 0x2840)	/* Write 1 at beg of TIMER_IT. Write 0 at exit */
FARPORTB (ADC_EOM_IT_TGL, 0x2841)		/* Write 1 at beg of TIMER_IT. Write 0 at exit */

FARPORTB (ADC_TGL, 0x2842) 		/* Write 1 when ADC started and 0 when finished (at beg of ADC_EOM_IT) */
FARPORTB (DSP_TGL, 0x2843) 		/* Write 1 when DSP started and 0 when finished */
FARPORTB (APP_TGL, 0x2844) 		/* Write 1 when APP started and 0 when finished */


#endif /* IOPORTS_H_ */
