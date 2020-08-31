/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <syslib.h>

/*
 * Linker symbols (these objects are not created in the memory)
 */
extern uint16 _bss_dp_start;
extern uint16 _bss_dp_end;

extern uint16 _bss_start;
extern uint16 _bss_end;

extern uint16 _data_load_start;

extern uint16 _data_dp_start;               /* .data.dp RAM             */
extern uint16 _data_dp_end;

extern uint16 _data_start;                  /* .data RAM                */
extern uint16 _data_end;

/*
 * C runtime initialization. Initializes .bss and .data RAM sections
 *
 *
 * NOTES:
 * Function should NOT be static and/or inlined otherwise space
 * on stack for local variables r and w could be allocated before calling
 * SET_STACK() in start()
 */
void _ram_section_init (void)
{
    uint16 *w;
#if !defined (RAM_APPLICATION)
    uint16 *r;
#endif /* !RAM_APPLICATION */


    /* clear .bss and .dp.bss sections */
    for (w = &_bss_dp_start; w < &_bss_dp_end; ) {
        *w++ = 0;
    }

    for (w = &_bss_start; w < &_bss_end; ) {
        *w++ = 0;
    }

#if !defined (RAM_APPLICATION)  /* skip ROM-to-RAM loading for RAM application */
    /* initialize .data and .dp.data sections */
    r = &_data_load_start;      /* copy data from rom */

    for (w = &_data_dp_start; w < &_data_dp_end; ) {
        *w++ = *r++;
    }

    for (w = &_data_start; w < &_data_end; ) {
        *w++ = *r++;
    }
#endif /* !RAM_APPLICATION */

}

/* EOF */
