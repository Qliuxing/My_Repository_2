/*
 * Copyright (C) 2011 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <pltf_version.h>

.global mlx_pltf_version
.global mlx_enter_protected_mode

    /* -------------------------------------------------------------
     * SW platform version (defined in version.h)
     */
    .section .pltf_version,"a",@progbits
mlx_pltf_version:
    .byte __MLX_PLTF_VERSION_MAJOR__
    .byte __MLX_PLTF_VERSION_MINOR__
    .byte __MLX_PLTF_VERSION_REVISION__
    .byte __MLX_PLTF_VERSION_CUSTOMER_BUILD__

#if 0
    /* -------------------------------------------------------------
     * Product number
     */
    .section .product_no,"a",@progbits
product_no:
	.string PRODUCT_STR
#endif

    /* -------------------------------------------------------------
     * Enter System mode (aka Protected mode) routine
     *
     * Notes:
     *  1. Should be linked to far page 0 (fp0)
     *  2. Should be aligned on 8 bytes
     */
    .section .system_services,"xa",@progbits
    .org    0x0, 0
mlx_enter_protected_mode:
	clrb MH.3
	ret

    nop     ; fill in

    end

/* EOF */


