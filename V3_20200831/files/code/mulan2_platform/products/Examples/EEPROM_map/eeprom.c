/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <ioports.h>
#include <syslib.h>

#include "eeprom_map.h"

eeprom_t ee __attribute__((ep)) =
{
    .patch0_a = 0x5678,
    .patch0_i = 0x1234,

    .crc8 = 0x5A,

    .rom_checksum = 0xAAAA,

	.direction = {
		.enable = 1,
		.right  = 5,
		.left   = 5,
	},
};

/* EOF */
