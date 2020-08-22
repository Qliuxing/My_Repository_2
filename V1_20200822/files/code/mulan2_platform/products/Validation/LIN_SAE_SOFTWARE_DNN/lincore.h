/*
 * Copyright (C) 2005-2013 Melexis N.V.
 *
 * MelexCM Software Platform
 *
 */
#ifndef LINCORE_H_
#define LINCORE_H_

#include <lin.h>
#include "lin_cfg.h"

/* Type definitions */
typedef enum {
    ML_RESET_ACCEPTED = 0,
    ML_RESET_REJECTED = 1
} ml_ResetStatus;

typedef enum {
    ML_TARGETED_RESET  = 0,
    ML_BROADCAST_RESET = 1
} ml_ResetDestination;

/* SAE J2602 API functions and events */
void ml_SAE_LinInitModule(ml_uint8 nad);
ml_Status ml_SAE_DataReady (ml_uint8 apinfo);

ml_ResetStatus mlu_SAE_Reset(ml_ResetDestination dest);
void mlu_SAE_MessageReceived (ml_MessageID midx);
void mlu_SAE_DataRequest (ml_MessageID midx);
void mlu_SAE_BroadcastReceived (ml_MessageID midx, ml_uint8 *data); 
void mlu_SAE_DataTransmitted (void);

extern void ld_serial_number_callout (ml_uint8 data[4]);

/* Macroses for generic bit access */
#define   setBit(var, bit_no)  ( (var) |=  (1u << (bit_no)) )
#define clearBit(var, bit_no)  ( (var) &= ~(1u << (bit_no)) )
#define  testBit(var, bit_no)  ( (var) &   (1u << (bit_no)) )

#endif /* LINCORE_H_ */
