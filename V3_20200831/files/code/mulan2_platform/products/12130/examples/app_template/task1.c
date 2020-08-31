/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Software Platform
 *
 */


#include <syslib.h>
#include <mmu_api.h>
#include "task1.h"

/*
 * Private RAM of the task
 *
 * \note
 *  1. Private RAM of the task is located in the dp section of the RAM (first 256 bytes of RAM).
 *     The full address access is not efficient for dp section and thus SHALL NOT be used.
 *     So, variables w/o dp attribute are not allowed in the private RAM of the task (checked
 *     in the linker script).
 *
 *  2. Variables from private RAM of the task are not initialized at startup,
 *     so they should be initialized in application before first usage.
 */

static uint16_t cnt_pr __DPAGE__;
static uint16_t cnt_tm __DPAGE__;

/* Interaction with background  */
volatile uint16_t task1_v_pr __DPAGE__;
volatile uint16_t task1_v_tm __DPAGE__;


/** ----------------------------------------------------------------------------
 *
 *  Initialization
 */
void Task1_Init (uint16_t arg)
{
    cnt_pr = arg;
    cnt_tm = arg;

    task1_v_pr = 0;
    task1_v_tm = 0;
}


/** ----------------------------------------------------------------------------
 *
 *  Pressure DSP
 */
void Task1_DSP_ProcessPressure (uint16_t arg)
{
    cnt_pr = arg;
    cnt_pr++;

    /* ...  */
    NOP();

    task1_v_pr = cnt_pr;
}

/** ----------------------------------------------------------------------------
 *
 *  Temperature DSP 
 */
void Task1_DSP_ProcessTemperature (uint16_t arg)
{
    cnt_tm = arg;
    cnt_tm++;

    /* ...  */
    NOP();

    task1_v_tm = cnt_tm;

}


/* EOF */
