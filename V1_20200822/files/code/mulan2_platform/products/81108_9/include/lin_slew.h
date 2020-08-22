/*
 * Copyright (C) 2008-2012 Melexis N.V.
 *
 * Software Platform
 */

#ifndef LIN_SLEW_H_
#define LIN_SLEW_H_

/*
 * Possible values for the LIN cell Slew Rate (Phymd)
 * Refer to the Mlx4 periphery documentation for the values of your chip
 * (see Phymd[1:0] in the FLAGS 1 dcom) 
 * SlewRate: ML_SLEWHIGH=20kbps/ML_SLEWLOW=10kbps/ML_SLEWFAST=max (fast protocol)
 *
 * Values for MLX8110x below
 */
#define    ML_SLEWHIGH     0    /* 4.5 us */
#define    ML_SLEWLOW      2    /* 9.5 us */
#define    ML_SLEWFAST     1    /* 0.2 us */


#endif /* LIN_SLEW_H_ */
