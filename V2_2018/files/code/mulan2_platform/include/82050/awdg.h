/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Analog Watchdog functions
 * 
 * NOTES:
 *  1. The Watchdog is clocked from completely separate 10kHz
 *     oscillator (AWD_CK).
 *  2. The watchdog automatically starts after reset with maximum timeout
 *     delay equal to: 1/10 kHz * 256 * 64 = 1.638 sec
 *  3. The application has no possibility to stop this analog watchdog.
 *  4. The analog watchdog shares its 2 interrupt sources (RST_WD_IT and
 *     WD_ATT_IT) with the digital watchdog included in MULAN2. The AWD_ATT
 *     and AWD_RST flags can be read to determine, that the watchdog reset
 *     was caused by analog watchdog.
 *  5. After half of the timeout (tAWD_TIMEOUT/2) the attention interrupt
 *     (AWD_ATT) is generated. When the timer reaches the AWD_TIMEOUT,
 *     a reset is issued.
 *  6. Two consecutive writes to AWD register must not follow faster than
 *     one period of AWD_CK, otherwise the status flag AWD_WRITE_FAIL will
 *     be set and the AWD_TIMER or AWD_CKDIV registers will not be updated.
 */
#ifndef AWDG_H_
#define AWDG_H_

#include <syslib.h>

/* Prescaler values */
#define AWDG_DIV_64      0
#define AWDG_DIV_16      1
#define AWDG_DIV_4       2
#define AWDG_DIV_1       3

/*
 *****************************************************************************
 * awdg_init
 * Reinitialize analog watchdog for a new timeout
 *
 *      AWD_timeout = 1/10 kHz * Prescaler *  Timer
 *
 *****************************************************************************
 */
__MLX_TEXT__  static INLINE void awdg_init (uint8 prescaler, uint8 timer)
{
    AWD_CTRL = ((uint16)(prescaler & 3) << 8) | timer;
}

/*
 *****************************************************************************
 * awdg_restart
 * Restart (acknowledge) analog watchdog
 * 
 * NOTES:
 *  1. Should be called with period > 200us, otherwise bit AWD_WRITE_FAIL will
 *     be set and further acknowledgment will fail during next 200 us
 *****************************************************************************
 */
__MLX_TEXT__  static INLINE void awdg_restart (void)
{
    uint16 temp;    /* use temp variable as a workaround for compiler issue [MLXCOMP-17] */

    temp = AWD_CTRL;
    AWD_CTRL = temp;
}

/*
 *****************************************************************************
 * awdg_get_reset_flag
 * Returns watchdog reset flag which memorises a analogue watchdog reset
 * Read only, cleared  by POR or WAKE UP or read
 *****************************************************************************
 */
__MLX_TEXT__  static INLINE uint8 awdg_get_reset_flag (void)
{
    return (AWD_CTRL & AWD_RST);
}

/*
 *****************************************************************************
 * awdg_get_info_flag
 * Returns watchdog info flag which memorises attention interrupt
 * Read only, cleared  by POR or WAKE UP or read
 *****************************************************************************
 */
__MLX_TEXT__  static INLINE uint8 awdg_get_info_flag (void)
{
    return (AWD_CTRL & AWD_ATT);
}

#endif /* AWDG_H_ */
