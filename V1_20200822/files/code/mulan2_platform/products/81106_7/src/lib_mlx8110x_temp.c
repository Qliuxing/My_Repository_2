/*
 * Copyright (C) 2014-2015 Melexis GmbH
 *
 * This file is part of the Mlx8110x module library.
 *
 *
 * Module prefix: mdl_TEMP
 * All module interface functions start with this letters.
 *
 * This library provides functions calculate the Temperature in °C
 * out of the ADC measurements results
 *
 *
 * ==========================================================================
 * History:
 *   Revision 1.0
 *     - Initial release
 *
 * ========================================================================== */

/* ==========================================================================
 * Includes
 * ========================================================================== */
#include <alib.h>                                    
#include <plib.h>
/* ==========================================================================
 * Private type definitions, macros, defines
 * ========================================================================== */
#define SCALE_FACTOR 	5		/* 2^5 -> 32 -> used for scaling temperature via fixpoint arithmetic */
#define LOWTEMP 		-40 	/* low temperature */
#define HIGHTEMP 		125 	/* high temperature */
#define MIDTEMP 		35 		/* middle temperature */

/* variables for the linear function of the internal temperature diode*/
int16 hightemp_m = 0;
int16 hightemp_b = 0;
int16 lowtemp_m = 0;
int16 lowtemp_b = 0;


/* ==========================================================================
 * Declaration public variables
 * ========================================================================== */

/* ==========================================================================
 * Declaration private (static) functions
 * ========================================================================== */

/* ==========================================================================
 * Implementation public functions
 * ========================================================================== */

/** Function name: mdl_TEMP_init
 * prepares the parameters m and b for the linear equation used in function mdl_TEMP_convert
 *
 * This function mdl_TEMP_init must be called once before using the function mdl_TEMP_convert.
 *
 * Parameters:
 * 			none
 *
 * Return:
 * 			none
 */
void mdl_TEMP_init(void){

	/* calculate linear dependency of internal temperature sensor y = m*x + b */

	/* calculate the parameters m and b for a temperature >35°C */
	hightemp_m = ((int16)(((int16)EEP_mTempHigh - (int16)EEP_mTempMid) <<SCALE_FACTOR) / (int16)((int16)HIGHTEMP - (int16)MIDTEMP));
	hightemp_b = (int16)(EEP_mTempMid << SCALE_FACTOR) - (hightemp_m * MIDTEMP);

	/* calculate the parameters m and b for a temperature <=35°C */
	lowtemp_m = ((int16)(((int16)EEP_mTempMid - (int16)EEP_mTempLow) <<	SCALE_FACTOR) / (int16)((int16)MIDTEMP - (int16)LOWTEMP));
	lowtemp_b = (int16)(EEP_mTempMid << SCALE_FACTOR) - (lowtemp_m * MIDTEMP);

} /* mdl_TEMP_init */


/** Function name: mdl_TEMP_convert
 *
 * convert the measured ADC value into a temperature value x = (y - b) /m
 *
 * Parameters:
 * 			ADC_Temperature_Value - measured internal temperature sensor voltage
 *
 * Return:
 * 			int16 internal temperature in °C
 */
int16 mdl_TEMP_convert(uint16 ADC_Temperature_Value){

	/* check if the temperature is above 35C */
	if (EEP_mTempMid > ADC_Temperature_Value) {
		/* calculate and return the temperature for a temperature >35°C */
		return ((int16)((ADC_Temperature_Value << (int16)SCALE_FACTOR) - hightemp_b) / hightemp_m);
	} else {
		/* calculate and return the temperature for a temperature <=35°C */
		return ((int16)((ADC_Temperature_Value << (int16)SCALE_FACTOR) - lowtemp_b) / lowtemp_m);
	}

} /* mdl_TEMP_convert */

/* ==========================================================================
 * Implementation private functions
 * ========================================================================== */

/* EOF */
