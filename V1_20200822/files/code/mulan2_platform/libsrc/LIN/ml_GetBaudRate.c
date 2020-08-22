/*
 * Copyright (C) 2014-2015 Melexis N.V.
 *
 * Software Platform
 */

#include <lin.h>
#include <mathlib.h>
#include <syslib.h>

extern volatile const ml_uint8 LINBaud;
extern volatile const ml_uint8 LINPresc;

/*
 * Returns the LIN baud rate (in bps) obtained from the LIN module
 *
 * \note
 *  1. Formula:
 *
 *                        PLL_in_Hz         PLL_in_Hz / 2       PLL_in_Hz / 2
 *  Baudrate_in_bps = ----------------- = ----------------- = -----------------
 *                    2^(1+presc) * div     2^presc * div        div << presc
 * 
 * 
 *  2. Limitations:
 *
 *  2.1 Function ml_GetBaudRate shall only be called from mlu_DataRequest. Calling from other places
 *      (including the background main loop) might return an incorrect result.
 *
 *  2.2 Current implementation supports only FPLL < 50000
 *  To use divU16_U32byU16 function the second argument shall be 16-bit.
 *  div = 100..200, therefore presc shall be less than 8 to fit into 16-bit for
 *  (div << presc).
 *
 *  With this max presc 8 the limit for PLL is:
 *  PLL_max = 2^(pres+1) * div * baud = 2^9 * 100 * 1000 = 51.2 MHz
 */
__MLX_TEXT__ ml_uint16 ml_GetBaudRate (void)
{
#if (FPLL < 50000)

    uint8_t presc = (uint8_t)((LINPresc & 0x00F0) >> 4);  /* Prescaller |XXXX|XXXX|PRES|XXXX| */
    uint16_t baud = divU16_U32byU16((FPLL * 1000UL) / 2U, (uint16_t)LINBaud << presc);

    return baud;

#else
    #warning "Function can't work with FPLL > 50MHz (overflow in 2nd argument of divU16_U32byU16)"
    return 0U;
#endif
}

/* EOF */
