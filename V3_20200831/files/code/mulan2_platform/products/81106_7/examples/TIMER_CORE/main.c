/*
 * Copyright (C) 2013 Melexis N.V.
 *
 * Software Platform
 *
 */

/* 
 * Core timer test example
 *
 * HV2 pin shall be toggles every 1 ms from the Core Timer Interrupt
 *
 */
#include <ioports.h>
#include <syslib.h>
#include <coretimerlib.h>
#include <awdg.h>
#include <plib.h>

/*
 * Prototypes
 */
static void LED_Init (void);
static void LED_On (void);
static void LED_Off (void);
static void TMR_Init (void);

/*
 *
 */
int main (void)
{
    LED_Init();
    TMR_Init();

    for (;;) {
        awdg_restart();
    }

    return 0;
}  

/*
 * Initializes core timer
 */
static void TMR_Init (void)
{
    CORE_TIMER_INIT(1000);      /* period 1000 uS */
    ATOMIC_CODE(
            CORE_TIMER_INT_ENABLE(4);   /* enable timer's interrupt with SW priority 4 */
    );
}

/*
 * Initialize LED
 */
static void LED_Init (void)
{
    /* init */
}

/*
 * LED On
 */
static void LED_On (void)
{
    HV2_SET_OD();
}

/*
 * LED Off
 */
static void LED_Off (void)
{
    HV2_RESET_OD();
}

/*
 * Core Timer ISR
 */
__interrupt__ void CORE_TIMER_ISR (void)  /* priority inside interrupt is 3 */
{
    static uint8_t led_state __attribute__((dp));


    if ( 0 == led_state ) {  /* change output depending on previous state */
        LED_On();       
    }
    else {   
        LED_Off();
    }

    led_state++;        /* toggle the state */
    led_state &= 1;
}

/* EOF */
