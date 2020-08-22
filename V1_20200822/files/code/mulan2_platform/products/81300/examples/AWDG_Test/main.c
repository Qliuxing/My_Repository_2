/*
 * Copyright (C) 2011-2015 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Test of analogue watchdog acknowledgment.
 * If watchdog is acknowledged with period less than 200 us
 * then bit AWD_WRITE_FAIL is set
 */
#include <syslib.h>

/*
 * Prototypes
 */
void awdg_ack (void);

volatile uint16_t low_period  = 10;
volatile uint16_t high_period = 1000;

/*
 * Main
 */
int main (void)
{
    uint16_t i = low_period;

    IO_EXTIO = IO4_ENABLE | IO5_ENABLE;

    while (1) {

        IO_EXTIO |= IO5_OUT;
        awdg_ack();
        IO_EXTIO &= ~IO5_OUT;

        DELAY(i);

        i++;
        if (i > high_period) {
            i = low_period;
        }

    }

    return 0;
}

/*
 * awdg_ack
 */
void awdg_ack (void)
{
    uint16 temp;

    temp  = AWD_CTRL;
    temp |= AWD_WRITE_FAIL;                 /* clean (by writing 1) sync error flag */
    AWD_CTRL = temp;

    if (AWD_CTRL & AWD_WRITE_FAIL) {        /* Toggle IO4 if sync error */
        IO_EXTIO ^= IO4_OUT;
    }
}

/* EOF */
