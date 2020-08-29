/*
 * Copyright (C) 2011-2013 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * LIN test example
 * 
 *
 * Notes:
 *  1. Generates PWM waveform on pin HV2 pin with frequncy equal to FPLL/1000 =
 *     = 20MHz/1000 = 20 KHz (i.e. period is 50us) to check RC oscillator
 *     trimming and PLL activation/stability. Duty cycle is 25%.
 *     To scope the waveform on HV2 pin emulator should be disconnected and external
 *     pull-up resistor should be connected to HV2.
 *
 *  2. After ~4 seconds of the LIN bus inactivity test example starts generation of
 *     wake-up pulses. By measuring time between last frame and first wake-up pulse,
 *     LIN sleep timeout can be verified.
 *
 *  3. Only answers to LIN2.0 Product Id request (NAD, Supplier Id, Fucntion Id
 *     are defined in lin_cfg.h file):
 *  
 *      0x3C, 0x7F, 0x06, 0xB2, 0x00, 0xFF, 0x7F, 0xFF, 0xFF, CK13
 *      0x3D, ...
 *
 *  4. LIN2.0 Response Error bit is NOT implemented
 *
 * GLOSSARY:
 *
 *  FID  - Frame ID
 *  MRF  - Diagnostic Master Request Frame (FID = 0x3C)
 *  SRF  - Diagnostic Slave Response Frame (FID = 0x3D)
 */

#include <syslib.h>
#include <plib.h>       /* product libs */
#include <pwmlib.h>
#include "mlu.h"

/*
 * Local functions
 */
static void hw_init(void);


/* ----------------------------------------------------------------------------
 *
 */
int main (void)
{
    hw_init();
    LIN_init();

    while (1) {         /* idle loop */
        WDG_Manager();  /* Restart watchdog */
    }

    return 0;
}


/* ----------------------------------------------------------------------------
 *  Hardware init
 */
static void hw_init (void)
{

    PWM3_INIT (1,           /* Fck / 1                          */
               0,           /* no predivider                    */
               1000,        /* period                           */
               1, 251,      /* thresholds                       */
               0,           /* compare is not used              */
               0,           /* compare interrupt (ECI) disabled */
               0,           /* pariod interrupt (EPI) disabled  */
               PWM_INDEPENDENT_MODE);


    /*
     * Connect drivers
     */
    PWM3_CONNECT_HV2();


}

/* EOF */
