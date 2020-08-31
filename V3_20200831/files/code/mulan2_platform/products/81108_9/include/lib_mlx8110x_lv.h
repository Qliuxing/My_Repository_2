/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef LIB_MLX8110X_LV_H_
#define LIB_MLX8110X_LV_H_

#include "ioports.h"

/*
 * Enable/disable interrupts from LV unit
 */
#define __LV_INT_ENABLE(n)     \
    do  {                       \
        XI4_PEND  = 1u << (n);  \
        XI4_MASK |= 1u << (n);  \
    } while(0)

#define __LV_INT_DISABLE(n)    \
    ( XI4_MASK &= ~(1u << (n)) )


/* LVIRFB .. Register: LV_CFG .. Bit 0 - 7
 * value = 1 falling edge interrupt
 * value = 0 rising edge interrupt
 */
#define LVIRFB_CLEAR  (uint16)~(0xFFu << 0)
#define LVIRFB0                (1u << 0)
#define LVIRFB1                (1u << 1)
#define LVIRFB2                (1u << 2)
#define LVIRFB3                (1u << 3)
#define LVIRFB4                (1u << 4)
#define LVIRFB5                (1u << 5)
#define LVIRFB6                (1u << 6)
#define LVIRFB7                (1u << 7)
                 
/* LVCFG.. Register: LV_CFG .. Bit 8-15 -> deprecated
 * use LVTMREN instead -> lib_mlx8110x_timer.h
 */
#define LVCFG_CLEAR   (uint16)~(0xFFu << 8)
#define LVCFG0                 (1u << 8)
#define LVCFG1                 (1u << 9)
#define LVCFG2                 (1u << 10)
#define LVCFG3                 (1u << 11)
#define LVCFG4                 (1u << 12)
#define LVCFG5                 (1u << 13)
#define LVCFG6                 (1u << 14)
#define LVCFG7                 (1u << 15)

/* LV0DEB .. Register: LV_DEB .. Bit 0-1 */
#define LV0DEB_CLEAR (uint16)~(3u << 0)
#define LV0DEB_OFF            (0u << 0)
#define LV0DEB_1MS            (1u << 0)
#define LV0DEB_4MS            (2u << 0)
#define LV0DEB_8MS            (3u << 0)

/* LV1DEB .. Register: LV_DEB .. Bit 2-3 */
#define LV1DEB_CLEAR (uint16)~(3u << 2)
#define LV1DEB_OFF            (0u << 2)
#define LV1DEB_1MS            (1u << 2)
#define LV1DEB_4MS            (2u << 2)
#define LV1DEB_8MS            (3u << 2)

/* LV2DEB .. Register: LV_DEB .. Bit 4-5 */
#define LV2DEB_CLEAR (uint16)~(3u << 4)
#define LV2DEB_OFF            (0u << 4)
#define LV2DEB_1MS            (1u << 4)
#define LV2DEB_4MS            (2u << 4)
#define LV2DEB_8MS            (3u << 4)

/* LV3DEB .. Register: LV_DEB .. Bit 6-7 */
#define LV3DEB_CLEAR (uint16)~(3u << 6)
#define LV3DEB_OFF            (0u << 6)
#define LV3DEB_1MS            (1u << 6)
#define LV3DEB_4MS            (2u << 6)
#define LV3DEB_8MS            (3u << 6)

/* LV4DEB .. Register: LV_DEB .. Bit 8-9 */
#define LV4DEB_CLEAR (uint16)~(3u << 8)
#define LV4DEB_OFF            (0u << 8)
#define LV4DEB_1MS            (1u << 8)
#define LV4DEB_4MS            (2u << 8)
#define LV4DEB_8MS            (3u << 8)

/* LV5DEB .. Register: LV_DEB .. Bit 10-11 */
#define LV5DEB_CLEAR (uint16)~(3u << 10)
#define LV5DEB_OFF            (0u << 10)
#define LV5DEB_1MS            (1u << 10)
#define LV5DEB_4MS            (2u << 10)
#define LV5DEB_8MS            (3u << 10)

/* LV6DEB .. Register: LV_DEB .. Bit 12-13 */
#define LV6DEB_CLEAR (uint16)~(3u << 12)
#define LV6DEB_OFF            (0u << 12)
#define LV6DEB_1MS            (1u << 12)
#define LV6DEB_4MS            (2u << 12)
#define LV6DEB_8MS            (3u << 12)

/* LV7DEB .. Register: LV_DEB .. Bit 14-15 */
#define LV7DEB_CLEAR (uint16)~(3u << 14)
#define LV7DEB_OFF            (0u << 14)
#define LV7DEB_1MS            (1u << 14)
#define LV7DEB_4MS            (2u << 14)
#define LV7DEB_8MS            (3u << 14)

/* LVOD .. Register: LV_OUTOD .. Bit 0-7 */
#define LVOD0 (1u << 0)
#define LVOD1 (1u << 1)
#define LVOD2 (1u << 2)
#define LVOD3 (1u << 3)
#define LVOD4 (1u << 4)
#define LVOD5 (1u << 5)
#define LVOD6 (1u << 6)
#define LVOD7 (1u << 7)

/* LVWU .. Register: LV_ENWU .. Bit 0-7 without Bit 6 (LV6)*/
#define LVENWU0 (1u << 0)
#define LVENWU1 (1u << 1)
#define LVENWU2 (1u << 2)
#define LVENWU3 (1u << 3)
#define LVENWU4 (1u << 4)
#define LVENWU5 (1u << 5)

#define LVENWU7 (1u << 7)

/* LVIN .. Register: LV_IN .. Bit 0-7 */
#define LVIN0 (1u << 0)
#define LVIN1 (1u << 1)
#define LVIN2 (1u << 2)
#define LVIN3 (1u << 3)
#define LVIN4 (1u << 4)
#define LVIN5 (1u << 5)
#define LVIN6 (1u << 6)
#define LVIN7 (1u << 7)

/* LVINEN .. Register: LV_INEN .. Bit 0-7 */
#define LVINEN0 (1u << 0)
#define LVINEN1 (1u << 1)
#define LVINEN2 (1u << 2)
#define LVINEN3 (1u << 3)
#define LVINEN4 (1u << 4)
#define LVINEN5 (1u << 5)
#define LVINEN6 (1u << 6)
#define LVINEN7 (1u << 7)

/* LVINT .. Register: ANA_INA .. Bit 0-7 */
#define LVINT0 (1u << 0)
#define LVINT1 (1u << 1)
#define LVINT2 (1u << 2)
#define LVINT3 (1u << 3)
#define LVINT4 (1u << 4)
#define LVINT5 (1u << 5)
#define LVINT6 (1u << 6)
#define LVINT7 (1u << 7)

/* ENVREF .. Register: LV_INEN .. Bit 15 */
#define ENVREF (1u << 15)

/** LV0 enable open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD0, Bit positions [0]
 */
#define LV0_SET_OD() LV_OUTOD |= LVOD0

/** LV0 set open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD0, Bit positions [0]
 */
#define LV0_RESET_OD() LV_OUTOD &= (uint16)~(LVOD0)

/** Toggle opendrain from LV0
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD0, Bit positions [0]
 */
#define LV0_TOGGLE_OD() LV_OUTOD ^= (uint16)LVOD0

/** LV0 enable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU0, Bit positions [0]
 */
#define LV0_ENABLE_WU() LV_ENWU |= LVWU0

/** LV0 disable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU0, Bit positions [0]
 */
#define LV0_DISABLE_WU() LV_ENWU &= (uint16)~(LVWU0)


/** LV0 enable falling edge
 * @param
 * @return void
 * Register: LV_CFG, Bit LVIRFB0, Bit positions [0]
 */
#define LV0_ENABLE_FALLING_EDGE() LV_CFG |= LVIRFB0

/** LV0 enable rising edge
 * @param
 * @return void
 * Register: LVPULL, Bit LVIRFB0, Bit positions [0]
 */
#define LV0_ENABLE_RISING_EDGE() LV_CFG &= (uint16)~(LVIRFB0)

/** set LV0 debouncing time
 * @param
 * @return void
 * Register: LV_DEB, Bit LV0DEB, Bit positions [1:0]
 */
#define LV0_SET_DEBTIME(LV0DEB_TIME)                       \
do {                                                       \
       LV_DEB = ((LV_DEB & LV0DEB_CLEAR) | (LV0DEB_TIME)); \
	} while (0)
	
/** LV0 enable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN0, Bit positions [0]
 */
#define LV0_ENABLE_COMP() LV_INEN |= LVINEN0

/** LV0 disable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN0, Bit positions [0]
 */
#define LV0_DISABLE_COMP() LV_INEN &= (uint16)~(LVINEN0)

/** LV0 get input state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: LV_INEN, Bit LVIN0, Bit positions [0]
 */
#define LV0_GET_STATE() ((LV_IN & LVIN0) >> 0)

/** LV0 get interrupt state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit LVINT0, Bit positions [0]
 */
#define LV0_GET_INT_STATE() ((ANA_INA & LVINT0) >> 0)

/**************/
/** LV1 enable open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD1, Bit positions [1]
 */
#define LV1_SET_OD() LV_OUTOD |= LVOD1

/** LV1 set open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD1, Bit positions [1]
 */
#define LV1_RESET_OD() LV_OUTOD &= (uint16)~(LVOD1)

/** Toggle opendrain from LV1
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD1, Bit positions [1]
 */
#define LV1_TOGGLE_OD() LV_OUTOD ^= (uint16)LVOD1

/** LV1 enable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU1, Bit positions [1]
 */
#define LV1_ENABLE_WU() LV_ENWU |= LVWU1

/** LV1 disable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU1, Bit positions [1]
 */
#define LV1_DISABLE_WU() LV_ENWU &= (uint16)~(LVWU1)

/** LV1 enable falling edge
 * @param
 * @return void
 * Register: LV_CFG, Bit LVIRFB1, Bit positions [1]
 */
#define LV1_ENABLE_FALLING_EDGE() LV_CFG |= LVIRFB1

/** LV1 enable rising edge
 * @param
 * @return void
 * Register: LVPULL, Bit LVIRFB1, Bit positions [1]
 */
#define LV1_ENABLE_RISING_EDGE() LV_CFG &= (uint16)~(LVIRFB1)

/** set LV1 debouncing time
 * @param
 * @return void
 * Register: LV_DEB, Bit LV1DEB, Bit positions [3:2]
 */
#define LV1_SET_DEBTIME(LV1DEB_TIME)                       \
do {                                                       \
       LV_DEB = ((LV_DEB & LV1DEB_CLEAR) | (LV1DEB_TIME)); \
	} while (0)

/** LV1 enable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN1, Bit positions [1]
 */
#define LV1_ENABLE_COMP() LV_INEN |= LVINEN1

/** LV1 disable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN1, Bit positions [1]
 */
#define LV1_DISABLE_COMP() LV_INEN &= (uint16)~(LVINEN1)

/** LV1 get input state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: LV_INEN, Bit LVIN1, Bit positions [1]
 */
#define LV1_GET_STATE() ((LV_IN & LVIN1) >> 1)

/** LV1 get interrupt state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit LVINT1, Bit positions [1]
 */
#define LV1_GET_INT_STATE() ((ANA_INA & LVINT1) >> 1)

/**************/
/** LV2 enable open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD2, Bit positions [2]
 */
#define LV2_SET_OD() LV_OUTOD |= LVOD2

/** LV2 set open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD2, Bit positions [2]
 */
#define LV2_RESET_OD() LV_OUTOD &= (uint16)~(LVOD2)

/** Toggle opendrain from LV2
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD2, Bit positions [2]
 */
#define LV2_TOGGLE_OD() LV_OUTOD ^= (uint16)LVOD2

/** LV2 enable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU2, Bit positions [2]
 */
#define LV2_ENABLE_WU() LV_ENWU |= LVWU2

/** LV2 disable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU2, Bit positions [2]
 */
#define LV2_DISABLE_WU() LV_ENWU &= (uint16)~(LVWU2)

/** LV2 enable falling edge
 * @param
 * @return void
 * Register: LV_CFG, Bit LVIRFB2, Bit positions [2]
 */
#define LV2_ENABLE_FALLING_EDGE() LV_CFG |= LVIRFB2

/** LV2 enable rising edge
 * @param
 * @return void
 * Register: LVPULL, Bit LVIRFB2, Bit positions [2]
 */
#define LV2_ENABLE_RISING_EDGE() LV_CFG &= (uint16)~(LVIRFB2)

/** set LV2 debouncing time
 * @param
 * @return void
 * Register: LV_DEB, Bit LV2DEB, Bit positions [5:4]
 */
#define LV2_SET_DEBTIME(LV2DEB_TIME)                       \
do {                                                       \
       LV_DEB = ((LV_DEB & LV2DEB_CLEAR) | (LV2DEB_TIME)); \
	} while (0)

/** LV2 enable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN2, Bit positions [2]
 */
#define LV2_ENABLE_COMP() LV_INEN |= LVINEN2

/** LV2 disable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN2, Bit positions [2]
 */
#define LV2_DISABLE_COMP() LV_INEN &= (uint16)~(LVINEN2)

/** LV2 get input state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: LV_INEN, Bit LVIN2, Bit positions [2]
 */
#define LV2_GET_STATE() ((LV_IN & LVIN2) >> 2)

/** LV2 get interrupt state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit LVINT2, Bit positions [2]
 */
#define LV2_GET_INT_STATE() ((ANA_INA & LVINT2) >> 2)

/**************/
/** LV3 enable open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD3, Bit positions [3]
 */
#define LV3_SET_OD() LV_OUTOD |= LVOD3

/** LV3 set open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD3, Bit positions [3]
 */
#define LV3_RESET_OD() LV_OUTOD &= (uint16)~(LVOD3)

/** Toggle opendrain from LV3
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD3, Bit positions [3]
 */
#define LV3_TOGGLE_OD() LV_OUTOD ^= (uint16)LVOD3

/** LV3 enable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU3, Bit positions [3]
 */
#define LV3_ENABLE_WU() LV_ENWU |= LVWU3

/** LV3 disable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU3, Bit positions [3]
 */
#define LV3_DISABLE_WU() LV_ENWU &= (uint16)~(LVWU3)

/** LV3 enable falling edge
 * @param
 * @return void
 * Register: LV_CFG, Bit LVIRFB3, Bit positions [3]
 */
#define LV3_ENABLE_FALLING_EDGE() LV_CFG |= LVIRFB3

/** LV3 enable rising edge
 * @param
 * @return void
 * Register: LVPULL, Bit LVIRFB3, Bit positions [3]
 */
#define LV3_ENABLE_RISING_EDGE() LV_CFG &= (uint16)~(LVIRFB3)

/** set LV3 debouncing time
 * @param
 * @return void
 * Register: LV_DEB, Bit LV3DEB, Bit positions [7:6]
 */
#define LV3_SET_DEBTIME(LV3DEB_TIME)                       \
do {                                                       \
       LV_DEB = ((LV_DEB & LV3DEB_CLEAR) | (LV3DEB_TIME)); \
	} while (0)

/** LV3 enable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN3, Bit positions [3]
 */
#define LV3_ENABLE_COMP() LV_INEN |= LVINEN3

/** LV3 disable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN3, Bit positions [3]
 */
#define LV3_DISABLE_COMP() LV_INEN &= (uint16)~(LVINEN3)

/** LV3 get input state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: LV_INEN, Bit LVIN3, Bit positions [3]
 */
#define LV3_GET_STATE() ((LV_IN & LVIN3) >> 3)

/** LV3 get interrupt state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit LVINT3, Bit positions [3]
 */
#define LV3_GET_INT_STATE() ((ANA_INA & LVINT3) >> 3)

/**************/
/** LV4 enable open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD4, Bit positions [4]
 */
#define LV4_SET_OD() LV_OUTOD |= LVOD4

/** LV4 set open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD4, Bit positions [4]
 */
#define LV4_RESET_OD() LV_OUTOD &= (uint16)~(LVOD4)

/** Toggle opendrain from LV4
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD4, Bit positions [4]
 */
#define LV4_TOGGLE_OD() LV_OUTOD ^= (uint16)LVOD4

/** LV4 enable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU4, Bit positions [4]
 */
#define LV4_ENABLE_WU() LV_ENWU |= LVWU4

/** LV4 disable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU4, Bit positions [4]
 */
#define LV4_DISABLE_WU() LV_ENWU &= (uint16)~(LVWU4)

/** LV4 enable falling edge
 * @param
 * @return void
 * Register: LV_CFG, Bit LVIRFB4, Bit positions [4]
 */
#define LV4_ENABLE_FALLING_EDGE() LV_CFG |= LVIRFB4

/** LV4 enable rising edge
 * @param
 * @return void
 * Register: LVPULL, Bit LVIRFB4, Bit positions [4]
 */
#define LV4_ENABLE_RISING_EDGE() LV_CFG &= (uint16)~(LVIRFB4)

/** set LV4 debouncing time
 * @param
 * @return void
 * Register: LV_DEB, Bit LV4DEB, Bit positions [9:8]
 */
#define LV4_SET_DEBTIME(LV4DEB_TIME)                       \
do {                                                       \
       LV_DEB = ((LV_DEB & LV4DEB_CLEAR) | (LV4DEB_TIME)); \
	} while (0)

/** LV4 enable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN4, Bit positions [4]
 */
#define LV4_ENABLE_COMP() LV_INEN |= LVINEN4

/** LV4 disable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN4, Bit positions [4]
 */
#define LV4_DISABLE_COMP() LV_INEN &= (uint16)~(LVINEN4)

/** LV4 get input state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: LV_INEN, Bit LVIN4, Bit positions [4]
 */
#define LV4_GET_STATE() ((LV_IN & LVIN4) >> 4)

/** LV4 get interrupt state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit LVINT4, Bit positions [4]
 */
#define LV4_GET_INT_STATE() ((ANA_INA & LVINT4) >> 4)

/**************/
/** LV5 enable open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD5, Bit positions [5]
 */
#define LV5_SET_OD() LV_OUTOD |= LVOD5

/** LV5 set open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD5, Bit positions [5]
 */
#define LV5_RESET_OD() LV_OUTOD &= (uint16)~(LVOD5)

/** Toggle opendrain from LV5
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD5, Bit positions [5]
 */
#define LV5_TOGGLE_OD() LV_OUTOD ^= (uint16)LVOD5

/** LV5 enable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU5, Bit positions [5]
 */
#define LV5_ENABLE_WU() LV_ENWU |= LVWU5

/** LV5 disable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU5, Bit positions [5]
 */
#define LV5_DISABLE_WU() LV_ENWU &= (uint16)~(LVWU5)

/** LV5 enable falling edge
 * @param
 * @return void
 * Register: LV_CFG, Bit LVIRFB5, Bit positions [5]
 */
#define LV5_ENABLE_FALLING_EDGE() LV_CFG |= LVIRFB5

/** LV5 enable rising edge
 * @param
 * @return void
 * Register: LVPULL, Bit LVIRFB5, Bit positions [5]
 */
#define LV5_ENABLE_RISING_EDGE() LV_CFG &= (uint16)~(LVIRFB5)

/** set LV5 debouncing time
 * @param
 * @return void
 * Register: LV_DEB, Bit LV5DEB, Bit positions [11:10]
 */
#define LV5_SET_DEBTIME(LV5DEB_TIME)                       \
do {                                                       \
       LV_DEB = ((LV_DEB & LV5DEB_CLEAR) | (LV5DEB_TIME)); \
	} while (0)

/** LV5 enable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN5, Bit positions [5]
 */
#define LV5_ENABLE_COMP() LV_INEN |= LVINEN5

/** LV5 disable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN5, Bit positions [5]
 */
#define LV5_DISABLE_COMP() LV_INEN &= (uint16)~(LVINEN5)

/** LV5 get input state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: LV_INEN, Bit LVIN5, Bit positions [5]
 */
#define LV5_GET_STATE() ((LV_IN & LVIN5) >> 5)

/** LV5 get interrupt state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit LVINT5, Bit positions [5]
 */
#define LV5_GET_INT_STATE() ((ANA_INA & LVINT5) >> 5)

/**************/
/** LV6 enable open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD6, Bit positions [6]
 */
#define LV6_SET_OD() LV_OUTOD |= LVOD6

/** LV6 set open drain
 * @param
 * @return void
 * Register: LV_OUTOD Bit LVOD6, Bit positions [6]
 */
#define LV6_RESET_OD() LV_OUTOD &= (uint16)~(LVOD6)

/** Toggle opendrain from LV6
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD6, Bit positions [6]
 */
#define LV6_TOGGLE_OD() LV_OUTOD ^= (uint16)LVOD6

/** LV6 enable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU6, Bit positions [6]
 */
#define LV6_ENABLE_WU() LV_ENWU |= LVWU6

/** LV6 disable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU6, Bit positions [6]
 */
#define LV6_DISABLE_WU() LV_ENWU &= (uint16)~(LVWU6)

/** LV6 enable falling edge
 * @param
 * @return void
 * Register: LV_CFG, Bit LVIRFB6, Bit positions [6]
 */
#define LV6_ENABLE_FALLING_EDGE() LV_CFG |= LVIRFB6

/** LV6 enable rising edge
 * @param
 * @return void
 * Register: LVPULL, Bit LVIRFB6, Bit positions [6]
 */
#define LV6_ENABLE_RISING_EDGE() LV_CFG &= (uint16)~(LVIRFB6)

/** set LV6 debouncing time
 * @param
 * @return void
 * Register: LV_DEB, Bit LV6DEB, Bit positions [13:12]
 */
#define LV6_SET_DEBTIME(LV6DEB_TIME)                       \
do {                                                       \
       LV_DEB = ((LV_DEB & LV6DEB_CLEAR) | (LV6DEB_TIME)); \
	} while (0)

/** LV6 enable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN6, Bit positions [6]
 */
#define LV6_ENABLE_COMP() LV_INEN |= LVINEN6

/** LV6 disable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN6, Bit positions [6]
 */
#define LV6_DISABLE_COMP() LV_INEN &= (uint16)~(LVINEN6)

/** LV6 get input state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: LV_INEN, Bit LVIN6, Bit positions [6]
 */
#define LV6_GET_STATE() ((LV_IN & LVIN6) >> 6)

/** LV6 get interrupt state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit LVINT6, Bit positions [6]
 */
#define LV6_GET_INT_STATE() ((ANA_INA & LVINT6) >> 6)

/**************/
/** LV7 enable open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD7, Bit positions [7]
 */
#define LV7_SET_OD() LV_OUTOD |= LVOD7

/** LV7 set open drain
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD7, Bit positions [7]
 */
#define LV7_RESET_OD() LV_OUTOD &= (uint16)~(LVOD7)

/** Toggle opendrain from LV7
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVOD7, Bit positions [7]
 */
#define LV7_TOGGLE_OD() LV_OUTOD ^= (uint16)LVOD7

/** LV7 enable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU7, Bit positions [7]
 */
#define LV7_ENABLE_WU() LV_ENWU |= LVWU7

/** LV7 disable wakeup
 * @param
 * @return void
 * Register: LV_ENWU, Bit LVWU7, Bit positions [7]
 */
#define LV7_DISABLE_WU() LV_ENWU &= (uint16)~(LVWU7)

/** LV7 enable falling edge
 * @param
 * @return void
 * Register: LV_CFG, Bit LVIRFB7, Bit positions [7]
 */
#define LV7_ENABLE_FALLING_EDGE() LV_CFG |= LVIRFB7

/** LV7 enable rising edge
 * @param
 * @return void
 * Register: LVPULL, Bit LVIRFB7, Bit positions [7]
 */
#define LV7_ENABLE_RISING_EDGE() LV_CFG &= (uint16)~(LVIRFB7)

/** set LV7 debouncing time
 * @param
 * @return void
 * Register: LV_DEB, Bit LV7DEB, Bit positions [15:14]
 */
#define LV7_SET_DEBTIME(LV7DEB_TIME)                       \
do {                                                       \
       LV_DEB = ((LV_DEB & LV7DEB_CLEAR) | (LV7DEB_TIME)); \
	} while (0)

/** LV7 enable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN7, Bit positions [7]
 */
#define LV7_ENABLE_COMP() LV_INEN |= LVINEN7

/** LV7 disable comparator
 * @param
 * @return void
 * Register: LV_INEN, Bit LVINEN7, Bit positions [7]
 */
#define LV7_DISABLE_COMP() LV_INEN &= (uint16)~(LVINEN7)

/** LV7 get input state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: LV_INEN, Bit LVIN7, Bit positions [7]
 */
#define LV7_GET_STATE() ((LV_IN & LVIN7) >> 7)

/** LV7 get interrupt state
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit LVINT7, Bit positions [7]
 */
#define LV7_GET_INT_STATE() ((ANA_INA & LVINT7) >> 7)

/** enable vref
 * @param
 * @return void
 * Register: LV_INEN, Bit ENVREF, Bit positions [15]
 */
#define VREF_ENABLE() LV_INEN |= ENVREF;

/** disable vref
 * @param
 * @return void
 * Register: LV_INEN, Bit ENVREF, Bit positions [15]
 */
#define VREF_DISABLE() LV_INEN &= (uint16)~(ENVREF)


#define LV0_INT_ENABLE()     __LV_INT_ENABLE(0)
#define LV0_INT_DISABLE()    __LV_INT_DISABLE(0)

#define LV1_INT_ENABLE()     __LV_INT_ENABLE(1)
#define LV1_INT_DISABLE()    __LV_INT_DISABLE(1)

#define LV2_INT_ENABLE()     __LV_INT_ENABLE(2)
#define LV2_INT_DISABLE()    __LV_INT_DISABLE(2)

#define LV3_INT_ENABLE()     __LV_INT_ENABLE(3)
#define LV3_INT_DISABLE()    __LV_INT_DISABLE(3)

#define LV4_INT_ENABLE()     __LV_INT_ENABLE(4)
#define LV4_INT_DISABLE()    __LV_INT_DISABLE(4)

#define LV5_INT_ENABLE()     __LV_INT_ENABLE(5)
#define LV5_INT_DISABLE()    __LV_INT_DISABLE(5)

#define LV6_INT_ENABLE()     __LV_INT_ENABLE(6)
#define LV6_INT_DISABLE()    __LV_INT_DISABLE(6)

#define LV7_INT_ENABLE()     __LV_INT_ENABLE(7)
#define LV7_INT_DISABLE()    __LV_INT_DISABLE(7)



#endif /* LIB_MLX8110X_LV_H_ */
