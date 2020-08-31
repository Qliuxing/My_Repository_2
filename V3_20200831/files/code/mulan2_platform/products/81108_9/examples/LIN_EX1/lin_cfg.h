/*
 * Copyright (C) 2011-2012 Melexis N.V.
 *
 * Software Platform
 *
 */
#ifndef LIN_CFG_H_
#define LIN_CFG_H_

/* 
 * LIN configuration
 */
#define MLX_INITIAL_NAD     0x0B    /* Node Address for Diagnostic */

/*--- LIN Product Identification --------------------------------------*/
#define MLX_SUPPLIER_ID     0x5AFE
#define MLX_FUNCTION_ID     0xBEAF
#define MLX_VARIANT_ID      0x01


/* 
 * Input Data Verification
 */
#if (MLX_INITIAL_NAD < 0x01) || (MLX_INITIAL_NAD > 0x7E)
#  error "Initial NAD (MLX_INITIAL_NAD) shall be in range [0x01..0x7E]"
#endif

#endif /* LIN_CFG_H_ */
