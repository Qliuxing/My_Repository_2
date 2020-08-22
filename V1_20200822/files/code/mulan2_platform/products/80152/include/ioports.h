/*
 * Copyright (C) 2010-2012 Melexis N.V.
 *
 * Software Platform
 *
 */
 
#ifndef IOPORTS_H_
#define IOPORTS_H_

#include <mmc16_io.h>

/*
 * ADC configuration
 *
 *  ADC_CFG[15:8]  -- channel selection
 *  ADC_CFG[7:0]   -- HW trigger source, ADC reference
 */

/* ADC Reference */
#define ADC_SREF_2V5    0x03
#define ADC_SREF_PAD    0x07

/* ADC Trigger selections */
#define ADC_SREF_HWT_BIT_SHIFT          3

#define ADC_SREF_HWT_PWMI               (0x00u << ADC_SREF_HWT_BIT_SHIFT)
#define ADC_SREF_HWT_PWMA               (0x01u << ADC_SREF_HWT_BIT_SHIFT)
#define ADC_SREF_HWT_nPWMA              (0x02u << ADC_SREF_HWT_BIT_SHIFT)
#define ADC_SREF_HWT_ICT1_TIMER1_INT    (0x09u << ADC_SREF_HWT_BIT_SHIFT)
#define ADC_SREF_HWT_ICT1_TIMER2_INT    (0x0Au << ADC_SREF_HWT_BIT_SHIFT)
#define ADC_SREF_HWT_ICT1_TIMER3_INT    (0x0Bu << ADC_SREF_HWT_BIT_SHIFT)
#define ADC_SREF_HWT_HSA                (0x0Cu << ADC_SREF_HWT_BIT_SHIFT)
#define ADC_SREF_HWT_LSA                (0x0Du << ADC_SREF_HWT_BIT_SHIFT)
#define ADC_SREF_HWT_CUREG_MASK         (0x12u << ADC_SREF_HWT_BIT_SHIFT)
#define ADC_SREF_HWT_nCURREG_MASK       (0x13u << ADC_SREF_HWT_BIT_SHIFT)


/* ADC Input multiplexer selection */
#define ADC_SIN_BIT_SHIFT               8

#define ADC_SIN_CURRENT_SENSOR          ( 0u << ADC_SIN_BIT_SHIFT)
#define ADC_SIN_VSUP_SENSOR             ( 1u << ADC_SIN_BIT_SHIFT)
#define ADC_SIN_TEMP_SENSOR             ( 2u << ADC_SIN_BIT_SHIFT)
#define ADC_SIN_ANA_IN_IO0              ( 3u << ADC_SIN_BIT_SHIFT)
#define ADC_SIN_ANA_IN_IO1              ( 4u << ADC_SIN_BIT_SHIFT)
#define ADC_SIN_ANA_IN_IO2              ( 5u << ADC_SIN_BIT_SHIFT)
#define ADC_SIN_ANA_IN_IO3              ( 6u << ADC_SIN_BIT_SHIFT)
#define ADC_SIN_ANA_IN_IO4              ( 7u << ADC_SIN_BIT_SHIFT)
#define ADC_SIN_ANA_IN_IO5              ( 8u << ADC_SIN_BIT_SHIFT)
#define ADC_SIN_VPHASE                  ( 9u << ADC_SIN_BIT_SHIFT)
#define ADC_SIN_VDRAIN                  (12u << ADC_SIN_BIT_SHIFT)
#define ADC_SIN_ANA_IN_IO6              (13u << ADC_SIN_BIT_SHIFT)
#define ADC_SIN_ANA_IN_IO7              (14u << ADC_SIN_BIT_SHIFT)
#define ADC_SIN_VBOOST_VSUP             (15u << ADC_SIN_BIT_SHIFT)

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
 *  Product specific SYSTEM ports
 * (accessed only in system mode of CPU)
 */

FARPORTB (EXTMEM_CTRL_STATUS,       0x202A)                                     /*! external memory control register */
    #define WAIT_CYCLE_CONFIG_0     (1u << 0)                                   /*! CPU speed: 00 - 2.667 MIPS, 1..3 - 3.2 MIPS */
    #define WAIT_CYCLE_CONFIG_1     (1u << 1)
    #define MEMORY_EXTENTION        (1u << 2)
    #define FLASH_OTPB              (1u << 3)                                   /*! Memory selection 0 - OTP, 1 - flash */
FARPORTB (STANDBY_CTRL,             0x202B)
    #define STANDBY_EN              (1u << 0)                                   /*! 1 - set standby mode */
    #define STANDBY_EN_WE           (1u << 1)                                   /*! write enable for the STANDBY_EN bit 1 - STANDBY_EN can be set*/
FARPORTW(ANALOG_TEST,               0x202C)                                     /*! testregister for analog tests */
FARPORTW(ANA_DIG_TEST,              0x202E)                                     /*! analog and digital testregister */
FARPORTW(DIG_TEST_A,                0x2030)                                     /*! first digital testregister */
FARPORTW(DIG_TEST_B,                0x2032)                                     /*! second digital testregister */
FARPORTB(OTP_TM,                    0x2034)
    #define OTP_TM_0                (1u << 0)    
    #define OTP_TM_1                (1u << 1)    
    #define OTP_TM_2                (1u << 2)    
    #define OTP_TM_3                (1u << 3)    
    #define OTP_TM_4                (1u << 4)    
    #define OTP_TM_5                (1u << 5)
FARPORTB(WAKEUP_CONFIG,             0x2035)                                     /*! configure the wake up capabilities of the chip */
    #define HVIO0_WU_EN             (1u << 0)                                   /*! enable the wake up capability of HVIO 0 */
    #define HVIO1_WU_EN             (1u << 1)                                   /*! enable the wake up capability of HVIO 1 */
    #define HVIO2_WU_EN             (1u << 2)                                   /*! enable the wake up capability of HVIO 2 */
/* Second level interrupt control/status registers */
FARPORTW (DIAGNOSTIC_MASK,          0x2038)                                     /*! the masking bits of the diagnostic interrupts */
    #define DIAG_MASK_VSUP_OV       (1u << 0)                                   /*! Vsup over voltage masking bit */
    #define DIAG_MASK_VSUP_UV       (1u << 1)                                   /*! Vsup under voltage masking bit */
    #define DIAG_MASK_VREG_UV       (1u << 2)                                   /*! Vreg under voltage masking bit */
    #define DIAG_MASK_VBOOST_UV     (1u << 3)                                   /*! Vboost under voltage masking bit */
    #define DIAG_MASK_OVERTEMP      (1u << 4)                                   /*! Over temperature masking bit */
    #define DIAG_MASK_VDS_ERROR     (1u << 5)                                   /*! VDS error masking bit */
    #define DIAG_MASK_VGS_ERROR     (1u << 6)                                   /*! VGS error masking bit */
    #define DIAG_MASK_OVER_CURRENT  (1u << 7)                                   /*! Over current masking bit */
    #define DIAG_MASK_HVIO_SHORT0   (1u << 8)                                   /*! Short circuit on HVIO0 interrupt masking bit */
    #define DIAG_MASK_HVIO_SHORT1   (1u << 9)                                   /*! Short circuit on HVIO1 interrupt masking bit */
    #define DIAG_MASK_HVIO_SHORT2   (1u << 10)                                  /*! Short circuit on HVIO2 interrupt masking bit */
FARPORTW (DIAGNOSTIC_PEND,          0x203A)
    #define DIAG_PEND_VSUP_OV       (1u << 0)                                   /*! Vsup over voltage pending bit */
    #define DIAG_PEND_VSUP_UV       (1u << 1)                                   /*! Vsup under voltage pending bit */
    #define DIAG_PEND_VREG_UV       (1u << 2)                                   /*! Vreg under voltage pending bit */
    #define DIAG_PEND_VBOOST_UV     (1u << 3)                                   /*! Vboost under voltage pending bit */
    #define DIAG_PEND_OVERTEMP      (1u << 4)                                   /*! Over temperature pending bit */
    #define DIAG_PEND_VDS_ERROR     (1u << 5)                                   /*! VDS error pending bit */
    #define DIAG_PEND_VGS_ERROR     (1u << 6)                                   /*! VGS error pending bit */
    #define DIAG_PEND_OVER_CURRENT  (1u << 7)                                   /*! Over current pending bit */
    #define DIAG_PEND_HVIO_SHORT0   (1u << 8)                                   /*! Short circuit on HVIO0 interrupt pending bit */
    #define DIAG_PEND_HVIO_SHORT1   (1u << 9)                                   /*! Short circuit on HVIO1 interrupt pending bit */
    #define DIAG_PEND_HVIO_SHORT2   (1u << 10)                                  /*! Short circuit on HVIO2 interrupt pending bit */
FARPORTW (DIAGNOSTIC_JMP_VECTOR,    0x203C)

FARPORTW (EXTIO_MASK,               0x203E)                                     /*! external IO masking bits */
    #define EXTIO_MASK_CURREG       (1u << 0)                                   /*! current regulation masking bit */
    #define EXTIO_MASK_SPI_OVF      (1u << 1)                                   /*! SPI overflow masking bit */
    #define EXTIO_MASK_SPI_REC_WORD (1u << 2)                                   /*! SPI word received masking bit */
    #define EXTIO_MASK_SPI_TR_WORD  (1u << 3)                                   /*! SPI word transmitted masking bit */
    #define EXTIO_MASK_EXT_IRQ_0    (1u << 4)                                   /*! External IRQ 0 masking bit */
    #define EXTIO_MASK_EXT_IRQ_1    (1u << 5)                                   /*! External IRQ 1 masking bit */
    #define EXTIO_MASK_EXT_IRQ_2    (1u << 6)                                   /*! External IRQ 2 masking bit */
    #define EXTIO_MASK_EXT_IRQ_3    (1u << 7)                                   /*! External IRQ 3 masking bit */
    #define EXTIO_MASK_EXT_IRQ_4    (1u << 8)                                   /*! External IRQ 4 masking bit */
    #define EXTIO_MASK_EXT_IRQ_5    (1u << 9)                                   /*! External IRQ 5 masking bit */
    #define EXTIO_MASK_EXT_IRQ_6    (1u << 10)                                  /*! External IRQ 6 masking bit */
FARPORTW (EXTIO_PEND,               0x2040)
    #define EXTIO_PEND_CURREG       (1u << 0)                                   /*! current regulation pending bit */
    #define EXTIO_PEND_SPI_OVF      (1u << 1)                                   /*! SPI overflow pending bit */
    #define EXTIO_PEND_SPI_REC_WORD (1u << 2)                                   /*! SPI word received pending bit */
    #define EXTIO_PEND_SPI_TR_WORD  (1u << 3)                                   /*! SPI word transmitted pending bit */
    #define EXTIO_PEND_EXT_IRQ_0    (1u << 4)                                   /*! External IRQ 0 pending bit */
    #define EXTIO_PEND_EXT_IRQ_1    (1u << 5)                                   /*! External IRQ 1 pending bit */
    #define EXTIO_PEND_EXT_IRQ_2    (1u << 6)                                   /*! External IRQ 2 pending bit */
    #define EXTIO_PEND_EXT_IRQ_3    (1u << 7)                                   /*! External IRQ 3 pending bit */
    #define EXTIO_PEND_EXT_IRQ_4    (1u << 8)                                   /*! External IRQ 4 pending bit */
    #define EXTIO_PEND_EXT_IRQ_5    (1u << 9)                                   /*! External IRQ 5 pending bit */
    #define EXTIO_PEND_EXT_IRQ_6    (1u << 10)                                  /*! External IRQ 6 pending bit */

FARPORTW (EXTIO_JMP_VECTOR,         0x2042)

FARPORTW (TIMER1_MASK,              0x2044)
    #define TIMER1_MASK_IRQ_1       (1u << 0)                                   /*! timer 1 interrupt 1 masking bit */
    #define TIMER1_MASK_IRQ_2       (1u << 1)                                   /*! timer 1 interrupt 2 masking bit */
    #define TIMER1_MASK_IRQ_3       (1u << 2)                                   /*! timer 1 interrupt 3 masking bit */
    #define TIMER1_MASK_IRQ_4       (1u << 3)                                   /*! timer 1 interrupt 4 masking bit */
    #define TIMER1_MASK_IRQ_5       (1u << 4)                                   /*! timer 1 interrupt 5 masking bit */
FARPORTW (TIMER1_PEND,              0x2046)
    #define TIMER1_PEND_IRQ_1       (1u << 0)                                   /*! timer 1 interrupt 1 pending bit */
    #define TIMER1_PEND_IRQ_2       (1u << 1)                                   /*! timer 1 interrupt 2 pending bit */
    #define TIMER1_PEND_IRQ_3       (1u << 2)                                   /*! timer 1 interrupt 3 pending bit */
    #define TIMER1_PEND_IRQ_4       (1u << 3)                                   /*! timer 1 interrupt 4 pending bit */
    #define TIMER1_PEND_IRQ_5       (1u << 4)                                   /*! timer 1 interrupt 5 pending bit */
FARPORTW (TIMER1_JMP_VECTOR,        0x2048)

FARPORTW (TIMER2_MASK,              0x204A)
    #define TIMER2_MASK_IRQ_1       (1u << 0)                                   /*! timer 2 interrupt 1 masking bit */
    #define TIMER2_MASK_IRQ_2       (1u << 1)                                   /*! timer 2 interrupt 2 masking bit */
    #define TIMER2_MASK_IRQ_3       (1u << 2)                                   /*! timer 2 interrupt 3 masking bit */
    #define TIMER2_MASK_IRQ_4       (1u << 3)                                   /*! timer 2 interrupt 4 masking bit */
    #define TIMER2_MASK_IRQ_5       (1u << 4)                                   /*! timer 2 interrupt 5 masking bit */
FARPORTW (TIMER2_PEND,              0x204C)
    #define TIMER2_PEND_IRQ_1       (1u << 0)                                   /*! timer 2 interrupt 1 pending bit */
    #define TIMER2_PEND_IRQ_2       (1u << 1)                                   /*! timer 2 interrupt 2 pending bit */
    #define TIMER2_PEND_IRQ_3       (1u << 2)                                   /*! timer 2 interrupt 3 pending bit */
    #define TIMER2_PEND_IRQ_4       (1u << 3)                                   /*! timer 2 interrupt 4 pending bit */
    #define TIMER2_PEND_IRQ_5       (1u << 4)                                   /*! timer 2 interrupt 5 pending bit */
FARPORTW (TIMER2_JMP_VECTOR,        0x204E)

FARPORTW (PWM_MASK,                 0x2050)
    #define PWM_MASK_PWMA           (1u << 0)                                   /*! PWMA masking bit */
    #define PWM_MASK_PWMI           (1u << 0)                                   /*! PWMI masking bit */
FARPORTW (PWM_PEND,                 0x2052)
    #define PWM_PEND_PWMA           (1u << 0)                                   /*! PWMA pending bit */
    #define PWM_PEND_PWMI           (1u << 0)                                   /*! PWMI pending bit */
FARPORTW (PWM_JMP_VECTOR,           0x2054)

/* ----------------------------------------------------------------------------
 *  Product specific USER ports
 */
FARPORTB(EXTIO_IN,                  0x281A)                                     /*! input register for the external IOs */
    #define EXTIO_IN_0              (1u << 0)                                   /*! external IO pin 0 */
    #define EXTIO_IN_1              (1u << 1)                                   /*! external IO pin 1 */
    #define EXTIO_IN_2              (1u << 2)                                   /*! external IO pin 2 */
    #define EXTIO_IN_3              (1u << 3)                                   /*! external IO pin 3 */
    #define EXTIO_IN_4              (1u << 4)                                   /*! external IO pin 4 */
    #define EXTIO_IN_5              (1u << 5)                                   /*! external IO pin 5 */
    #define EXTIO_IN_6              (1u << 6)                                   /*! external IO pin 6 */
FARPORTB(EXTIO_OPEN_DRAIN,          0x281B)                                     /*! open drain control bits of the external IOs */
    #define EXTIO_OPEN_DRAIN_0     (1u << 0)                                    /*! open drain of external IO 0 */
    #define EXTIO_OPEN_DRAIN_1     (1u << 1)                                    /*! open drain of external IO 1 */
    #define EXTIO_OPEN_DRAIN_2     (1u << 2)                                    /*! open drain of external IO 2 */
    #define EXTIO_OPEN_DRAIN_3     (1u << 3)                                    /*! open drain of external IO 3 */
    #define EXTIO_OPEN_DRAIN_4     (1u << 4)                                    /*! open drain of external IO 4 */
    #define EXTIO_OPEN_DRAIN_5     (1u << 5)                                    /*! open drain of external IO 5 */
    #define EXTIO_OPEN_DRAIN_6     (1u << 6)                                    /*! open drain of external IO 6 */
FARPORTB(DRV_CTRL,                  0x281C)                                     /*! control register for the motor driver */
    #define PWM_START               (1u << 0)                                   /*! start the clock of the PWM module */
    #define CURRENT_REG_EN          (1u << 1)                                   /*! enable the current regulation */
FARPORTB(CP_CTRL,                   0x281D)                                     /*! charge pump control register */
    #define CP_EN                   (1u << 0)                                   /*! enable the charge pump */
    #define CP_DOUBLE               (1u << 1)                                   /*! activate the double chargepump */
    #define CP_PARALLEL             (1u << 2)                                   /*! double charge pump in parallel mode */
    #define CP_SLOPEDIS             (1u << 3)                                   /*! disable the slope control of the charge pump */
    #define CP_DISCHARGE            (1u << 4)                                   /*! */
    #define CP_CLOCK_0              (1u << 5)                                   /*! charge pump clock selector, bit 0 */
    #define CP_CLOCK_1              (1u << 6)                                   /*! charge pump clock selector, bit 1 */
FARPORTB(DRV_CONFIG,                0x281E)                                     /*! configuration register of the driver */
    #define PWM_SPEED_0             (1u << 0)                                   /*! PWM speed configuration, bit 0 */
    #define PWM_SPEED_1             (1u << 1)                                   /*! PWM speed configuration, bit 1 */
    #define PWM_SPEED_2             (1u << 2)                                   /*! PWM speed configuration, bit 2 */
FARPORTB(DEADTIME_VDS_CONFIG,       0x281F)                                     /*! configure the deadtime for the driver and the VDS detector */
    #define DRV_DEADTIME_0          (1u << 0)                                   /*! driver deadtime, bit 0 */
    #define DRV_DEADTIME_1          (1u << 1)                                   /*! driver deadtime, bit 1 */
    #define DRV_DEADTIME_2          (1u << 2)                                   /*! driver deadtime, bit 2 */
    #define DRV_DEADTIME_3          (1u << 3)                                   /*! driver deadtime, bit 3 */
    #define VDS_MASKTIME_0          (1u << 4)                                   /*! VDS masktime bit 0 */
    #define VDS_MASKTIME_1          (1u << 5)                                   /*! VDS masktime bit 1 */
    #define VDS_MASKTIME_2          (1u << 6)                                   /*! VDS masktime bit 2 */
    #define VDS_MASKTIME_3          (1u << 7)                                   /*! VDS masktime bit 3 */
FARPORTW(NEXT_MOTOR_STATE,          0x2822)                                     /*! next motor state register */
    #define NEXT_MOTOR_STATE_0      (1u << 0)                                   /*! next motor state, bit 0 */
    #define NEXT_MOTOR_STATE_1      (1u << 1)                                   /*! next motor state, bit 1 */
    #define NEXT_MOTOR_STATE_2      (1u << 2)                                   /*! next motor state, bit 2 */
    #define NEXT_MOTOR_STATE_3      (1u << 3)                                   /*! next motor state, bit 3 */
FARPORTB(PWMA_COMP,                 0x2824)                                     /*! PWM A duty cycle register */
FARPORTB(PWMI_COMP,                 0x2827)                                     /*! PWM I duty cycle register */
FARPORTB(CUR_REG_CTRL,              0x2830)                                     /*! current regulation control register */
    #define CURREG_MASK_HS_LSB      (1u << 2)                                   /*! mask the current regulation based on the high or the low side */
    #define CURREG_MASK_RIS_FALLB   (1u << 3)                                   /*! mask the current regulation on the rising or fallin edge of the PWM output */
    #define CURREG_MASK_TIME_0      (1u << 4)                                   /*! masking time of the current regulation, bit 0 */
    #define CURREG_MASK_TIME_1      (1u << 5)                                   /*! masking time of the current regulation, bit 1 */
    #define CURREG_MASK_TIME_2      (1u << 6)                                   /*! masking time of the current regulation, bit 2 */
    #define CURREG_MASK_TIME_3      (1u << 7)                                   /*! masking time of the current regulation, bit 3 */
FARPORTB(CUR_REG_DAC,               0x2831)                                     /*! current regulation DAC value */
PORTW   (TIMER1_REGA,               0x2832)                                     /*! timer 1 capture compare register A */
PORTW   (TIMER1_REGB,               0x2834)                                     /*! timer 1 capture compare register B */
PORTW   (TIMER1_CTRL,               0x2836)                                     /*! timer 1 control register */
    #define TIMER1_1MHZ_16MHZ_B     (1u << 0)                                   /*! input clock selector */
    #define TIMER1_START_BIT        (1u << 1)                                   /*! startbit */
    #define TIMER1_EDGEA_0          (1u << 2)                                   /*! edge selector for capture A event bit 0 */
    #define TIMER1_EDGEA_1          (1u << 3)                                   /*! edge selector for capture A event bit 1 */
    #define TIMER1_EDGEB_0          (1u << 4)                                   /*! edge selector for capture B event bit 0 */
    #define TIMER1_EDGEB_1          (1u << 5)                                   /*! edge selector for capture B event bit 1 */
    #define TIMER1_DIN_0            (1u << 6)                                   /*! timer 1 PWM mode selector bit 0 */
    #define TIMER1_DIN_1            (1u << 7)                                   /*! timer 1 PWM mode selector bit 1 */
    #define TIMER1_OVRA             (1u << 8)                                   /*! timer 1 overwrite A bit */
    #define TIMER1_OVRB             (1u << 9)                                   /*! timer 1 overwrite B bit */
    #define TIMER1_ENCMP            (1u << 10)                                  /*! timer 1 enable comparators */
    #define TIMER1_MODE_0           (1u << 11)                                  /*! timer 1 mode selector, bit 0 */
    #define TIMER1_MODE_1           (1u << 12)                                  /*! timer 1 mode selector, bit 1 */
    #define TIMER1_MODE_2           (1u << 13)                                  /*! timer 1 mode selector, bit 2 */
    #define TIMER1_DIV_0            (1u << 14)                                  /*! timer 1 clock divider selector bit 0 */
    #define TIMER1_DIV_1            (1u << 15)                                  /*! timer 1 clock divider selector bit 1 */
PORTW   (TIMER2_REGA,               0x2838)                                     /*! timer 2 capture compare register A */
PORTW   (TIMER2_REGB,               0x283A)                                     /*! timer 2 capture compare register B */
PORTW   (TIMER2_CTRL,               0x283C)                                     /*! timer 2 control register */
    #define TIMER2_1MHZ_16MHZ_B     (1u << 0)                                   /*! input clock selector */
    #define TIMER2_START_BIT        (1u << 1)                                   /*! startbit */
    #define TIMER2_EDGEA_0          (1u << 2)                                   /*! edge selector for capture A event bit 0 */
    #define TIMER2_EDGEA_1          (1u << 3)                                   /*! edge selector for capture A event bit 1 */
    #define TIMER2_EDGEB_0          (1u << 4)                                   /*! edge selector for capture B event bit 0 */
    #define TIMER2_EDGEB_1          (1u << 5)                                   /*! edge selector for capture B event bit 1 */
    #define TIMER2_DIN_0            (1u << 6)                                   /*! timer 2 PWM mode selector bit 0 */
    #define TIMER2_DIN_1            (1u << 7)                                   /*! timer 2 PWM mode selector bit 1 */
    #define TIMER2_OVRA             (1u << 8)                                   /*! timer 2 overwrite A bit */
    #define TIMER2_OVRB             (1u << 9)                                   /*! timer 2 overwrite B bit */
    #define TIMER2_ENCMP            (1u << 10)                                  /*! timer 2 enable comparators */
    #define TIMER2_MODE_0           (1u << 11)                                  /*! timer 2 mode selector, bit 0 */
    #define TIMER2_MODE_1           (1u << 12)                                  /*! timer 2 mode selector, bit 1 */
    #define TIMER2_MODE_2           (1u << 13)                                  /*! timer 2 mode selector, bit 2 */
    #define TIMER2_DIV_0            (1u << 14)                                  /*! timer 2 clock divider selector bit 0 */
    #define TIMER2_DIV_1            (1u << 15)                                  /*! timer 2 clock divider selector bit 1 */
FARPORTW(SPI_DATA,                  0x283E)                                     /*! SPI data register */
FARPORTW(TIMER1_CNT,                0x2840)                                     /*! timer 1 counter register */
FARPORTW(TIMER2_CNT,                0x2842)                                     /*! timer 2 counter register */
FARPORTW(SEL_IO_CONFIG,             0x2844)                                     /*! configuration register for the external IOs */
    #define SEL_IO_0_0              (1u << 0)                                   /*! IO function selector IO 0, bit 0 */
    #define SEL_IO_0_1              (1u << 1)                                   /*! IO function selector IO 0, bit 1 */
    #define SEL_IO_1_0              (1u << 2)                                   /*! IO function selector IO 1, bit 0 */
    #define SEL_IO_1_1              (1u << 3)                                   /*! IO function selector IO 1, bit 1 */
    #define SEL_IO_2_0              (1u << 4)                                   /*! IO function selector IO 2, bit 0 */
    #define SEL_IO_2_1              (1u << 5)                                   /*! IO function selector IO 2, bit 1 */
    #define SEL_IO_3_0              (1u << 6)                                   /*! IO function selector IO 3, bit 0 */
    #define SEL_IO_3_1              (1u << 7)                                   /*! IO function selector IO 3, bit 1 */
    #define SEL_IO_4_0              (1u << 8)                                   /*! IO function selector IO 4, bit 0 */
    #define SEL_IO_4_1              (1u << 9)                                   /*! IO function selector IO 4, bit 1 */
    #define SEL_IO_5_0              (1u << 10)                                  /*! IO function selector IO 5, bit 0 */
    #define SEL_IO_5_1              (1u << 11)                                  /*! IO function selector IO 5, bit 1 */
    #define SEL_IO_6_0              (1u << 12)                                  /*! IO function selector IO 6, bit 0 */
    #define SEL_IO_6_1              (1u << 13)                                  /*! IO function selector IO 6, bit 1 */
FARPORTB(EXTMEM_SED_POR_STATUS,    0x2846)                                      /*! status register of the POR and the external memory */
    #define EXTMEM_SED_STAT         (1u << 0)
    #define EXTMEM_DED_STAT         (1u << 1)
    #define EXT_WD_ERROR_STAT       (1u << 2)                                   /*! external watchdog error */
    #define HVIO0_WU_STAT           (1u << 3)                                   /*! wake up because of HVIO 0 */
    #define HVIO1_WU_STAT           (1u << 4)                                   /*! wake up because of HVIO 1*/
    #define HVIO2_WU_STAT           (1u << 5)                                   /*! wake up because of HVIO 2 */
FARPORTB(IO_CONFIG,                 0x2847)                                     /*! configuration of the IOs */
    #define DIS_APD                 (1u << 0)                                   /*! disable the automatic pull down */
    #define DRV_DIS_BY_EXTIO3       (1u << 1)                                   /*! enable/disable the switch off of the driver on a rising edge on IO 3 */
    #define ANA_IN_IO0_ON           (1u << 2)                                   /*! connect/disconnect HVIO to the analog input of IO 0 */
    #define PULLUP_IO0              (1u << 3)                                   /*! enable/disable the pull up to Vss on pin HVIO0*/
    #define SET_IO1_VREG            (1u << 4)                                   /*! */
    #define SET_IO2_VREG            (1u << 5)                                   /*! */
    #define SET_IO5_VREG            (1u << 6)                                   /*! */
    #define HVIO_SHORTDET_DIS       (1u << 7)                                   /*! disable the short detection on the HVIO pins */
FARPORTB(EXT_WD_CTRL, 0x2848)                                                   /*! control the external watchdog */
    #define EXT_WD_ERROR_EN         (1u << 0)                                   /*! */
    #define EXT_WD_ACK              (1u << 1)                                   /*! acknowledge the external watchdog */
    #define EXT_WD_ERROR_EN_WE      (1u << 2)                                   /*! enable writing in the EXT_WD_ERROR_EN bit */
FARPORTB(MIXED,                     0x2849)                                     /*! some miscellaneous bits */
    #define SPI_EN                  (1u << 0)                                   /*! enable the SPI interface */
    #define SPI_CLK_R_FB            (1u << 1)                                   /*! select rising or falling edge for the SPI clock */
    #define BYTEMODE                (1u << 2)                                   /*! select byte or word mode for the SPI cell */
    #define ADC_SPEED               (1u << 3)                                   /*! select the slow or fast speed for the ADC */
    #define ADC_COMP_ON             (1u << 4)                                   /*! switch on the ADC comparators */
    #define ISENSE_ZERO             (1u << 5)                                   /*! */
    #define HS_LSB                  (1u << 6)                                   /*! */
FARPORTB(TIMER1_INPUT_SELECT,       0x284A)                                     /*! input selection register for timer 1 */
    #define TIMER1_CAPA_SELECT_0    (1u << 0)                                   /*! capture A selector, bit 0 */
    #define TIMER1_CAPA_SELECT_1    (1u << 1)                                   /*! capture A selector, bit 1 */
    #define TIMER1_CAPA_SELECT_2    (1u << 2)                                   /*! capture A selector, bit 2 */
    #define TIMER1_CAPA_SELECT_3    (1u << 3)                                   /*! capture A selector, bit 3 */
    #define TIMER1_CAPB_SELECT_0    (1u << 4)                                   /*! capture B selector, bit 0 */
    #define TIMER1_CAPB_SELECT_1    (1u << 5)                                   /*! capture B selector, bit 1 */
    #define TIMER1_CAPB_SELECT_2    (1u << 6)                                   /*! capture B selector, bit 2 */
    #define TIMER1_CAPB_SELECT_3    (1u << 7)                                   /*! capture B selector, bit 3 */
FARPORTB(TIMER2_INPUT_SELECT,       0x284B)                                     /*! input selection register for timer 2 */
    #define TIMER2_CAPA_SELECT_0    (1u << 0)                                   /*! capture A selector, bit 0 */
    #define TIMER2_CAPA_SELECT_1    (1u << 1)                                   /*! capture A selector, bit 1 */
    #define TIMER2_CAPA_SELECT_2    (1u << 2)                                   /*! capture A selector, bit 2 */
    #define TIMER2_CAPA_SELECT_3    (1u << 3)                                   /*! capture A selector, bit 3 */
    #define TIMER2_CAPB_SELECT_0    (1u << 4)                                   /*! capture B selector, bit 0 */
    #define TIMER2_CAPB_SELECT_1    (1u << 5)                                   /*! capture B selector, bit 1 */
    #define TIMER2_CAPB_SELECT_2    (1u << 6)                                   /*! capture B selector, bit 2 */
    #define TIMER2_CAPB_SELECT_3    (1u << 7)                                   /*! capture B selector, bit 3 */
FARPORTB(DRV_DISABLE_CONFIG,         0x284C)                                    /*! configuration of the sources that can disable the driver */
    #define DRV_DIS_BY_VSUP_OV      (1u << 0)                                   /*! 1 - disable the driver when over Vsup voltage occurs */
    #define DRV_DIS_BY_VSUP_UV      (1u << 1)                                   /*! 1 - disable the driver when under Vsup voltage occurs */
    #define DRV_DIS_BY_VREG_UV      (1u << 2)                                   /*! 1 - disable the driver when under voltage of Vreg occurs */
    #define DRV_DIS_BY_VBOOST_UV    (1u << 3)                                   /*! 1 - disable the driver when Vboost under voltage occurs */
    #define DRV_DIS_BY_OVER_TEMP    (1u << 4)                                   /*! 1 - disable the driver when over temperature occurs */
    #define DRV_DIS_BY_VDS_ERROR    (1u << 5)                                   /*! 1 - disable the driver when VDS error occurs */
    #define DRV_DIS_BY_VGS_ERROR    (1u << 6)                                   /*! 1 - disable the driver when VGS error occurs */
    #define DRV_DIS_BY_OVER_CURRENT (1u << 7)                                   /*! 1 - disable the driver when overcurrent occurs */
FARPORTB(CP_DISABLE_CONFIG,         0x284D)                                     /*! configure the sources that can disable the chargepump */
    #define CP_DIS_BY_VSUP_OV       (1u << 0)                                   /*! 1 - disable the charge pump when over Vsup voltage occurs */
FARPORTB(HS_OS_TRIM,                0x284E)                                     /*! */
    #define HS_OS_TRIM_0            (1u << 0)                                   /*! */
    #define HS_OS_TRIM_1            (1u << 1)                                   /*! */
    #define HS_OS_TRIM_2            (1u << 2)                                   /*! */
    #define HS_OS_TRIM_3            (1u << 3)                                   /*! */
    #define HS_OS_TRIM_4            (1u << 4)                                   /*! */
    
/* ANA_OUTA register bit definitions */
    #define SREF_TRIM_0             (1u << 0)                                   /*! trimming of the SREF, bit 0 */
    #define SREF_TRIM_1             (1u << 1)                                   /*! trimming of the SREF, bit 1 */
    #define SREF_TRIM_2             (1u << 2)                                   /*! trimming of the SREF, bit 2 */
    #define SREF_TRIM_3             (1u << 3)                                   /*! trimming of the SREF, bit 3 */
    #define SREF_TRIM_4             (1u << 4)                                   /*! trimming of the SREF, bit 4 */
    #define SREF_TRIM_5             (1u << 5)                                   /*! trimming of the SREF, bit 5 */
    #define SREF_TRIM_6             (1u << 6)                                   /*! trimming of the SREF, bit 6 */
    #define RCO_0                   (1u << 8)                                   /*! RCO trimming bits, bit 0 */
    #define RCO_1                   (1u << 9)                                   /*! RCO trimming bits, bit 1 */
    #define RCO_2                   (1u << 10)                                  /*! RCO trimming bits, bit 2 */
    #define RCO_3                   (1u << 11)                                  /*! RCO trimming bits, bit 3 */
    #define RCO_4                   (1u << 12)                                  /*! RCO trimming bits, bit 4 */
    #define RCO_5                   (1u << 13)                                  /*! RCO trimming bits, bit 5 */
    #define RCO_6                   (1u << 14)                                  /*! RCO trimming bits, bit 6 */
    #define RCO_7                   (1u << 15)                                  /*! RCO trimming bits, bit 7 */

/* ANA_OUTB register bit definitions */
    #define V3V_SET_0               (1u << 0)                                   /*! V3V trimming bits, bit 0 */
    #define V3V_SET_1               (1u << 1)                                   /*! V3V trimming bits, bit 1 */
    #define V3V_SET_2               (1u << 2)                                   /*! V3V trimming bits, bit 2 */
    #define V3V_SET_3               (1u << 3)                                   /*! V3V trimming bits, bit 3 */
    #define BGT_0                   (1u << 4)                                   /*! bandgap trimming, bit 0 */
    #define BGT_1                   (1u << 5)                                   /*! bandgap trimming, bit 1 */
    #define BGT_2                   (1u << 6)                                   /*! bandgap trimming, bit 2 */
    #define IREF_0                  (1u << 7)                                   /*! Iref trimming bits, bit 0 */
    #define IREF_1                  (1u << 8)                                   /*! Iref trimming bits, bit 1 */
    #define IREF_2                  (1u << 9)                                   /*! Iref trimming bits, bit 2 */
    #define IREF_3                  (1u << 10)                                  /*! Iref trimming bits, bit 3 */

/* ANA_OUTC register bit definitions */
    #define VREG_SET_0              (1u << 0)                                   /*! select Vreg output levels, bit 0 */
    #define VREG_SET_1              (1u << 1)                                   /*! select Vreg output levels, bit 1 */
    #define VREG_SET_2              (1u << 2)                                   /*! select Vreg output levels, bit 2 */
    #define UV_SET_0                (1u << 3)                                   /*! set the UV level, bit 0 */
    #define UV_SET_1                (1u << 4)                                   /*! set the UV level, bit 1 */
    #define VDS_REF_0               (1u << 5)                                   /*! VDS trimming for the analog, bit 0 */
    #define VDS_REF_1               (1u << 6)                                   /*! VDS trimming for the analog, bit 1 */
    #define VDS_REF_2               (1u << 7)                                   /*! VDS trimming for the analog, bit 2 */
    #define OVERCURRENT_LIMIT_0     (1u << 8)                                   /*! DAC level, bit 0 */
    #define OVERCURRENT_LIMIT_1     (1u << 9)                                   /*! DAC level, bit 1 */
    #define OVERCURRENT_LIMIT_2     (1u << 10)                                  /*! DAC level, bit 2 */
    #define OVERCURRENT_LIMIT_3     (1u << 11)                                  /*! DAC level, bit 3 */
    #define ISENSE_GAIN_0           (1u << 14)                                  /*! gain of the second amplifier in the current sensor */
    #define ISENSE_GAIN_1           (1u << 15)                                  /*! gain of the second amplifier in the current sensor */

/* ANALOG_TEST register bit definitions */
    #define ATM_CODE_0              (1u << 0)                                   /*! ATM code, bit 0 */
    #define ATM_CODE_1              (1u << 1)                                   /*! ATM code, bit 1 */
    #define ATM_CODE_2              (1u << 2)                                   /*! ATM code, bit 2 */
    #define ATM_CODE_3              (1u << 3)                                   /*! ATM code, bit 3 */
    #define ATM_CODE_4              (1u << 4)                                   /*! ATM code, bit 4 */
    #define ATM_CODE_5              (1u << 5)                                   /*! ATM code, bit 5 */
    #define ATM_CONFIG_0            (1u << 6)                                   /*! ATM config, bit 0 */
    #define ATM_CONFIG_1            (1u << 7)                                   /*! ATM config, bit 1 */
    #define ATM_CONFIG_2            (1u << 8)                                   /*! ATM config, bit 2 */
    #define ATM_CONFIG_3            (1u << 9)                                   /*! ATM config, bit 3 */
    #define ATM_CONFIG_4            (1u << 10)                                  /*! ATM config, bit 4 */
    #define ATM_CONFIG_5            (1u << 11)                                  /*! ATM config, bit 5 */

#endif /* IOPORTS_H_ */
