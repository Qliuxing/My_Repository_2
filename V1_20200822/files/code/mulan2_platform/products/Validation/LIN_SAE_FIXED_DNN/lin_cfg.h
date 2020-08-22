/*
 * Copyright (C) 2005-2012 Melexis N.V.
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
/* 
 * According to SAE J2602, NAD shall be in range: 0x60 .. 0x6F
 *
 * Note: NAD 0x6E can be used, but range of Frame IDs for this node will not be
 *       automatically assigned and shall be assigned at runtime via
 *       Configuration messages.
 *       NAD 0x6F shall not be used for nodes with Fixed DNN, since it's used
 *       for unconfigured nodes which DNN can be changed at runtime 
 */
#define MLX_INITIAL_NAD     0x65   /* initial NAD */


/*--- LIN Product Identification --------------------------------------*/
#define MLX_SUPPLIER_ID     0x5AFE
#define MLX_FUNCTION_ID     0xFEED
#define MLX_VARIANT_ID      0x01

/*--- Diagnostic and Configuration messages support ------------------------*/
#define LIN_CFG_ASSIGN_FRAME_ID     1  /* support Assign Frame ID request */
#define LIN_CFG_READ_BY_ID          1  /* support Read by Identifier  */

/*--- Application frames ----------------------------------------------------*/
#define NUM_MSG             4    /* Number of messages per node... */
                                 /* ... Shall be in powers of 2, i.e 4, 8 or 16 */

#define msg_PUB1             0x00   /* Message Index */
#define msg_PUB1_MID         0x1300 /* LIN2.0 message identifier */

#define msg_SUB1             0x01   /* Message Index */
#define msg_SUB1_MID         0x1301 /* LIN2.0 message identifier */



/* 
 ******************************************************************************
 *            Input Data Verification
 ******************************************************************************
 */
#if (MLX_INITIAL_NAD < 0x60) || (MLX_INITIAL_NAD > 0x6E)
# error "For Fixed DNN node, MLX_INITIAL_NAD shall be in range [0x60 .. 0x6E]\
  as per SAE J2602-1 specification"

#elif (MLX_INITIAL_NAD == 0x6E)
# warning "Range of Frame IDs for initial NAD = 0x6E will not be automaticaly\
  assigned. Frame IDs SHALL be assigned at runtime via Configuration Messages,\
  e.g. via Assignd FrameID services"
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
