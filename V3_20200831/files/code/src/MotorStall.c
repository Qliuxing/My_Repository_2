/*! ----------------------------------------------------------------------------
 * \file		MotorStall.c
 * \brief		MLX81310 Motor stall detectors handling
 *
 * \note		project MLX81310
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	MotorStallInitA()
 *				MotorStallCheckA()
 *				MotorStallInitO()
 *				MotorStallCheckO()
 *				MotorStallInitH()
 *				MotorStallCheckH()
 *
 * MELEXIS Microelectronic Integrated Systems
 * 
 * Copyright (C) 2012-2015 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 * ****************************************************************************	*/

#include "Build.h"
#if _SUPPORT_STALLDET_H
#include "Diagnostic.h"
#endif /* _SUPPORT_STALLDET_H */
#include "MotorStall.h"
#include "MotorDriver.h"
#include "ADC.h"
#include "private_mathlib.h"
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */

/* ****************************************************************************	*
 *	NORMAL PAGE 0 IMPLEMENTATION ( TINY Memory Space < 0x100)					*
 * ****************************************************************************	*/
#pragma space dp
uint8 l_u8StallCountA = 0;
uint8 l_u8StallCountO = 0;														/* MMP140330-1 */
uint8 g_u8StallTypeComm = (uint8) C_STALL_NOT_FOUND;
#pragma space none

/* ****************************************************************************	*
 *	NORMAL FAR IMPLEMENTATION	( NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp																/* __NEAR_SECTION__ */
/* Stall "A": Current increase */
uint16 l_u16MotorCurrentStallThrshldxN;											/* Stall-detector current-threshold x 4..16 */

#if _SUPPORT_STALLDET_O
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
int8 l_i8StallIgnoreCount = 0;
#define C_CURROSC_SZ	(2*C_MICROSTEP_PER_FULLSTEP)
uint16 l_au16CurrentCoilA[C_CURROSC_SZ];
uint16 l_au16CurrentCoilB[C_CURROSC_SZ];
uint16 g_u16CurrStallO = 0;
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
#endif /* _SUPPORT_STALLDET_O */

#if _SUPPORT_STALLDET_H
uint8 l_u8StallCountH = 0;
uint8 l_u8StallCountReboundH = 0;
uint16 l_u16HallMicroStepThrshld;
uint16 l_u16HallMicroStepIdxPre;
#endif /* _SUPPORT_STALLDET_H */
#pragma space none																/* __NEAR_SECTION__ */


#if _SUPPORT_STALLDET_A
/* ****************************************************************************	*
 * MotorStallInitA()
 *
 * Initialise Stall detector "A"
 * ****************************************************************************	*/
void MotorStallInitA( void)
{
	g_u8StallTypeComm = (uint8) C_STALL_NOT_FOUND;								/* Used for communication */

	l_u8StallCountA = 0u;														/* Stall-counter */

} /* End of MotorStallInitA() */

/* ****************************************************************************	*
 * MotorStallCheckA()
 *
 * Pre:		Nothing
 * Post:	(uint16) C_STALL_NOT_FOUND: No stall found
 *					 C_STALL_FOUND: Stall have been found 
 *
 * Check if motor is stalled
 * Stall detector "A" is based on fast current increase
 * Performance: 5us @ 20MHz
 * ****************************************************************************	*/
uint16 MotorStallCheckA( void)
{
	uint16 u16Result = C_STALL_NOT_FOUND;
	if ( (g_u16StartupDelay == 0u) && (g_u16MotorCurrentMovAvgxN > ((uint16)C_MIN_MOTORCURRENT << 4u)) )	/* MMP130916-1 */
	{
		/* Running stall detection, based on current increase */
		/* A actuator running at it's target speed, generate BEMF. When the rotor 
		   blocks, the BEMF drops to zero, and therefore the motor current increases.
		   This increase is monitored by calculating the difference (delta) between
		   a LPF filter (slowly increase) and the actual motor current. If this delta
		   increases above a specified threshold, stall is detected. */
		uint16 u16Threshold;
		if ( NVRAM_STALL_SPEED_DEPENDED )
		{
			u16Threshold = (NVRAM_STALL_CURR_THRSHLD + 120u) + (g_u8MotorStatusSpeed << 3u);	/* Speed depended Threshold */
		}
		else
		{
			u16Threshold = (NVRAM_STALL_CURR_THRSHLD + 128u);					/* Fixed Threshold */
		}
		l_u16MotorCurrentStallThrshldxN = (uint16)(((uint32)g_u16MotorCurrentLPFx64 * u16Threshold) >> 7u);
		if ( g_u16MotorCurrentMovAvgxN > l_u16MotorCurrentStallThrshldxN )
		{
			l_u8StallCountA++;
			if ( l_u8StallCountA >= 3u )
			{
				/* Real stall */
				u16Result = C_STALL_FOUND;
			}
		}
		else
		{
			if ( l_u8StallCountA > 0u)
			{
				l_u8StallCountA--;
			}
		}
	}
	return ( u16Result );
} /* End of MotorStallCheckA() */

#endif /* _SUPPORT_STALLDET_A */

#if _SUPPORT_STALLDET_O
/* ****************************************************************************	*
 * MotorStallInitO()
 *
 * Initialise Stall detector "O"
 * ****************************************************************************	*/
void MotorStallInitO( void)
{
	l_u8StallCountO = 0u;														/* MMP140330-1 */
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
	l_i8StallIgnoreCount = -C_CURROSC_SZ;
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
} /* End of MotorStallInitO() */

/* ****************************************************************************	*
 * MotorStallCheckO()
 *
 * Pre:		Nothing
 * Post:	(uint16) C_STALL_NOT_FOUND: No stall found
 *					 C_STALL_FOUND: Stall have been found
 *
 * Check if motor is stalled
 * Stall detector "O" is based on Current Oscillation feedback
 *
 * ****************************************************************************	*/
uint16 MotorStallCheckO()
{
	uint16 u16Result = C_STALL_NOT_FOUND;										/* MMP140330-1 - Begin */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
	/* Dual motor-driver current measurement. Stall based on current per coil (A & B) */
	if ( l_i8StallIgnoreCount < 0 )
	{
		/* Fill buffer with current samples, for each coil a separate buffer */
		l_au16CurrentCoilA[C_CURROSC_SZ + l_i8StallIgnoreCount] = g_u16CurrentMotorCoilA;
		l_au16CurrentCoilB[C_CURROSC_SZ + l_i8StallIgnoreCount] = g_u16CurrentMotorCoilB;
		l_i8StallIgnoreCount++;
	}
	else
	{
		/* Determine difference between current measurements now and half a (electric) rotation backwards */
		uint16 u16CurrentCoilB_Prev;
		uint16 u16CurrentCoilA_Prev = l_au16CurrentCoilA[l_i8StallIgnoreCount];	/* Coil A previous-current */
		l_au16CurrentCoilA[l_i8StallIgnoreCount] = g_u16CurrentMotorCoilA;
		u16CurrentCoilB_Prev = l_au16CurrentCoilB[l_i8StallIgnoreCount];	/* Coil B previous-current */
		l_au16CurrentCoilB[l_i8StallIgnoreCount] = g_u16CurrentMotorCoilB;
		l_i8StallIgnoreCount = ((l_i8StallIgnoreCount + 1) & (C_CURROSC_SZ - 1));
		if ( g_u16StartupDelay == 0 )
		{
			if ( u16CurrentCoilA_Prev > g_u16CurrentMotorCoilA )
			{
				g_u16CurrStallO = (u16CurrentCoilA_Prev - g_u16CurrentMotorCoilA);	/* Delta current Coil-A */
			}
			else
			{
				g_u16CurrStallO = (g_u16CurrentMotorCoilA - u16CurrentCoilA_Prev);
			}
			{
				uint16 u16StallCurrO_B;
				if ( u16CurrentCoilB_Prev > g_u16CurrentMotorCoilB )
				{
					u16StallCurrO_B = (u16CurrentCoilB_Prev - g_u16CurrentMotorCoilB);	/* Delta current Coil-B */
				}
				else
				{
					u16StallCurrO_B = (g_u16CurrentMotorCoilB - u16CurrentCoilB_Prev);
				}
				if ( u16StallCurrO_B > g_u16CurrStallO )
				{
					g_u16CurrStallO = u16StallCurrO_B;
				}
			}
			{
				uint16 u16Threshold = NVRAM_STALL_O_THRSHLD;
				if ( g_u8MotorStatusSpeed == C_MOTOR_SPEED_LOW )
					u16Threshold -= (u16Threshold >> 3);						/* 87.5% */
				else if ( g_u8MotorStatusSpeed == C_MOTOR_SPEED_HIGH )
					u16Threshold += (u16Threshold >> 2);						/* 125% */
				u16Threshold = muldivU16_U16byU16byU16( g_u16MotorCurrentMovAvgxN, u16Threshold, (256U * C_MOVAVG_SZ));
				if ( g_u16CurrStallO > u16Threshold )
				{
					l_u8StallCountO++;											/* Suspect current oscillation due to stall */
					if ( l_u8StallCountO >= NVRAM_STALL_O_WIDTH )				/* 1-8 uSteps out of 16 uSteps */
					{
						u16Result = C_STALL_FOUND;
					}
				}
				else if (l_u8StallCountO != 0)
				{
					l_u8StallCountO--;
				}
			}
		}
	}
#else  /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
	/* Single motor-driver current measurement */
	if ( (g_u16StartupDelay == 0) && (g_u16MotorCurrentMovAvgxN > (C_MIN_MOTORCURRENT << 4)) )	/* MMP130916-1 */
	{
		uint16 u16LastIdx = (l_u16MotorCurrentRawIdx - 1) & (C_MOVAVG_SZ - 1);
		uint16 u16LastCurr = l_au16MotorCurrentRaw[u16LastIdx];					/* Last current measured */
		uint16 u16CompIdx = (l_u16MotorCurrentRawIdx - (8 << NVRAM_STALL_O_OFFSET) - 1) & (C_MOVAVG_SZ - 1); /* MMP140428-1 */
		uint16 u16CompCurr = l_au16MotorCurrentRaw[u16CompIdx];					/* One full-step back measured current */
		uint16 u16DiffCurr;														/* (absolute) Difference between last current and one full-step back */
		if ( u16LastCurr > u16CompCurr )
		{
			u16DiffCurr = u16LastCurr - u16CompCurr;
		}
		else
		{
			u16DiffCurr = u16CompCurr - u16LastCurr;
		}
		if ( u16DiffCurr > (g_u16MotorCurrentMovAvgxN/(4*C_MOVAVG_SZ)) )		/* 25% of MovAvg value */
		{
			l_u8StallCountO++;													/* Suspect current oscillation due to stall */
			if ( l_u8StallCountO >= NVRAM_STALL_O_WIDTH )						/* 1-8 uSteps out of 16 uSteps */
			{
				u16Result = C_STALL_FOUND;
			}
		}
		else if (l_u8StallCountO != 0)
		{
			l_u8StallCountO--;
		}
	}
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
	return ( u16Result );														/* MMP140330-1 - End */

} /* End of MotorStallCheckO() */
#endif /* _SUPPORT_STALLDET_O */

#if _SUPPORT_STALLDET_H
/* ****************************************************************************	*
 * MotorStallInitH()
 *
 * Initialise Stall detector "H"
 * ****************************************************************************	*/
void MotorStallInitH( void)
{
	g_u8StallTypeComm = (uint8) C_STALL_NOT_FOUND;								/* Used for communication */
	l_u8StallCountH = 0u;
	l_u8StallCountReboundH = 0u;
	g_u16HallMicroStepIdx = g_u16ActuatorActPos;
	l_u16HallMicroStepIdxPre = g_u16HallMicroStepIdx;

	/* Use NVRAM_STALL_CURR_THRSHLD as threshold offset */
	/* Stepper-mode: Hall-sensor switches every 180 degrees of a electric-rotation;
	 * Set threshold at 270 degrees (75% of a full electric rotation) */
	/* l_u16HallMicroStepThrshld = muldivU16_U16byU16byU16( g_u16MotorMicroStepsPerElecRotation, 12, 4);  */ 
	/* MMP130819-3 */
	l_u16HallMicroStepThrshld = C_MOTOR_HALL_STALLDET_STEP;	/* 24 full steps equals 360 degrees */
} /* End of MotorStallInitH() */

/* ****************************************************************************	*
 * MotorStallCheckH()
 *
 * Pre:		Nothing
 * Post:	(uint16) C_STALL_NOT_FOUND: No stall found
 *					 C_STALL_FOUND: Stall have been found
 *
 *
 * ****************************************************************************	*/
uint16 MotorStallCheckH( void)
{
	if ( g_u16StartupDelay == 0u )
	{
		/* stuck stall */
		uint16 u16DeltaPosition;

		if(g_u16ActuatorActPos > l_u16HallMicroStepIdxPre)
		{
			u16DeltaPosition = g_u16ActuatorActPos - l_u16HallMicroStepIdxPre;
		}
		else
		{
			u16DeltaPosition = l_u16HallMicroStepIdxPre - g_u16ActuatorActPos;
		}
		if(u16DeltaPosition > l_u16HallMicroStepThrshld)
		{
			g_u16falg += 5;
			l_u8StallCountH++;
			if ( l_u8StallCountH >= 1u )
			{
				return ( C_STALL_FOUND );
			}
		}
		else
		{
			if ( l_u8StallCountH > 0u)
			{
				l_u8StallCountH--;
			}
		}
		/* rebounding stall:new hall edge captured  */
		if(l_u16HallMicroStepIdxPre != g_u16HallMicroStepIdx)
		{
			if(g_u16HallMicroStepIdx > l_u16HallMicroStepIdxPre)
			{
				u16DeltaPosition = g_u16HallMicroStepIdx - l_u16HallMicroStepIdxPre;
			}
			else
			{
				u16DeltaPosition = l_u16HallMicroStepIdxPre - g_u16HallMicroStepIdx;
			}
			/* Normal 6 full steps for one hall signal. */
			if((u16DeltaPosition <= ((uint16)10)) || (u16DeltaPosition >= ((uint16)22)))
			{
				g_u16falg += 1;
				l_u8StallCountReboundH++;
				if ( l_u8StallCountReboundH >= 5u )
				{
					l_u8StallCountReboundH = 0u;
					return ( C_STALL_FOUND );
				}
			}

			/* GM spec:rebounding stall accumulator should not be cleared  */
			/*
			else if(l_u8StallCountReboundH)
			{
				l_u8StallCountReboundH--;
			}
			*/
			l_u16HallMicroStepIdxPre = g_u16HallMicroStepIdx;
		}
	}
	else
	{
		/* During start-up delay, follow each micro-step */
		g_u16HallMicroStepIdx = g_u16ActuatorActPos;
		l_u16HallMicroStepIdxPre = g_u16HallMicroStepIdx;
	}
	return ( C_STALL_NOT_FOUND );
} /* End of MotorStallCheckH() */
#endif /* _SUPPORT_STALLDET_H */

/* EOF */
