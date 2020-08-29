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
 * see product's linker script to define address of system_services section)
 */
#if defined (__WITH_ITC_BUG_WORKAROUND__)

#define __SYS_ENTER_PROTECTED_MODE  __asm__ __volatile__ (          \
                                                    "mov R, #0\n\t" \
                                                    "call fp0:0xB0" \
                                    )
#else

#define __SYS_ENTER_PROTECTED_MODE  __asm__ __volatile__ (          \
                                                    "call fp0:0xB0" \
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
 /* SCI ports */
PORTW (SCI_PORT_BASE, 0x281A )     /* SCI_Address10 */
PORTB (SCI_Address2, 0x281C )
      #define M_MAX_BYTES  (31U<<0U)
PORTB (SCI_Address3, 0x281D )
      #define M_PROT_ERR_MEM        (1U << 7U)
PORTB (SCI_Address4, 0x281E )
      #define M_BYTE_COUNTER        (31U << 3U)
      #define M_BIT_COUNTER         (7U << 0U)
PORTB (SCI_Address5, 0x281F )
      #define M_EMPTY_FRAME         (1U << 7U)
      #define M_SS_SYNC             (1U << 6U)
      #define M_COUNT_8             (1U << 5U)
      #define M_USE_DEFAULT_BASE    (1U << 4U)
      #define M_DEFAULT_FRAME_USED  (1U << 1U)
      #define M_DMA_OVERFLOW        (1U << 0U)
/* --- */

PORTW (TIMA, 0x2820 )
      #define M_ADC_INSEL (15U << 4U)
PORTB (TIMA_L, 0x2820 )
PORTB (TIMA_H, 0x2821 )
PORTB (ADCFREQRATIO, 0x2822 )
PORTB (ANAFINEGAIN, 0x2823 )
      #define M_ENVCG               (1U << 7U)
      #define M_ENVS                (1U << 6U)
      #define M_ANA_FINE_GAIN       (63U << 0U)

/* ANA_OUTA */
      #define M_FREE1_BIT           (1U << 13U)
/*ANA_OUTA_H*/
      #define M_BYTE_RESET_MASTER   (1U << 7U)
      #define M_BYTE_FREE1_BIT      (1U << 5U)
      #define M_WORD_RESET_MASTER   (1U << 15U)
      #define M_OVER_CLK            (1U << 3U)
      #define M_PLATES_SHORTED      (1U << 2U)
      #define M_PIN_FILTER          (3U << 0U)
/*ANA_OUTA_L*/
      #define M_I_TRIM              (7U << 5U)
      #define M_OSCTRIM             (31U << 0U)
/*ANA_OUTB_H*/
      #define M_RING_OUT            (1U << 7U)
      #define M_PLATE_ALL_ON        (1U << 6U)
      #define M_PLATE_ALL_OFF       (1U << 5U)
      #define M_PLATE_HALF          (1U << 4U)
      #define M_FT                  (15U << 0U)   
/*ANA_OUTB_L*/
      #define M_TEST_CMP            (1u << 6U)
      #define M_TEST_BIAS           (1u << 5U)
      #define M_TEST_OA             (1u << 4U)
      #define M_TEST_CMPI           (1u << 3U)
      #define M_TEST_REF            (1u << 2U)
      #define M_TEST_NEGIMP         (1u << 1U)
      #define M_TEST_STROBE         (1u << 0U)
      #define M_TEST_DAC            (63U << 0U)
/*ANA_OUTC_H*/
      #define M_ADC_MODE            (3U << 6U)
      #define M_DISABLING21         (3U << 4U)
      #define M_COARSEGAIN          (15U << 0U)
/*ANA_OUTC_L*/
      #define M_DISABLING0          (1U << 7U)
      #define M_OFFSET              (127U << 0U)
/*ANA_TEST_H*/
/*ANA_TEST_L*/
      #define M_IDDQ_DIG            (1u << 2U)
      #define M_IDDQ_CK             (1u << 1U)
      #define M_IDDQ_CUR_REF        (1u << 0U)      
/*MASK*/
      #define EN_SCI_IT         	EN_EXT1_IT /* redirect */
      #define EN_SWI_IT          	EN_SOFT_IT /* redirect */

#endif /* IOPORTS_H_ */
