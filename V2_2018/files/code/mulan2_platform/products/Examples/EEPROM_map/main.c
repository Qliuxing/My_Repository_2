/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Application template
 */
#include <ioports.h>
#include <syslib.h>

#include "eeprom_map.h"

volatile uint16_t v;


/*
 *
 */
int main (void)
{

    v = ee.patch0_a;
    v = ee.patch0_i;

    v = ee.crc8;

    v = ee.rom_checksum;

    v = ee.direction.left;
    v = ee.direction.right;
    v = ee.direction.enable;


    while (1) {
        /* main loop */
    }

    return 0;
}

/* EOF */
