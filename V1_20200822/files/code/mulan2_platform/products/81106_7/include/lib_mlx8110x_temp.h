/*
 * Copyright (C) 2014-2015 Melexis GmbH
 *
 * This file is part of the Mlx8110x module library.
 *
 *
 * Module prefix: mdl_TEMP
 * All module interface functions start with this letters.
 *
 * This library provides functions calculate the Temperature out of the ADC measurements
 *
 *
 * ==========================================================================
 * History:
 *   Revision 1.0
 *     - Initial release
 *
 * ========================================================================== */

#ifndef MDL_TEMP_H_
#define MDL_TEMP_H_
/* ==========================================================================
 * Public defines
 * ========================================================================== */

/* ==========================================================================
 * Public variables
 * ========================================================================== */

/* ==========================================================================
 * Public functions
 * ========================================================================== */
/* prepares the parameters m and b for the linear equation used in function mdl_TEMP_convert */
extern void mdl_TEMP_init(void);

/* convert the measured ADC value into a temperature value x = (y - b) /m */
extern int16 mdl_TEMP_convert(uint16 ADC_Temperature_Value);

#endif /* MDL_TEMP_H_ */
