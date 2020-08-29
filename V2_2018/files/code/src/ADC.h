/*! ----------------------------------------------------------------------------
 * \file		ADC.h
 * \brief		MLX81300 ADC handling
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
 *
 *				
 *
 * MELEXIS Microelectronic Integrated Systems
 * 
 * Copyright (C) 2012-2014 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * ****************************************************************************	*/

#ifndef ADC_H_
#define ADC_H_

#include "Build.h"

#define C_ADC_MIN			0x000U
#define C_ADC_MAX			0x3FFU												/* 10-bit ADC */
#define C_ADC_MID			((C_ADC_MAX - C_ADC_MIN) / 2U)

#define C_ADC_ISR_NONE		0U													/* No ADC Interrupt Service */
#define C_ADC_ISR_LIN_AA	1U													/* LIN-AutoAddressing Interrupt Service */
#define C_ADC_ISR_BEMF		2U													/* BEMF Interrupt Service */
#define C_ADC_ISR_SRP		3U													/* SRP Interrupt Service */

#define ZC_MID				0U													/* Zero-crossing detection on mid-supply level */
#define ZC_SUPPLY			1U													/* Zero-crossing detection on supply level */

/* ADC Channels */
#define ADC_VS				(ADC_CH0  | (uint16) ADC_REF_2_50_V)				/*  0			2.5V	Vs-unfiltered (divided by 14) */
#define ADC_TJ				(ADC_CH1  | (uint16) ADC_REF_2_50_V)				/*  1			2.5V	Chip-Junction temperature */
#define ADC_VDDD			(ADC_CH2  | (uint16) ADC_REF_2_50_V)				/*  2			2.5V	VDDD voltage */
#define ADC_VDDA			(ADC_CH3  | (uint16) ADC_REF_2_50_V)				/*  3			2.5V	VDDA voltage (divided by 2) */
#if _SUPPORT_VSMFILTERED
#define ADC_VSM				(ADC_CH4  | (uint16) ADC_REF_2_50_V)				/*  4			2.5V	Vsm-filtered (divided by 14) */
#else  /* _SUPPORT_VSMFILTERED */
#define ADC_VSM				(ADC_CH14 | (uint16) ADC_REF_2_50_V)				/* 14			2.5V	Vsm-unfiltered (divided by 14) */
#endif /* _SUPPORT_VSMFILTERED */
#define ADC_VPHT			(ADC_CH25 | (uint16) ADC_REF_2_50_V)				/* 25			2.5V	Phase T voltage (divided by 14) */
#define ADC_VPHU			(ADC_CH9  | (uint16) ADC_REF_2_50_V)				/*  9			2.5V	Phase U voltage (divided by 14) */
#define ADC_VPHV			(ADC_CH10 | (uint16) ADC_REF_2_50_V)				/* 10			2.5V	Phase V voltage (divided by 14) */
#define ADC_VPHW			(ADC_CH11 | (uint16) ADC_REF_2_50_V)				/* 11			2.5V	Phase W voltage (divided by 14) */
#define ADC_MCUR			(ADC_CH13 | (uint16) ADC_REF_2_50_V)				/* 13			2.5V	Motor-driver Current-unfiltered */
#define ADC_MCURF			(ADC_CH29 | (uint16) ADC_REF_2_50_V)				/* 29			2.5V	Motor-driver Current-filtered */
#define ADC_IO0				(ADC_CH5  | (uint16) ADC_REF_2_50_V)				/*  5			2.5V	IO[0] (X-Y Resolver) */
#define ADC_IO1				(ADC_CH6  | (uint16) ADC_REF_2_50_V)				/*  6			2.5V	IO[1] (X-Y Resolver) */
#define ADC_IO2				(ADC_CH7  | (uint16) ADC_REF_2_50_V)				/*  7			2.5V	IO[2] (X-Y Resolver) */
#define ADC_IO3				(ADC_CH8  | (uint16) ADC_REF_2_50_V)				/*  8			2.5V	IO[3] (X-Y Resolver) */
#define ADC_IO4				(ADC_CH21 | (uint16) ADC_REF_2_50_V)				/* 21			2.5V	IO[4] test-purpose */
#define ADC_IO5				(ADC_CH22 | (uint16) ADC_REF_2_50_V)				/* 22			2.5V	IO[5] */
#define ADC_IO6				(ADC_CH26 | (uint16) ADC_REF_2_50_V)				/* 21			2.5V	IO[6] */
#define ADC_IO7				(ADC_CH27 | (uint16) ADC_REF_2_50_V)				/* 22			2.5V	IO[7] */
#define ADC_IO3HV			(ADC_CH24 | (uint16) ADC_REF_2_50_V)				/* 24			2.5V	IO[3].HV test-purpose */
#define ADC_VREF			(ADC_CH17 | (uint16) ADC_REF_2_50_V)				/* 17			2.5V	Vref (divided by 16) */
#define ADC_EOT				0xFFFFU

#define C_MIN_VDDA			644U												/* 3.15V --> 3.15V/2 = 1.575V/2.5Vref * 1023 = 644.49 */
#define C_MAX_VDDA			706U												/* 3.45V --> 3.45V/2 = 1.725V/2.5Vref * 1023 = 705.87 */
#define C_ABS_MAX_VDDA		736U												/* Abs. Max Rating: 3.6V --> 3.6V/2 = 1.8V/2.5Vref * 1023 = 736.56 */
#define C_MIN_VDDD			724U												/* 1.77V --> 1.77V/2.5Vref * 1023 = 724.284 */
#define C_MAX_VDDD			900U												/* 2.20V --> 2.20V/2.5Vref * 1023 = 900.24 */
#define C_ABS_MAX_VDDD		798U												/* Abs. Max Rating: 1.95V --> 1.95V/2.5Vref * 1023 = 797.94 */
#define C_MAX_IO_LOW		164U												/* 0.4V --> 0.4V/2.5Vref * 1023 = 163.68 */

/* MMP: PC-lint first typedef always fails; So create a dummy */
typedef unsigned char lint_typedef_fail_adc_h;

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR)
typedef struct /* _T_ADC_MOTORRUN_STEPPER4 */
{
	uint16 UnfilteredDriverCurrent;												/* 25/25%: Unfiltered Motor Driver Current */
#if _SUPPORT_PHASE_SHORT_DET
	uint16 FilteredPhaseVoltage;												/* --/44%: Motor Phase Voltage */
#endif /* _SUPPORT_PHASE_SHORT_DET */
	uint16 FilteredSupplyVoltage;												/* 50/62%: Filtered Supply Voltage */
	uint16 IntTemperatureSensor;												/* 75/81%: Internal Temperature Sensor */
	uint16 FilteredDriverVoltage;												/*  100%:  Motor Driver Voltage */
} T_ADC_MOTORRUN_STEPPER4;
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM)
typedef struct /* _T_ADC_MOTORRUN_STEPPER4 */
{
#if _SUPPORT_PHASE_SHORT_DET
	uint16 FilteredPhaseVoltage;												/* --/20%: Motor Phase Voltage */
#endif /* _SUPPORT_PHASE_SHORT_DET */
	uint16 FilteredSupplyVoltage;												/* 25/40%: Filtered Supply Voltage */
	uint16 IntTemperatureSensor;												/* 50/60%: Internal Temperature Sensor */
	uint16 FilteredDriverVoltage;												/* 75/80%: Motor Driver Voltage */
	uint16 UnfilteredDriverCurrent;												/*  100%:  Unfiltered Motor Driver Current */
} T_ADC_MOTORRUN_STEPPER4;
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND)
typedef struct /* _T_ADC_MOTORRUN_STEPPER4 */
{
	uint16 FilteredSupplyVoltage;												/* 25/20%: Filtered Supply Voltage */
#if _SUPPORT_PHASE_SHORT_DET
	uint16 FilteredPhaseVoltage;												/* --/40%: Motor Phase Voltage */
#endif /* _SUPPORT_PHASE_SHORT_DET */
	uint16 UnfilteredDriverCurrent;												/* 50/60%: Unfiltered Motor Driver Current */
	uint16 FilteredDriverVoltage;												/* 75/80%: Motor Driver Voltage */
	uint16 IntTemperatureSensor;												/*   100%: Internal Temperature Sensor */
} T_ADC_MOTORRUN_STEPPER4;
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
typedef struct /* _T_ADC_MOTORRUN_STEPPER4 */
{
	uint16 IntTemperatureSensor;												/* 17/17%: Internal Temperature Sensor */
	uint16 FilteredSupplyVoltage;												/* 25/33%: Filtered Supply Voltage */
	uint16 UnfilteredDriverCurrent2;											/* 50/50%: Unfiltered Motor Driver Current */
#if _SUPPORT_PHASE_SHORT_DET
	uint16 FilteredPhaseVoltage;												/* --/67%: Motor Phase Voltage */
#endif /* _SUPPORT_PHASE_SHORT_DET */
	uint16 FilteredDriverVoltage;												/* 83/83%: Motor Driver Voltage */
	uint16 UnfilteredDriverCurrent;												/*  100%:  Unfiltered Motor Driver Current */
} T_ADC_MOTORRUN_STEPPER4;
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */

#if (_SUPPORT_MOTOR_SELFTEST != FALSE)
typedef struct /* _T_ADC_SELFTEST_4PH */
{
	uint16 IntTemperatureSensor;												/* 100%: IC Junction Temperature */
	uint16 PhaseU_HighVoltage;													/*  50%: Phase-U High-voltage */
	uint16 PhaseU_LowVoltage;													/* 100%: Phase-U Low-voltage */
	uint16 PhaseV_HighVoltage;													/*  50%: Phase-V High-voltage */
	uint16 PhaseV_LowVoltage;													/* 100%: Phase-V Low-voltage */
	uint16 PhaseW_HighVoltage;													/*  50%: Phase-W High-voltage */
	uint16 PhaseW_LowVoltage;													/* 100%: Phase-W Low-voltage */
	uint16 PhaseT_HighVoltage;													/*  50%: Phase-T High-voltage */
	uint16 PhaseT_LowVoltage;													/* 100%: Phase-T Low-voltage */
	uint16 FilteredSupplyVoltage;												/*  50%: Filtered Supply Voltage */
	uint16 UnfilteredDriverCurrent;												/* 100%: Unfiltered Motor Driver Current */
} T_ADC_SELFTEST_4PH;
#endif /* (_SUPPORT_MOTOR_SELFTEST != FALSE) */

#define ADC_SETTING (uint16)(((2U*PLL_freq)/(1000000U*(2U*CYCLES_PER_INSTR))) + 1U)	/* 2us: 2us*PLL-freq/(10000000us/s * #cycles/instruction) */

static INLINE uint16 GetRawVsupplyMotor( void)
{
extern T_ADC_MOTORRUN_STEPPER4 volatile g_AdcMotorRunStepper4;					/* ADC results Stepper mode */

	return ( g_AdcMotorRunStepper4.FilteredDriverVoltage );
} /* End of GetRawVsupplyMotor() */

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
extern void ADC_Init( void);													/* Initialise ADC */
#if _SUPPORT_PHASE_SHORT_DET
extern void ADC_Start( uint16 u16Mode);											/* Start ADC */
extern void ADC_SetupShortDetection( uint16 u16Mode);							/* Setup ADC for correct motor phase voltage measurement */
#else  /* _SUPPORT_PHASE_SHORT_DET */
extern void ADC_Start( void);													/* Start ADC */
#endif /* _SUPPORT_PHASE_SHORT_DET */
extern void ADC_Stop( void);													/* Stop ADC */
extern void ADC_PowerOff( void);												/* Power-off ADC */
extern void GetVsupply( void);													/* Get Supply-voltage [10mV] */
extern void  GetVsupplyMotor( void);											/* Get Motor-driver-voltage [10mV] */
#if _SUPPORT_PHASE_SHORT_DET
extern void GetPhaseMotor( void);												/* Get Phase-voltage [10mV] */
#endif /* _SUPPORT_PHASE_SHORT_DET */
extern void ResetChipTemperature( void);										/* Reset Chip temperature */
extern void GetChipTemperature( uint16 u16Init);								/* Get Chip temperature [C] */
extern int16  GetMotorDriverCurrent( void);										/* Get Motor Driver Current [mA] */
extern uint16 GetRawMotorDriverCurrent( void);									/* Get raw Motor Driver Current [ADC-LSB] */
extern void MeasureVsupplyAndTemperature( void);								/* Measure (filtered) Supply and (chip) temperature */
extern void MeasureMotorCurrent( void);											/* Measure Motor-driver (filtered) current */
#if _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE)
extern void MeasurePhaseVoltage( uint16 u16AdcSbase);							/* Measure Phase voltage */
#endif /* _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE) */

#if (_SUPPORT_MOTOR_SELFTEST != FALSE)
extern uint16 const tAdcSelfTest4A[12];
extern uint16 const tAdcSelfTest4B[12];
#endif /* (_SUPPORT_MOTOR_SELFTEST != FALSE) */

/* ****************************************************************************	*
 *	P u b l i c   v a r i a b l e s												*
 * ****************************************************************************	*/
extern uint16 g_u16MCurrgain;													/* MMP160616-1 */
#if _SUPPORT_LIN_AA
extern uint8 g_u8AdcIsrMode;													/* ADC ISR mode */
#endif /* _SUPPORT_LIN_AA */
#if ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL))
extern uint16 g_u16CurrentMotorCoilA;
extern uint16 g_u16CurrentMotorCoilB;
#endif /* ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)) */

#endif /* ADC_H_ */

/* EOF */
