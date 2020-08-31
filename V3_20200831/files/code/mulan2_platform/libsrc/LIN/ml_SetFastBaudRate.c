/*
 * Copyright (C) 2015 Melexis N.V.
 *
 * Software Platform
 */

#include <lin.h>
#include <mathlib.h>
#include <syslib.h>

/* ----------------------------------------------------------------------------
 * Set custom baudrate for Fast Protocol
 *
 * \param FastBaudRate  Required baudrate[kBd]
 *
 * Input (global):
 *      FPLL            System clock frequency[kHz]
 *
 */
__MLX_TEXT__ void ml_SetFastBaudRate (uint8_t FastBaudRate)
{
    /*
     * In the fast mode the split mode of the baudrate counter is used.
     * In the split mode baudrate counter is acts as two chained counters Cpt[5:0]
     * and Cpt[7:6]
     * Cpt[7:6] should be 0..3 (always fixed to 2 in this implementation)
     * Cpt[5:0] should be 0..63
     *
     *                                             PLL_in_Hz
     *  Baudrate_fast_bps = --------------------------------------------------------------
     *                        2^(1 + ml_FastPresc) * 2 * (ml_CptHi + 1) * ml_CptLowTemp
     */

    uint16_t ml_CptHi = 2;

    uint16_t ml_CptLowTemp = divU16_U32byU16( (FPLL * 10), (FastBaudRate * 2 * (ml_CptHi + 1)) );

    uint8_t ml_FastPresc = 0;
    uint8_t ml_FastDivider;

    /* Searching of ml_FastPresc by ml_CptLowTemp */
    for (uint8_t presc_cnt = 0; presc_cnt < 3; presc_cnt++) {

        /* Divide by two if value out of range */
        if (ml_CptLowTemp >= 635) {
            ml_CptLowTemp >>= 1;
        }
        /* Otherwise set ml_FastPresc and finish */
        else {
            ml_FastPresc = (presc_cnt - 1) & 0xF;
            break;
        }
    }

    /* Evaluate ml_FastDivider according to ml_CptLowTemp value*/
    ml_FastDivider = 64 * ml_CptHi + divU16_U32byU16((ml_CptLowTemp + 5), 10);

    /* Setting up the Fast Protocol Baudrate */
    ml_SetBaudRate(ml_FastPresc, ml_FastDivider);
}

/* EOF */
