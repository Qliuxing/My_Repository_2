/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * PWM test example
 *
 * Generates PWM waveform with frequency FPLL/1000 and duy cycle 25% on W (PWM2) output
 */
#include <syslib.h>
#include <awdg.h>
#include <pwmlib.h>

static void hw_init (void);

/*
 *
 */
int main (void)
{
    hw_init();

    while (1) {         /* idle loop */
        awdg_restart(); /* Restart watchdog */
    }

    return 0;
}

/*
 *
 */
static void hw_init (void)
{
    PWM2_INIT (1,           /* Fck / 1                          */
               0,           /* no predivider                    */
               1000,        /* period                           */
               1, 251,      /* thresholds LT, HT                */
               0,           /* compare is not used              */
               0,           /* compare interrupt (ECI) disabled */
               0,           /* period interrupt (EPI) disabled  */
               PWM_INDEPENDENT_MODE);

    /*
     * Connect drivers
     */
    DRVCFG = DIS_OT | DIS_OC | DIS_UV | DIS_OV
            | DRV_CFG_U_PWM | DRV_CFG_V_PWM | DRV_CFG_W_PWM;
}

/* EOF */
