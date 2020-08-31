/*
 * Copyright (C) 2013 Melexis N.V.
 *
 * ML16 CPU configurations
 *
 */

#ifndef MLX16_CFG_H_
#define MLX16_CFG_H_

/* Check if MLX16 co-processor is available */
#if defined (__MLX16_X8__) || defined (__MLX16_FX__) \
  || defined (__MLX16_EX__)
#define HAS_MLX16_COPROCESSOR
#endif

/* Check if new extra instructions are available, e.g. FSB / SFB  */
#if defined (__MLX16_8__) || defined (__MLX16_X8__) || defined (__MLX16_FX__) \
  || defined (__MLX16_E8__) || defined (__MLX16_EX__)
#define HAS_MLX16_FSB_SFB_INSTRUCTIONS
#endif

#endif /* MLX16_CFG_H_ */
