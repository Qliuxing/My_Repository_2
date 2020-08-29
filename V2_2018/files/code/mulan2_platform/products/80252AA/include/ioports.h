/*! \file IO.h
 *  \brief Definition of the IO registers and general purpose macros
 *  of the MLX80250 / MLX80251 / MLX80252
 *
 * \note		project MLX80250 / MLX80251 / MLX80252
 * 
 * \author 		Geert De Smedt
 *   
 * \date 		17 June 2009
 *   
 * \version 	0.1 - preliminary
 *
 * MELEXIS Microelectronic Integrated Systems
 *   
 * These are the definitions of the IO registers and general purpose macros 
 * of the MLX80250 / MLX80251 / MLX80252 
 *
 * Copyright (C) 2009-2010 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement. 
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


/*******************************************************************************
 *                  definition of the custom user ports                        *
 ******************************************************************************/
PORTW(DIAG_IRQ_STAT, 0x281A)
    #define OVER_CURRENT        0x0001
    #define VGS_ERROR           0x0002
    #define VDS_ERROR           0x0004
    #define OVER_TEMP           0x0008
    #define VBOOST_UV           0x0010
    #define VREG_UV             0x0020
    #define VSUP_UV             0x0040
    #define VSUP_OV             0x0080
    #define CURREG              0x0100
PORTB(DRV_CTRL, 0x281C)                                                         /*! control register for the driver outputs */
    #define PWM_START           0x01
    #define PWM_SPEED           0x02
    #define CURRENT_REG_EN      0x04
    #define STATE_TIMER_START   0x08
    #define ZC_TIMER_START      0x10
    #define ZC_EDGEDET_START    0x20
    #define NEXT_STATE_OPTION   0x40
PORTB(CP_CTRL, 0x281D)                                                          /*! Charge pump control register */
    #define CP_EN               0x01
    #define CP_DOUBLE           0x02
    #define CP_PARALLEL         0x04
    #define CP_SLOPEDIS         0x08
    #define CP_DISCHARGE        0x10
    #define CP_CLOCK_0          0x20
    #define CP_CLOCK_1          0x40
PORTW(DRV_CONFIG, 0x281E)                                                       /*! Configuration register for the drivers */
    #define MOTOR_CONTROL_CLOCK_0   0x0001
    #define MOTOR_CONTROL_CLOCK_1   0x0002
    #define MOTOR_CONTROL_CLOCK_2   0x0004
    #define ROTOR_POSITION_0        0x0008
    #define ROTOR_POSITION_1        0x0010
    #define DRV_DEADTIME_0          0x0020
    #define DRV_DEADTIME_1          0x0040
    #define DRV_DEADTIME_2          0x0080
	#define DRV_DEADTIME_3       	0x0100	
    #define VDS_MASKTIME_0          0x0200
    #define VDS_MASKTIME_1          0x0400
    #define VDS_MASKTIME_2          0x0800
    #define VDS_MASKTIME_3          0x1000

PORTW(ZC_CONFIG_STATUS, 0x2820)                                                 /*! zero crossing configuration register */
    #define ZC_MASK_SELECT_0        0x0001
    #define ZC_MASK_SELECT_1        0x0002
    #define ZC_MASK_HS_LSB          0x0004
    #define ZC_MASK_RIS_FALLB       0x0008
    #define ZC_MASK_TIME_0          0x0010
    #define ZC_MASK_TIME_1          0x0020
    #define ZC_MASK_TIME_2          0x0040
    #define ZC_MASK_TIME_3          0x0080
    #define ZC_INPUT_SELECT_0       0x0100
    #define ZC_INPUT_SELECT_1       0x0200
    #define ZC_RIS_FALLB            0x0400
    #define ZC_BLANKED              0x0800
    #define ZC_EDGE                 0x1000
PORTW(NEXT_MOTOR_STATE, 0x2822)                                                 /*! Next commutation state of the motor */
    #define NEXT_MOTOR_STATE_A_0    0x0001
    #define NEXT_MOTOR_STATE_A_1    0x0002
    #define NEXT_MOTOR_STATE_A_2    0x0004
    #define NEXT_MOTOR_STATE_A_3    0x0008
    #define NEXT_MOTOR_STATE_A_4    0x0010
    #define NEXT_MOTOR_STATE_B_0    0x0020
    #define NEXT_MOTOR_STATE_B_1    0x0040
    #define NEXT_MOTOR_STATE_B_2    0x0080
    #define NEXT_MOTOR_STATE_B_3    0x0100
    #define NEXT_MOTOR_STATE_B_4    0x0200
    #define NEXT_MOTOR_STATE_C_0    0x0400
    #define NEXT_MOTOR_STATE_C_1    0x0800
    #define NEXT_MOTOR_STATE_C_2    0x1000
    #define NEXT_MOTOR_STATE_C_3    0x2000
    #define NEXT_MOTOR_STATE_C_4    0x4000
PORTB(PWMA_COMP, 0x2824)                                                        /*! PWM A compare register */
PORTB(PWMB_COMP, 0x2825)                                                        /*! PWM B compare register */
PORTB(PWMC_COMP, 0x2826)                                                        /*! PWM C compare register */
PORTB(PWMI_COMP, 0x2827)                                                        /*! PWM I compare register */
PORTW(STATE_TIME, 0x2828)                                                       /*! State timer register */
PORTW(STATE_TIME_COMP, 0x282A)                                                  /*! State timer comparator register */
PORTW(ZC_BLANK_TIME_COMP, 0x282C)                                               /*! Zero corssing blanking timer comparator register */
PORTW(ZC_PERIOD, 0x282E)                                                        /*! Zero crossing period */
PORTB(CUR_REG_CTRL, 0x2830)                                                     /*! current regulation control register */
    #define NEXT_CURREG_MASK_SEL_0  0x01
    #define NEXT_CURREG_MASK_SEL_1  0x02
    #define CURREG_MASK_HS_LSB      0x04
    #define CURREG_MASK_RIS_FALLB   0x08
    #define CURREG_MASK_TIME_0      0x10    
    #define CURREG_MASK_TIME_1      0x20    
    #define CURREG_MASK_TIME_2      0x40    
    #define CURREG_MASK_TIME_3      0x80
PORTB(CUR_REG_DAC, 0x2831)                                                       /*! Current limiting DAC */
PORTW(EXTIO, 0x2832)
    #define EXTIO_IN_0              0x0001    
    #define EXTIO_IN_1              0x0002    
    #define EXTIO_IN_2              0x0004    
    #define EXTIO_IN_3              0x0008    
    #define EXTIO_IN_4              0x0010
    #define EXTIO_IN_5              0x0020
    #define EXTIO_OPEN_DRAIN_0      0x0100    
    #define EXTIO_OPEN_DRAIN_1      0x0200    
    #define EXTIO_OPEN_DRAIN_2      0x0400    
    #define EXTIO_OPEN_DRAIN_3      0x0800    
    #define EXTIO_OPEN_DRAIN_4      0x1000    
    #define EXTIO_OPEN_DRAIN_5      0x2000
    #define SET_IO1_VREG            0x4000
    #define SET_IO5_VREG            0x8000              
PORTB(PWM_IO_IRQ_MASK, 0x2834)                                                  /*! interupt masking of the external IOs and PWM */
    #define EXTIO_IRQ_MASK_0        0x01    
    #define EXTIO_IRQ_MASK_1        0x02    
    #define EXTIO_IRQ_MASK_2        0x04    
    #define EXTIO_IRQ_MASK_3        0x08    
    #define EXTIO_IRQ_MASK_4        0x10    
    #define EXTIO_IRQ_MASK_5        0x20
    #define PWMI_IRQ_MASK           0x40
    #define PWMA_IRQ_MASK           0x80
PORTB(PWM_IO_IRQ_STAT, 0x2835)                                                  /*! interrupt status flags of the external IOs and the PWM */
    #define EXTIO_IRQ_0             0x01    
    #define EXTIO_IRQ_1             0x02    
    #define EXTIO_IRQ_2             0x04    
    #define EXTIO_IRQ_3             0x08    
    #define EXTIO_IRQ_4             0x10    
    #define EXTIO_IRQ_5             0x20
    #define PWMI_IRQ                0x40
    #define PWMA_IRQ                0x80
PORTW(SPI_TIMERS_IRQ_STAT, 0x2836)                                              /*! interrupt status flags of the SPI and the timers */
    #define SPI_TR_FRAME_IRQ        0x0001
    #define SPI_REC_FRAME_IRQ       0x0002
    #define SPI_OVF_IRQ             0x0004
    #define TIMER1_IRQ_1            0x0008     
    #define TIMER1_IRQ_2            0x0010     
    #define TIMER1_IRQ_3            0x0020     
    #define TIMER1_IRQ_4            0x0040     
    #define TIMER1_IRQ_5            0x0080
    #define TIMER2_IRQ_1            0x0100     
    #define TIMER2_IRQ_2            0x0200     
    #define TIMER2_IRQ_3            0x0400     
    #define TIMER2_IRQ_4            0x0800     
    #define TIMER2_IRQ_5            0x1000
PORTW(SPI_DATA, 0x2838)                                                         /*! spi data register */
PORTW(TIMER1_REGA, 0x283A)                                                      /*! first register of timer 1 */
PORTW(TIMER1_REGB, 0x283C)                                                      /*! second register of timer 1 */
PORTW(TIMER1_CTRL, 0x283E)                                                      /*! control register of timer 1 */
    #define TIMER1_1MHZ_16MHZ_B     0x0001     
    #define TIMER1_START_BIT        0x0002     
    #define TIMER1_EDGEA_0          0x0004     
    #define TIMER1_EDGEA_1          0x0008     
    #define TIMER1_EDGEB_0          0x0010     
    #define TIMER1_EDGEB_1          0x0020     
    #define TIMER1_DIN_0            0x0040     
    #define TIMER1_DIN_1            0x0080     
    #define TIMER1_OVRA             0x0100     
    #define TIMER1_OVRB             0x0200     
    #define TIMER1_ENCMP            0x0400     
    #define TIMER1_MODE_0           0x0800     
    #define TIMER1_MODE_1           0x1000     
    #define TIMER1_MODE_2           0x2000
    #define TIMER1_DIV_0            0x4000
    #define TIMER1_DIV_1            0x8000
FARPORTW(TIMER1_CNT, 0x2840)                                                    /*! timer1 counter register */
FARPORTW(TIMER2_REGA, 0x2842)                                                   /*! first register of timer 2 */
FARPORTW(TIMER2_REGB, 0x2844)                                                   /*! second register of timer 2 */
FARPORTW(TIMER2_CTRL, 0x2846)                                                   /*! control register of timer 2 */
    #define TIMER2_1MHZ_16MHZ_B     0x0001     
    #define TIMER2_START_BIT        0x0002     
    #define TIMER2_EDGEA_0          0x0004     
    #define TIMER2_EDGEA_1          0x0008     
    #define TIMER2_EDGEB_0          0x0010     
    #define TIMER2_EDGEB_1          0x0020     
    #define TIMER2_DIN_0            0x0040     
    #define TIMER2_DIN_1            0x0080     
    #define TIMER2_OVRA             0x0100     
    #define TIMER2_OVRB             0x0200     
    #define TIMER2_ENCMP            0x0400     
    #define TIMER2_MODE_0           0x0800     
    #define TIMER2_MODE_1           0x1000     
    #define TIMER2_MODE_2           0x2000
    #define TIMER2_DIV_0            0x4000
    #define TIMER2_DIV_1            0x8000
FARPORTW(TIMER2_CNT, 0x2848)                                                    /*! timer 2 counter register */
FARPORTW(DIAGNOSTIC_MASK, 0x284A)                                               /*! mask regiser of the diagnostics IRQ: 1 = IRQ enabled, 0 = IRQ disabled */
    #define OVER_CURRENT_IRQ_MASK   0x0001 
    #define VGS_ERROR_IRQ_MASK      0x0002 
    #define VDS_ERROR_IRQ_MASK      0x0004
    #define OVER_TEMP_IRQ_MASK      0x0008
    #define VBOOST_UV_IRQ_MASK      0x0010
    #define VREG_UV_IRQ_MASK        0x0020
    #define VSUP_UV_IRQ_MASK        0x0040
    #define VSUP_OV_IRQ_MASK        0x0080
    #define CURREG_IRQ_MASK         0x0100
FARPORTW(SPI_TIMERS_IRQ_MASK, 0x284C)                                           /*! mask regiser of the timers and SPI interrupt: 1 = IRQ enabled, 0 = IRQ disabled */
    #define SPI_TR_FRAME_IRQ_MASK   0x0001
    #define SPI_REC_FRAME_IRQ_MASK  0x0002
    #define SPI_OVF_IRQ_MASK        0x0004
    #define TIMER1_IRQ_MASK_1       0x0008      
    #define TIMER1_IRQ_MASK_2       0x0010
    #define TIMER1_IRQ_MASK_3       0x0020
    #define TIMER1_IRQ_MASK_4       0x0040
    #define TIMER1_IRQ_MASK_5       0x0080
    #define TIMER2_IRQ_MASK_1       0x0100      
    #define TIMER2_IRQ_MASK_2       0x0200
    #define TIMER2_IRQ_MASK_3       0x0400
    #define TIMER2_IRQ_MASK_4       0x0800
    #define TIMER2_IRQ_MASK_5       0x1000
FARPORTW(SEL_IO_CONFIG, 0x284E)                                                 /*! IO pin selector register */
    #define SEL_IO_0_0              0x0001
    #define SEL_IO_0_1              0x0002
    #define SEL_IO_1_0              0x0004
    #define SEL_IO_1_1              0x0008
    #define SEL_IO_2_0              0x0010
    #define SEL_IO_2_1              0x0020
    #define SEL_IO_3_0              0x0040
    #define SEL_IO_3_1              0x0080
    #define SEL_IO_4_0              0x0100
    #define SEL_IO_4_1              0x0200
    #define SEL_IO_5_0              0x0400
    #define SEL_IO_5_1              0x0800
    #define DIS_APD                 0x1000
    #define DIS_EXTIO2_ERROR        0x2000
    #define ANA_IN_IO0_ON           0x4000
	#define PULLUP_IO0				0x8000
FARPORTB(EXT_WD_CTRL, 0x2850)                                                   /*! external watchdog register */
    #define EXT_WD_EN               0x01
    #define EXT_WD_ACK              0x02
	#define EXT_WD_EN_WE			0x04
FARPORTB(MIXED, 0x2851)
	#define	DIS_DIAG_IRQ			0x01
	#define SPI_EN					0x02
	#define SPI_CLK_R_FB			0x04
	#define BYTEMODE				0x08
	#define ADC_SPEED				0x10
	#define ADC_COMP_ON				0x20
	#define ISENSE_ZERO				0x40
FARPORTB(TIMER1_INPUT_SELECT, 0x2852)                                           /*! input selector for timer 1 */
    #define TIMER1_CAPA_SELECT_0    0x01
    #define TIMER1_CAPA_SELECT_1    0x02
    #define TIMER1_CAPA_SELECT_2    0x04
    #define TIMER1_CAPA_SELECT_3    0x08
    #define TIMER1_CAPB_SELECT_0    0x10
    #define TIMER1_CAPB_SELECT_1    0x20
    #define TIMER1_CAPB_SELECT_2    0x40
    #define TIMER1_CAPB_SELECT_3    0x80
FARPORTB(TIMER2_INPUT_SELECT, 0x2853)                                           /*! input selector for timer 2 */
    #define TIMER2_CAPA_SELECT_0    0x01
    #define TIMER2_CAPA_SELECT_1    0x02
    #define TIMER2_CAPA_SELECT_2    0x04
    #define TIMER2_CAPA_SELECT_3    0x08
    #define TIMER2_CAPB_SELECT_0    0x10
    #define TIMER2_CAPB_SELECT_1    0x20
    #define TIMER2_CAPB_SELECT_2    0x40
    #define TIMER2_CAPB_SELECT_3    0x80

/*******************************************************************************
 *                 definition of the custom system ports                       *
 ******************************************************************************/
FARPORTB(EXTMEM_CTRL, 0x202A)
    #define WAIT_CYCLE_CONFIG_0 0x01 
    #define WAIT_CYCLE_CONFIG_1 0x02
    #define MEMORY_EXTENSION    0x04
    #define FLASH_OTP_B         0x08
FARPORTB(EXMEM_DED_STATUS, 0x202B)
    #define EXTMEM_DED          0x01
FARPORTW(ANALOG_TEST, 0x202C)
FARPORTW(ANA_DIG_TEST, 0x202E)
FARPORTW(DIG_TEST_A, 0x2030)
FARPORTW(DIG_TEST_B, 0x2032)
FARPORTW(OTP_TM, 0x2034)

/*******************************************************************************
 *                 definition of the bits in the ANA_OUTx registers            *
 ******************************************************************************/
/* ANA_OUTA */
#define SREF_TRIM_0             0x0001
#define SREF_TRIM_1             0x0002
#define SREF_TRIM_2             0x0004
#define SREF_TRIM_3             0x0008
#define SREF_TRIM_4             0x0010
#define SREF_TRIM_5             0x0020
#define SREF_TRIM_6             0x0040
#define RCO_0                   0x0100
#define RCO_1                   0x0200
#define RCO_2                   0x0400
#define RCO_3                   0x0800
#define RCO_4                   0x1000
#define RCO_5                   0x2000
#define RCO_6                   0x4000

/* ANA_OUTB */
#define V3V_SET_0               0x0001
#define V3V_SET_1               0x0002
#define V3V_SET_2               0x0004
#define V3V_SET_3               0x0008
#define BGT_0                   0x0010
#define BGT_1                   0x0020
#define BGT_2                   0x0040
#define IREF_0                  0x0080
#define IREF_1                  0x0100
#define IREF_2                  0x0200
#define IREF_3                  0x0400

/* ANA_OUTC */
#define VREG_SET_0              0x0001
#define VREG_SET_1              0x0002
#define VREG_SET_2              0x0004
#define UV_SET_0                0x0008
#define UV_SET_1                0x0010
#define OVERCURRENT_LIMIT_0     0x0100
#define OVERCURRENT_LIMIT_1     0x0200
#define OVERCURRENT_LIMIT_2     0x0400
#define OVERCURRENT_LIMIT_3     0x0800
#define ISENSE_GAIN_0           0x1000
#define ISENSE_GAIN_1           0x2000
#define VDS_REF_0               0x4000
#define VDS_REF_1               0x8000

#endif /* IOPORTS_H_ */
