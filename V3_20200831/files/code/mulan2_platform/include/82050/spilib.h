/*
 * Copyright (C) 2005-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef SPILIB_H_
#define SPILIB_H_

#include "syslib.h"

/*
 * Data register -- DR[15:0]
 * Baud Rate register -- BRR[11:0]
 *
 * Control register -- PCR[7:0], alias of CTRL[7:0])
 *   Bits:
 *       RIE[7] - receiver interrupt enabled (1) or disabled (0)
 *       TIE[6] - transmitter interrupt enabled (1) or disabled (0)
 *       BYTEMOD[5] - select word (0) or byte (1) mode
 *       MSTR[4] - select master (1) or slave (0) mode
 *       CPOL[3] - select clock polarity
 *       CPHA[2] - select clock phase 
 *       EN[1] - unit is in enabled (0) or disabled (1)
 *       CKEN[0] - clock is enebled (1) or disabled (0)
 *
 * Status and control register -- PSCR[7:0], alias of CTRL[15:8]
 *   Read-only bits:
 *       RF[7] - receiver full
 *       TF[6] - transmitter full
 *       OVRF[5] - overflow (RX or TX)
 *       MODF[4] - fault
 *   R/W bits:
 *       FRSSOEN[3] - Enable /SS output pin. Master mode only
 *       MODFEN[2] - MODF flag enabled (1) or disabled (0). Slave mode only
 *       MSTRONLY[1] - transmit only (1) or full-duplex (0). Master mode only
 *       ERRIE[0] - MODF and OVRF interrupt enabled (1) or disabled (0)
 */

#define SPI_FORMAT_00   0
#define SPI_FORMAT_01   1
#define SPI_FORMAT_10   2
#define SPI_FORMAT_11   3

/* Fspi = Fck / (SPI_RATE + 1) */
#define SPI_RATE(f)     ((FPLL / (double)(f) + 0.5) - 1)


/* SPI master:
 * frame lenght 8 bits
 * interrupts enabled 
 * Drive /SS pin (FRSSOE = 1)
 */
#define __SPI_MASTER_INIT(unit, rate, format)                                           \
do  {                                                                                   \
    SPI##unit##_CTRL = 1;                                                               \
    SPI##unit##_BRR  = (rate);                                                          \
    SPI##unit##_CTRL = (1<<11) | (1<<7) | (1<<6) | (1<<5) | (1<<4) | ((format)<<2) | 3; \
} while(0)

#define SPI1_MASTER_INIT(rate, format)  \
    __SPI_MASTER_INIT(1, rate, format)

#define SPI2_MASTER_INIT(rate, format)  \
    __SPI_MASTER_INIT(2, rate, format)

/*
 * SPI slave
 */
#define __SPI_SLAVE_INIT(unit, format)                                  \
do  {                                                                   \
    SPI##unit##_CTRL = 1;                                               \
    SPI##unit##_BRR  = 0;                                               \
    SPI##unit##_CTRL = (1<<7) | (1<<6) | (1<<5) | ((format)<<2) | 3;    \
} while(0)

#define SPI1_SLAVE_INIT(format)     \
    __SPI_SLAVE_INIT(1, format)

#define SPI2_SLAVE_INIT(format)     \
    __SPI_SLAVE_INIT(2, format)

/*
 * Enable/disable interrupts from SPI unit
 */
#define __SPI_INT_ENABLE(n)     \
    do  {                       \
        XI3_PEND  = 1u << (n);  \
        XI3_MASK |= 1u << (n);  \
    } while(0)

#define __SPI_INT_DISABLE(n)    \
    ( XI3_MASK &= ~(1u << (n)) )

#define SPI1_RI_ENABLE()    __SPI_INT_ENABLE(15)
#define SPI1_RI_DISABLE()   __SPI_INT_DISABLE(15)

#define SPI1_TI_ENABLE()    __SPI_INT_ENABLE(14)
#define SPI1_TI_DISABLE()   __SPI_INT_DISABLE(14)

#define SPI2_RI_ENABLE()    __SPI_INT_ENABLE(13)
#define SPI2_RI_DISABLE()   __SPI_INT_DISABLE(13)

#define SPI2_TI_ENABLE()    __SPI_INT_ENABLE(12)
#define SPI2_TI_DISABLE()   __SPI_INT_DISABLE(12)

#endif /* SPILIB_H_ */
