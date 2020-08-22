/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <syslib.h>

/*
 * Keep .app_version in loader empty
 */
const uint32 ml_ldr_version __attribute__((section(".app_version"))) = 0xFFFFFFFFUL;


int main (void)
{
    /*
    * The main is never called from crt0 in the loader
    */

	return 0;
}

/* EOF */
