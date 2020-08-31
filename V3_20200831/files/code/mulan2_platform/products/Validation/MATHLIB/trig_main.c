/*
 * Copyright (C) 2007-2013 Melexis N.V.
 *
 * Math Library
 *
 */

#include <syslib.h>
#include <mathlib.h>


/* Product and board specific (defined in product's mlx_test.c) */
extern void MLX_InitTestFramework (void);
extern void MLX_ToggleTestPin (void);

extern void sincos_test  (void);
extern void tan_test  (void);
extern void atan_test (void);

/*
 *
 */
int main (void)
{
    MLX_InitTestFramework();
    ENTER_USER_MODE();

    for(;;) {
        sincos_test();
        tan_test();
        atan_test();

        MLX_ToggleTestPin();
    }

    return 0;
}

/* EOF */
