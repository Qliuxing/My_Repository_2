/*
 * Copyright (C) 2005-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef PWMLIB_H_
#define PWMLIB_H_

#include "syslib.h"

/*
 *
 *             Fck
 * Fpwm = -------------- ,
 *              N
 *         M * 2  * PERIOD
 *
 *   Fck = system frequency equal to FPLL
 *   M = [1..16] predivider, PSCL[7:4] = M-1
 *   N = [0..11] predivider, PSCL[3:0] = N
 *   PERIOD = [1..0xFFFF] period, PER[0:15] = PERIOD-1
 *
 * Resolution vs. Frequency:
 * For operating at highest possible resolution with the PWM frequency needed,
 * the following definition scheme is recommended :
 * - select first the minimum value for N
 * - select the maximum value for the period PERIOD
 * - choose the M value so that the target PWM frequency requirements are met
 *
 * The PWM unit in Master mode can provide clock to other PWM units (Slave mode)
 * to guarantee synchronous operation
 */

/*
 * ----------------------------------------------------------------------------
 * Control register - CTRL[7:0]
 *
 *  Bits:
 *      CUST[7:5] - reserved
 *      ECI[4]  = 0/1  - disable/enable compare interrupt generation
 *      EPI[3]  = 0/1  - disable/enable interrupt when PWM counter gets equal 0
 *      MODE[2] = 0/1  - independent/mirror mode
 *      EXT[1]  = 0/1  - master/slave mode
 *      EBLK[0] = 1  - unit is enabled
 * ----------------------------------------------------------------------------
 * Lower threshold register - LT[15:0]
 *  lower threshold < high threshold
 * 
 * Higher threshold register - HT[15:0]
 *  higher threshold < period
 * 
 * Due to a double buffer mechanism the port values HT and LT can be updated at
 * any time. Writing the PWM low threshold (LT) enables the transfer of the port
 * values into the double-buffer shadow registers by default. The shadow registers
 * will be written next time the counter CNT gets equal 0
 * ----------------------------------------------------------------------------
 * Comparator threshold register - CMP[15:0]
 * 
 */


/*
 * PWM waveform mode
 */
#define PWM_INDEPENDENT_MODE    0
#define PWM_MIRROR_MODE         1

/*
 * Initialize PWM unit in a Master (default) mode
 */
#define __PWM_INIT(U, M, N, PER, LT, HT, CMP, ECI, EPI, MODE)       \
do  {                                                               \
    PWM##U##_PSCL = ((((M)-1) & 0x0F) << 4) | ((N) & 0x0F);         \
    PWM##U##_PER  = (PER) - 1;                                      \
    PWM##U##_CMP  = CMP;                                            \
    PWM##U##_LT   = LT;                                             \
    PWM##U##_HT   = HT;                                             \
    PWM##U##_CTRL = 1|((ECI)<<4)|((EPI)<<3)|((MODE)<<2);            \
} while(0)

#define PWM1_INIT(M, N, PER, LT, HT, CMP, ECI, EPI, MODE)           \
    __PWM_INIT(1, M, N, PER, LT, HT, CMP, ECI, EPI, MODE);

#define PWM2_INIT(M, N, PER, LT, HT, CMP, ECI, EPI, MODE)           \
    __PWM_INIT(2, M, N, PER, LT, HT, CMP, ECI, EPI, MODE);

#define PWM3_INIT(M, N, PER, LT, HT, CMP, ECI, EPI, MODE)           \
    __PWM_INIT(3, M, N, PER, LT, HT, CMP, ECI, EPI, MODE);

#define PWM4_INIT(M, N, PER, LT, HT, CMP, ECI, EPI, MODE)           \
    __PWM_INIT(4, M, N, PER, LT, HT, CMP, ECI, EPI, MODE);

#define PWM5_INIT(M, N, PER, LT, HT, CMP, ECI, EPI, MODE)           \
    __PWM_INIT(5, M, N, PER, LT, HT, CMP, ECI, EPI, MODE);

#define PWM6_INIT(M, N, PER, LT, HT, CMP, ECI, EPI, MODE)           \
    __PWM_INIT(6, M, N, PER, LT, HT, CMP, ECI, EPI, MODE);

#define PWM7_INIT(M, N, PER, LT, HT, CMP, ECI, EPI, MODE)           \
    __PWM_INIT(7, M, N, PER, LT, HT, CMP, ECI, EPI, MODE);

#define PWM8_INIT(M, N, PER, LT, HT, CMP, ECI, EPI, MODE)           \
    __PWM_INIT(8, M, N, PER, LT, HT, CMP, ECI, EPI, MODE);


/*
 * Initialize PWM unit in a Slave mode
 */
#define __PWM_INIT_SLAVE(U, LT, HT, CMP, ECI, EPI, MODE)            \
do  {                                                               \
    PWM##U##_CMP  = CMP;                                            \
    PWM##U##_LT   = LT;                                             \
    PWM##U##_HT   = HT;                                             \
    PWM##U##_CTRL = 3|((ECI)<<4)|((EPI)<<3)|((MODE)<<2);            \
} while(0)

#define PWM1_INIT_SLAVE(LT, HT, CMP, ECI, EPI, MODE)                \
    __PWM_INIT_SLAVE(1, LT, HT, CMP, ECI, EPI, MODE);

#define PWM2_INIT_SLAVE(LT, HT, CMP, ECI, EPI, MODE)                \
    __PWM_INIT_SLAVE(2, LT, HT, CMP, ECI, EPI, MODE);

#define PWM3_INIT_SLAVE(LT, HT, CMP, ECI, EPI, MODE)                \
    __PWM_INIT_SLAVE(3, LT, HT, CMP, ECI, EPI, MODE);

#define PWM4_INIT_SLAVE(LT, HT, CMP, ECI, EPI, MODE)                \
    __PWM_INIT_SLAVE(4, LT, HT, CMP, ECI, EPI, MODE);

#define PWM5_INIT_SLAVE(LT, HT, CMP, ECI, EPI, MODE)                \
    __PWM_INIT_SLAVE(5, LT, HT, CMP, ECI, EPI, MODE);

#define PWM6_INIT_SLAVE(LT, HT, CMP, ECI, EPI, MODE)                \
    __PWM_INIT_SLAVE(6, LT, HT, CMP, ECI, EPI, MODE);

#define PWM7_INIT_SLAVE(LT, HT, CMP, ECI, EPI, MODE)                \
    __PWM_INIT_SLAVE(7, LT, HT, CMP, ECI, EPI, MODE);

#define PWM8_INIT_SLAVE(LT, HT, CMP, ECI, EPI, MODE)                \
    __PWM_INIT_SLAVE(8, LT, HT, CMP, ECI, EPI, MODE);

/*
 * Enable/disable interrupts from PWM unit
 */
#define __PWM_INT_ENABLE(n)     \
    do  {                       \
        XI2_PEND  = 1u << (n);  \
        XI2_MASK |= 1u << (n);  \
    } while(0)

#define __PWM_INT_DISABLE(n)    \
    ( XI2_MASK &= ~(1u << (n)) )

#define PWM1_CMPI_ENABLE()  __PWM_INT_ENABLE(15)
#define PWM1_CMPI_DISABLE() __PWM_INT_DISABLE(15)

#define PWM1_CNTI_ENABLE()  __PWM_INT_ENABLE(14)
#define PWM1_CNTI_DISABLE() __PWM_INT_DISABLE(14)

#define PWM2_CMPI_ENABLE()  __PWM_INT_ENABLE(13)
#define PWM2_CMPI_DISABLE() __PWM_INT_DISABLE(13)

#define PWM2_CNTI_ENABLE()  __PWM_INT_ENABLE(12)
#define PWM2_CNTI_DISABLE() __PWM_INT_DISABLE(12)

#define PWM3_CMPI_ENABLE()  __PWM_INT_ENABLE(11)
#define PWM3_CMPI_DISABLE() __PWM_INT_DISABLE(11)

#define PWM3_CNTI_ENABLE()  __PWM_INT_ENABLE(10)
#define PWM3_CNTI_DISABLE() __PWM_INT_DISABLE(10)

#define PWM4_CMPI_ENABLE()  __PWM_INT_ENABLE(9)
#define PWM4_CMPI_DISABLE() __PWM_INT_DISABLE(9)

#define PWM4_CNTI_ENABLE()  __PWM_INT_ENABLE(8)
#define PWM4_CNTI_DISABLE() __PWM_INT_DISABLE(8)

#define PWM5_CMPI_ENABLE()  __PWM_INT_ENABLE(7)
#define PWM5_CMPI_DISABLE() __PWM_INT_DISABLE(7)

#define PWM5_CNTI_ENABLE()  __PWM_INT_ENABLE(6)
#define PWM5_CNTI_DISABLE() __PWM_INT_DISABLE(6)

#define PWM6_CMPI_ENABLE()  __PWM_INT_ENABLE(5)
#define PWM6_CMPI_DISABLE() __PWM_INT_DISABLE(5)

#define PWM6_CNTI_ENABLE()  __PWM_INT_ENABLE(4)
#define PWM6_CNTI_DISABLE() __PWM_INT_DISABLE(4)

#define PWM7_CMPI_ENABLE()  __PWM_INT_ENABLE(3)
#define PWM7_CMPI_DISABLE() __PWM_INT_DISABLE(3)

#define PWM7_CNTI_ENABLE()  __PWM_INT_ENABLE(2)
#define PWM7_CNTI_DISABLE() __PWM_INT_DISABLE(2)

#define PWM8_CMPI_ENABLE()  __PWM_INT_ENABLE(1)
#define PWM8_CMPI_DISABLE() __PWM_INT_DISABLE(1)

#define PWM8_CNTI_ENABLE()  __PWM_INT_ENABLE(0)
#define PWM8_CNTI_DISABLE() __PWM_INT_DISABLE(0)

#endif /* PWMLIB_H_ */
