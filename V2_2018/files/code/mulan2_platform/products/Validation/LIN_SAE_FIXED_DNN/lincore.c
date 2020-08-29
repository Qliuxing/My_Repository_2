/*
 * Copyright (C) 2005-2015 Melexis N.V.
 *
 * MelexCM Software Platform
 *
 */
#include <syslib.h>
#include "lincore.h"


static void ml_SAE_SetConfiguration (ml_uint8 nad, ml_uint8 numMsg);
static ml_MessageID find_msg_index (ml_uint16 msg_id_lin20);

/*
 * Message Index to 16-bit LIN2.0 Message ID association list
 */
const ml_uint16 MID_list[] = {
    msg_PUB1_MID,  /* LIN2.0 Message ID for message index 0 */
    msg_SUB1_MID
};

/*
 * Store application Product ID / initial NAD in predefined ROM locations
 */
const ml_uint8 ml_ProductID[] = {
    (MLX_SUPPLIER_ID & 0x00FF),
    (MLX_SUPPLIER_ID >> 8),
    (MLX_FUNCTION_ID & 0x00FF),
    (MLX_FUNCTION_ID >> 8),
     MLX_VARIANT_ID
};

static ml_uint8 currentNAD;   /* configured (current) NAD */

/* 
 * Type of response that will be send upon receiving 0x3D (SRF) header.
 * Additional non-diagnostic frames can be sent between 0x3C(MRF) and
 * 0x3D(SRF)
 */
typedef enum {
    respNoResponse = 0x00,   /* discard the SRF frame */
    respLinProductID,        /* answer to Read by Identifier (0) request  */
    respSerialNumber,        /* answer to Read by Identifier (1) request  */
    respNegativeResponse,    /* answer to non-supported Read by ID request */
    respPositiveAssignFID,   /* answer to Assign frame id request    */
    respPositiveAssignNAD,   /* response to Assign NAD request */
    respPositiveTargetedReset,
    respNegativeTargetedReset,
} SlaveResponseType;

static volatile SlaveResponseType pendingSlaveResponse;

/*
 * Error flags numeration is chosen in a way to help direct
 * mapping the flags to SAE J2602 Error states
 *
 * Error flag 0 (flagDummy) is mapped to 'No Detected Fault' state
 * and shall not be used
 *
 * Error flags can be set by error handler (evERR event) or by
 * targeted reset handler (flagReset)
 *
 * Flags are cleared by the mlu_DataTransmitted handler
 * 
 * Current SAE Error state (for reporting via SAE J2602 Status Byte)
 * can be retrieved by applying 'find last set bit' function
 * to the 'SAE_errFlags' byte
 */

/* Bit definition for SAE_errFlags byte */
enum {
    /* flagDummy            = 0, */     /* flag 0 shall NOT be used! */
    flagReset               = 1,        /* lowest priority */
    /* flagReserved         = 2, */
    /* flagReserved         = 3, */
    flagDataError           = 4,
    flagChecksumError       = 5,
    flagByteFramingError    = 6,    
    flagIDParityError       = 7         /* highest priority */
};

static ml_uint8 SAE_errFlags;
static ml_uint8 currError;     /* current reporting error */


/* Flags definition for SAE_flags */
enum {
    flagHasSAEStatus = 0
};

static ml_uint8 SAE_flags;


/* ----------------------------------------------------------------------------
 *  ml_SAE_LinInitModule 
 *
 *  Initializes the MLX4 LIN module:
 *    - Sets baudrate and LIN module parameters
 *    - Sets initial NAD and Node Configuration
 *
 *  Notes:
 *      1. Must be called only in disconnected state
 */
void ml_SAE_LinInitModule(ml_uint8 nad)
{  
    ml_InitLinModule();
    ml_SetDefaultBaudRate(); /* baudrate is defined by ML_BAUDRATE platform's variable */

    /* Configure the Mlx4 software
     *   IDStopBitLength : 0 (default)/1/2/3 -> 1 / 1.5 / 2 / 2.5 stop bits
     *   TXStopBitLength : 0 (default)/1     -> 1 / 2 stop bits
     *   StateChangeSignal : enabled (default)/disabled
     *   SleepMode: lightsleep
     */
    (void)ml_SetOptions(1U, 0U, ML_ENABLED, ML_LIGHTSLEEP);
   
    ml_SetSlewRate(ML_SLEWLOW);

    ml_SAE_SetConfiguration(nad, NUM_MSG);

    pendingSlaveResponse = respNoResponse; /* clear the response */

    SAE_errFlags = (1u << flagReset);  /* set Reset flag at start-up */
}


/*
 ******************************************************************************
 *  LIN API event: MessageReceived (slave RX)
 ******************************************************************************
 */
void mlu_MessageReceived(ml_MessageID midx)
{
    /* ------------------------------------------------------------------------
     *  Configuration messages 
     * ------------------------------------------------------------------------
     */
    if (midx == D_DIA) {   /* ----------- Diagnostic MRF (RX, 8 bytes) */

        /* A new MRF received (no matter to what NAD),
         * so reset pending response
         */
        pendingSlaveResponse = respNoResponse; 

        /*
         *  Addressed to current NAD or wildcard
         *  SIDs:
         *  0xB1  Assign Frame ID
         *  0xB2  Read by Identifier
         *  0xB5  Targeted Reset (SAE)
         */
        if ((LinFrameDataBuffer[0] == currentNAD) || (LinFrameDataBuffer[0] == 0x7F)) {
       
            switch (LinFrameDataBuffer[2]) {   /* SID */

#if (LIN_CFG_ASSIGN_FRAME_ID == 1)
                /*
                 *  LIN 2.0: Assign Frame ID request (optional for SAE J2602)
                 */
                case 0xB1:   /* ---------------------------- Assign FrameID */

                     /* if PCI, Supplier ID (or wildcard) */
                     if ( (LinFrameDataBuffer[1] == 0x06)   /* check PCI */
                           && /* Supplier ID (or wildcard) */
                           (   ((LinFrameDataBuffer[3] == ml_ProductID[0]) && (LinFrameDataBuffer[4] == ml_ProductID[1]))
                               || ((LinFrameDataBuffer[3] == 0xFF)         && (LinFrameDataBuffer[4] == 0x7F))
                           )
                        )
                     {
                         ml_uint16    msgID_lin20;  /* 16-bit LIN2.0 message identifier */
                         ml_FrameID   pid;
                         ml_MessageID index;


                         msgID_lin20 = ((ml_uint16)LinFrameDataBuffer[6] << 8) | LinFrameDataBuffer[5];
                         pid = LinFrameDataBuffer[7];

                         index = find_msg_index (msgID_lin20);

                         /* if index found */
                         if (index != 0xFF) {
                             ml_Disconnect();  /* MUST be disconnected state */
                             /* 
                              * 0x40 (non valid Protected ID) is used in this request to unassign FrameId
                              * for this message [see LIN 2.0 spec, page 121]. Unassigned FrameId can be used
                              * for other message then.
                              *
                              * MLX4 LIN FW doesn't remove message-to-FrameId association from the it's internal
                              * RAM (LIN_IDs array), but marks the message as disabled. So this message
                              * is excluded from FrameId-to-message_index search
                              */
                             if (0x40 == pid) {  
                                ml_DisableMessage(index);
                                pendingSlaveResponse = respPositiveAssignFID;
                             }
                             else {
                                 if (ml_AssignFrameToMessageID(index, pid) == 0) {  /* OK */
                                     pendingSlaveResponse = respPositiveAssignFID;
                                 }
                             }

                             ml_Connect();
                         }
                         /* else: valid index was not found */ 
                     }
                     /* Product/Fuction ID are not matched */
                     else {
                     }
                     break; /* !case 0xB1 */
#endif


#if (LIN_CFG_READ_BY_ID == 1)
                /*
                 *  LIN 2.x: Read by Identifier request (optional for SAE J2602)
                 */
                case 0xB2:  /*------------------- Read by Identifier request */
                     if ( (LinFrameDataBuffer[1] == 0x06)   /* check PCI */
                           &&  /* Supplier ID or wildcard */
                           (   ((LinFrameDataBuffer[4] == ml_ProductID[0]) && (LinFrameDataBuffer[5] == ml_ProductID[1]))  
                               || ((LinFrameDataBuffer[4] == 0xFF)         && (LinFrameDataBuffer[5] == 0x7F))
                           )
                           &&  /* Function ID or wildcard */
                           (   ((LinFrameDataBuffer[6] == ml_ProductID[2]) && (LinFrameDataBuffer[7] == ml_ProductID[3]))  
                               || ((LinFrameDataBuffer[6] == 0xFF)            && (LinFrameDataBuffer[7] == 0xFF))
                           )
                        )
                     {
                         switch (LinFrameDataBuffer[3]) { /* requested Identifier */  
                             case 0x00: 	/* LIN Product Identification */
                                 pendingSlaveResponse = respLinProductID;
                                 break;
         	             
                             case 0x01:  /* Serial number (optional) */
                                 pendingSlaveResponse = respSerialNumber;
                                 break;

                             default:
                                 pendingSlaveResponse = respNegativeResponse;
                                 break;
                         }
                     }
                   
                     break;  /* !case 0xB2 */
#endif
                
                /*
                 *  SAE J2602: Targeted/Broadcast Reset request 3C NAD 01 B5 FF FF FF FF FF
                 */
                case 0xB5:   /* -------------------- Targeted Reset request */
                     if (LinFrameDataBuffer[1] == 0x01) {  /* check PCI */
                         uint8 addr;
                         ml_ResetDestination dest;


                         addr = LinFrameDataBuffer[0];  /* store node address */

                         /* Verify message destination address */
                         if (0x7F == addr) {
                            dest = ML_BROADCAST_RESET;
                         }
                         else {
                            dest = ML_TARGETED_RESET;
                         }

                         if (mlu_SAE_Reset(dest) == ML_RESET_ACCEPTED) {
                             SAE_errFlags = 0; /* TBD! reset all errors */
                             setBit(SAE_errFlags, flagReset);

                             if (ML_TARGETED_RESET == dest) {
                                pendingSlaveResponse = respPositiveTargetedReset;
                             }
                             else {
                                pendingSlaveResponse = respNoResponse;  /* for broadcast */
                             }
                         }
                         else {
                             if (ML_TARGETED_RESET == dest) {
                                pendingSlaveResponse = respNegativeTargetedReset;
                             }
                             else {
                                pendingSlaveResponse = respNoResponse;  /* for broadcast */
                             }
                         }
                     }
                     break;

                default:
                     break;
            }
        }
        /* Message is not addressed to our node */
        else {
        }
    }
    /* call User Application */
    else {
        mlu_SAE_MessageReceived (midx);
    }
}


/*
 ******************************************************************************
 *  LIN API event: Data Request (slave TX)
 ******************************************************************************
 */
void mlu_DataRequest (ml_MessageID midx)
{
    if (midx == R_DIA) {    /*---- Diagnostic SRF  (assigned to FID=0x3D) --*/
        
        switch (pendingSlaveResponse) {

            case respNoResponse:
                ml_DiscardFrame();
                break;


#if (LIN_CFG_READ_BY_ID == 1)
            case respLinProductID:
                /* Fill the buffer with slave Response */
                /* Response format : NAD PCI RSID D1 D2 D3 D4 D5 */
                LinFrameDataBuffer[0] = currentNAD;
                LinFrameDataBuffer[1] = 0x06; /* PCI */
                LinFrameDataBuffer[2] = 0xF2; /* RSID = SID+0x40 = 0xb2+0x40 */

                LinFrameDataBuffer[3] = ml_ProductID[0];
                LinFrameDataBuffer[4] = ml_ProductID[1];
                LinFrameDataBuffer[5] = ml_ProductID[2];
                LinFrameDataBuffer[6] = ml_ProductID[3];
                LinFrameDataBuffer[7] = ml_ProductID[4];
        
                ml_DataReady(ML_END_OF_TX_DISABLED);     /* Signal to MLX4 */
                break;

            case respSerialNumber:
                LinFrameDataBuffer[0] = currentNAD;
                LinFrameDataBuffer[1] = 0x05; /* PCI */
                LinFrameDataBuffer[2] = 0xF2; /* RSID = SID+0x40 = 0xb2+0x40 */

                ld_serial_number_callout(&LinFrameDataBuffer[3]); /* Insert Serial Number into data bytes 3..6 */

                LinFrameDataBuffer[7] = 0xFF;
                
                ml_DataReady(ML_END_OF_TX_DISABLED);
                break;

            case respNegativeResponse:
                LinFrameDataBuffer[0] = currentNAD;
                LinFrameDataBuffer[1] = 0x03;  /* PCI */
                LinFrameDataBuffer[2] = 0x7F;  /* RSID = 0x7F  */ 

                LinFrameDataBuffer[3] = 0xB2;  /* Data1: Requested ID  */
                LinFrameDataBuffer[4] = 0x12;  /* Data2: Error code    */
                LinFrameDataBuffer[5] = 0xFF;  /* Data3                */
                LinFrameDataBuffer[6] = 0xFF;  /* Data4                */
                LinFrameDataBuffer[7] = 0xFF;  /* Data5                */
                
                ml_DataReady(ML_END_OF_TX_DISABLED);     /* Signal to MLX4 */
                break;
#endif


#if (LIN_CFG_ASSIGN_FRAME_ID == 1)
            case respPositiveAssignFID:
                 LinFrameDataBuffer[0] = currentNAD;
                 LinFrameDataBuffer[1] = 0x01; /* PCI */
                 LinFrameDataBuffer[2] = 0xF1; /* RSID = 0xB1+0x40 */

                 LinFrameDataBuffer[3] = 0xFF;
                 LinFrameDataBuffer[4] = 0xFF;
                 LinFrameDataBuffer[5] = 0xFF;
                 LinFrameDataBuffer[6] = 0xFF;
                 LinFrameDataBuffer[7] = 0xFF;

                 ml_DataReady(ML_END_OF_TX_DISABLED);                      /* Signal to MLX4 */
                 break;
#endif

           case respPositiveTargetedReset:

                /* Fill the buffer with slave Response */
                LinFrameDataBuffer[0] = currentNAD;
                LinFrameDataBuffer[1] = 0x06;   /* PCI */
                LinFrameDataBuffer[2] = 0xF5; /* positive response */
                LinFrameDataBuffer[3] = ml_ProductID[0]; /* SupplierID LSB */
                LinFrameDataBuffer[4] = ml_ProductID[1]; /* SupplierID MSB */
                LinFrameDataBuffer[5] = ml_ProductID[2]; /* FunctionID LSB */
                LinFrameDataBuffer[6] = ml_ProductID[3]; /* FunctionID MSB */
                LinFrameDataBuffer[7] = ml_ProductID[4]; /* VariantID */          
   
                ml_DataReady(ML_END_OF_TX_DISABLED);  /* Signal to MLX4 */
                break;
 
           case respNegativeTargetedReset:

                /* Fill the buffer with slave Response */
                LinFrameDataBuffer[0] = currentNAD;
                LinFrameDataBuffer[1] = 0x06;   /* PCI */
                LinFrameDataBuffer[2] = 0x7F; /* negative response */
                LinFrameDataBuffer[3] = ml_ProductID[0]; /* SupplierID LSB */
                LinFrameDataBuffer[4] = ml_ProductID[1]; /* SupplierID MSB */
                LinFrameDataBuffer[5] = ml_ProductID[2]; /* FunctionID LSB */
                LinFrameDataBuffer[6] = ml_ProductID[3]; /* FunctionID MSB */
                LinFrameDataBuffer[7] = ml_ProductID[4]; /* VariantID */          
   
                ml_DataReady(ML_END_OF_TX_DISABLED);  /* Signal to MLX4 */
                break;

            default:
                ml_DiscardFrame();
                break;  
        }  /* switch (pendingSlaveResponse) */

        /* reset pending response after processing */
        pendingSlaveResponse = respNoResponse;
    }
    else {
        mlu_SAE_DataRequest(midx); /* call User Application */
    }
}


/*
 ******************************************************************************
 *  LIN API event: mlu_DataTransmitted
 ******************************************************************************
 */
void mlu_DataTransmitted(void)
{
    /*
     * Frame successfully transmitted. If it was a frame with SEA J2602
     * Status Byte, then clear current Error state, which was successfully
     * reported in this frame
     */
    if ( testBit(SAE_flags, flagHasSAEStatus)) {
        clearBit(SAE_flags, flagHasSAEStatus);
        clearBit(SAE_errFlags, currError);
    }

    /* Notify user application about end of transmission */
    mlu_SAE_DataTransmitted();
	return;
}


/*
 ******************************************************************************
 *  LIN API event: mlu_ErrorDetected
 ******************************************************************************
 */
void mlu_ErrorDetected(ml_LinError Error)
{
    /*
     * NB: Application shall not use any data from LinFrameDataBuffer
     *     since they are not valid
     */

    /* --- SAE J2602 --------------------------------------- */
    clearBit(SAE_flags, flagHasSAEStatus);
    
    /* Map MLX LIN API errors to SAE errors */
    switch (Error) {
        case ml_erIdParity:
             setBit(SAE_errFlags, flagIDParityError);
             break;

        case ml_erCheckSum:
             setBit(SAE_errFlags, flagChecksumError);
             break;

        case ml_erBit:
        case ml_erSynchField:
             setBit(SAE_errFlags, flagDataError);
             break;

        case ml_erDataFraming:
        case ml_erStopBitTX:
             setBit(SAE_errFlags, flagByteFramingError);
             break;

        case ml_erShortDone:
        case ml_erLinModuleReset:
        case ml_erIdFraming:
        case ml_erBufferLocked:
        case ml_erShort:
        case ml_erTimeOutResponse:
        case ml_erBreakDetected:
        case ml_erWakeUpInit:
        default:     
             break;
    }
}


/*
 ******************************************************************************
 *  LIN API event: mlu_LinSleepMode
 ******************************************************************************
 */
void mlu_LinSleepMode(ml_StateReason Reason)
{
    (void)Reason;   /* not used */

    pendingSlaveResponse = respNoResponse; /* clear the response */

}


/* ----------------------------------------------------------------------------
 *  Sets new NAD (correctness of NAD is not checked) and
 *  assign range of FrameIDs to application's MessageIDs depending on NAD
 *
 *  Notes:
 *  1. Functions must be executed in disconnected state of the LIN driver.
 *  2. It's not required to unassign previously assigned FrameIDs, since
 *     they will be overwritten by new FrameIDs during FrameID to Message
 *     Index assignment.
 *
 *                                   LIN    LIN    SAE
 *     NAD       Comment             2.0    2.1   J2602
 *   --------------------------------------------------
 *      0        Goto Sleep Command   -      -      -
 *   0x01..0x5F                       ok     ok     -
 *   0x60..0x6F  SAE J2602 NADs       ok     ok     ok
 *   0x70..0x7D                       ok     ok     -
 *   0x7E        functional node      ok      -     -
 *   0x7F        broadcast            -       -     -
 *   0x80..0xFF  Reserved             -       -     -
 */
static void ml_SAE_SetConfiguration (ml_uint8 nad, ml_uint8 numMsg)
{
    ml_uint8 frameID;
    uint_fast8_t i;     /* iterator */

    
    currentNAD = nad;
    
    if (nad < 0x6E) {
        frameID = (nad & 0x0F) * 4; /* base frame ID for selected NAD/DNN */

        for (i = 0; i < numMsg; i++) {
            if (frameID > 0x37) {
                break;  /* out of range: incorrect combination NAD vs
                         * Number of Messages
                         */
            }
            ml_AssignFrameToMessageID(i, frameID++);
        }
    }
    else {  /* for NAD = 0x6E or 0x6F */
        for (i = 0; i < numMsg; i++) {
            ml_DisableMessage(i);   /* mark protected IDs as invalid */
        }
    }
}


/*
 ************************************************************************
 * ml_SAE_DataReady
 * 
 * 1. Inserts SAE J2602 Status byte as Data Byte 0 in LinFrameDataBuffer
 * 2. Sends ml_DataReady command
 ************************************************************************
 */
ml_Status ml_SAE_DataReady (ml_uint8 apinfo)
{
    ml_uint8 val;
    ml_uint8 bitcnt;
    ml_Status status;

    val = SAE_errFlags;

    if (val) {
        /* Find the last (most significant) bit set
         * i.e. highest prio error
         */
        for (bitcnt = 0; val != 1; bitcnt++) {
            val >>= 1;    
        }
    }
    /* if all bits are clear, then setNo Detected Fault state (0) */
    else {
        bitcnt = 0;
    }
    
    currError = bitcnt; /* current Error state to report */

    /* insert SAE J2602 Status byte as a first byte of the frame */
    LinFrameDataBuffer[0] = (currError << 5) | (apinfo & 0x1F) ;

    setBit(SAE_flags, flagHasSAEStatus); /* mark the frame */

    /* The DataTransmitted event is ENABLED for this frame */
    status = ml_DataReady(ML_END_OF_TX_ENABLED);
    return (status);
}

#if (LIN_CFG_ASSIGN_FRAME_ID == 1)
/*
 *****************************************************************************
 *  find_msg_index
 *
 * Return:
 *   Message Index associated with 16-bit LIN2.0 Message ID (msgID_lin20)
 *   0xFF (non-valid index) if association is not found
 *****************************************************************************
 */
static ml_MessageID find_msg_index (ml_uint16 msgID_lin20)
{
    ml_MessageID index;
    ml_uint8 i;
    

    index = 0xFF;  /* set to non valid index */

    /* Find the internal index of requested frame */
    for (i = 0; i < (sizeof(MID_list) / sizeof(MID_list[0])) ; i++) {
        if (MID_list[i] == msgID_lin20) {
            index = i;
            break;  /* index is found */
        }
    }
    return index;
}
#endif

/* EOF */
