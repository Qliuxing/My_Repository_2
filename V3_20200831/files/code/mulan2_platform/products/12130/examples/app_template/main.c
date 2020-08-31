/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <ioports.h>
#include <syslib.h>


#include <mmu_api.h>
#include "task1.h"
#include "task2.h"


/*
 *
 */
int main (void)
{

    MMU_Init();

    /*
     *  ... project specific initialization
     */


    MMU_RunAs (MMU_TASK1_ID,            /* task ID                      */ 
               Task1_Init,              /* function                     */
               0,                       /* argument (uint16_t)          */
               TASK1_INIT_STACK_SIZE);  /* stack size for the function  */

    MMU_RunAs (MMU_TASK2_ID, 
               Task2_Init,
               0,
               TASK2_INIT_STACK_SIZE);


    while (1) {     /* main loop */

        
    }

    return 0;
}

/* EOF */
