/*
 * Copyright (C) 2011-2013 Melexis N.V.
 *
 * Software Platform
 *
 */

/* 
 * PWM test example

 * Notes:
 *  1. Generates PWM waveform on pin HV2 pin with frequncy equal to FPLL/1000 =
 *     = 20MHz/1000 = 20 KHz (i.e. period is 50us) to check RC oscillator
 *     trimming and PLL activation/stability. Duty cycle is 25%.
 *     To scope the waveform on HV2 pin emulator should be disconnected and external
 *     pull-up resistor should be connected to HV2.
 */

#include <syslib.h>
#include <plib.h>       /* product libs */
#include <pwmlib.h>

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
