/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * PWM test example
 *
 * Generates PWM waveform with frequency FPLL/1000 and duy cycle 25%
 * on the PWM1 output
 */
#include <syslib.h>
#include <plib.h>       /* product libs */
#include <pwmlib.h>


static void hw_init (void);

/*
 *
 */
int main (void)
{
	hw_init();

    while (1) {            /* idle loop */
        WDG_Manager();
    }

    return 0;
}

/*
 *
 */
static void hw_init (void)
{

    PWM1_INIT (1,           /* Fck / 1                          */
               0,           /* no predivider                    */
               1000,        /* period                           */
               1, 251,      /* thershoulds                      */
               0,           /* compare is not used              */
               0,           /* compare interrupt (ECI) disabled */
               0,           /* pariod interrupt (EPI) disabled  */
               PWM_INDEPENDENT_MODE);

    ANA_OUTG = 7;   /* enable PS and PWM1, PWM2 */
}

/* EOF */
