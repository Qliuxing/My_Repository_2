/*
 * Copyright (C) 2013 Melexis N.V.
 *
 * Software Platform
 *
 */
#ifndef LOADER_STATE_
#define LOADER_STATE_

#include <syslib.h>
#include <lin.h>        /* LIN driver types: ml_uintX */


typedef struct {
    uint8_t app_disabled;
    uint8_t app_disabled_padding[3];    /* no other data allowed in the same 32-bit unit (enforced by Flash ECC) */

    uint8_t app_enabled;
    uint8_t app_enabled_padding[3];     /* no other data allowed in the same 32-bit unit (enforced by Flash ECC) */

    uint8_t  dummy[120];                /* padding flags to the size of the Flash page (128 bytes) */
} LoaderFlagsPage_Type;

extern const volatile LoaderFlagsPage_Type loader_flags;
extern const volatile uint16_t loader_rst_state;

extern ml_uint8 LDR_GetState (void);


#endif /* LOADER_STATE_ */
