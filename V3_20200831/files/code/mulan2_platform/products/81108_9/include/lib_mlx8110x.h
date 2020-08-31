/*
 * Copyright (C) 2008-2015 Melexis N.V.
 *
 * Software Platform
 * Product/board specific functions
 *
 */

#ifndef _LIB_MLX8110X_H_
#define _LIB_MLX8110X_H_

#define MLX8110x_CLK_MHz                  (FPLL/1000)
#define MLX8110x_Cycle_PER_INSTRUCTION    5                     /* 4 cycle per instruction + 1 FLASH wait state */
#define MLX8110x_INSTRUCTION_TIME         ((float)((float)MLX8110x_CLK_MHz / (float)MLX8110x_Cycle_PER_INSTRUCTION))

/** MLX81106/7/8/9 delay for 'us' microseconds
 * @param uint16 Delay_usec (max. 13653)
 * @return 
 */
#define MLX8110x_USEC_DELAY(Delay_usec)   __asm__ volatile ("djnz x,." : : "x" ((uint16)(((float)Delay_usec * (float)MLX8110x_INSTRUCTION_TIME) - (float)0.5)));


#include "lib_mlx8110x_wdg.h"
#include "lib_mlx8110x_adc.h"
#include "lib_mlx8110x_adc_hvdiff.h"
#include "lib_mlx8110x_temp.h"
#include "lib_mlx8110x_hv.h"
#include "lib_mlx8110x_lv.h"
#include "lib_mlx8110x_lin_specialmode.h"
#include "lib_mlx8110x_pwm.h"
#include "lib_mlx8110x_spi.h"
#include "lib_mlx8110x_monitoring.h"
#include "lib_mlx8110x_trim.h"
#include "lib_mlx8110x_timer.h"
#include "lib_mlx8110x_wakeup.h"
#include "lib_mlx8110x_rc_ctrl.h"


#endif /* _LIB_MLX8110X_H_ */
