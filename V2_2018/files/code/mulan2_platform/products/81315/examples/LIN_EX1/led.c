/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Software Platform
 *
 */


#include <syslib.h>
#include "led.h"

/**
 * Initialize LED
 */
void LED_Init (void)
{
    IO_EXTIO = IO4_ENABLE;
}


/**
 * Toggle LED 
 */
void LED_Toggle (void)
{
    IO_EXTIO ^= IO4_OUT;
}


/**
 * On LED
 */
void LED_On (void)
{
    IO_EXTIO |= IO4_OUT;
}


/**
 * Off LED
 */
void LED_Off (void)
{
    IO_EXTIO &= ~IO4_OUT;
}

/* EOF */
