/*
 * Copyright (C) 2005-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef UARTLIB_H_
#define UARTLIB_H_

#include "syslib.h"

/*
 * UART/LIN selector -- SEL[7:0]
 * Bits:
 *      LDC[7] - LIN specific, set to zero for UART
 *      LSC[6] - LIN specific, set to zero for UART
 *      LBC[5] - LIN specific, set to zero for UART
 *      BSC[4:3] - bit scrambler selector (data format RZ, NRZ or MPE)
 *      MLS[2:0] - message length selector, 0 = 9 bits; 7 = 16 bits
 *
 * -------------------------------------------------------------------
 * Control register -- CTRL[7:0]
 * Bits:
 *      LTE[7] - LIN specific, set to zero for UART
 *      LSE[6] - LIN specific, set to zero for UART
 *      LBE[5] - LIN specific, set to zero for UART
 *      RESERVED[4]
 *      ISB[3] - TX idle (inverted)
 *      REE[2] - receive enabled
 *      TRE[1] - transmit enabled
 *      EBLK[0] - unit enabled
 * --------------------------------------------------------------------
 * Baudrate register -- BRR[15:0]
 *
 * --------------------------------------------------------------------
 * UART flags -- INT[0:7]
 *  Bits:
 *      SBE[7] - Stop Bit Error
 *      NBR[6] - Noisy Bit Received
 *      RRF[5] - Receive IO port Full
 *      RSB[4] - Receive Shifter Busy
 *      RSO[3] - Receive Shifter Overflow
 *      TSB[2] - Transmit Shifter Busy
 *      TRB[1] - Transmit IO port Busy
 *      TRO[0] - Transmit IO Overflow
 */

#define UART_LENGTH_9   0
#define UART_LENGTH_10  1
#define UART_LENGTH_11  2
#define UART_LENGTH_12  3
#define UART_LENGTH_13  4
#define UART_LENGTH_14  5
#define UART_LENGTH_15  6
#define UART_LENGTH_16  7

#define UART_FORMAT_LSB 0
#define UART_FORMAT_MSB 1
#define UART_FORMAT_MPE 2
#define UART_FORMAT_RZ  3

/*
 * Fbit = Fck / (16 * (UART_RATE+1))
 */
#define UART_RATE(f)        ((FPLL*1000.0/16.0)/(f)-0.5)

/*
 * Initialize UART unit
 */
#define __UART_INIT(unit, rate, length, format)     \
do  {                                               \
    URT##unit##_CTRL = 1;                           \
    URT##unit##_SEL  = (length) | ((format)<<3);    \
    URT##unit##_BRR  = (rate);                      \
    URT##unit##_CTRL = 7;                           \
} while(0)

#define UART1_INIT(rate, length, format)     \
    __UART_INIT(1, rate, length, format)

#define UART2_INIT(rate, length, format)     \
    __UART_INIT(2, rate, length, format)


/*
 * Enable/disable interrupts from UART unit
 */
#define __UART_INT_ENABLE(n)    \
do  {                           \
    XI3_PEND  = 1u << (n);      \
    XI3_MASK |= 1u << (n);      \
} while(0)

#define __UART_INT_DISABLE(n)   \
    ( XI3_MASK &= ~(1u << (n)) )

/* UART1 */
#define UART1_RRI_ENABLE()  __UART_INT_ENABLE(11)
#define UART1_RRI_DISABLE() __UART_INT_DISABLE(11)

#define UART1_TRI_ENABLE()  __UART_INT_ENABLE(10)
#define UART1_TRI_DISABLE() __UART_INT_DISABLE(10)

#define UART1_RSI_ENABLE()  __UART_INT_ENABLE(9)
#define UART1_RSI_DISABLE() __UART_INT_DISABLE(9)

#define UART1_TSI_ENABLE()  __UART_INT_ENABLE(8)
#define UART1_TSI_DISABLE() __UART_INT_DISABLE(8)

#define UART1_SBI_ENABLE()  __UART_INT_ENABLE(7)
#define UART1_SBI_DISABLE() __UART_INT_DISABLE(7)

#define UART1_TEI_ENABLE()  __UART_INT_ENABLE(6)
#define UART1_TEI_DISABLE() __UART_INT_DISABLE(6)

/* UART2 */
#define UART2_RRI_ENABLE()  __UART_INT_ENABLE(5)
#define UART2_RRI_DISABLE() __UART_INT_DISABLE(5)

#define UART2_TRI_ENABLE()  __UART_INT_ENABLE(4)
#define UART2_TRI_DISABLE() __UART_INT_DISABLE(4)

#define UART2_RSI_ENABLE()  __UART_INT_ENABLE(3)
#define UART2_RSI_DISABLE() __UART_INT_DISABLE(3)

#define UART2_TSI_ENABLE()  __UART_INT_ENABLE(2)
#define UART2_TSI_DISABLE() __UART_INT_DISABLE(2)

#define UART2_SBI_ENABLE()  __UART_INT_ENABLE(1)
#define UART2_SBI_DISABLE() __UART_INT_DISABLE(1)

#define UART2_TEI_ENABLE()  __UART_INT_ENABLE(0)
#define UART2_TEI_DISABLE() __UART_INT_DISABLE(0)


/*
 * TRB - transmitter register is busy
 */
#define __UART_TX_BUSY(unit)    \
    ((URT##unit##_INT & (1u << 1)) != 0)

#define UART1_TX_BUSY() __UART_TX_BUSY(1)
#define UART2_TX_BUSY() __UART_TX_BUSY(2)

#define UART1_TX_POLLING()  \
do  {                       \
}   while(UART1_TX_BUSY())

#define UART2_TX_POLLING()  \
do  {                       \
}   while(UART2_TX_BUSY())


/*
 * Puts raw data (without start and stop bits) to UART
 *
 * Notes:
 *  1. Function is applicable for all data formats:
 *     UART_FORMAT_LSB, UART_FORMAT_MSB, UART_FORMAT_MPE and
 *     UART_FORMAT_RZ
 */
#define __UART_PUT_RAW(unit, x)         \
        URT##unit##_TR = (x)

#define UART1_PUT_RAW(d)    __UART_PUT_RAW(1, d)
#define UART2_PUT_RAW(d)    __UART_PUT_RAW(2, d)


/*
 * Puts data frame with start and stop bits to UART
 *
 * Notes:
 *  1. Functions UART_PUT_FRAME_xx are only applicable for UART_FORMAT_LSB
 *     format (defined during initialization, see UART_INIT)
 *
 */
#define __UART_PUT_FRAME(unit, length, x)       \
    URT##unit##_TR = ((uint16)(x) << 1) | (1u << ((length) + 1))

#define UART1_PUT_FRAME_7N1(d)      __UART_PUT_FRAME(1,  7, d)
#define UART1_PUT_FRAME_8N1(d)      __UART_PUT_FRAME(1,  8, d)
#define UART1_PUT_FRAME_9N1(d)      __UART_PUT_FRAME(1,  9, d)
#define UART1_PUT_FRAME_10N1(d)     __UART_PUT_FRAME(1, 10, d)
#define UART1_PUT_FRAME_11N1(d)     __UART_PUT_FRAME(1, 11, d)
#define UART1_PUT_FRAME_12N1(d)     __UART_PUT_FRAME(1, 12, d)
#define UART1_PUT_FRAME_13N1(d)     __UART_PUT_FRAME(1, 13, d)
#define UART1_PUT_FRAME_14N1(d)     __UART_PUT_FRAME(1, 14, d)

#define UART2_PUT_FRAME_7N1(d)      __UART_PUT_FRAME(2,  7, d)
#define UART2_PUT_FRAME_8N1(d)      __UART_PUT_FRAME(2,  8, d)
#define UART2_PUT_FRAME_9N1(d)      __UART_PUT_FRAME(2,  9, d)
#define UART2_PUT_FRAME_10N1(d)     __UART_PUT_FRAME(2, 10, d)
#define UART2_PUT_FRAME_11N1(d)     __UART_PUT_FRAME(2, 11, d)
#define UART2_PUT_FRAME_12N1(d)     __UART_PUT_FRAME(2, 12, d)
#define UART2_PUT_FRAME_13N1(d)     __UART_PUT_FRAME(2, 13, d)
#define UART2_PUT_FRAME_14N1(d)     __UART_PUT_FRAME(2, 14, d)


/*
 * Notes:
 *  1. Function UARTx_PUT is deprecated. Use UARTx_PUT_FRAME_8N1 instead
 */
inline __attribute__((always_inline, deprecated))
static void UART1_PUT (int16 c)
{
    UART1_PUT_FRAME_8N1(c);
}

inline __attribute__((always_inline, deprecated))
static void UART2_PUT (int16 c)
{
    UART2_PUT_FRAME_8N1(c);
}


/*
 * RRF - receive register is full
 */
#define __UART_RX_FULL(unit)    \
    ((URT##unit##_INT & (1u << 5)) != 0)

#define UART1_RX_FULL()     __UART_RX_FULL(1)
#define UART2_RX_FULL()     __UART_RX_FULL(2)

#define UART1_RX_POLLING()  \
do  {                       \
}   while(!UART1_RX_FULL())

#define UART2_RX_POLLING()  \
do  {                       \
}   while(!UART2_RX_FULL())

#define UART1_GET()     (URT1_RR)
#define UART2_GET()     (URT2_RR)

#endif /*  UARTLIB_H_ */
