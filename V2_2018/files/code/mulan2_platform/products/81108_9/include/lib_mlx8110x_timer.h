/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef LIB_MLX8110X_TIMER_H_
#define LIB_MLX8110X_TIMER_H_

#include "ioports.h"

/* T1_INA_LV .. Register: LV_TMR .. Bit 0-7 */                                
#define T1_INA_LV0    (uint16) ~(1u << 0)
#define T1_INA_LV1    (uint16) ~(1u << 1)
#define T1_INA_LV2    (uint16) ~(1u << 2)
#define T1_INA_LV3    (uint16) ~(1u << 3)
#define T1_INA_LV4    (uint16) ~(1u << 4)
#define T1_INA_LV5    (uint16) ~(1u << 5)
#define T1_INA_LV6    (uint16) ~(1u << 6)
#define T1_INA_LV7    (uint16) ~(1u << 7)

/* T1_INB_LV .. Register: LV_TMR .. Bit 0-7 */
#define T1_INB_LV0              (1u << 0)
#define T1_INB_LV1              (1u << 1)
#define T1_INB_LV2              (1u << 2)
#define T1_INB_LV3              (1u << 3)
#define T1_INB_LV4              (1u << 4)
#define T1_INB_LV5              (1u << 5)
#define T1_INB_LV6              (1u << 6)
#define T1_INB_LV7              (1u << 7)

/* LVTMREN .. Register: LV_CFG .. Bit 8-15 */
#define LVTMREN_CLEAR (uint16) ~(0xFFu << 8)
#define LVTMREN_LV0             (1u << 8)
#define LVTMREN_LV1             (1u << 9)
#define LVTMREN_LV2             (1u << 10)
#define LVTMREN_LV3             (1u << 11)
#define LVTMREN_LV4             (1u << 12)
#define LVTMREN_LV5             (1u << 13)
#define LVTMREN_LV6             (1u << 14)
#define LVTMREN_LV7             (1u << 15)

/* T1_INA_HV .. Register: HV_TMR .. Bit 0-3 */                                    
#define T1_INA_HV0    (uint16) ~(1u << 0)
#define T1_INA_HV1    (uint16) ~(1u << 1)
#define T1_INA_HV2    (uint16) ~(1u << 2)
#define T1_INA_HV3    (uint16) ~(1u << 3)

/* T1_INB_HV .. Register: HV_TMR .. Bit 0-3 */     
#define T1_INB_HV0              (1u << 0)
#define T1_INB_HV1              (1u << 1)
#define T1_INB_HV2              (1u << 2)
#define T1_INB_HV3              (1u << 3)

/* HVTMREN .. Register: HV_CFG .. Bit 8-11 */
#define HVTMREN_CLEAR (uint16) ~(0xFu << 8)
#define HVTMREN_HV0             (1u << 8)
#define HVTMREN_HV1             (1u << 9)
#define HVTMREN_HV2             (1u << 10)
#define HVTMREN_HV3             (1u << 11)


/** Enable Timer1 input A for chosen input
 * @param
 * @return void
 * Register: LV_TMR, Bit LV_TMR, Bit positions [7:0]
 */
#define TIMER1_SET_LV_INA(T1_INA_CHANNEL)                      \
do {                                                           \
		LV_TMR = (LV_TMR & T1_INA_CHANNEL);                      \
	} while (0)
	
/** Enable LV for Timer1 input
 * @param
 * @return void
 * Register: LV_CFG, Bit LVTMREN_CLEAR, Bit positions [15:8]
 */
#define TIMER1_ROUTING_LV(LVTMREN_)                            \
do {                                                           \
		LV_CFG = ((LV_CFG & LVTMREN_CLEAR) | (LVTMREN_));        \
	} while (0)	

/** Enable Timer1 input B for chosen input
 * @param
 * @return void
 * Register: LV_TMR, Bit LV_TMR, Bit positions [7:0]
 */
#define TIMER1_SET_LV_INB(T1_INB_CHANNEL)                      \
do {                                                           \
		LV_TMR = (LV_TMR | T1_INB_CHANNEL);                      \
	} while (0)
	
/** Enable Timer1 input A for chosen input
 * @param
 * @return void
 * Register: HV_TMR, Bit HV_TMR, Bit positions [3:0]
 */
#define TIMER1_SET_HV_INA(T1_INA_CHANNEL)                      \
do {                                                           \
		HV_TMR = (HV_TMR & T1_INA_CHANNEL);                      \
	} while (0)
	
/** Enable HV for Timer1 input
 * @param
 * @return void
 * Register: HV_CFG, Bit HVTMREN_CLEAR, Bit positions [11:8]
 */
#define TIMER1_ROUTING_HV(HVTMREN_)                            \
do {                                                           \
		HV_CFG = ((HV_CFG & HVTMREN_CLEAR) | (HVTMREN_));        \
	} while (0)	

/** Enable Timer1 input B for chosen input
 * @param
 * @return void
 * Register: HV_TMR, Bit HV_TMR, Bit positions [3:0]
 */
#define TIMER1_SET_HV_INB(T1_INB_CHANNEL)                      \
do {                                                           \
		HV_TMR = (HV_TMR | T1_INB_CHANNEL);                      \
	} while (0)


#endif /* LIB_MLX8110X_TIMER_H_ */
