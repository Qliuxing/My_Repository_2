/*
 * Copyright (C) 2013-2014 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <syslib.h>


#define USER_MODE   (1u << 3)

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

    SET_PRIORITY(USER_MODE | 7);    /* User mode, low priority (7) */
}

/* EOF */
