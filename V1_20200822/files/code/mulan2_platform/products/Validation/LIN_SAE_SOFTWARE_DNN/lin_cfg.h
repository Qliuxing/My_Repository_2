/*
 * Copyright (C) 2005-2013 Melexis N.V.
 *
 * MelexCM Software Platform
 *
 */
#ifndef LIN_CFG_H_
#define LIN_CFG_H_

/* 
 ******************************************************************************
 *               Values that CAN be changed by the user
 ******************************************************************************
 */

/* 
 * ----------------------------------------------------------------------------
 *              LIN configuration
 * ----------------------------------------------------------------------------
 */
/* According to SAE J2602 in case of Hardware/Software selectable NAD,
   initial NAD shall be 0x6F
 */
#define MLX_INITIAL_NAD     0x6F   /* initial NAD */


/*--- LIN Product Identification --------------------------------------*/
#define MLX_SUPPLIER_ID     0x5AFE
#define MLX_FUNCTION_ID     0xFEED
#define MLX_VARIANT_ID      0x01

/*--- Diagnostic and Configuration messages support ------------------------*/
#define LIN_CFG_ASSIGN_NAD          1  /* support Assign NAD request      */
#define LIN_CFG_ASSIGN_FRAME_ID     1  /* support Assign Frame ID request */
#define LIN_CFG_READ_BY_ID          1  /* support Read by Identifier  */

/*--- Application frames ----------------------------------------------------*/
#define NUM_MSG             4    /* Number of messages per node... */
                                 /* ... Shall be in powers of 2, i.e 4, 8 or 16 */

#define msg_PUB1             0x00   /* Message Index */
#define msg_PUB1_MID         0x1300 /* LIN2.0 message identifier */

#define msg_SUB1             0x01   /* Message Index */
#define msg_SUB1_MID         0x1301 /* LIN2.0 message identifier */

#define msg_PUB2             0x02   /* Message Index */
#define msg_PUB2_MID         0x1302 /* LIN2.0 message identifier */

#define msg_SUB2             0x03   /* Message Index */
#define msg_SUB2_MID         0x1303 /* LIN2.0 message identifier */


/* 
 ******************************************************************************
 *            Input Data Verification
 ******************************************************************************
 */
#if (MLX_INITIAL_NAD != 0x6F)
# warning "SAE J2602: For Hardware/Software selectable NAD, inital NAD 0x6F\
  shall be used."
#endif


#if (NUM_MSG > 16)
# error "Incorrect NUM_MSG: Maximum 16 user messages are supported"
#endif

#if (NUM_MSG < 4) || (NUM_MSG & (NUM_MSG - 1))
# error "Incorrect NUM_MSG: Should be at least 4 assigned messages. If more \
  messages are required, the number shall be in a powers of 2, i.e. 4, 8, 16"
#endif


#if (ML_BAUDRATE != 10417)
# warning "Suspicious baudrate setting: SAE J2602 requires 10417 bps "
#endif

#endif /* LIN_CFG_H_ */
