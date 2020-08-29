/*! \file		Main.h
 *  \brief		MLX81300 Main
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2011-04-17
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	main()
 *				
 *
 * MELEXIS Microelectronic Integrated Systems
 * 
 * Copyright (C) 2008-2015 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 * ****************************************************************************	*/

#ifndef MAIN_H_
#define MAIN_H_

#ifndef SYSLIB_H_
#include "syslib.h"
#endif /* SYSLIB_H_ */
#if LIN_COMM
#ifndef LIN_COMMUNICATION_H_
#include "LIN_Communication.h"													/* LIN Communication support */
#endif /* LIN_COMMUNICATION_H_ */
#endif /* LIN_COMM */
#include "MotorDriver.h"														/* Motor-driver support */

/* ****************************************************************************	*
 *                            D E F I N I T I O N S								*
 * ****************************************************************************	*/
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
#define CYCLES_PER_INSTR		5												/* 5 clocks per instruction */
#define PWM_FREQ				20000U											/* PWM signal frequency in Hz */
#define PWM_PRESCALER_M			1U												/* Define the PWM timer clock frequency */
#define PWM_PRESCALER_N			0U												/* as F = Fpll / ( Mx2^N ) */
#define PWM_PRESCALER			(((PWM_PRESCALER_M - 1) << 4 ) + PWM_PRESCALER_N ) 	/* Pre-scaler value */
#define PWM_TIMER_CLOCK			(PLL_freq / ((PWM_PRESCALER_M * 1) << PWM_PRESCALER_N))/* Counter frequency */
#define PWM_REG_PERIOD			((PWM_TIMER_CLOCK / PWM_FREQ) - 2)				/* Value of the period register; Fpwm = Fcnt/(PWM_period_reg+1) ==> 24KHz */
#define CompareRegMaster		((PWM_REG_PERIOD + 1) / 4)						/* PWM_period_reg/4; */
#define	PWM_SCALE_OFFSET		(uint16)((PWM_REG_PERIOD + 1) / 2)				/* offset to have the PWM wave always in the positive area */
#define TIMER_PRESCALER			16U												/* Timer divider is 1, 16 or 256; Minimum speed: >= 134rpm @ 28MHz/2PP, 89rpm @ 28MHz/3PP, 67rpm @ 28MHz/4PP, 54rpm @ 28MHz/5PP */
#define TIMER_CLOCK				((uint32) (PLL_freq/TIMER_PRESCALER))
#if (TIMER_PRESCALER == 1)
#define C_TMRx_CTRL_MODE0	((0 * TMRx_DIV0) | (0 * TMRx_MODE0) | TMRx_T_EBLK)	/* Timer mode 0, Divider 1 */
#else /* (TimerPrescaler == 1) */
#if (TIMER_PRESCALER == 16)
#define C_TMRx_CTRL_MODE0	((1 * TMRx_DIV0) | (0 * TMRx_MODE0) | TMRx_T_EBLK)	/* Timer mode 0, Divider 16 */
#else /* (TIMER_PRESCALER == 16) */
#define C_TMRx_CTRL_MODE0	((2 * TMRx_DIV0) | (0 * TMRx_MODE0) | TMRx_T_EBLK)	/* Timer mode 0, Divider 256 */
#endif /* (TIMER_PRESCALER == 16) */
#endif /* (TIMER_PRESCALER == 1) */

#define C_SLEEP_PERIOD			((uint16)((PLL_freq/256U) * 0.05))				/* Sleep period of 50 ms */

#define C_MLX4_STATE_TIMEOUT	(PI_TICKS_PER_MILLISECOND * 500)				/* 500 ms */
#define C_MLX4_STATE_ERROR_THRSHLD	4											/* After 'n' LIN MLX4 state error, re-initialise LIN interface */
#define C_MLX4_STATE_IMMEDIATE_RST	0x80										/* Reset MLX4 immediate, without check */
#define C_MLX4_STATE_NOT_LOGGED		0x40										/* Do not log MLX4 reset as error (e.g. switch between PWM to LIN) */

#define C_CHIP_OVERTEMP_LEVEL				150U								/* Chip Over-temperature level is 150C */
#define C_OVERTEMP_TO_PERMDEFECT_THRSHLD	4U

#define C_TEMP_STABIL_TIMEOUT				C_PI_TICKS_250MS					/* Temperature interval of 250ms */
#define C_TEMP_STABIL_INT_FILTER_COEF		8U									/* Temperature stability integrator filter coefficient: 1/(2^n) */
#define C_TEMP_STABIL_THRESHOLD				20U									/* Temperature threshold (output integrator filter) 30C * (1 - ((2^n-1)/2^n)^(4*60)) = 19 */

static INLINE void NopDelay(uint16 u16DelayCount)
{
	for (; u16DelayCount > 0; u16DelayCount-- )
	{
		NOP();
	}
}
#define DELAY_4us	(uint16)(((    4U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*   4us delay */
#define DELAY_7us	(uint16)(((    7U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*   7us delay */
#define DELAY_10us	(uint16)(((   10U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*  10us delay */
#define DELAY_50us	(uint16)(((   50U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*  50us delay */
#define DELAY_60us	(uint16)(((   60U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*  60us delay */
#define DELAY_100us	(uint16)(((  100U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/* 100us delay */
#define DELAY_1ms	(uint16)((( 1000U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*   1ms delay */
#define DELAY_1m25s	(uint16)((( 1250U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/* 1m25s delay */
#define DELAY_10ms	(uint16)(((10000U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*  10ms delay */
#define DELAY_16ms	(uint16)(((16000U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/*  16ms delay (maximum at 32 MHz PLL) */
#if (PLL_freq <= 20000000)
#define DELAY_25ms	(uint16)(((25000U * (PLL_freq/1000000))/(2*CYCLES_PER_INSTR))-1)	/* 25ms delay (maximum at 20 MHz PLL) */
#endif /* (PLL_freq <= 20000000) */
#define DELAY_mPWM	(uint16)((PLL_freq/((uint32)PWM_FREQ*2*CYCLES_PER_INSTR))-1)	/* Motor-PWM period delay */
#define ADC_DELAY	(uint16)((PLL_freq/((uint32)PWM_FREQ*2*4*CYCLES_PER_INSTR))+1)	/* 25% of PLL-freq/PWM-Freq/(#cycles/instruction)/#instructions */

#define C_REWIND_DIRECTION_CCW		0x01U		/* bit 0: Last rotational direction; Must be bit 0 (g_e8MotorDirectionCCW) */
#define C_REWIND_STALL_DETECT		0x02U		/* bit 1: Stall have been detected; Must be bit 1 (DC-motors) */
#define C_REWIND_DIRECTION_AUTO		0x04U		/* bit 2: Auto-direction (POR) */
#define C_REWIND_ACTIVE				0x08U		/* bit 3: Rewind active */
#define C_REWIND_FULLAUTO			0x10U		/* bit 4: Full Automatic actuator mode */
#define C_REWIND_REWIND				0x20U		/* bit 5: Rewinding of the Rewind is active */

/* MMP: PC-lint first typedef always fails; So create a dummy */
typedef unsigned char lint_typedef_fail_main_h;

/* LIN Auto-Addressing information structure (debugging/screening) */
#if LIN_AA_INFO
typedef struct _SNPD_DATA
{
	uint8 byStepAndFlags;						/* 0x00 (MMP130818-1) */
	uint8 byIshunt1;							/* 0x01 */
	uint8 byIshunt2;							/* 0x02 */
	uint8 byIshunt3;							/* 0x03 */
#if LIN_AA_SCREENTEST
	uint16 u16CM_1;								/* 0x04 */
	uint16 u16DM_1;								/* 0x06 */
	uint16 u16CM_2;								/* 0x0A */
	uint16 u16DM_2;								/* 0x0A */
	uint16 u16CM_3;								/* 0x0C */
	uint16 u16DM_3;								/* 0x0E */
#endif /* LIN_AA_SCREENTEST */
} SNPD_DATA, *PSNPD_DATA;
#endif /* LIN_AA_INFO */

#if USE_MULTI_PURPOSE_BUFFER
/* Multi-purpose buffer */
#define MP_BUF_SZ					256U
#define VT_BUF_SZ					(MP_BUF_SZ / sizeof(uint16))
#define LIN_AA_INFO_SZ				(MP_BUF_SZ / sizeof(SNPD_DATA))
typedef struct _MP_BUF
{
	uint8	u8Type;
	uint8	u8Usage;
	union
	{
#if LIN_AA_INFO
		SNPD_DATA aSNPD_Data[LIN_AA_INFO_SZ];			/* LIN Auto-addressing info storage */
#endif /* LIN_AA_INFO */
	} u;
} MP_BUF, *PMP_BUF;

typedef enum
{
	C_MP_BUF_NONE = 0,									/* 0: No purpose defined */
	C_MP_BUF_MOTOR_DRIVER,								/* 1: Motor driver */
	C_MP_BUF_LIN_AA										/* 2: LIN Auto Addressing */
} MP_BUF_TYPE;

typedef enum
{
	C_MP_BUF_FREE = 0,
	C_MP_BUF_INUSE
} MP_BUF_USAGE;

#if LIN_AA_INFO
#define LIN_AA_DATA		g_MPBuf.u.aSNPD_Data
#endif /* LIN_AA_INFO */

#else  /* USE_MULTI_PURPOSE_BUFFER */

/* Motor driver acceleration table (velocity-timer values) */
#define VT_BUF_SZ					128
#define VELOCITY_TIMER	l_au16VelocityTimer

/* LIN Auto-addressing info */
#if LIN_AA_INFO
#define LIN_AA_INFO_SZ				16
#define LIN_AA_DATA		l_aSNPD_Data
#endif /* LIN_AA_INFO */

#endif /* USE_MULTI_PURPOSE_BUFFER */

/* Motor Request types */
typedef enum
{
	C_MOTOR_REQUEST_NONE = 0,													/* 0: No request */
	C_MOTOR_REQUEST_STOP,														/* 1: Request to STOP motor */
	C_MOTOR_REQUEST_INIT,														/* 2: Request to initialise motor */
	C_MOTOR_REQUEST_START,														/* 3: Request to START/SET motor */
	C_MOTOR_REQUEST_SERVICE,													/* 4: Request to enter service mode */
	C_MOTOR_REQUEST_CALIBRATION,												/* 5: Request to start calibration */
	C_MOTOR_REQUEST_SLEEP,														/* 6: Request to sleep */
	C_MOTOR_REQUEST_EMRUN,														/* 7: Request to Emergency Run */
	C_MOTOR_REQUEST_SPEED_CHANGE,												/* 8: Request to change speed */
	C_MOTOR_REQUEST_CALIB_FACTORY												/* 9: Request to calibrate (factory mode) */
} MOTOR_REQUEST;

typedef enum
{
	C_CALIB_NONE = 0,															/* 0: No calibration */
	C_CALIB_START,																/* 1: Start calibration */
	C_CALIB_SETUP_HI_ENDPOS,													/* 2: Setup movement towards High Endstop */
	C_CALIB_CHECK_HI_ENDPOS,													/* 3: High Endstop reached */
	C_CALIB_PAUSE_HI_ENDSTOP,													/* 4: High Endstop Pause */
	C_CALIB_SETUP_LO_ENDPOS,													/* 5: Setup movement towards Low Endstop */
	C_CALIB_CHECK_LO_ENDPOS,													/* 6: Low Endstop reached */
	C_CALIB_SETUP_HM_ENDPOS,													/* 7: Low Endstop reached */
	C_CALIB_CHECK_HM_ENDPOS,													/* 8: Low Endstop reached */
	C_CALIB_END,																/* 9: End of calibration */
	C_CALIB_DONE,																/* 10: Calibration successfully done */
} CALIB_MODE;

typedef enum
{
	C_CALIB_LOW_POS = 0,														/* 0: Half-automatic, towards low position */
	C_CALIB_HIGH_POS,															/* 1: Half-automatic, towards high position */
	C_CALIB_FULL_AUTOMATIC,														/* 2: Full-automatic */
	C_CALIB_UNKNOWN,
	C_CALIB_FINAL = 0x80
} CALIB_AUTO_MODE;

typedef enum
{
	C_STALLDET_NONE = 0,														/* 0: No stall detector enabled */
	C_STALLDET_A,																/* 1: Stall detector "A" enabled */
	C_STALLDET_O,																/* 2: Stall detector "O" enabled */
	C_STALLDET_AnO,																/* 3: Stall detector "A" and "O" enabled */
	C_STALLDET_H,																/* 4: Stall detector "H" enabled */
	C_STALLDET_HnA,																/* 5: Stall detector "H" and "A" enabled */
	C_STALLDET_HnO,																/* 6: Stall detector "H" and "O" enabled */
	C_STALLDET_ALL,																/* 7: Stall detector "H", "A" and "O" enabled */
	C_STALLDET_CALIB															/* 8: Stall detector calibration enabled */
} STALLDET_MODE;

/* Motor speeds (g_u8MotorCtrlSpeed/g_u8MotorStatusSpeed) */
#define C_MOTOR_SPEED_STOP			0U											//NVRAM_MIN_SPEED
#define C_MOTOR_SPEED_LOW			1U											//NVRAM_SPEED_TORQUE_BOOST
#define C_MOTOR_SPEED_MID_LOW		2U											//NVRAM_SPEED0
#define C_MOTOR_SPEED_MID			3U											//NVRAM_SPEED1
#define C_MOTOR_SPEED_MID_HIGH		4U											//NVRAM_SPEED2
#define C_MOTOR_SPEED_HIGH 			5U											//NVRAM_SPEED3
#define C_DEFAULT_MOTOR_SPEED		C_MOTOR_SPEED_MID

/* Motor Control mode (g_e8MotorCtrlMode) */
#define C_MOTOR_CTRL_STOP			0x00U
#define C_MOTOR_CTRL_NORMAL			0x01U
/* Motor Status mode (g_e8MotorStatusMode) */
#define C_MOTOR_STATUS_STOP			0x00U										/* Actuator stopped */
#define C_MOTOR_STATUS_RUNNING		0x01U										/* bit 0: Actuator Running */
#define C_MOTOR_STATUS_INIT			0x02U										/* bit 1: Actuator initialisation (Only internal state) */
#define C_MOTOR_STATUS_SELFTEST		0x04U										/* bit 2: Actuator Self-test (Only internal state) */
#define C_MOTOR_STATUS_STOPPING		(0x08U | C_MOTOR_STATUS_RUNNING)			/* bit 3+0: (Going to) stopping but still running */
#define C_MOTOR_STATUS_APPL_STOP	0x40U										/* bit 6: Application stopped */
#define C_MOTOR_STATUS_DEGRADED		0x80U										/* bit 7: Actuator in degraded mode (Only Cooling 2.3 and only Status) */

/* Motor position type */
#define C_POSTYPE_INIT			0U
#define C_POSTYPE_TARGET		1U

/* Over-temperature error (g_e8ErrorOverTemperature) */
#define C_ERR_OTEMP_NO			FALSE
#define C_ERR_OTEMP_YES			TRUE
#define C_TEMPERATURE_HYS		3U												/* Temperature hysteric: 3 degrees Celsius */
#define C_TEMPERATURE_JUMP		10U												/* Maximum temperature "jump" per measurement-period: 10 degrees Celsius */

/* Electronics error (g_e8ErrorElectric) */
#define C_ERR_ELECTRIC_NO		0U
#define C_ERR_ELECTRIC_YES		1U
#define C_ERR_ELECTRIC_PERM		2U

/* Voltage error (g_e8ErrorVoltage) */
#define C_ERR_VOLTAGE_IN_RANGE	0U
#define C_ERR_VOLTAGE_UNDER		1U
#define C_ERR_VOLTAGE_OVER		2U
#define C_VOLTAGE_HYS			((int16) 50)									/* Voltage hysteric:  0.5V (  50 x 10mV) */

/* Motor rotational direction (g_e8MotorDirectionCCW) */
#define C_MOTOR_DIR_OPENING		0												/* Positioning: Moving from low-to-high Pos */
#define C_MOTOR_DIR_CW			0												/* Continueos */
#define C_MOTOR_DIR_CLOSING		1												/* Positioning: Moving from high-to-low Pos */
#define C_MOTOR_DIR_CCW			1												/* Continueos */
#define C_MOTOR_DIR_UNKNOWN		2

/* Debounce error filter; An error has to be detected twice in a row */
#define C_DEBFLT_ERR_NONE			0x00U
#define C_DEBFLT_ERR_PHASE_SHORT	0x01U										/* Bit 0: Phase short to ground error */
#define C_DEBFLT_ERR_OVT			0x02U										/* Bit 1: Over-Temperature error */
#define C_DEBFLT_ERR_UV				0x04U										/* Bit 2: Under-Voltage error */
#define C_DEBFLT_ERR_OV				0x08U										/* Bit 3: Over-Voltage error */
#define C_DEBFLT_ERR_TEMP_PROFILE	0x10U										/* Bit 4: Chip Temperature profile error */

/* MLX4/LIN State (MMP130812-1) */
#define C_LIN_SLEEP				0U												/* MLX4/LIN Bus-Timeout or Sleep-request */
#define C_LIN_AWAKE				1U												/* LIN-bus active */

#define LOW						FALSE
#define HIGH					TRUE

//EXV Position
#if _SUPPORT_DOUBLE_USTEP
#define	C_EXV_MICRO_STEP	(1 << (NVRAM_MICRO_STEPS + 0))						/* Number of micro-steps per full-step (2, 4, 8 or 16) */
#else  /* _SUPPORT_DOUBLE_USTEP */
#define	C_EXV_MICRO_STEP	(1 << NVRAM_MICRO_STEPS)							/* Number of micro-steps per full-step (1, 2, 4 or 8) */
#endif
#define C_EXV_HALL_DETECTION_STEP	((uint16)24*C_EXV_MICRO_STEP) //24 full steps for hall stall detection
#define C_EXV_ZERO_POS				((uint16)100*C_EXV_MICRO_STEP)
#define C_EXV_TOLERANCE_LO			(g_NvramUser.DefTravelToleranceLo*C_EXV_MICRO_STEP)
#define C_EXV_TOLERANCE_UP			(g_NvramUser.DefTravelToleranceUp*C_EXV_MICRO_STEP)
#define C_EXV_DEF_TRAVEL			(g_NvramUser.DefTravel*C_EXV_MICRO_STEP)
#define C_EXV_RANGE_MAX				(C_EXV_DEF_TRAVEL + C_EXV_TOLERANCE_LO + C_EXV_TOLERANCE_UP + C_EXV_HALL_DETECTION_STEP)
#define C_EXV_RANGE_MIN				(C_EXV_DEF_TRAVEL - C_EXV_TOLERANCE_LO - C_EXV_TOLERANCE_UP + C_EXV_HALL_DETECTION_STEP)

#define C_EXV_FULLY_CLOSE_LIN		0x000
#define C_EXV_FULLY_OPEN_LIN		0x3FF
#define C_EXV_POSITION_STD			((uint16)288*C_EXV_MICRO_STEP)

//EXV Move Enable(g_e8EXVMoveEnableRequestFlag)
#define C_EXV_MOVE_DISABLE				0											/* 0: EXV move disabled */
#define C_EXV_MOVE_ENABLE				1											/* 1: EXV move enabled */

//EXV Init Direction(g_e8EXVInitDirection)
#define C_EXV_INIT_DIR_NO_SELECT		0
#define C_EXV_INIT_DIR_LOWENDSTOP		1
#define C_EXV_INIT_DIR_HIGHENDSTOP		2

#define C_GMCV_INIT_DIR_NO_SELECT		0
#define C_GMCV_INIT_DIR_OPEN_FIRST		1
#define C_GMCV_INIT_DIR_CLOSE_FIRST		2

//EXV Warning temperature
#define C_CHIP_WARNING_OVERTEMP_LEVEL	145U
#define C_WARNING_OTEMP_YES				01U
#define C_WARNING_OTEMP_NO				00U
/* ****************************************************************************	*
 * P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
extern int16 main( void);
extern void UpdateMotorSpeed(void);

/* ****************************************************************************	*
 *	P u b l i c   v a r i a b l e s												*
 * ****************************************************************************	*/
#pragma space dp 																/* __TINY_SECTION__ */
extern uint8 g_e8MotorRequest;		
extern volatile uint8 g_e8ErrorElectric;										/* Status-flags electric error */
extern volatile uint8 g_e8ErrorCoil;
extern volatile uint8 g_e8ErrorVoltage;											/* Status-flags voltage */
extern uint8 g_e8MotorDirectionCCW;												/* Control/Status-flag motor rotational direction Counter Clock-wise */
extern volatile uint8 g_u8ChipResetOcc;											/* Status-flag indicate chip-reset occurred (POR) */
extern volatile uint8 g_u8StallOcc;												/* Status-flag indicate stall occurred */
extern volatile uint8 g_u8EmergencyRunOcc;										/* Status-flag indicate Emergency-run occurred */
extern volatile uint8 g_e8ErrorOverTemperature;									/* Status-flag over-temperature */
extern uint8 g_e8DegradedMotorRequest;											/* Degraded Motor Request */
extern volatile uint8 g_u8RewindFlags;
extern volatile int16 g_i16ChipTemperature;										/* Chip internal temperature */
#if _SUPPORT_AMBIENT_TEMP
extern volatile int16 g_i16AmbjTemperature;										/* Ambient Temperature */
#endif /* _SUPPORT_AMBIENT_TEMP */
extern volatile int16 g_i16MotorVoltage;										/* Motor-driver voltage */
extern uint8 g_u8StallTypeComm;													/* Stall type occurred (communication) (MMP130917-3) */
extern uint8 g_u8MotorStatusSpeed;												/* (Status) Actual motor-speed */
extern uint8 g_e8CalibrationStep;												/* Calibration step */
/* MMP151118-2 */
extern uint8 g_e8MotorCtrlMode __attribute__ ((section(".dp.noinit")));			/* Control-flags motor mode (from Master) [WD] */
extern volatile uint8 g_e8MotorStatusMode __attribute__ ((section(".dp.noinit")));	/* Status-flags motor mode (to Master) */
extern uint8 g_e8StallDetectorEna __attribute__ ((section(".dp.noinit")));		/* Control-flag Stall-detector enabled [WD] */
extern uint8 g_u8MotorHoldingCurrEna __attribute__ ((section(".dp.noinit")));	/* Control-flag motor Holding-current enabled [WD] */
extern uint16 g_u16ActualPosition __attribute__ ((section(".dp.noinit")));		/* (Control/Status) Actual motor-rotor position [WD] */
extern uint16 g_u16TargetPosition __attribute__ ((section(".dp.noinit")));		/* (Control) Target motor-rotor position (invalid) [WD] */
extern uint8 g_u8MotorCtrlSpeed __attribute__ ((section(".dp.noinit")));		/* (Control) Selected motor-speed */
extern uint16 g_u16CalibTravel __attribute__ ((section(".dp.noinit")));			/* Number of steps between two end-stops */

extern uint8 g_e8EXVMoveEnableRequestFlag;										//EXV Move enable flag, Ban
extern uint8 g_e8EXVInitDirection;												//EXV Init direction, Ban
extern uint8 g_e8WarningOverTemperature;										//EXV warning temperature, Ban
extern uint8 g_e8EXVFault;														//EXV error, Ban
extern uint8 g_e8EXVErrorBlock;													//EXV unexpect stall

#pragma space none																/* __TINY_SECTION__ */

extern volatile int16 g_i16SupplyVoltage;										/* Supply Voltage */
#if _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE)
extern volatile int16 g_i16PhaseVoltage;										/* Phase Voltage */
#endif /* _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE) */
extern volatile int16 g_i16Current;												/* Supply Current */
extern uint16 g_u16TargetPositionRewind;
#if _SUPPORT_CHIP_TEMP_PROFILE
extern uint16 g_u16TemperatureStabilityCounter;									/* Temperature stability counter */
#endif /* _SUPPORT_CHIP_TEMP_PROFILE */
#if (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44)				/* MMP150128-1 - Begin */
extern uint16 u16DegradeDelay;													/* Disable degrade delay timer */
#endif /* (_SUPPORT_DEGRADE_DELAY != FALSE) && (LINPROT == LIN2X_ACT44) */		/* MMP150128-1 - End */
#if _SUPPORT_LIN_BUS_ACTIVITY_CHECK
extern uint16 g_u16Mlx4StateCheckCounter;										/* State check counter */
extern uint8 g_u8Mlx4ErrorState;												/* Number of MLX4 Error states occurred */
#endif /* _SUPPORT_LIN_BUS_ACTIVITY_CHECK */
extern uint8 g_u8OverTemperatureCount;											/* Number of over-temperature events */
extern uint8 g_u8MotorStartDelay;												/* Motor start delay (500us) */
extern uint8 g_e8ErrorVoltageComm;												/* Status-flags voltage (Communication) */
extern uint8 g_u8MechError;														/* No mechanical error */
extern uint8 g_u8TorqueBoostRequest;											/* Torque Boost */
extern uint8 g_e8CalibPostMotorRequest;											/* Post calibration Motor Request */
extern uint16 g_u16CalibPauseCounter;

extern uint16 g_u16EXVTargetPositionTemp;

extern uint8 g_e8EXVStatusCommErr;
extern uint8 g_e8EXVStatusFaultState;
extern uint8 g_e8EXVStatusMovInProcs;
extern uint16 g_u16EXVStatusCurrentPositon;
extern uint8 g_e8EXVStatusInitStat;

#if USE_MULTI_PURPOSE_BUFFER
extern MP_BUF g_MPBuf;
#endif /* USE_MULTI_PURPOSE_BUFFER */

#endif /* MAIN_H_ */

/* EOF */
