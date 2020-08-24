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

#ifndef SYSLIB_H_
#include "syslib.h"
#endif /* SYSLIB_H_ */

#define C_ADC_ISR_NONE		0													/* No ADC Interrupt Service */
#define C_ADC_ISR_LIN_AA	1													/* LIN-AutoAddressing Interrupt Service */
#define C_ADC_ISR_BEMF		2													/* BEMF Interrupt Service */
#define C_ADC_ISR_SRP		3													/* SRP Interrupt Service */

#define ZC_MID		0															/* Zero-crossing detection on mid-supply level */
#define ZC_SUPPLY	1															/* Zero-crossing detection on supply level */

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

#define ADC_SETTING (uint16)(((2U*PLL_freq)/(1000000U*2*CYCLES_PER_INSTR)) + 1)	/* 2us: 2us*PLL-freq/(10000000us/s * #cycles/instruction) */

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
extern void GetChipTemperature( uint16 u16Init);								/* Get Chip temperature [C] (MMP131020-1) */
extern int16  GetMotorDriverCurrent( void);										/* Get Motor Driver Current [mA] */
extern uint16 GetRawMotorDriverCurrent( void);									/* Get raw Motor Driver Current [ADC-LSB] */
extern void MeasureVsupplyAndTemperature( void);								/* Measure (filtered) Supply and (chip) temperature */
extern void MeasureMotorCurrent( void);											/* Measure Motor-driver (filtered) current */
#if _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE)
extern void MeasurePhaseVoltage( uint16 u16AdcSbase);							/* Measure Phase voltage (MMP130919-1) */
#endif /* _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE) */

#if (_SUPPORT_MOTOR_SELFTEST != FALSE)
extern uint16 const tAdcSelfTest4A[12];
extern uint16 const tAdcSelfTest4B[12];
#endif /* (_SUPPORT_MOTOR_SELFTEST != FALSE) */

/* ****************************************************************************	*
 *	P u b l i c   v a r i a b l e s												*
 * ****************************************************************************	*/
#if ((LINPROT & LINXX) != LIN2J)
extern uint8 g_u8AdcIsrMode;													/* ADC ISR mode */
#endif /* ((LINPROT & LINXX) != LIN2J) */
#if ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL))
extern uint16 g_u16CurrentMotorCoilA;
extern uint16 g_u16CurrentMotorCoilB;
#endif /* ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)) */

#endif /* ADC_H_ */

/* EOF */
