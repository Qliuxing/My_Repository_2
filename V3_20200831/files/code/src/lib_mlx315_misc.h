#ifndef _LIB_MLX315_MISC_H
#define _LIB_MLX315_MISC_H

/* Digital Watchdog */
#define WatchDog_ModeTimer			0x12U										/* Auto Timer mode is selected - basic functionality */
#define WatchDog_Prescaler			0x02U										/* Pre-scaler (3) = 512, (2) = 128 */
#define WatchDog_MaxPeriod			0xFFU										/* Period selected = (512/250000)*255 = 522ms */
#define WatchDog_PeriodOf250ms		122U										/* Period selected = (512/250000)*122 = 250ms */
#define WatchDog_PeriodOf100ms		195U										/* Period selected = (128/250000)*195 = 100ms */
/* Analogue Watchdog time-out */
#define C_AWD_PERIOD_525MS			82U											/* Period selected = (64/10kHz)*82  = 524.8 ms */
#define C_AWD_PERIOD_1S				156U										/* Period selected = (10kHz/64)*156 = 0.9984 s */
#define C_AWD_PERIOD_819MS			128U										/* Period selected = (10kHz/64)*128 = 0.8192 s */
#define C_AWD_PERIOD_800MS			125U										/* Period selected = (10kHz/64)*125 = 0.80 s */
#define C_AWD_PERIOD_250MS			156U										/* Period selected = (10kHz/16)*156 = 250 ms */
#define C_AWD_PERIOD_160MS			100U										/* Period selected = (10kHz/16)*100 = 160 ms */
#define C_AWD_PERIOD_150MS			94U											/* Period selected = (10kHz/16)*94  = 150 ms */
#define C_AWD_PERIOD_100MS			250U										/* Period selected = (10kHz/4)*250  = 100 ms */
#define C_AWD_PERIOD_500US			5U											/* Period selected = (10kHz/1)*5    = 0.5 ms */
#define C_AWD_PERIOD_5MS			50U											/* Period selected = (10kHz/1)*50   = 5.0 ms */
#define C_AWD_PERIOD_25MS			250U										/* Period selected = (10kHz/1)*250  = 25.0 ms */

/* Chip */
#define CYCLES_PER_INSTR		5U												/* 5 clocks per instruction */
#define PWM_FREQ				20000UL											/* PWM signal frequency in Hz */
#define PWM_PRESCALER_M			1U												/* Define the PWM timer clock frequency */
#define PWM_PRESCALER_N			0U												/* as F = Fpll / ( Mx2^N ) */
#define PWM_PRESCALER			(((PWM_PRESCALER_M - 1U) << 4U ) + PWM_PRESCALER_N ) 			/* Pre-scaler value */
#define PWM_TIMER_CLOCK			(PLL_freq / (PWM_PRESCALER_M * (1UL << PWM_PRESCALER_N)))		/* Counter frequency */
#define PWM_REG_PERIOD			((PWM_TIMER_CLOCK / PWM_FREQ) - 2U)								/* Value of the period register; Fpwm = Fcnt/(PWM_period_reg+1) ==> 24KHz */
#define CompareRegMaster		((PWM_REG_PERIOD + 1U) / 4U)									/* PWM_period_reg/4; */
#define	PWM_SCALE_OFFSET		((PWM_REG_PERIOD + 1U) / 2U)									/* offset to have the PWM wave always in the positive area */
#define TIMER_PRESCALER			16U																/* Timer divider is 1, 16 or 256; Minimum speed: >= 134rpm @ 28MHz/2PP, 89rpm @ 28MHz/3PP, 67rpm @ 28MHz/4PP, 54rpm @ 28MHz/5PP */
#define TIMER_CLOCK				(PLL_freq/TIMER_PRESCALER)
#if (TIMER_PRESCALER == 1U)
#define C_TMRx_CTRL_MODE0	((0U * TMRx_DIV0) | (0U * TMRx_MODE0) | TMRx_T_EBLK)	/* Timer mode 0, Divider 1 */
#else /* (TimerPrescaler == 1) */
#if (TIMER_PRESCALER == 16U)
#define C_TMRx_CTRL_MODE0	((1U * TMRx_DIV0) | (0U * TMRx_MODE0) | TMRx_T_EBLK)	/* Timer mode 0, Divider 16 */
#else /* (TIMER_PRESCALER == 16) */
#define C_TMRx_CTRL_MODE0	((2u * TMRx_DIV0) | (0u * TMRx_MODE0) | TMRx_T_EBLK)	/* Timer mode 0, Divider 256 */
#endif /* (TIMER_PRESCALER == 16) */
#endif /* (TIMER_PRESCALER == 1) */

#define FET_SETTING (((10u*PLL_freq)/(1000000u * CYCLES_PER_INSTR * 2u)) + 1u)			/* 10us: 10us*PLL-freq/(10000000us/s * #cycles/instruction) * instructions */

#define CT_PERIODIC_RATE		500U											/* core timer periodic rate:500us */


/* Enable the driver and the PWM phase W, V and U */
#define DRVCFG_DIS()		{DRVCFG |= DIS_DRV;}						/* MMP140903-1 */
/* 3-Phase: U, V and W */
#define DRVCFG_DIS_UVW()	{DRVCFG &= ~(DIS_DRV|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U);}
#define DRVCFG_DIS_UVWz()	{DRVCFG &= ~(DRV_CFG_W|DRV_CFG_V|DRV_CFG_U);}
#define DRVCFG_PWM_UVW()	{DRVCFG &= ~(DIS_DRV|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U);DRVCFG |= (DRV_CFG_W_PWM|DRV_CFG_V_PWM|DRV_CFG_U_PWM));}
#define DRVCFG_GND_UVW()	{DRVCFG &= ~(DRV_CFG_W|DRV_CFG_V|DRV_CFG_U);DRVCFG |= (DRV_CFG_W_0|DRV_CFG_V_0|DRV_CFG_U_0);}
#define DRVCFG_VSUP_UVW()	{DRVCFG &= ~(DRV_CFG_W|DRV_CFG_V|DRV_CFG_U); DRVCFG |= (DRV_CFG_W_1|DRV_CFG_V_1|DRV_CFG_U_1);}
#define DRVCFG_CNFG_UVW(x)	{DRVCFG &= ~(DIS_DRV|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U);DRVCFG |= x;}

/* 4-Phase: U, V, W and T */
#define DRVCFG_DIS_UVWT()	{DRVCFG &= ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U);}
#define DRVCFG_PWM_UVWT()	{DRVCFG &= ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U);DRVCFG |= (DRV_CFG_T_PWM|DRV_CFG_W_PWM|DRV_CFG_V_PWM|DRV_CFG_U_PWM);}
#define DRVCFG_GND_UVWT()	{DRVCFG &= ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U);DRVCFG |= (DRV_CFG_T_0|DRV_CFG_W_0|DRV_CFG_V_0|DRV_CFG_U_0);}
#define DRVCFG_VSUP_UVWT()	{DRVCFG &= ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U);DRVCFG |= (DRV_CFG_T_1|DRV_CFG_W_1|DRV_CFG_V_1|DRV_CFG_U_1);}
#define DRVCFG_CNFG_UVWT(x)	{DRVCFG &= ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U);DRVCFG |= x;}
#define DRVCFG_DIS_T()		{DRVCFG &= ~(DIS_DRV|DRV_CFG_T);}
#define DRVCFG_PWM_T()		{DRVCFG &= ~(DIS_DRV|DRV_CFG_T);DRVCFG |= (DRV_CFG_T_PWM);}

#define DRVCFG_PWM_VW()		{DRVCFG &= ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U);DRVCFG |= (DRV_CFG_W_PWM|DRV_CFG_V_PWM);}
#define DRVCFG_PWM_UT()		{DRVCFG &= ~(DIS_DRV|DRV_CFG_T|DRV_CFG_W|DRV_CFG_V|DRV_CFG_U);DRVCFG |= (DRV_CFG_T_PWM|DRV_CFG_U_PWM);}


void MLX315_SystemInit(void);
void MLX315_GotoSleep(void);


#endif
