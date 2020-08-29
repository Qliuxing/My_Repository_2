/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Application:
 *   Blinking LED on KEY0 pin
 */
#include <syslib.h>
#include <plib.h>       /* product libs */

/*
 * Prototypes
 */
static void LED_Init (void);
static void LED_On (void);
static void LED_Off (void);
static void LED_Toggle (void);

/*
 * Main
 */
int main (void)
{
    LED_Init();

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
    KEY_PULL = 1u << 0;     /* pull-up KEY0             */
    LED_On();
    LED_Off();
}


/*
 * LED On (low level on output)
 */
static void LED_On (void)
{
    KEY_OD |= 1u << 0;      /* low level output */
}

/*
 * LED Off (high level on output)
 */
static void LED_Off (void)
{
    KEY_OD &= ~(1u << 0);   /* Z-state output (high level via pull-up) */
}

/*
 * Toggle LED
 */
static void LED_Toggle (void)
{
    KEY_OD ^= 1u << 0;
}

/* EOF */
