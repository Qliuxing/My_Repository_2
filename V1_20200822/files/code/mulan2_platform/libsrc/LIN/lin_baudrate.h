/*
 * Copyright (C) 2008-2014 Melexis N.V.
 *
 * MelexCM Software Platform
 *
 */
#ifndef LIN_BAUDRATE_H_
#define LIN_BAUDRATE_H_

#if STANDALONE_LOADER == 0

/* --- LIN baudrate -------------------------------------------------------- */
#if (ML_BAUDRATE < 1000) || (ML_BAUDRATE > 20000)
#warning "Requested LIN baud rate (ML_BAUDRATE) is out of LIN specification (1000 - 20 000 bps)"
#endif

/*
 * LIN baudrate constant calculation:
 * - Constants depend on FPLL, ML_BAUDRATE
 * - ML_CA_BAUD must be in range: 99 .. 200
 * - ML_CA_PRESC must be in range: 0 .. 11
 * 
 *                              PLL_in_Hz
 *  Baudrate_in_bps = -------------------------------
 *                     2^(1+ML_CA_PRESC) * ML_CA_BAUD
 */
#define ML_TEMP_BAUD    (FPLL * 1000UL * 10 / ML_BAUDRATE / 2)  /* fix-point math for preprocessor */


#if ( (ML_TEMP_BAUD > 984) && (ML_TEMP_BAUD < 2005) )  /* 98.5 .. 200.4 */
#   define ML_CA_PRESC  0
#   define ML_CA_BAUD   ((ML_TEMP_BAUD + 5) / 10)

#elif ( (ML_TEMP_BAUD/2 > 984) && (ML_TEMP_BAUD/2 < 2005) )
#   define ML_CA_PRESC  1
#   define ML_CA_BAUD   ((ML_TEMP_BAUD/2 + 5) / 10)

#elif ( (ML_TEMP_BAUD/4 > 984) && (ML_TEMP_BAUD/4 < 2005) )
#   define ML_CA_PRESC  2
#   define ML_CA_BAUD   ((ML_TEMP_BAUD/4 + 5) / 10)

#elif ( (ML_TEMP_BAUD/8 > 984) && (ML_TEMP_BAUD/8 < 2005) )
#   define ML_CA_PRESC  3
#   define ML_CA_BAUD   ((ML_TEMP_BAUD/8 + 5) / 10)

#elif ( (ML_TEMP_BAUD/16 > 984) && (ML_TEMP_BAUD/16 < 2005) )
#   define ML_CA_PRESC  4
#   define ML_CA_BAUD   ((ML_TEMP_BAUD/16 + 5) / 10)

#elif ( (ML_TEMP_BAUD/32 > 984) && (ML_TEMP_BAUD/32 < 2005) )
#   define ML_CA_PRESC  5
#   define ML_CA_BAUD   ((ML_TEMP_BAUD/32 + 5) / 10)

#elif ( (ML_TEMP_BAUD/64 > 984) && (ML_TEMP_BAUD/64 < 2005) )
#   define ML_CA_PRESC  6
#   define ML_CA_BAUD   ((ML_TEMP_BAUD/64 + 5) / 10)

#elif ( (ML_TEMP_BAUD/128 > 984) && (ML_TEMP_BAUD/128 < 2005) )
#   define ML_CA_PRESC  7
#   define ML_CA_BAUD   ((ML_TEMP_BAUD/128 + 5) / 10)

#else
# error "It is not possible to calculate Baudrate constants"
#endif

/* Set default baudrate defined in <profile>.mk */
#define ml_SetDefaultBaudRate()     (void)ml_SetBaudRate(ML_CA_PRESC, ML_CA_BAUD)
#endif /* STANDALONE_LOADER */

#if (LIN_PIN_LOADER != 0)

/* --- Fast loader baudrate ----------------------------------------------- */
#if (ML_FAST_BAUDRATE < 25000 ) || (ML_FAST_BAUDRATE > 125000)
#error "Fast loader baud rate (constant ML_FAST_BAUDRATE in <profile>.mk) should be in range 50000 - 125000 bps"
#endif

/*
 * In the fast mode the split mode of the baudrate counter is used.
 * In the split mode baudrate counter is acts as two chained counters Cpt[5:0]
 * and Cpt[7:6]
 * Cpt[7:6] should be 0..3 (always fixed to 2 in this implementation)
 * Cpt[5:0] should be 0..63
 *
 *                                             PLL_in_Hz
 *  Baudrate_fast_bps = --------------------------------------------------------------
 *                        2^(1+ML_FAST_PRESC) * 2 * (ML_CPT_HI + 1) * ML_CPT_LO_TEMP
 */
#define ML_CPT_HI           2 /*  Cpt[7:6] */
#define ML_CPT_LO_TEMP      (FPLL * 1000UL * 10 / ML_FAST_BAUDRATE / 2 / (ML_CPT_HI + 1)) /* fix-point math */

#if ( ML_CPT_LO_TEMP < 635 )  /* 0.. 63.4 */
#   define ML_FAST_PRESC    15   /* -1 */
#   define ML_FAST_DIVIDER  (64 * ML_CPT_HI + (ML_CPT_LO_TEMP + 5) / 10)

#elif( ML_CPT_LO_TEMP/2 < 635 )
#   define ML_FAST_PRESC    0
#   define ML_FAST_DIVIDER  (64 * ML_CPT_HI + (ML_CPT_LO_TEMP/2 + 5) / 10)

#elif( ML_CPT_LO_TEMP/4 < 635 )
#   define ML_FAST_PRESC    1
#   define ML_FAST_DIVIDER  (64 * ML_CPT_HI + (ML_CPT_LO_TEMP/4 + 5) / 10)
#else
# error "Specified ML_FAST_BAUDRATE is not supported"
#endif

/* Define minimal Fast loader baudrate value[kBd] (round up) */
#define ML_MIN_FAST_BAUDRATE_K  ((FPLL * 10) / (635 * 4 * 2 * 3) + 1)

/* Set default baudrate defined in <profile>.mk
 *
 * Note: The ml_SetFastBaudRate(baudrate) function could be used to set custom baudrate on runtime
 */
#define ml_SetFastLoaderBaudRate()  ml_SetBaudRate(ML_FAST_PRESC, ML_FAST_DIVIDER)

#endif /* LIN_PIN_LOADER */

#endif /* LIN_BAUDRATE_H_ */
