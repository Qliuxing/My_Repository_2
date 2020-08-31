/*
 * Copyright (C) 2008-2015 Melexis N.V.
 *
 * Software Platform
 * Product/board specific functions
 *
 */

#ifndef LIB_MLX81315_H_
#define LIB_MLX81315_H_

#include <syslib.h>

/*
 * Macro for generating delays in microseconds with fixing according to flash wait
 * states. This is the analog of the same from syslib.h: DELAY_US().
 */
#if (FPLL <= 20000) /* if CPU clock <= 20 MHz */
    #define MLX813xx_USEC_DELAY(us) DELAY((FPLL * (uint32)(us) + 2000) / 4000)

    static INLINE void MLX813xx_MSEC_DELAY(int16 msec)
    {
        int16 i;
        for(i = 0; i < msec; i++)
        {
            __asm__ __volatile__ (
                "mov X, %[cnt]\n\t"
                "djnz X,."
                :
                : [cnt] "i" (FPLL/4)
                : "X"
            );
        }
    }
#else
    #define MLX813xx_USEC_DELAY(us) DELAY((FPLL * (uint32)(us) + 2000) / 5000)

    static INLINE void MLX813xx_MSEC_DELAY(int16 msec)
    {
        int16 i;
        for(i = 0; i < msec; i++)
        {
            __asm__ __volatile__ (
                "mov X, %[cnt]\n\t"
                "djnz X,."
                :
                : [cnt] "i" (FPLL/5)
                : "X"
            );
        }
    }
#endif

#endif /* LIB_MLX81315_H_ */
