/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Math Library
 *
 */


#include <syslib.h>

/*
 *  Stubs to be used instead of products functions for simulations
 */


void _low_level_init (void)
{

}


void premain(void)
{
    SET_PRIORITY(7);        /* System mode, low priority (7) */
}

/* EOF */
