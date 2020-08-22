/*
 * Copyright (C) 2011-2015 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Application:
 *   Blinking LED on IO4
 */
#include <syslib.h>
#include <plib.h>       /* product libs */

#include <lin.h>

/*
 * Prototypes
 */
static void LED_Init (void);
static void LED_Toggle (void);

/*
 * Main
 */
int main (void)
{
    LED_Init();

    /*
     * Init and start standalone loader
     */
    ml_InitLinModule();
    ml_SetFastLoaderBaudRate();                 /* default Fast Loader baud rate : defined by ML_FAST_BAUDRATE */
    (void)ml_SetSlewRate(ML_SLEWFAST);
    ml_Connect();

    while (1) {
        WDG_Manager();
        LED_Toggle();
        MSEC_DELAY(100);
    }

    return 0;
}


/*
 * Initialize LED
 */
static void LED_Init (void)
{
    IO_EXTIO = IO4_ENABLE;
}


/*
 * Toggle LED
 */
static void LED_Toggle (void)
{
    IO_EXTIO ^= IO4_OUT;
}


/* ----------------------------------------------------------------------------
 *  LIN API event: mlu_ApplicationStop
 */
ml_Status mlu_ApplicationStop(void)
{
	return ML_SUCCESS;  /* return that the application has stopped */
}

/* EOF */
