/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef LIB_MLX8110X_HV_H_
#define LIB_MLX8110X_HV_H_

#include "ioports.h"

/*
 * Enable/disable interrupts from HV unit
 */
#define __HV_INT_ENABLE(n)     \
    do  {                       \
        XI4_PEND  = 1u << (n);  \
        XI4_MASK |= 1u << (n);  \
    } while(0)

#define __HV_INT_DISABLE(n)    \
    ( XI4_MASK &= ~(1u << (n)) )


/* HVIRFB .. Register: HV_CFG .. Bit 0 - 3
 * value = 1 falling edge interrupt
 * value = 0 rising edge interrupt
 */
#define HVIRFB  (uint16)~(0xFu << 0)
#define HVIRFB0 (1u << 0)
#define HVIRFB1 (1u << 1)
#define HVIRFB2 (1u << 2)
#define HVIRFB3 (1u << 3)
   
/* IBIAS_SEL .. Register: ANA_OUTG .. Bit 2 */
#define IBIAS_SEL      (1u << 2)   
/* DISABLE_PULLUP .. Register: ANA_OUTG .. Bit 1 */
#define DISABLE_PULLUP (1u << 1)
                 
/* HVCFG .. Register: HV_CFG .. Bit 8-11 -> deprecated
 * use HVTMREN instead -> lib_mlx8110x_timer.h
 */
#define HVCFG_CLEAR  (uint16)~(0xFu << 8)
#define HVCFG0                (1u << 8)
#define HVCFG1                (1u << 9)
#define HVCFG2                (1u << 10)
#define HVCFG3                (1u << 11)

/* HV0DEB .. Register: HV_DEB .. Bit 0-1 */
#define HV0DEB_CLEAR (uint16)~(3u << 0)
#define HV0DEB_OFF            (0u << 0)
#define HV0DEB_1MS            (1u << 0)
#define HV0DEB_4MS            (2u << 0)
#define HV0DEB_8MS            (3u << 0)

/* HV0CURRENT .. Register: ANA_OUTF .. Bit 0-3 */
#define HV0CURRENT_CLEAR (uint16)~(0xFu << 0)
#define HV0CURRENT_ODMODE     (0u  << 0)
#define HV0CURRENT_3mA        (8u  << 0)
#define HV0CURRENT_6mA        (9u  << 0)
#define HV0CURRENT_9mA        (10u << 0)
#define HV0CURRENT_12mA       (11u << 0)
#define HV0CURRENT_15mA       (12u << 0)
#define HV0CURRENT_18mA       (13u << 0)
#define HV0CURRENT_21mA       (14u << 0)
#define HV0CURRENT_24mA       (15u << 0)
#define HV0CURRENT_27mA       (6u  << 0)
#define HV0CURRENT_30mA       (7u  << 0)

/* HV1DEB .. Register: HV_DEB .. Bit 2-3 */
#define HV1DEB_CLEAR (uint16)~(3u << 2)
#define HV1DEB_OFF            (0u << 2)
#define HV1DEB_1MS            (1u << 2)
#define HV1DEB_4MS            (2u << 2)
#define HV1DEB_8MS            (3u << 2)

/* HV1CURRENT .. Register: ANA_OUTF .. Bit 4-7 */
#define HV1CURRENT_CLEAR (uint16)~(0xFu << 4)
#define HV1CURRENT_ODMODE     (0u  << 4)
#define HV1CURRENT_3mA        (8u  << 4)
#define HV1CURRENT_6mA        (9u  << 4)
#define HV1CURRENT_9mA        (10u << 4)
#define HV1CURRENT_12mA       (11u << 4)
#define HV1CURRENT_15mA       (12u << 4)
#define HV1CURRENT_18mA       (13u << 4)
#define HV1CURRENT_21mA       (14u << 4)
#define HV1CURRENT_24mA       (15u << 4)
#define HV1CURRENT_27mA       (6u  << 4)
#define HV1CURRENT_30mA       (7u  << 4)

/* HV2DEB .. Register: HV_DEB .. Bit 4-5 */
#define HV2DEB_CLEAR (uint16)~(3u << 4)
#define HV2DEB_OFF            (0u << 4)
#define HV2DEB_1MS            (1u << 4)
#define HV2DEB_4MS            (2u << 4)
#define HV2DEB_8MS            (3u << 4)

/* HV2CURRENT .. Register: ANA_OUTF .. Bit 8-11 */
#define HV2CURRENT_CLEAR (uint16)~(0xFu << 8)
#define HV2CURRENT_ODMODE     (0u  << 8)
#define HV2CURRENT_3mA        (8u  << 8)
#define HV2CURRENT_6mA        (9u  << 8)
#define HV2CURRENT_9mA        (10u << 8)
#define HV2CURRENT_12mA       (11u << 8)
#define HV2CURRENT_15mA       (12u << 8)
#define HV2CURRENT_18mA       (13u << 8)
#define HV2CURRENT_21mA       (14u << 8)
#define HV2CURRENT_24mA       (15u << 8)
#define HV2CURRENT_27mA       (6u  << 8)
#define HV2CURRENT_30mA       (7u  << 8)

/* HV3DEB .. Register: HV_DEB .. Bit 6-7 */
#define HV3DEB_CLEAR (uint16)~(3u << 6)
#define HV3DEB_OFF            (0u << 6)
#define HV3DEB_1MS            (1u << 6)
#define HV3DEB_4MS            (2u << 6)
#define HV3DEB_8MS            (3u << 6)

/* HV3CURRENT .. Register: ANA_OUTF .. Bit 12-15 */
#define HV3CURRENT_CLEAR (uint16)~(0xFu << 12)
#define HV3CURRENT_ODMODE     (0u  << 12)
#define HV3CURRENT_3mA        (8u  << 12)
#define HV3CURRENT_6mA        (9u  << 12)
#define HV3CURRENT_9mA        (10u << 12)
#define HV3CURRENT_12mA       (11u << 12)
#define HV3CURRENT_15mA       (12u << 12)
#define HV3CURRENT_18mA       (13u << 12)
#define HV3CURRENT_21mA       (14u << 12)
#define HV3CURRENT_24mA       (15u << 12)
#define HV3CURRENT_27mA       (6u  << 12)
#define HV3CURRENT_30mA       (7u  << 12)

/* HVOD .. Register: HV_OUTOD .. Bit 0-3 */
#define HVOD0 (1u << 0)
#define HVOD1 (1u << 1)
#define HVOD2 (1u << 2)
#define HVOD3 (1u << 3)

/* HVENWU .. Register: HV_ENWU .. Bit 0-3 */
#define HVENWU0 (1u << 0)
#define HVENWU1 (1u << 1)
#define HVENWU2 (1u << 2)
#define HVENWU3 (1u << 3)

/* HVIN .. Register: HV_IN .. Bit 0-3 */
#define HVIN0 (1u << 0)
#define HVIN1 (1u << 1)
#define HVIN2 (1u << 2)
#define HVIN3 (1u << 3)

/* HVINEN .. Register: HV_INEN .. Bit 0-3 */
#define HVINEN0 (1u << 0)
#define HVINEN1 (1u << 1)
#define HVINEN2 (1u << 2)
#define HVINEN3 (1u << 3)

/* HVINEN .. Register: HV_INEN .. Bit 8-11 */
#define HVENFCM0 (1u << 8)
#define HVENFCM1 (1u << 9)
#define HVENFCM2 (1u << 10)
#define HVENFCM3 (1u << 11)

/* HVDIVSEL .. Register: HV_INEN .. Bit 12-15 */
#define HVDIVSEL0 (1u << 12)
#define HVDIVSEL1 (1u << 13)
#define HVDIVSEL2 (1u << 14)
#define HVDIVSEL3 (1u << 15)

/* HVINT .. Register: ANA_INA .. Bit 8-11 */
#define HVINT0 (1u << 8)
#define HVINT1 (1u << 9)
#define HVINT2 (1u << 10)
#define HVINT3 (1u << 11)


/** HV0,1,2 disable internal pull-up resistors
 * @param
 * @return void
 * Register: ANA_OUTG, Bit DISABLE_PULLUP, Bit positions [1]
 */
#define HV0_1_2_DISABLE_PULLUP() ANA_OUTG |= (DISABLE_PULLUP)

/** HV0,1,2 enable internal pull-up resistors
 * @param
 * @return void
 * Register: ANA_OUTG, Bit DISABLE_PULLUP, Bit positions [1]
 */
#define HV0_1_2_ENABLE_PULLUP() ANA_OUTG &= (uint16)~(DISABLE_PULLUP)

/** HV0,1,2,3 disable double current output
 * @param
 * @return void
 * Register: ANA_OUTG, Bit IBIAS_SEL, Bit positions [2]
 */
#define HVx_DISABLE_DOUBLE_CURRENT() ANA_OUTG &= (uint16)~(IBIAS_SEL)

/** HV0,1,2,3 enable double current output
 * @param
 * @return void
 * Register: ANA_OUTG, Bit IBIAS_SEL, Bit positions [2]
 */
#define HVx_ENABLE_DOUBLE_CURRENT() ANA_OUTG |= (IBIAS_SEL)

/** HV0 enable open drain
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVOD0, Bit positions [0]
 */
#define HV0_SET_OD() HV_OUTOD |= HVOD0

/** HV0 set open drain
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVOD0, Bit positions [0]
 */
#define HV0_RESET_OD() HV_OUTOD &= (uint16)~(HVOD0)

/** Toggle opendrain from HV0
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVOD0, Bit positions [0]
 */
#define HV0_TOGGLE_OD() HV_OUTOD ^= (uint16)HVOD0

/** HV0 set current
 * @param
 * @return void
 * Register: ANA_OUTF, RBG current selection HV0, Bit positions [3:0]
 */
#define HV0_SET_CURRENTSOURCE(HV0CURRENT)                              \
do {                                                                   \
       ANA_OUTF = ((ANA_OUTF & HV0CURRENT_CLEAR) | (HV0CURRENT));      \
	} while (0)

/** HV0 enable wakeup
 * @param
 * @return void
 * Register: HV_ENWU, Bit HVENWU0, Bit positions [0]
 */
#define HV0_ENABLE_WU() HV_ENWU |= HVENWU0

/** HV0 disable wakeup
 * @param
 * @return void
 * Register: HV_ENWU, Bit HVENWU0, Bit positions [0]
 */
#define HV0_DISABLE_WU() HV_ENWU &= (uint16)~(HVENWU0)


/** HV0 enable falling edge
 * @param
 * @return void
 * Register: HV_CFG, Bit HVIRFB0, Bit positions [0]
 */
#define HV0_ENABLE_FALLING_EDGE() HV_CFG |= HVIRFB0

/** HV0 enable rising edge
 * @param
 * @return void
 * Register: HVPULL, Bit HVIRFB0, Bit positions [0]
 */
#define HV0_ENABLE_RISING_EDGE() HV_CFG &= (uint16)~(HVIRFB0)

/** set HV0 debouncing time
 * @param
 * @return void
 * Register: HV_DEB, Bit HV0DEB, Bit positions [1:0]
 */
#define HV0_SET_DEBTIME(HV0DEB_TIME)                       \
do {                                                       \
       HV_DEB = ((HV_DEB & HV0DEB_CLEAR) | (HV0DEB_TIME)); \
	} while (0)
	
/** HV0 enable comparator
 * @param
 * @return void
 * Register: HV_INEN, Bit HVINEN0, Bit positions [0]
 */
#define HV0_ENABLE_COMP() HV_INEN |= HVINEN0

/** HV0 disable comparator
 * @param
 * @return void
 * Register: HV_INEN, Bit HVINEN0, Bit positions [0]
 */
#define HV0_DISABLE_COMP() HV_INEN &= (uint16)~(HVINEN0)

/** HV0 get input state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: HV_IN, Bit HVIN0, Bit positions [0]
 */
#define HV0_GET_STATE() ((HV_IN & HVIN0) >> 0)

/** HV0 get interrupt state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit HVINT0, Bit positions [8]
 */
#define HV0_GET_INT_STATE() ((ANA_INA & HVINT0) >> 8)

/**************/
/** HV1 enable open drain
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVOD1, Bit positions [1]
 */
#define HV1_SET_OD() HV_OUTOD |= HVOD1

/** HV1 set open drain
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVOD1, Bit positions [1]
 */
#define HV1_RESET_OD() HV_OUTOD &= (uint16)~(HVOD1)

/** Toggle opendrain from HV1
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVOD1, Bit positions [1]
 */
#define HV1_TOGGLE_OD() HV_OUTOD ^= (uint16)HVOD1

/** HV1 set current
 * @param
 * @return void
 * Register: ANA_OUTF, RBG current selection HV1, Bit positions [7:4]
 */
#define HV1_SET_CURRENTSOURCE(HV1CURRENT)                              \
do {                                                                   \
       ANA_OUTF = ((ANA_OUTF & HV1CURRENT_CLEAR) | (HV1CURRENT));      \
	} while (0)

/** HV1 enable wakeup
 * @param
 * @return void
 * Register: HV_ENWU, Bit HVENWU1, Bit positions [1]
 */
#define HV1_ENABLE_WU() HV_ENWU |= HVENWU1

/** HV1 disable wakeup
 * @param
 * @return void
 * Register: HV_ENWU, Bit HVENWU1, Bit positions [1]
 */
#define HV1_DISABLE_WU() HV_ENWU &= (uint16)~(HVENWU1)

/** HV1 enable falling edge
 * @param
 * @return void
 * Register: HV_CFG, Bit HVIRFB1, Bit positions [1]
 */
#define HV1_ENABLE_FALLING_EDGE() HV_CFG |= HVIRFB1

/** HV1 enable rising edge
 * @param
 * @return void
 * Register: HVPULL, Bit HVIRFB1, Bit positions [1]
 */
#define HV1_ENABLE_RISING_EDGE() HV_CFG &= (uint16)~(HVIRFB1)

/** set HV1 debouncing time
 * @param
 * @return void
 * Register: HV_DEB, Bit HV1DEB, Bit positions [3:2]
 */
#define HV1_SET_DEBTIME(HV1DEB_TIME)                       \
do {                                                       \
       HV_DEB = ((HV_DEB & HV1DEB_CLEAR) | (HV1DEB_TIME)); \
	} while (0)

/** HV1 enable comparator
 * @param
 * @return void
 * Register: HV_INEN, Bit HVINEN1, Bit positions [1]
 */
#define HV1_ENABLE_COMP() HV_INEN |= HVINEN1

/** HV1 disable comparator
 * @param
 * @return void
 * Register: HV_INEN, Bit HVINEN1, Bit positions [1]
 */
#define HV1_DISABLE_COMP() HV_INEN &= (uint16)~(HVINEN1)

/** HV1 get input state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: HV_IN, Bit HVIN1, Bit positions [1]
 */
#define HV1_GET_STATE() ((HV_IN & HVIN1) >> 1)

/** HV1 get interrupt state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit HVINT1, Bit positions [9]
 */
#define HV1_GET_INT_STATE() ((ANA_INA & HVINT1) >> 9)

/**************/
/** HV2 enable open drain
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVOD2, Bit positions [2]
 */
#define HV2_SET_OD() HV_OUTOD |= HVOD2

/** HV2 set open drain
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVOD2, Bit positions [2]
 */
#define HV2_RESET_OD() HV_OUTOD &= (uint16)~(HVOD2)

/** Toggle opendrain from HV2
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVOD2, Bit positions [2]
 */
#define HV2_TOGGLE_OD() HV_OUTOD ^= (uint16)HVOD2

/** HV2 set current
 * @param
 * @return void
 * Register: ANA_OUTF, RBG current selection HV2, Bit positions [11:8]
 */
#define HV2_SET_CURRENTSOURCE(HV2CURRENT)                              \
do {                                                                   \
       ANA_OUTF = ((ANA_OUTF & HV2CURRENT_CLEAR) | (HV2CURRENT));      \
	} while (0)
	
/** HV2 enable wakeup
 * @param
 * @return void
 * Register: HV_ENWU, Bit HVENWU2, Bit positions [2]
 */
#define HV2_ENABLE_WU() HV_ENWU |= HVENWU2

/** HV2 disable wakeup
 * @param
 * @return void
 * Register: HV_ENWU, Bit HVENWU2, Bit positions [2]
 */
#define HV2_DISABLE_WU() HV_ENWU &= (uint16)~(HVENWU2)

/** HV2 enable falling edge
 * @param
 * @return void
 * Register: HV_CFG, Bit HVIRFB2, Bit positions [2]
 */
#define HV2_ENABLE_FALLING_EDGE() HV_CFG |= HVIRFB2

/** HV2 enable rising edge
 * @param
 * @return void
 * Register: HVPULL, Bit HVIRFB2, Bit positions [2]
 */
#define HV2_ENABLE_RISING_EDGE() HV_CFG &= (uint16)~(HVIRFB2)

/** set HV2 debouncing time
 * @param
 * @return void
 * Register: HV_DEB, Bit HV2DEB, Bit positions [5:4]
 */
#define HV2_SET_DEBTIME(HV2DEB_TIME)                       \
do {                                                       \
       HV_DEB = ((HV_DEB & HV2DEB_CLEAR) | (HV2DEB_TIME)); \
	} while (0)

/** HV2 enable comparator
 * @param
 * @return void
 * Register: HV_INEN, Bit HVINEN2, Bit positions [2]
 */
#define HV2_ENABLE_COMP() HV_INEN |= HVINEN2

/** HV2 disable comparator
 * @param
 * @return void
 * Register: HV_INEN, Bit HVINEN2, Bit positions [2]
 */
#define HV2_DISABLE_COMP() HV_INEN &= (uint16)~(HVINEN2)

/** HV2 get input state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: HV_IN, Bit HVIN2, Bit positions [2]
 */
#define HV2_GET_STATE() ((HV_IN & HVIN2) >> 2)

/** HV2 get interrupt state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit HVINT2, Bit positions [10]
 */
#define HV2_GET_INT_STATE() ((ANA_INA & HVINT2) >> 10)

/**************/
/** HV3 enable open drain
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVOD3, Bit positions [3]
 */
#define HV3_SET_OD() HV_OUTOD |= HVOD3

/** HV3 set open drain
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVOD3, Bit positions [3]
 */
#define HV3_RESET_OD() HV_OUTOD &= (uint16)~(HVOD3)

/** Toggle opendrain from HV3
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVOD3, Bit positions [3]
 */
#define HV3_TOGGLE_OD() HV_OUTOD ^= (uint16)HVOD3

/** HV3 set current
 * @param
 * @return void
 * Register: ANA_OUTF, RBG current selection HV3, Bit positions [15:12]
 */
#define HV3_SET_CURRENTSOURCE(HV3CURRENT)                              \
do {                                                                   \
       ANA_OUTF = ((ANA_OUTF & HV3CURRENT_CLEAR) | (HV3CURRENT));      \
	} while (0)
	
/** HV3 enable wakeup
 * @param
 * @return void
 * Register: HV_ENWU, Bit HVENWU3, Bit positions [3]
 */
#define HV3_ENABLE_WU() HV_ENWU |= HVENWU3

/** HV3 disable wakeup
 * @param
 * @return void
 * Register: HV_ENWU, Bit HVENWU3, Bit positions [3]
 */
#define HV3_DISABLE_WU() HV_ENWU &= (uint16)~(HVENWU3)

/** HV3 enable falling edge
 * @param
 * @return void
 * Register: HV_CFG, Bit HVIRFB3, Bit positions [3]
 */
#define HV3_ENABLE_FALLING_EDGE() HV_CFG |= HVIRFB3

/** HV3 enable rising edge
 * @param
 * @return void
 * Register: HVPULL, Bit HVIRFB3, Bit positions [3]
 */
#define HV3_ENABLE_RISING_EDGE() HV_CFG &= (uint16)~(HVIRFB3)

/** set HV3 debouncing time
 * @param
 * @return void
 * Register: HV_DEB, Bit HV3DEB, Bit positions [7:6]
 */
#define HV3_SET_DEBTIME(HV3DEB_TIME)                       \
do {                                                       \
       HV_DEB = ((HV_DEB & HV3DEB_CLEAR) | (HV3DEB_TIME)); \
	} while (0)

/** HV3 enable comparator
 * @param
 * @return void
 * Register: HV_INEN, Bit HVINEN3, Bit positions [3]
 */
#define HV3_ENABLE_COMP() HV_INEN |= HVINEN3

/** HV3 disable comparator
 * @param
 * @return void
 * Register: HV_INEN, Bit HVINEN3, Bit positions [3]
 */
#define HV3_DISABLE_COMP() HV_INEN &= (uint16)~(HVINEN3)

/** HV3 get input state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: HV_IN, Bit HVIN3, Bit positions [3]
 */
#define HV3_GET_STATE() ((HV_IN & HVIN3) >> 3)

/** HV3 get interrupt state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit HVINT3, Bit positions [11]
 */
#define HV3_GET_INT_STATE() ((ANA_INA & HVINT3) >> 11)


#define HV0_INT_ENABLE()     __HV_INT_ENABLE(8)
#define HV0_INT_DISABLE()    __HV_INT_DISABLE(8)

#define HV1_INT_ENABLE()     __HV_INT_ENABLE(9)
#define HV1_INT_DISABLE()    __HV_INT_DISABLE(9)

#define HV2_INT_ENABLE()     __HV_INT_ENABLE(10)
#define HV2_INT_DISABLE()    __HV_INT_DISABLE(10)

#define HV3_INT_ENABLE()     __HV_INT_ENABLE(11)
#define HV3_INT_DISABLE()    __HV_INT_DISABLE(11)

/** Set current for HV0, HV1, HV2 and HV3
 * @param
 * @return void
 * Register: ANA_OUTF
 */
#define HV_SET_CURRENTSOURCES(HV0CURRENT,HV1CURRENT,HV2CURRENT,HV3CURRENT)      \
do {                                                                            \
       ANA_OUTF = ((HV0CURRENT) | (HV1CURRENT) | (HV2CURRENT) | (HV3CURRENT));  \
	} while (0)


#endif /* LIB_MLX8110X_HV_H_ */
