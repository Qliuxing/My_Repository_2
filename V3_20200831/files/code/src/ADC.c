/*! ----------------------------------------------------------------------------
 * \file		ADC.c
 * \brief		MLX81310 ADC handling
 *
 * \note		project MLX81310
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	ADC_Init()
 *				ADC_Start()
 *				ADC_SetupShortDetection()
 *				ADC_Stop()
 *				ADC_PowerOff()
 *				ADC_IT()
 *				GetVsupply()
 *				GetVsupplyMotor()
 *				GetPhaseMotor()
 *				GetChipTemperature()
 *				GetRawMotorDriverCurrent()
 *				GetMotorDriverCurrent()
 *				MeasureVsupplyAndTemperature()
 *				MeasurePhaseVoltage()
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

#include "Build.h"
#include "ADC.h"
#include "MotorDriver.h"
#include "private_mathlib.h"
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */
#include "Timer.h"

/* ****************************************************************************	*
 *    NORMAL FAR IMPLEMENTATION	( NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp																/* __NEAR_SECTION__ */
uint16 l_u16CurrentZeroOffset = 100;											/* Zero-current ADC-offset */
#if ((LINPROT & LINXX) != LIN2J)
uint8 g_u8AdcIsrMode = C_ADC_ISR_NONE;											/* ADC ISR mode = None */
#endif /* ((LINPROT & LINXX) != LIN2J) */
uint8 l_u8AdcPowerOff = TRUE;

T_ADC_MOTORRUN_STEPPER4 volatile g_AdcMotorRunStepper4;							/* ADC results Stepper mode */
#if ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL))
uint16 g_u16CurrentMotorCoilA = 0;
uint16 g_u16CurrentMotorCoilB = 0;
#endif /* ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)) */

int16 g_i16SupplyVoltage = 1200;												/* Supply Voltage */
int16 g_i16MotorVoltage = 1200;													/* Motor driver voltage */
int16 g_i16ChipTemperature = 25;												/* Chip internal temperature */

#if _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE)
int16 g_i16PhaseVoltage = 1200U;												/* Phase Voltage */
#endif /* _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE) */

int16 g_i16Current = 0;															/* Supply Current */

#pragma space none																/* __NEAR_SECTION__ */

/* ****************************************************************************	*
 *	Internal function prototypes												*
 * ****************************************************************************	*/

/* ****************************************************************************	*
 *    ROM-Code tables															*
 * ****************************************************************************	*/
#define ADC_TJ		(ADC_CH1  | ADC_REF_2_50_V)									/*  1			2.5V	Internal Temperature Sensor */

#if _SUPPORT_VSFILTERED
#define ADC_VS		(ADC_CH4  | ADC_REF_2_50_V)									/*  4			2.5V	Vs-filtered (divided by 14) */
#else  /* _SUPPORT_VSFILTERED */
#define ADC_VS		(ADC_CH0  | ADC_REF_2_50_V)									/*  0			2.5V	Vs-unfiltered (divided by 14) */
#endif /* _SUPPORT_VSFILTERED */
#if _SUPPORT_VSMFILTERED
#define ADC_VSM		(ADC_CH4  | ADC_REF_2_50_V)									/*  4			2.5V	Vs-filtered (divided by 14) */
#else  /* _SUPPORT_VSMFILTERED */
#define ADC_VSM		(ADC_CH14 | ADC_REF_2_50_V)									/* 14			2.5V	Vs-unfiltered (divided by 14) */
#endif /* _SUPPORT_VSMFILTERED */

#if (_SUPPORT_PWM_100PCT != FALSE) && ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL))
/* In BIPOLAR_PWM_SINGLE_INDEPENDED_VSM or BIPOLAR_PWM_SINGLE_MIRRORSPECIAL mode:
 * Maximum Motor PWM duty-cycle: 100% (using filtered current channel) */
#define ADC_MCUR	(ADC_CH29 | ADC_REF_2_50_V)									/* Filtered Motor-driver Current */
#else  /* (_SUPPORT_PWM_100PCT != FALSE) && ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)) */
/* In BIPOLAR_PWM_SINGLE_INDEPENDED_VSM or BIPOLAR_PWM_SINGLE_MIRRORSPECIAL mode:
 * Maximum Motor PWM duty-cycle: 97.5% (using unfiltered current channel) */
#define ADC_MCUR	(ADC_CH13 | ADC_REF_2_50_V)									/* Unfiltered Motor-driver Current */
#endif /* (_SUPPORT_PWM_100PCT != FALSE) && ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)) */

#define ADC_PHU		(ADC_CH9  | ADC_REF_2_50_V)									/*  9			2.5V	Voltage on U Driver output (divided by 14) */
#define ADC_PHV		(ADC_CH10 | ADC_REF_2_50_V)									/* 10			2.5V	Voltage on V Driver output (divided by 14) */
#define ADC_PHW		(ADC_CH11 | ADC_REF_2_50_V)									/* 11			2.5V	Voltage on W Driver output (divided by 14) */
#define ADC_PHT		(ADC_CH25 | ADC_REF_2_50_V)									/* 15			2.5V	Voltage on T Driver output (divided by 14) */

/* ADC Vref disable */															/* Ch  Trigger	Vref	Description */
uint16 const SBASE_VREF_OFF[2] = { (ADC_CH1  | ADC_REF_OFF), 0xFFFFu};			/*  1	F/W		0.0V	Internal Temperature Sensor */

/* Current offset measurement */												/* Ch  Trigger	Vref	Description */
uint16 const SBASE_CURROFF[2] = {ADC_MCUR, 0xFFFFu };							/* 13	 --%	2.5V	Unfiltered Current */

/* Temperature measurement */													/* Ch  Trigger	Vref	Description */
uint16 const SBASE_TEMP[2] = { ADC_TJ, 0xFFFFu};									/*  1	F/W		2.5V	Internal Temperature Sensor */

/* Supply voltage measurement */												/* Ch  Trigger	Vref	Description */
uint16 const SBASE_SUPPLYVOLT[2] = { ADC_VS, 0xFFFFu};							/*  0	F/W		2.5V	Voltage Vs */
uint16 const SBASE_MOTORVOLT[2] = { ADC_VSM, 0xFFFFu};							/* 14	F/W		2.5V	Voltage Vsm */

/* Motor-driver current measurement */											/* Ch  Trigger	Vref	Description */
uint16 const SBASE_CURRENT[2] = { ADC_MCUR, 0xFFFFu};							/* 29   F/W		2.5V	Filtered Current */

/* Stepper-mode
 * ADC-duration: 1 x Motor PWM period
 */
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR)
uint16 const SBASE_INIT_4PH[5] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CMP),										/* 13	 25%	2.5V	Unfiltered Current */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 50%	2.5V	Voltage VS-filtered (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM3_CMP),										/*  1	 75%	2.5V	Internal Temperature Sensor */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM1_CNT),										/* 14	100%	2.5V	Vsm (divided by 14) */
	0xFFFF																		/* End-of-table marker */
};
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM)
uint16 const SBASE_INIT_4PH[5] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_VS   | ADC_HW_TRIGGER_PWM1_CMP),										/*  0	 25%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM2_CMP),										/*  1	 50%	2.5V	Internal Temperature Sensor */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM3_CMP),										/* 14	 75%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	100%	2.5V	Unfiltered Current */
	0xFFFF																		/* End-of-table marker */
};
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND)
uint16 const SBASE_INIT_4PH[5] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_VS   | ADC_HW_TRIGGER_PWM1_CMP),										/*  0	 25%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM2_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM3_CMP),										/* 14	 75%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CNT),										/*  1	100%	2.5V	Internal Temperature Sensor */
	0xFFFF																		/* End-of-table marker */
};
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
uint16 const SBASE_INIT_4PH[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CMP),										/*  1	 17%	2.5V	Internal Temperature Sensor */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 33%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM3_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM4_CMP),										/* 14	 75%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	100%	2.5V	Unfiltered Current */
	0xFFFFu																		/* End-of-table marker */
};
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */

#if (_SUPPORT_MOTOR_SELFTEST != FALSE)
uint16 const tAdcSelfTest4A[12] =
{
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CNT),										/*  1   100%	2.5V	Temperature */
	(ADC_PHU  | ADC_HW_TRIGGER_PWM2_CMP),										/*  9	 50%	2.5V	Phase-U voltage (divided by 14) */
	(ADC_PHU  | ADC_HW_TRIGGER_PWM1_CNT),										/*  9	100%	2.5V	Phase-U voltage (divided by 14) */
	(ADC_PHV  | ADC_HW_TRIGGER_PWM2_CMP),										/* 10	 50%	2.5V	Phase-V voltage (divided by 14) */
	(ADC_PHV  | ADC_HW_TRIGGER_PWM1_CNT),										/* 10	100%	2.5V	Phase-V voltage (divided by 14) */
	(ADC_PHW  | ADC_HW_TRIGGER_PWM2_CMP),										/* 11	 50%	2.5V	Phase-W voltage (divided by 14) */
	(ADC_PHW  | ADC_HW_TRIGGER_PWM1_CNT),										/* 11	100%	2.5V	Phase-W voltage (divided by 14) */
	(ADC_PHT  | ADC_HW_TRIGGER_PWM2_CMP),										/* 25	 50%	2.5V	Phase-T voltage (divided by 14) */
	(ADC_PHT  | ADC_HW_TRIGGER_PWM1_CNT),										/* 25	100%	2.5V	Phase-T voltage (divided by 14) */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 50%	2.5V	Voltage VS (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	100%	2.5V	Unfiltered Current */
	0xFFFF																		/* End-of-table marker */
};
uint16 const tAdcSelfTest4B[12] =
{
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CNT),										/*  1   100%	2.5V	Temperature */
	(ADC_PHU  | ADC_HW_TRIGGER_PWM2_CMP),										/*  9	 50%	2.5V	Phase-U voltage (divided by 14) */
	(ADC_PHU  | ADC_HW_TRIGGER_PWM1_CNT),										/*  9	100%	2.5V	Phase-U voltage (divided by 14) */
	(ADC_PHV  | ADC_HW_TRIGGER_PWM2_CMP),										/* 10	 50%	2.5V	Phase-V voltage (divided by 14) */
	(ADC_PHV  | ADC_HW_TRIGGER_PWM1_CNT),										/* 10	100%	2.5V	Phase-V voltage (divided by 14) */
	(ADC_PHW  | ADC_HW_TRIGGER_PWM2_CMP),										/* 11	 50%	2.5V	Phase-W voltage (divided by 14) */
	(ADC_PHW  | ADC_HW_TRIGGER_PWM1_CNT),										/* 11	100%	2.5V	Phase-W voltage (divided by 14) */
	(ADC_PHT  | ADC_HW_TRIGGER_PWM2_CMP),										/* 25	 50%	2.5V	Phase-T voltage (divided by 14) */
	(ADC_PHT  | ADC_HW_TRIGGER_PWM1_CNT),										/* 25	100%	2.5V	Phase-T voltage (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM2_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_VS   | ADC_HW_TRIGGER_PWM1_CNT),										/*  0	100%	2.5V	Voltage VS (divided by 14) */
	0xFFFF																		/* End-of-table marker */
};
#endif /* (_SUPPORT_MOTOR_SELFTEST != FALSE) */

/* ****************************************************************************	*
 * ADC_StartSoftTrig()
 *
 * Start ADC measurement using Software trigger.
 * ****************************************************************************	*/
void ADC_StartSoftTrig( void)													/* MMP140709-1 - Begin */
{
	PEND = CLR_ADC_IT;
	ADC_CTRL = ADC_START;
	if ( l_u8AdcPowerOff != 0u )
	{
		NopDelay( DELAY_50us); /*lint !e522 */
	}
	l_u8AdcPowerOff = FALSE;
	NopDelay( ADC_SETTING); /*lint !e522 */
	ADC_CTRL = (ADC_START | ADC_SOFT_TRIG);										/* Single shot */
	while ((ADC_CTRL & ADC_START) != 0u) 			/* lint -e{722} */ 			/* Wait for ADC result */
	{
		
	}
} /* End of ADC_StartSoftTrig() */													/* MMP140709-1 - End */

/* ****************************************************************************	*
 * ADC_Init()
 *
 * Measure Zero-current offset
 * ADC ISR priority: 3
 * ****************************************************************************	*/
void ADC_Init( void)
{
	uint16 volatile u16ZCO;
	ADC_Stop();																	/* clear the ADC control register */
	ADC_SBASE = (uint16) SBASE_CURROFF;
	ADC_DBASE = (uint16) &u16ZCO;
	ADC_StartSoftTrig();														/* MMP140709-1 */
	l_u16CurrentZeroOffset = /* lint -e{530} */ u16ZCO;
	ADC_CTRL = (ADC_START | ADC_SOFT_TRIG);										/* Single shot */
	while ((ADC_CTRL & ADC_START) != 0u) /* lint -e{722} */ 						/* Wait for ADC result */
	{
		
	}
	l_u16CurrentZeroOffset = (l_u16CurrentZeroOffset + u16ZCO) >> 1u;

#ifdef _SUPPORT_CALIBRATED_ZERO_CURRENT
	/* Validate zero-offset current measurement against calibration */
	uint16 u16Delta;
	if ( l_u16CurrentZeroOffset > EE_OMCURR )
		u16Delta = l_u16CurrentZeroOffset - EE_OMCURR;
	else
		u16Delta = EE_OMCURR - l_u16CurrentZeroOffset;
	if ( u16Delta > 6 )															/* Need to define this value: 6 LSB's is approx. 5mA */
		l_u16CurrentZeroOffset = EE_OMCURR;										/* Take calibrated current offset, insetad of measured offset */
#endif /* SUPPORT_CALIBRATED_ZERO_CURRENT */


	PRIO &= ~((uint16)3u << 2u);												/* ADC IRQ Priority: 5 (3..6) (MMP150106-1) */
	PRIO |= ((uint16)(5u - 3u) << 2u);
} /* End of ADC_Init() */

/* ****************************************************************************	*
 * ADC_Start()
 *
 * ADC ISR priority: 3
 * ****************************************************************************	*/
void ADC_Start( void)
{
	ADC_Stop();																	/* clear the ADC control register */
	ADC_SBASE = (uint16) SBASE_INIT_4PH;
	ADC_DBASE = (uint16) &g_AdcMotorRunStepper4;
	ADC_CTRL  = (ADC_LOOP | ADC_TRIG_SRC | ADC_SYNC_SOC);						/* Loop cycle of conversion is done */
	ADC_CTRL |= ADC_START;														/* Start ADC */
	if ( l_u8AdcPowerOff != 0u)													/* MMP140618-1: Add delay */
	{
		NopDelay( DELAY_mPWM); /*lint !e522 */
	}
	l_u8AdcPowerOff = FALSE;
} /* End of ADC_Start() */


/* ****************************************************************************	*
 * ADC_Stop()
 *
 * Stop ADC (with waiting for pending ADC conversions to be finished)
 * ****************************************************************************	*/
void ADC_Stop(void)
{	
	if ( (ADC_CTRL & ADC_START) != 0u )											/* In case ADC is active, wait to finish it */
	{
		ADC_CTRL &= ~(uint16)(ADC_LOOP | ADC_TRIG_SRC | ADC_SYNC_SOC);			/* Stop looping and HW-triggers */
		while ( (ADC_CTRL & ADC_START) != 0u )									/* As long as the ADC is active ... */
		{
			ADC_CTRL |= ADC_SOFT_TRIG;											/* ... Set S/W trigger */
			NopDelay( DELAY_7us); /*lint !e522 */
		}
	}
	ADC_CTRL = 0u;																/* Clear the ADC control register */
	BEGIN_CRITICAL_SECTION();
	MASK &= ~EN_ADC_IT;															/* Disable ADC Interrupt */
	END_CRITICAL_SECTION();
#if ((LINPROT & LINXX) != LIN2J)
	g_u8AdcIsrMode = C_ADC_ISR_NONE;
#endif /* ((LINPROT & LINXX) != LIN2J) */
} /* End of ADC_Stop() */


/* ****************************************************************************	*
 * ADC_PowerOff()
 *
 * Power-off ADC, by disabling reference voltage
 * ****************************************************************************	*/
void ADC_PowerOff( void)
{
	ADC_Stop();																	/* Stop ADC conversion, and disable ADC-IRQ */
	ADC_SBASE = (uint16) SBASE_VREF_OFF;
	PEND = CLR_ADC_IT;
	ADC_CTRL = ADC_START;
	ADC_Stop();
	l_u8AdcPowerOff = TRUE;
} /* End of ADC_PowerOff() */

/* ****************************************************************************	*
 * ADC_IT()
 *
 * ADC Interrupt Service Routine
 * In case no ADC_ISR action required, this ISR has 4us overhead (is approx: 9.5% at 24kHz PWM)
 * (push/pop + check for LIN-AA + Check BEMF ZC)
 * ****************************************************************************	*/
__interrupt__ void ADC_IT(void) 
{
#if ((LINPROT & LINXX) != LIN2J)
	if ( g_u8AdcIsrMode == C_ADC_ISR_LIN_AA ) 									/* LIN-AutoAddressing sequence */
	{
		/* AutoAddressingReadADCResult(); */	 									/* See MELEXIS doc */
	}
#endif /* ((LINPROT & LINXX) != LIN2J) */
} /* End of ADC_IT() */

/* ****************************************************************************	*
 * GetVsupply()
 *
 * Get Supply-voltage [10mV]
 * ****************************************************************************	*/
void GetVsupply( void)
{
	uint16 u16FilteredSupplyVoltage = g_AdcMotorRunStepper4.FilteredSupplyVoltage;
#if _SUPPORT_VSFILTERED
	g_i16SupplyVoltage = (int16) ((mulI32_I16byU16( (int16)(u16FilteredSupplyVoltage - EE_OVOLTAGE), EE_GVOLTAGE) + (C_GVOLTAGE_DIV/2)) / C_GVOLTAGE_DIV);
#else  /* _SUPPORT_VSFILTERED */
	g_i16SupplyVoltage = (int16) ((mulI32_I16byI16( (int16)(u16FilteredSupplyVoltage - EE_OADC), EE_GADC) + (C_GVOLTAGE_DIV/2)) / C_GVOLTAGE_DIV);
#endif /* _SUPPORT_VSFILTERED */
} /* End of GetVsupply() */

#if _SUPPORT_MLX_DEBUG_MODE
uint16 GetRawChipSupply( void)
{
	uint16 u16FilteredSupplyVoltage = g_AdcMotorRunStepper4.FilteredSupplyVoltage;
	return ( u16FilteredSupplyVoltage );
} /* End of GetRawChipSupply() */
#endif /* _SUPPORT_MLX_DEBUG_MODE */

/* ****************************************************************************	*
 * GetVsupplyMotor()
 *
 * Get Supply-voltage [10mV]
 * ****************************************************************************	*/
void GetVsupplyMotor( void)
{
	uint16 u16FilteredDriverVoltage = g_AdcMotorRunStepper4.FilteredDriverVoltage;
#if _SUPPORT_VSMFILTERED
	g_i16MotorVoltage = (int16) ((mulI32_I16byU16( (int16)(u16FilteredDriverVoltage - EE_OVOLTAGE), EE_GVOLTAGE) + (C_GVOLTAGE_DIV/2)) / C_GVOLTAGE_DIV);
#else  /* _SUPPORT_VSMFILTERED */
	g_i16MotorVoltage = (int16) ((mulI32_I16byU16( (int16)(u16FilteredDriverVoltage - EE_OADC), EE_GADC) + (C_GVOLTAGE_DIV/2)) / C_GVOLTAGE_DIV);
#endif /* _SUPPORT_VSMFILTERED */
} /* End of GetVsupplyMotor() */

/* ****************************************************************************	*
 * ResetChipTemperature()
 *
 * Reset Chip temperature
 * ****************************************************************************	*/
void ResetChipTemperature( void)
{
	g_AdcMotorRunStepper4.IntTemperatureSensor = 0u;

} /* End of ResetChipTemperature() */

/* ****************************************************************************	*
 * GetChipTemperature()
 *
 * Get Chip temperature [C]
 * ****************************************************************************	*/
void GetChipTemperature( uint16 u16Init)										/* MMP131020-1 */
{
	uint16 u16ChipTemperatureSensor = g_AdcMotorRunStepper4.IntTemperatureSensor;
	int16 i16ChipTemperature, i16ChipTempDelta;

#if _SUPPORT_TWO_LINE_TEMP_INTERPOLATION
	if ( u16ChipTemperatureSensor < EE_OTEMP )
	{
		/* Temperature above 35C */
		i16ChipTemperature = (int16) muldivU16_U16byU16byU16( (EE_OTEMP - u16ChipTemperatureSensor), (uint16)(125 - 35), (EE_OTEMP - EE_HIGHTEMP)) + EE_MIDTEMP;
	}
	else
	{
		/* Temperature below 35C */
		i16ChipTemperature = EE_MIDTEMP - (int16) muldivU16_U16byU16byU16( (u16ChipTemperatureSensor - EE_OTEMP), (uint16)(35 - (-40)), (EE_LOWTEMP - EE_OTEMP));
	}
#else  /* _SUPPORT_TWO_LINE_TEMP_INTERPOLATION */
	i16ChipTemperature = (mulI32_I16byI16( (EE_OTEMP - u16ChipTemperatureSensor), EE_GTEMP) / C_GTEMP_DIV) + EE_MIDTEMP;
#endif /* _SUPPORT_TWO_LINE_TEMP_INTERPOLATION */

	if ( u16Init == FALSE )														/* MMP131020-1 */
	{
		i16ChipTempDelta = i16ChipTemperature - g_i16ChipTemperature;			/* Delta-temp = new-temp - previous-temp */
		if ( i16ChipTempDelta < 0 )
		{
			i16ChipTempDelta = -i16ChipTempDelta;								/* Absolute temperature change */
		}
		if ( (uint16) i16ChipTempDelta > C_TEMPERATURE_JUMP )					/* Temperature change small, then accept new temperature */
		{
			if ( i16ChipTemperature > g_i16ChipTemperature )					/* To great temperature change; Check temperature change "direction" */
			{
				i16ChipTemperature = g_i16ChipTemperature + 1;					/* Increase by one degree */
			}
			else
			{
				i16ChipTemperature = g_i16ChipTemperature - 1;					/* Decrease by one degree */
			}
		}
	}																			/* MMP131020-1 */
	g_i16ChipTemperature = i16ChipTemperature;
} /* End of GetChipTemperature() */

#if _SUPPORT_MLX_DEBUG_MODE
uint16 GetRawTemperature( void)
{
	uint16 u16ChipTemperatureSensor = g_AdcMotorRunStepper4.IntTemperatureSensor;
	return ( u16ChipTemperatureSensor );
} /* End of GetRawTemperature() */
#endif /* _SUPPORT_MLX_DEBUG_MODE */

/* ****************************************************************************	*
 * GetRawMotorDriverCurrent()
 *
 * Get (raw) Motor Driver Current [ADC-LSB]
 * ****************************************************************************	*/
uint16 GetRawMotorDriverCurrent( void)
{
	uint16 u16Current = 0;
#if ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL))
	u16Current = g_AdcMotorRunStepper4.UnfilteredDriverCurrent;
	if ( u16Current > l_u16CurrentZeroOffset )
	{
		g_u16CurrentMotorCoilA = u16Current -  l_u16CurrentZeroOffset;
	}
	else
	{
		g_u16CurrentMotorCoilA = 0u;
	}
	
	u16Current = g_AdcMotorRunStepper4.UnfilteredDriverCurrent2;
	if ( u16Current > l_u16CurrentZeroOffset )
	{
		g_u16CurrentMotorCoilB = u16Current - l_u16CurrentZeroOffset;
	}
	else
	{
		g_u16CurrentMotorCoilB = 0u;
	}
	u16Current = g_u16CurrentMotorCoilA + g_u16CurrentMotorCoilB;
#else  /* ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)) */
	u16Current = (uint16) g_AdcMotorRunStepper4.UnfilteredDriverCurrent;
	if ( u16Current > l_u16CurrentZeroOffset )
	{
		u16Current = u16Current - l_u16CurrentZeroOffset;
	}
#endif /* ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)) */

	return ( u16Current );
} /* End of GetRawMotorDriverCurrent() */

/* ****************************************************************************	*
 * GetMotorDriverCurrent()
 *
 * Get Motor Driver Current [mA]
 * ****************************************************************************	*/
int16 GetMotorDriverCurrent( void)
{
	uint16 u16Current = GetRawMotorDriverCurrent();
	u16Current = (uint16) ((mulU32_U16byU16( u16Current, EE_GMCURR) + (C_GMCURR_DIV/2u)) / C_GMCURR_DIV);	/* MMP131117-1 */
	return ( (int16) u16Current);
} /* End of GetMotorDriverCurrent() */

/* ****************************************************************************	*
 * MeasureVsupplyAndTemperature()
 *
 * Measure Vbat and Temperature (single-shot, software triggered)
 * ****************************************************************************	*/
void MeasureVsupplyAndTemperature( void)
{	
	ADC_Stop();
	ADC_SBASE = (uint16) SBASE_MOTORVOLT;										/* switch ADC input source to Voltage */
	ADC_DBASE = (uint16) &g_AdcMotorRunStepper4.FilteredDriverVoltage;
	ADC_StartSoftTrig();														/* MMP140709-1 */

	ADC_SBASE = (uint16) SBASE_SUPPLYVOLT;										/* switch ADC input source to Voltage */
	ADC_DBASE = (uint16) &g_AdcMotorRunStepper4.FilteredSupplyVoltage;
	ADC_StartSoftTrig();														/* MMP140709-1 */

	ADC_SBASE = (uint16) SBASE_TEMP;											/* switch ADC input source to Temperature */
	ADC_DBASE = (uint16) &g_AdcMotorRunStepper4.IntTemperatureSensor;
	ADC_StartSoftTrig();														/* MMP140709-1 */
} /* End of MeasureVsupplyAndTemperature() */

/* ****************************************************************************	*
 * MeasureMotorCurrent()
 *
 * Measure Motor-current (filtered) (single-shot, software triggered)
 * ****************************************************************************	*/
void MeasureMotorCurrent( void)
{
	ADC_Stop();
	ADC_SBASE = (uint16) SBASE_CURRENT;											/* switch ADC input source to Motor-driver current */
	ADC_DBASE = (uint16) &g_AdcMotorRunStepper4.UnfilteredDriverCurrent;
	ADC_StartSoftTrig();														/* MMP140709-1 */

} /* End of MeasureMotorCurrent() */

#if (_SUPPORT_PHASE_SHORT_DET != FALSE) || (_SUPPORT_MOTOR_SELFTEST != FALSE)
/* ****************************************************************************	*
 * MeasurePhaseVoltage()
 *
 * Measure Phase voltage (single-shot, software triggered)
 * ****************************************************************************	*/
void MeasurePhaseVoltage( uint16 u16AdcSbase)			/* MMP130919-1 - Begin */
{
	uint16 u16PhaseVoltage;
	
	ADC_Stop();
	ADC_SBASE = (uint16) u16AdcSbase;											/* switch ADC input source to Motor-driver current */
	ADC_DBASE = (uint16) &u16PhaseVoltage;
	ADC_StartSoftTrig();														/* MMP140709-1 */

	g_i16PhaseVoltage = (int16) mulI16_I16byI16RndDiv64((int16)(u16PhaseVoltage - EE_OADC), EE_GADC);

} /* End of MeasurePhaseVoltage() */				


void MeasureSelfTest4PH(uint16 u16AdcSbase,T_ADC_SELFTEST_4PH *padcMotorSelfTest4Ph)
{
	ADC_Stop();																/* clear the ADC control register */

	ADC_SBASE = (uint16) u16AdcSbase;										
	ADC_DBASE = (uint16) padcMotorSelfTest4Ph;
	ADC_CTRL |= (ADC_TRIG_SRC | ADC_SYNC_SOC);								/* Single cycle of conversion is done */
	ADC_CTRL |= ADC_START;													/* Start ADC */

	/* This takes about 4 Motor PWM-periods per self-test */
	while ((ADC_CTRL & ADC_START) != 0u)			/* lint -e{722} */					/* Wait for ADC result (Time-out?) */
	{
		
	}
}

/* MMP130919-1 - End */
#endif /* _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE) */

/* EOF */
