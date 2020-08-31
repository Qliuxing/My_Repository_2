/*
 * Copyright (C) 2011-2012 Melexis N.V.
 *
 * MULAN2 Software Platform
 */
#ifndef LIN_NAD_H_
#define LIN_NAD_H_

/* Type of structure for  NAD info (NAD value and security key) in fixed ram memory. */
typedef struct _stFixedRamNAD_
{
    uint32      key;
    ml_uint8    nad;

} ST_FIXED_RAM_NAD;

#if (LIN_PIN_LOADER != 0)

/* Variables and constants defined in lin.c file */

/* Global value. Current value of NAD for "Enter to Program Mode" */
extern ml_uint8  LIN_nad __attribute__ ((dp));

/*
 * Store NAD info (NAD value and security key) in fixed ram memory.
 * This NAD is accessed from Application, System, and LINLoader.
 */
extern volatile ST_FIXED_RAM_NAD stFixedRamNAD __attribute__ ((section(".ram_lin_fixed")));

/* Security key (magic value) to store NAD in ram between reprogramming steps */
static const uint32 _mlx_NAD_Security_Key = 0xE86172EF;

#endif /* LIN_PIN_LOADER */


#endif /* LIN_NAD_H_ */
