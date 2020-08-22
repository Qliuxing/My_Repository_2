/*
 * Copyright (C) 2008 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <syslib.h>

/* Linker symbols (these objects are not created in the memory) */
extern uint16 stack;

/* Extern functions */
extern int  main(void);
extern void _low_level_init(void);
extern void _premain(void);
extern void _fatal(void);
extern void _ram_section_init (void);


/*
 * Start function
 * The function defines stack, executes low level initialization of the hardware,
 * C-runtime initialization and executes main function of the application
 *
 * \note:
 *  1. This function is called by Reset vector. System mode of the COU with highest
 *     priority (mov UPR, #0) is set before calling start()
 *
 *  2. Function directly changes the CPU stack pointer, thus creating any local
 *     variables/objects inside this function is FORBIDDEN!
 */
void start (void)
{
    SYS_clearCpuStatus();   /* Initialize M register.
                             * Note that UPR register (== M[11:8]) was already
                             * initialized during execution of the reset vector
                             * (see JMPVECTOR macro)
                             */

    PRIO = 0xFFFF;          /* set lowest priorties (undefined after reset) */

    SET_STACK(&stack);

    _low_level_init();
    _ram_section_init();    /* Initialize .data and .bss sections */
    _premain();
    (void)main();
    _fatal();
}

/* EOF */
