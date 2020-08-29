/*
 * Copyright (C) 2013 Melexis N.V.
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
    .incbin "mlx4_loop.bin"


/*
 *  Handler for MLX4 interrupt (normally never called)
 */
    .global _ml_LinInterruptHandler
    .section .mlx_text,"xa",@progbits

_ml_LinInterruptHandler:
    jmp __fatal

    end

/* EOF */
