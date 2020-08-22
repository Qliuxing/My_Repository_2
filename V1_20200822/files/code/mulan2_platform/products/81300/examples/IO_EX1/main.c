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
#include <awdg.h>

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

    while (1) {
        awdg_restart();
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

/* EOF */
