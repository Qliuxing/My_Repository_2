/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef _LIB_MLX8110X_TRIM_H_
#define _LIB_MLX8110X_TRIM_H_

#include "ioports.h"

/* TR_RCO .. Register: ANA_OUTC .. Bit 8-14 */
#define TR_RCO  (uint16)~(0x7Fu << 8)

/* TR_V5V6 .. Register: ANA_OUTC .. Bit 0-2 */
#define TR_V5V6 (uint16)~(0x07u << 0)

/* TR_BG .. Register: ANA_OUTB .. Bit 12-15 */
#define TR_BG   (uint16)~(0xFu << 12)

/* TR_BIAS .. Register: ANA_OUTB .. Bit 6-11 */
#define TR_BIAS (uint16)~(0x3Fu << 6)

/* TR_V1V8 .. Register: ANA_OUTB .. Bit 3-5 */
#define TR_V1V8 (uint16)~(0x7u << 3)

/* TR_VDDA .. Register: ANA_OUTB .. Bit 0-2 */
#define TR_VDDA (uint16)~(0x7u << 0)

/** Trim RCO (Attention: RCO is already trimmed during startup)
 * @param
 * @return void
 * Register: ANA_OUTC, Bit TR_RCO[6:0], Bit positions [14:8]
 * Register: ANA_OUTD, Bit TR_RCO[7], Bit positions [15]
 */
#define TR_SET_RCOTRIM(TR_RCO_TRIM)                          \
do {                                                         \
	    CONTROL |= OUTC_WE;                                   \
       ANA_OUTC = ((ANA_OUTC & TR_RCO) | ((0x7Fu & TR_RCO_TRIM) << 8));     \
       ANA_OUTD = ((ANA_OUTD & 0x7FFF) | ((0x80u & TR_RCO_TRIM) << 8));     \
       CONTROL &= (uint16)~(OUTC_WE);                        \
	} while (0)

/** Trim V5V6
 * @param
 * @return void
 * Register: ANA_OUTC, Bit TR_V5V6, Bit positions [2:0]
 */
#define TR_SET_V5V6TRIM(TR_V5V6_TRIM)                        \
do {                                                         \
	    CONTROL |= OUTC_WE;                                   \
       ANA_OUTC = ((ANA_OUTC & TR_V5V6) | (0x7u  & TR_V5V6_TRIM));   \
       CONTROL &= (uint16)~(OUTC_WE);                        \
	} while (0)

/** Trim Bandgap (Attention: Bandgap is already trimmed during startup)
 * @param
 * @return void
 * Register: ANA_OUTB, Bit TR_BG, Bit positions [15:12]
 */
#define TR_SET_BGTRIM(TR_BG_TRIM)                            \
do {                                                         \
	    CONTROL |= OUTB_WE;                                   \
       ANA_OUTB = ((ANA_OUTB & TR_BG) | ((0xFu & TR_BG_TRIM)<<12));       \
       CONTROL &= (uint16)~(OUTB_WE);                        \
	} while (0)

/** Trim Bias (Attention: Bias is already trimmed during startup)
 * @param
 * @return void
 * Register: ANA_OUTB, Bit TR_BIAS, Bit positions [11:6]
 */
#define TR_SET_BIASTRIM(TR_BIAS_TRIM)                        \
do {                                                         \
	    CONTROL |= OUTB_WE;                                   \
       ANA_OUTB = ((ANA_OUTB & TR_BIAS) | ((0x3Fu & TR_BIAS_TRIM)<<6));   \
       CONTROL &= (uint16)~(OUTB_WE);                        \
	} while (0)

/** Trim VDDD->V1V8 (Attention: VDDD is already trimmed during startup)
 * @param
 * @return void
 * Register: ANA_OUTB, Bit TR_V1V8, Bit positions [5:3]
 */
#define TR_SET_V1V8TRIM(TR_V1V8_TRIM)                        \
do {                                                         \
	    CONTROL |= OUTB_WE;                                   \
       ANA_OUTB = ((ANA_OUTB & TR_V1V8) | ((0x7u & TR_V1V8_TRIM)<<3));   \
       CONTROL &= (uint16)~(OUTB_WE);                        \
	} while (0)

/** Trim VDDA (Attention: VDDA is already trimmed during startup)
 * @param
 * @return void
 * Register: ANA_OUTB, Bit TR_VDDA, Bit positions [2:0]
 */
#define TR_SET_VDDATRIM(TR_VDDA_TRIM)                        \
do {                                                         \
	    CONTROL |= OUTB_WE;                                   \
       ANA_OUTB = ((ANA_OUTB & TR_VDDA) | (0x7u & TR_VDDA_TRIM));   \
       CONTROL &= (uint16)~(OUTB_WE);                        \
	} while (0)

#endif /* _LIB_MLX8110X_TRIM_H_ */
