/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef LIB_MLX8110X_SPI_H_
#define LIB_MLX8110X_SPI_H_

#include "ioports.h"

/* SPI_LV3 .. Register: LV_TMR .. Bit 11 */
#define SPI_LV3 (1u<<11)

/* SPI_LV4 .. Register: LV_TMR .. Bit 12 */
#define SPI_LV4 (1u<<12)

/* SPI_LV5 .. Register: LV_TMR .. Bit 13 */
#define SPI_LV5 (1u<<13)

/* SPI_LV7 .. Register: LV_TMR .. Bit 14 */
#define SPI_LV7 (1u<<14)
#define SPI_LV6 SPI_LV7 

/* SPI_HV0 .. Register: HV_TMR .. Bit 8 */
#define SPI_HV0 (1u<<8)

/* SPI_HV1 .. Register: HV_TMR .. Bit 9 */
#define SPI_HV1 (1u<<9)

/* SPI_HV2 .. Register: HV_TMR .. Bit 10 */
#define SPI_HV2 (1u<<10)

/* SPI_HV3 .. Register: HV_TMR .. Bit 11 */
#define SPI_HV3 (1u<<11)

/** Connect SPI-Block to LV3
 * @param
 * @return void
 * Register: LV_TMR, Bit LVSPI3, Bit positions [11]
 */
#define SPI_CONNECT_LV3()        LV_TMR |= (SPI_LV3)

/** Disconnect SPI-Block LV3
 * @param
 * @return void
 * Register: LV_TMR, Bit LVSPI3, Bit positions [11]
 */
#define SPI_DISCONNECT_LV3()        LV_TMR &= (uint16)~(SPI_LV3)

/** Connect SPI-Block to LV4
 * @param
 * @return void
 * Register: LV_TMR, Bit LVSPI4, Bit positions [12]
 */
#define SPI_CONNECT_LV4()        LV_TMR |= (SPI_LV4)

/** Disconnect SPI-Block LV4
 * @param
 * @return void
 * Register: LV_TMR, Bit LVSPI4, Bit positions [12]
 */
#define SPI_DISCONNECT_LV4()        LV_TMR &= (uint16)~(SPI_LV4)

/** Connect SPI-Block to LV5
 * @param
 * @return void
 * Register: LV_TMR, Bit LVSPI5, Bit positions [13]
 */
#define SPI_CONNECT_LV5()        LV_TMR |= (SPI_LV5)

/** Disconnect SPI-Block LV5
 * @param
 * @return void
 * Register: LV_TMR, Bit LVSPI5, Bit positions [13]
 */
#define SPI_DISCONNECT_LV5()        LV_TMR &= (uint16)~(SPI_LV5)

/** Connect SPI-Block to LV6
 * @param
 * @return void
 * Register: LV_TMR, Bit LVSPI6, Bit positions [14]
 */
#define SPI_CONNECT_LV6()        LV_TMR |= (SPI_LV6)

/** Disconnect SPI-Block LV6
 * @param
 * @return void
 * Register: LV_TMR, Bit LVSPI6, Bit positions [14]
 */
#define SPI_DISCONNECT_LV6()        LV_TMR &= (uint16)~(SPI_LV6)

/** Connect SPI-Block to HV0
 * @param
 * @return void
 * Register: HV_TMR, Bit HVSPI0, Bit positions [8]
 */
#define SPI_CONNECT_HV0()        HV_TMR |= (SPI_HV0)

/** Disconnect SPI-Block HV0
 * @param
 * @return void
 * Register: HV_TMR, Bit HVSPI0, Bit positions [8]
 */
#define SPI_DISCONNECT_HV0()        HV_TMR &= (uint16)~(SPI_HV0)

/** Connect SPI-Block to HV1
 * @param
 * @return void
 * Register: HV_TMR, Bit HVSPI1, Bit positions [9]
 */
#define SPI_CONNECT_HV1()        HV_TMR |= (SPI_HV1)

/** Disconnect SPI-Block HV1
 * @param
 * @return void
 * Register: HV_TMR, Bit HVSPI1, Bit positions [9]
 */
#define SPI_DISCONNECT_HV1()        HV_TMR &= (uint16)~(SPI_HV1)

/** Connect SPI-Block to HV2
 * @param
 * @return void
 * Register: HV_TMR, Bit HVSPI2, Bit positions [10]
 */
#define SPI_CONNECT_HV2()        HV_TMR |= (SPI_HV2)

/** Disconnect SPI-Block HV2
 * @param
 * @return void
 * Register: HV_TMR, Bit HVSPI2, Bit positions [10]
 */
#define SPI_DISCONNECT_HV2()        HV_TMR &= (uint16)~(SPI_HV2)

/** Connect SPI-Block to HV3
 * @param
 * @return void
 * Register: HV_TMR, Bit HVSPI3, Bit positions [11]
 */
#define SPI_CONNECT_HV3()        HV_TMR |= (SPI_HV3)

/** Disconnect SPI-Block HV3
 * @param
 * @return void
 * Register: HV_TMR, Bit HVSPI3, Bit positions [11]
 */
#define SPI_DISCONNECT_HV3()        HV_TMR &= (uint16)~(SPI_HV3)


#endif /* LIB_MLX8110X_SPI_H_ */
