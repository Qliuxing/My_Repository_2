/*
 * Copyright (C) 2005-2011 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * Wrapper to include MLX4 LIN FW binary image
 */

    .global mlx4_prog

    .section .mlx4_prog,"xa",@progbits

mlx4_prog:

#if (defined(MCU_PLL_MULT) && (MCU_PLL_MULT == 48)) \
    || (FPLL == 12000)   /* --- 12 MHz --------------------------------------------- */

    #if defined (MLX4_FW_LIN2X)
        .incbin "lin2b_12mhz.bin"           /* Single image for all LIN2.x baudrates */
    #elif defined (MLX4_FW_LIN13_9600)
        .incbin "lin2b_v13_9600_12mhz.bin"
    #elif defined (MLX4_FW_LIN13_19200)
        .incbin "lin2b_v13_19200_12mhz.bin"
    #elif defined (MLX4_FW_STANDALONE_LOADER)
        .incbin "fast2b.bin"                /* Standalone loader image */
    #else
    # error "It is not specified which MLX4 FW image to use"
    #endif

#elif (defined(MCU_PLL_MULT) && (MCU_PLL_MULT == 72)) \
    || (FPLL == 18000)  /* --- 18 MHz ---------------------------------------------- */

    #if defined (MLX4_FW_LIN2X)
        .incbin "lin2b_18mhz.bin"           /* Single image for all LIN2.x baudrates */
    #elif defined (MLX4_FW_LIN13_9600)
        .incbin "lin2b_v13_9600_18mhz.bin"
    #elif defined (MLX4_FW_LIN13_19200)
        .incbin "lin2b_v13_19200_18mhz.bin"
    #elif defined (MLX4_FW_STANDALONE_LOADER)
        .incbin "fast2b.bin"                /* Standalone loader image */
    #else
    # error "It is not specified which MLX4 FW image to use"
    #endif

#elif (defined(MCU_PLL_MULT) && (MCU_PLL_MULT == 80)) \
    || (FPLL == 20000)  /* --- 20 MHz ---------------------------------------------- */

    #if defined (MLX4_FW_LIN2X)
        .incbin "lin2b_20mhz.bin"           /* Single image for all LIN2.x baudrates */
    #elif defined (MLX4_FW_LIN13_9600)
        .incbin "lin2b_v13_9600_20mhz.bin"
    #elif defined (MLX4_FW_LIN13_19200)
        .incbin "lin2b_v13_19200_20mhz.bin"
    #elif defined (MLX4_FW_STANDALONE_LOADER)
        .incbin "fast2b.bin"                /* Standalone loader image */
    #else
    # error "It is not specified which MLX4 FW image to use"
    #endif

#elif (defined(MCU_PLL_MULT) && (MCU_PLL_MULT == 96)) \
    || (FPLL == 24000)  /* --- 24 MHz ---------------------------------------------- */

    #if defined (MLX4_FW_LIN2X)
        .incbin "lin2b_24mhz.bin"           /* Single image for all LIN2.x baudrates */
    #elif defined (MLX4_FW_LIN13_9600)
        .incbin "lin2b_v13_9600_24mhz.bin"
    #elif defined (MLX4_FW_LIN13_19200)
        .incbin "lin2b_v13_19200_24mhz.bin"
    #elif defined (MLX4_FW_STANDALONE_LOADER)
        .incbin "fast2b.bin"                /* Standalone loader image */
    #else
    # error "It is not specified which MLX4 FW image to use"
    #endif

#elif (defined(MCU_PLL_MULT) && (MCU_PLL_MULT == 100)) \
    || (FPLL == 25000)  /* --- 25 MHz ---------------------------------------------- */

    #if defined (MLX4_FW_LIN2X)
        .incbin "lin2b_25mhz.bin"           /* Single image for all LIN2.x baudrates */
    #elif defined (MLX4_FW_LIN13_9600)
        .incbin "lin2b_v13_9600_25mhz.bin"
    #elif defined (MLX4_FW_LIN13_19200)
        .incbin "lin2b_v13_19200_25mhz.bin"
    #elif defined (MLX4_FW_STANDALONE_LOADER)
        .incbin "fast2b.bin"                /* Standalone loader image */
    #else
    # error "It is not specified which MLX4 FW image to use"
    #endif

#elif (defined(MCU_PLL_MULT) && (MCU_PLL_MULT == 112)) \
    || (FPLL == 28000)  /* --- 28 MHz ---------------------------------------------- */

    #if defined (MLX4_FW_LIN2X)
        .incbin "lin2b_28mhz.bin"           /* Single image for all LIN2.x baudrates */
    #elif defined (MLX4_FW_LIN13_9600)
        .incbin "lin2b_v13_9600_28mhz.bin"
    #elif defined (MLX4_FW_LIN13_19200)
        .incbin "lin2b_v13_19200_28mhz.bin"
    #elif defined (MLX4_FW_STANDALONE_LOADER)
        .incbin "fast2b.bin"                /* Standalone loader image */
    #else
    # error "It is not specified which MLX4 FW image to use"
    #endif

#elif (defined(MCU_PLL_MULT) && (MCU_PLL_MULT == 120)) \
    || (FPLL == 30000)  /* --- 30 MHz ---------------------------------------------- */

    #if defined (MLX4_FW_LIN2X)
        .incbin "lin2b_30mhz.bin"           /* Single image for all LIN2.x baudrates */
    #elif defined (MLX4_FW_LIN13_9600)
        .incbin "lin2b_v13_9600_30mhz.bin"
    #elif defined (MLX4_FW_LIN13_19200)
        .incbin "lin2b_v13_19200_30mhz.bin"
    #elif defined (MLX4_FW_STANDALONE_LOADER)
        .incbin "fast2b.bin"                /* Standalone loader image */
    #else
    # error "It is not specified which MLX4 FW image to use"
    #endif

#elif (defined(MCU_PLL_MULT) && (MCU_PLL_MULT == 128)) \
    || (FPLL == 32000)  /* --- 32 MHz ---------------------------------------------- */

    #if defined (MLX4_FW_LIN2X)
        .incbin "lin2b_32mhz.bin"           /* Single image for all LIN2.x baudrates */
    #elif defined (MLX4_FW_LIN13_9600)
        .incbin "lin2b_v13_9600_32mhz.bin"
    #elif defined (MLX4_FW_LIN13_19200)
        .incbin "lin2b_v13_19200_32mhz.bin"
    #elif defined (MLX4_FW_STANDALONE_LOADER)
        .incbin "fast2b.bin"                /* Standalone loader image */
    #else
    # error "It is not specified which MLX4 FW image to use"
    #endif

#else
# error "This CPU frequency (defined by MCU_PLL_MULT or FPLL) is not supported by MLX4 FW"
#endif

    end

/* EOF */
