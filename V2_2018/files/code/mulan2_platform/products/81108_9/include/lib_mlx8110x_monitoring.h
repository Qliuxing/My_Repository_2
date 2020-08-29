/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef _LIB_MLX8110X_MONITORING_H_
#define _LIB_MLX8110X_MONITORING_H_

#include "ioports.h"

/*
 * Enable/disable interrupts from Safety unit
 */
#define __SAFETY_INT_ENABLE(n)     \
    do  {                          \
        XI4_PEND  = 1u << (n);     \
        XI4_MASK |= 1u << (n);     \
    } while(0)

#define __SAFETY_INT_DISABLE(n)    \
    ( XI4_MASK &= ~(1u << (n)) )
    
/* PRUV .. Register: ANA_OUTG .. Bit 8-9 */
#define PRUV   (uint16)~(3u << 8)
#define UV_6V           (0u << 8)
#define UV_7V           (1u << 8)
#define UV_8V           (2u << 8)
#define UV_9V           (3u << 8)

/* INACTIVE_OVT .. Register: ANA_OUTG .. Bit 7 */
#define INACTIVE_OVT    (1u << 7)

/* DISABLE_OVHV .. Register: ANA_OUTG .. Bit 4 */
#define DISABLE_OVHV    (1u << 4)

/* DISABLE_OV8V .. Register: ANA_OUTG .. Bit 3 */
#define DISABLE_OV8V    (1u << 3)

/* OV_VS .. Register: ANA_INA .. Bit 13 */
#define OV_VS           (1u << 13)

/* UV_VS .. Register: ANA_INA .. Bit 14 */
#define UV_VS           (1u << 14)

/* OVT .. Register: ANA_INA .. Bit 15 */
#define OVT             (1u << 15)


/** Set undervoltage detection threshold
 * @param
 * @return void
 * Register: ANA_OUTG, Bit PRUV, Bit positions [9:8]
 */
#define UV_SET_VOLTAGE(UV_VALUE)                    \
do {                                                \
       ANA_OUTG = ((ANA_OUTG & PRUV) | (UV_VALUE));   \
	} while (0)

/** Get undervoltage VS interrupt
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit UV_VS, Bit positions [14]
 */
#define UV_VS_GET_STATE() ((ANA_INA & UV_VS) >> 14)

/** Get overvoltage VS interrupt
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit OV_VS, Bit positions [13]
 */
#define OV_VS_GET_STATE() ((ANA_INA & OV_VS) >> 13)

/** DISABLE overtemperature detection
 * @param
 * @return void
 * Register: ANA_OUTG, Bit INACTIVE_OVT, Bit positions [7]
 */
#define OVT_DISABLE() ANA_OUTG |= INACTIVE_OVT

/** Enable overtemperature detection
 * @param
 * @return void
 * Register: ANA_OUTG, Bit INACTIVE_OVT, Bit positions [7]
 */
#define OVT_ENABLE() ANA_OUTG &= (uint16)~(INACTIVE_OVT)

/** Get Overtemperature interrupt
 * @param
 * @return uint16 value (1=set, 0=not set)
 * Register: ANA_INA, Bit OVT, Bit positions [15]
 */
#define OVT_GET_STATE() ((ANA_INA & OVT) >> 15)

/** DISABLE overvoltage comparator at HV3
 * @param
 * @return void
 * Register: ANA_OUTG, Bit DISABLE_OVHV, Bit positions [4]
 */
#define DISABLE_OVERVOLTAGE_COMP_HV3() ANA_OUTG |= DISABLE_OVHV

/** Enable overvoltage comparator at HV3
 * @param
 * @return void
 * Register: ANA_OUTG, Bit DISABLE_OVHV, Bit positions [4]
 */
#define ENABLE_OVERVOLTAGE_COMP_HV3() ANA_OUTG &= ~(DISABLE_OVHV)

/** DISABLE overvoltage comparator at LV3
 * @param
 * @return void
 * Register: ANA_OUTG, Bit DISABLE_OVHV, Bit positions [3]
 */
#define DISABLE_OVERVOLTAGE_COMP_LV3() ANA_OUTG |= DISABLE_OV8V

/** Enable overvoltage comparator at LV3
 * @param
 * @return void
 * Register: ANA_OUTG, Bit DISABLE_OVHV, Bit positions [3]
 */
#define ENABLE_OVERVOLTAGE_COMP_LV3() ANA_OUTG &= ~(DISABLE_OV8V)


#define OV_VS_INT_ENABLE()    __SAFETY_INT_ENABLE(13)
#define OV_VS_INT_DISABLE()   __SAFETY_INT_DISABLE(13)

#define UV_VS_INT_ENABLE()    __SAFETY_INT_ENABLE(14)
#define UV_VS_INT_DISABLE()   __SAFETY_INT_DISABLE(14)

#define OVT_INT_ENABLE()      __SAFETY_INT_ENABLE(15)
#define OVT_INT_DISABLE()     __SAFETY_INT_DISABLE(15)

#endif /* _LIB_MLX8110X_MONITORING_H_ */
