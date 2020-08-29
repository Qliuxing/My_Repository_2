/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <syslib.h>

/*
 * Function premain() is invoked by start-up code just before calling main()
 * At this point all runtime-initialization is done thus variables/objects 
 * get their initial values
 *
 * NOTE: Function with the same name (i.e. premain) linked from
 * application directory overrides this function
 */
void _premain (void)
{

    /* ... */

    SET_PRIORITY(7);        /* System mode, low priority (7) */
}

/* EOF */
