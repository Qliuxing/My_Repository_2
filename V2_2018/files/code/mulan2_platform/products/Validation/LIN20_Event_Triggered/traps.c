/*
 * Copyright (C) 2011-2015 Melexis N.V.
 *
 * Software Platform
 *
 * Interrupt traps
 */
#include <plib.h>

__MLX_TEXT__  __attribute__((interrupt))
void STACK_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}


__MLX_TEXT__  __attribute__((interrupt))
void PROT_ERR_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

__MLX_TEXT__  __attribute__((interrupt))
void INV_AD_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

__MLX_TEXT__  __attribute__((interrupt))
void PROG_ERR_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

__MLX_TEXT__  __attribute__((interrupt))
void EXCHANGE_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

__MLX_TEXT__  __attribute__((interrupt))
void TASK_RST_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}


__MLX_TEXT__  __attribute__((interrupt))
void WD_ATT_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

__MLX_TEXT__  __attribute__((interrupt))
void M4_MUTEX_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}


__MLX_TEXT__  __attribute__((interrupt))
void TIMER_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

__MLX_TEXT__  __attribute__((interrupt))
void ADC_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

__MLX_TEXT__  __attribute__((interrupt))
void EE_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

__MLX_TEXT__  __attribute__((interrupt))
void EXT0_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

__MLX_TEXT__  __attribute__((interrupt))
void EXT1_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

__MLX_TEXT__  __attribute__((interrupt))
void EXT2_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

__MLX_TEXT__  __attribute__((interrupt))
void EXT3_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

__MLX_TEXT__  __attribute__((interrupt))
void EXT4_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

__MLX_TEXT__  __attribute__((interrupt))
void SOFT_IT (void)
{
    for (;;) {
        WDG_Manager();

    }
}

/* EOF */
