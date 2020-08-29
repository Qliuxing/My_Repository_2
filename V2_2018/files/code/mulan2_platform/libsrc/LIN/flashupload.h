/*
 * Copyright (C) 2005-2015 Melexis N.V.
 *
 * MelexCM Software Platform
 */
#ifndef FLASHUPLOAD_H_
#define FLASHUPLOAD_H_

#include <lin.h>
#include <pltf_version.h>
#include <loader_state.h>

/*
 * Default value of NAD for "Enter programming mode" frame (magic frame)
 * Application should not use this NAD
 */
#define MLX_NAD_DEFAULT     0x01

/* Possible values for ml_driver_mode */
enum {
    kLinAppMode    = 0x00U,
    kLinLoaderMode = 0x55U
};

/* Global variables */
extern ml_uint8 ml_driver_mode;                 /* LIN driver mode kLinAppMode or kLinLoaderMode  */

#if defined (LDR_HAS_PAGE_BUFFER_ON_STACK)
extern ml_uint8 *page_buffer;
#endif /* LDR_HAS_PAGE_BUFFER_ON_STACK */

/* Function prototype */
extern void ml_DiagRequest (void);
extern void ml_DiagReceived (void);

extern ml_bool ldr_isReadByIdMessage (const void *buffer, ml_bool CheckWildcard);
extern void ml_ldr_ReadByIdMessage (uint8_t Id);
extern void ml_ldr_SwitchToProgMode (ml_bool Reset);
extern void ml_ldr_ErrorDetected (ml_LinError Error);
extern void ml_SetFastBaudRate (uint8_t FastBaudRate);


/* ----------------------------------------------------------------------------
 * Returns SW Platform version
 *
 * E.g. 0x01060F00 represents version 1.6.15.0
 */
__MLX_TEXT__ static INLINE uint32_t ml_GetPlatformVersion (void)
{
    uint32_t version;

    version = ((uint32_t)__MLX_PLTF_VERSION_MAJOR__    << 24)
            | ((uint32_t)__MLX_PLTF_VERSION_MINOR__    << 16)
            | ((uint32_t)__MLX_PLTF_VERSION_REVISION__ <<  8)
            | ((uint32_t)__MLX_PLTF_VERSION_CUSTOMER_BUILD__);

    return version;
}

#endif /* FLASHUPLOAD_H_ */

/* EOF */
