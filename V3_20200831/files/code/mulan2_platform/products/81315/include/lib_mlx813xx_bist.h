/*
 * Copyright (C) 2008-2015 Melexis N.V.
 *
 * Software Platform
 * Product/board specific functions
 *
 */

#ifndef LIB_MLX81315_BIST_H_
    #define LIB_MLX81315_BIST_H_

    #include <syslib.h>
    #include <wdlib.h>

    #define C_CHIP_HEADER                       (0x8000U | ((('M'-'@')<<10) | (('L'-'@')<<5) | ('X'-'@')))

    #define C_CHIP_STATE_COLD_START             0x0000U
    #define C_CHIP_STATE_WATCHDOG_RESET         ((('W'-'@')<<10) | (('D'-'@')<<5) | ('R'-'@'))  /* Watchdog reset (Warm Start) */
    #define C_CHIP_STATE_FATAL_RECOVER_ENA      ((('F'-'@')<<10) | (('R'-'@')<<5) | ('E'-'@'))  /* Fatal Recovery Enabled */
    #define C_CHIP_STATE_FATAL_CRASH_RECOVERY   ((('F'-'@')<<10) | (('C'-'@')<<5) | ('R'-'@'))  /* Fatal Crash Recovery */
    #define C_CHIP_STATE_LIN_CMD_RESET          ((('L'-'@')<<10) | (('C'-'@')<<5) | ('R'-'@'))  /* LIN Command (chip) Reset */
    #define C_CHIP_STATE_LOADER_PROG_RESET      ((('L'-'@')<<10) | (('P'-'@')<<5) | ('R'-'@'))  /* Loader enter Programming mode Reset */
    #define C_CHIP_STATE_UV_RESET               ((('U'-'@')<<10) | (('V'-'@')<<5) | ('R'-'@'))  /* UV-reset */


    extern volatile uint16 bistHeader    __attribute__((section(".bist_stat")));
    extern volatile uint16 bistResetInfo __attribute__((section(".bist_stat")));
    extern volatile uint16 bistError     __attribute__((section(".bist_stat")));
    extern volatile uint16 bistErrorInfo __attribute__((section(".bist_stat")));


    /*
     *****************************************************************************
     * bist_CheckReset
     * Check the reason of the previous reset and store in non initialized memory
     *
     *****************************************************************************
     */
    __MLX_TEXT__  static INLINE void bist_CheckReset (void)
    {
        if ( bistHeader == C_CHIP_HEADER )                                     /* Chip header is valid and chip successfully initialized by ... */
        {
            if ( ((AWD_CTRL & AWD_RST) != 0) || WD_BOOT_CHECK() )              /* Check both Analog Watchdog and Digital Watchdog */
            {
                if ( (bistResetInfo != C_CHIP_STATE_LIN_CMD_RESET) &&
                     (bistResetInfo != C_CHIP_STATE_LOADER_PROG_RESET) )
                {
                    if ( (bistResetInfo == C_CHIP_STATE_COLD_START) ||         /* Chip-state is Cold-start or (pass including RAM-init) or ... */
                         (bistResetInfo == C_CHIP_STATE_FATAL_RECOVER_ENA) )   /* ... Chip-state is fatal-recovery enabled (fully initialized) */
                    {
                        bistResetInfo = C_CHIP_STATE_WATCHDOG_RESET;
                    }
                    else
                    {
                        bistResetInfo = C_CHIP_STATE_COLD_START;
                    }
                }
                else
                {
                    /* LIN Command Reset
                     * or
                     * Loader enter Programming mode Reset */
                }
            }
            else if ( (bistResetInfo != C_CHIP_STATE_LIN_CMD_RESET) &&         /* LIN Command Reset must be passed through Analod Watchdog reset */
                      (bistResetInfo != C_CHIP_STATE_LOADER_PROG_RESET) )      /* Loader Prog Reset must be passed through Analog Watchdog reset */
            {
                bistResetInfo = C_CHIP_STATE_COLD_START;
            }
            else if ( bistResetInfo == C_CHIP_STATE_FATAL_RECOVER_ENA )
            {
                bistResetInfo = C_CHIP_STATE_UV_RESET;
            }
        }
        else
        {
            bistResetInfo = C_CHIP_STATE_COLD_START;
            bistHeader = C_CHIP_HEADER;
        }
    }

#endif /* LIB_MLX81315_BIST_H_ */
