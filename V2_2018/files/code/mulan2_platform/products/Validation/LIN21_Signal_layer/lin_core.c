/*
 * LIN Core API
 * Signal Interaction and Notification
 *
 * This file is application specific and depends on LDF/NCF files
 * Generated by configuration tool C:\EVWS\_LIBRARY_PLATFORMS\library_platform_mulan2\bin\ldf_nodegen.exe (version 1.4.0)
 *
 * Copyright (C) 2007-2015 Melexis N.V.
 */

#include "lin_api.h"


#if LIN_VERSION >= 20
#pragma space nodp
static ml_uint16 volatile ifcStatus;
#pragma space none
#endif /* LIN_VERSION >= 20 */


/*
 *****************************************************************************
 * l_sys_init
 *
 * Performs the initialization of the LIN core: clears signals flag, error
 * counters, messages status etc.
 * The call to the l_sys_init is the first call a user must use in the
 * LIN core before using any other API functions.
 *
 * Returns: Zero If the initialization succeeded
 *          Non-zero If the initialization failed
 *****************************************************************************
 */
l_bool l_sys_init (void)
{
    return 0U;
}

/*
 *****************************************************************************
 * l_ifc_init_i1
 *
 * Initializes the LIN interface i1, i.e. sets up internal functions such
 * as the baud rate. This is the first call a user must perform, before using
 * any other interface related LIN API functions.
 *
 * Returns:
 *     LIN2.0: none
 *     LIN2.1: 0 if success, non-zero if failed
 *****************************************************************************
 */
#if (LIN_VERSION == 20) || (LIN_VERSION == 13)
void l_ifc_init_i1 (void)
#elif (LIN_VERSION >= 21)
l_bool l_ifc_init_i1 (void)
#else
#error "LIN_VERSION not supported"
#endif /* LIN_VERSION */
{
    (void)ml_InitLinModule();

    ml_SetAutoBaudRateMode(ML_ABR_ON_FIRST_FRAME);  /* set autobaudrate detection on first frame */

    /* Configure the Mlx4 software
     *   IDStopBitLength : 0 (default)/1/2/3 -> 1 / 1.5 / 2 / 2.5 stop bits
     *   TXStopBitLength : 0 (default)/1     -> 1 / 2 stop bits
     *   StateChangeSignal : enabled (default)/disabled
     *   SleepMode:
     *        ML_LIGHTSLEEP - firmware wakeup detection with timeout
     *        ML_DEEPSLEEP  - hardware wakeup detection with timeout
     */
    (void)ml_SetOptions(1U, 0U, ML_ENABLED, ML_LIGHTSLEEP);

    /*
     * SlewRate:
     *  ML_SLEWHIGH for baudrates above 15kbps
     *  ML_SLEWLOW  for baudrates below 15kbps
     *  ML_SLEWFAST for fast protocol mode
     */
    (void)ml_SetSlewRate((LDF_BAUDRATE < 15000) ? ML_SLEWLOW : ML_SLEWHIGH);

#if LIN_VERSION >= 20
    ml_DiagInit();  /* initialize diagnostics layer */
#endif

#if LIN_VERSION >= 21
    return (l_bool)ml_Connect();
#endif /* LIN_VERSION >= 21 */
}

/*
 *****************************************************************************
 * l_sys_irq_disable
 *
 * Disables LIN communication interrupts
 *
 * Returns: LIN interrupts state before the function call
 *****************************************************************************
 */
l_irqmask l_sys_irq_disable (void)
{
    l_irqmask mask;


    mask = GET_STATUS_MLX4_INT();
    DISABLE_MLX4_INT();
    return mask;
}

/*
 *****************************************************************************
 * l_sys_irq_restore
 *
 * Restores LIN interrupts state which was saved in 'previous'
 *
 * Returns: none
 *****************************************************************************
 */
void l_sys_irq_restore (l_irqmask previous)
{
    if (previous != 0U) {
        ENABLE_MLX4_INT();
    }
    else {
        DISABLE_MLX4_INT();
    }
}

#if (LIN_VERSION >= 20)
/*
 *****************************************************************************
 * l_ifc_read_status_i1
 *
 * This function will return the status of the previous communication.
 * The call is a read-reset call; meaning that after the call has returned,
 * the status word is set to 0
 *
 * Returns:  The call returns the status word (16 bit value)
 *  Last frame PID  [15:8]
 *  Save configuration  [6]
 *  Event triggered frame collision [5]
 *  Bus activity    [4]
 *  Goto to Sleep   [3] 
 *  Overrun [2]
 *  Successful transfer [1]
 *  Error in response [0]
 *****************************************************************************
 */
l_u16 l_ifc_read_status_i1 (void)
{
    ml_uint16 d;
    l_irqmask m;

 
    m = l_sys_irq_disable();

#if LIN_VERSION >= 21
    ml_GetState(ML_CLR_LIN_BUS_ACTIVITY);
    if ((LinStatus & ML_LIN_BUS_ACTIVITY) != 0U)
    {
        /* bus activity detected */
        /* set this bit in status word */
        ifcStatus |= ML_IFC_BUS_ACTIVITY;
    }
#endif /* LIN_VERSION >= 21 */

    d = ifcStatus;
    /*
     * The call is a read-reset call; meaning that after the call
     * the status word is set to 0
     */
    ifcStatus = 0U;
    l_sys_irq_restore (m);
    
    return d;
}

/*
 *****************************************************************************
 * l_ifc_write_status_flags_i1
 *
 * This function writes the interface status flags
 *****************************************************************************
 */
void l_ifc_write_status_flags_i1 (l_u8 flags)
{
    ifcStatus |= flags;
}

/*
 *****************************************************************************
 * l_ifc_write_status_pid_i1
 *
 * This function writes Protected ID of the last frame to the interface
 * status word. Last frame PID is set simultaneously with successful transfer
 * or error in response bits of the status word.
 * Thus the l_ifc_write_status_pid_i1 is called from:
 *  - mlu_MessageReceived
 *  - mlu_DataTransmitted
 *  - mlu_ErrorDetected
 *****************************************************************************
 */
void l_ifc_write_status_pid_i1 (l_u8 pid)
{
    /*
     * Mask status flags (LSB part of the ifcStatus) and check last PID (MSB
     * part of the ifcStatus). If last PID is not 0, then there were some
     * unread PIDs since last call to l_ifc_read_status
     */
    if ((ifcStatus & 0xFF00U) != 0U) {
        ifcStatus |= ML_IFC_OVERRUN;    /* set OVERRUN flag */
        ifcStatus &= 0x00FFU;           /* clear previous PID */
    }
    ifcStatus |= (((ml_uint16) pid) << 8);   /* set new PID */
}
#endif /* LIN_VERSION >= 20 */


/* EOF */
