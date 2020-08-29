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

extern void mul_test  (void);
extern void div_test  (void);
extern void rand_test (void);
extern void lfsr_test (void);
extern void parity_test (void);
extern void bitrev_test (void);
extern void interleave_test (void);
extern void isqrt_test(void);
extern void ilog_test (void);


/*
 *
 */
int main (void)
{
    MLX_InitTestFramework();
    ENTER_USER_MODE();

    while (1)
    {
        mul_test();
        div_test();

        rand_test();
        lfsr_test();
        parity_test();

        bitrev_test();
        interleave_test();

        isqrt_test ();
        ilog_test ();

        MLX_ToggleTestPin();
    }

    return 0;
}

/* EOF */
