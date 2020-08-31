/*
 * LIN Low Level LIN FW API
 * Signal Interaction and Notification
 *
 * This file is application specific and depends on LDF/NCF files
 * Generated by configuration tool C:\EVWS\_LIBRARY_PLATFORMS\library_platform_mulan2\bin\ldf_nodegen.exe (version 1.4.0)
 *
 * Copyright (C) 2007-2015 Melexis N.V.
 */

#include "lin_api.h"
#include <stdbool.h>

#if LIN_VERSION >= 20
#define l_bool_wr_XXXResponse_ErrorXXX(n)	( l_bool_wr_Response_Error_s((n)) )
#define l_flg_clr_XXXResponse_ErrorXXX()	( l_flg_clr_Response_Error_s() )
#define l_flg_tst_XXXResponse_ErrorXXX()	( l_flg_tst_Response_Error_s() )
#endif /* LIN_VERSION >= 20 */



#if defined(HAS_EVENT_TRIGGERED_FRAMES)
/*
 * Event-triggered frame
 * ---------------------
 * isEventTriggered is used to notify mlu_DataTransmitted function that it was
 * transmission of Event Triggered Frame (ETF).
 *
 * isEventTriggered is set in mlu_DataRequest (1) only if MessageIndex  < ML_NUMBER_OF_DYNAMIC_MESSAGES
 * and assosiated signals have been changed since last transmission.
 *
 * isEventTriggered is reset in:
 *    2a) mlu_ErrorDetected (error during transmission of event-triggered frame)
 *    2b) mlu_DataTransmitted (event-triggered frame successfully transmitted)
 * 
 * +-------+-----------------------------+
 * |ETF_TX |                             |
 * +-------+-----------------------------+
 *         :                             :
 *      (1)*=======* (2a)                :
 *      (1)*=============================* (2b)
 *         :                             :
 *         :                             mlu_DataTransmitted
 *         mlu_DataRequest 
 *
 */
static ml_bool isEventTriggered;
static ml_MessageID midx_;
#endif /* HAS_EVENT_TRIGGERED_FRAMES */

/* Prototypes of the local functions */
static void setAttachedFlags (const ml_MessageID MessageIndex);


/* ------------------------------------------------------------------------------------
 * Sleep mode notification handler (default implementation)
 *
 */
__attribute__((weak)) void l_ifc_sleep_entered_callout (ml_StateReason Reason)
{
    switch (Reason) {
        /*
         * Go-to-Sleep frame has been received from Master
         */
        case ml_reasonMaster:
        case ml_reasonTimeOut:
        case ml_reasonWakeupAbort:
        case ml_reasonCommand:
        case ml_reasonWakeup:
        case ml_reasonWakeupResponse:
        case ml_reasonTimeOutDominant:
        default:
            /*
             * Application specific handlers
             *
             * Note:
             *  Application can implement l_ifc_sleep_entered_callout()
             *  function w/o weak attribute to override this default
             *  implementation.
             */
            break;
    }
}
/*
 ******************************************************************************
 * mlu_LinSleepMode event
 ******************************************************************************
 */
void mlu_LinSleepMode(ml_StateReason Reason)
{

#if LIN_VERSION >= 20
    if (Reason == ml_reasonMaster) {
        l_ifc_write_status_flags_i1(ML_IFC_GOTO_SLEEP);

        /*
         * MLX4 FW handles Goto Sleep frame (0x3C, 0x00 ...) automatically
         * and does not report it via mlu_MessageReceived event.
         * Hence write Protected ID 0x3C explicitly.
         */
        l_ifc_write_status_pid_i1(0x3CU);
        l_ifc_write_status_flags_i1(ML_IFC_SUCCESSFUL_TRANSFER);

    }
    /* else: other reasons of sleep don't required special care */

#endif /* LIN_VERSION >= 20 */

    l_ifc_sleep_entered_callout(Reason);    /* notify application on the sleep reason */
}


/*
 ******************************************************************************
 * mlu_MessageReceived event
 *
 * A relevant master-to-slave frame has been received and available
 * in the LinFrameDataBuffer 
 *
 * !! Note that interrupts are not disabled globally while current frame buffer
 * (LinFrameDataBuffer) is copied to message buffer bit-field structure.
 * The message buffer is shared between this LIN ISR and Signal API functions.
 * Thus these signals/flags API functions shall NOT be used in other interrupts,
 * (actually in the interrupts which have higher priority than LIN ISR),
 * otherwise race conditions possible
 ******************************************************************************
 */
void mlu_MessageReceived(ml_MessageID MessageIndex)
{
    ml_uint8  volatile *dest;
    ml_uint8  volatile *src;
    ml_uint16 size; /* type uint16 is chosen to increase speed on native 16-bit CPU */

#if LIN_VERSION >= 20
    /* 
     * Successful transfer is set if a frame has been
     * transmitted/received without an error
     */
    l_ifc_write_status_flags_i1(ML_IFC_SUCCESSFUL_TRANSFER);
    l_ifc_write_status_pid_i1(LinProtectedID);
#endif /* LIN_VERSION >= 20 */

    switch(MessageIndex) {

#if LIN_VERSION >= 20
    /*
     * if diagnostic Master Request frame is received
     */
    case D_DIA:
        ml_DiagMasterRequest();
        /* LIN buffer is not released here so can be parsed for application 
           specific frames 
        */
        break;
#endif /* LIN_VERSION >= 20 */
    default:
        if (MessageIndex < ML_NUMBER_OF_DYNAMIC_MESSAGES) {

            /* if dataBuffer is defined in frame_list[] for this Message Index */
            dest = frame_list[MessageIndex].dataBuffer;
            if (dest != (void *) 0) {
               /*
                * Copy from LinFrameDataBuffer to message data buffer 'dataBuffer'
                */
                src  = &LinFrameDataBuffer[0];
                size = frame_list[MessageIndex].dataBufferSize;

                while (size != 0U) {
                    size--;
                                        /*lint -esym(960, 17.4) -esym(961, 12.13) permitted by MISRA 2012 */
                    *dest++ = *src++;   /*lint +esym(960, 17.4) +esym(961, 12.13) */
                }

                /*
                 * Set all attached flags to 1 since signal values have just been updated
                 * from received frame ("considered to be received" as per LIN spec)
                 */
                setAttachedFlags(MessageIndex);
            }
        }
        else {
            /*
             * Unexpected frame, ignore it
             */
        }
        break;
    }
}

/*
 ******************************************************************************
 * mlu_DataRequest event
 *
 * Header of the relevant slave-to-master frame has been received
 ******************************************************************************
 */
void mlu_DataRequest (ml_MessageID MessageIndex)
{
    ml_uint8  volatile *dest;
    ml_uint8  volatile *src;
    ml_uint16 size; /* type uint16 is chosen to increase speed on native 16-bit CPU */

#if defined(HAS_EVENT_TRIGGERED_FRAMES)
    midx_ = MessageIndex; /* store message index (for mlu_DataTransmitted event) */
#endif  /* HAS_EVENT_TRIGGERED_FRAMES */ 

    /* if Message index is in the frame_list[] */
    if (MessageIndex < ML_NUMBER_OF_DYNAMIC_MESSAGES) {

#if defined(HAS_EVENT_TRIGGERED_FRAMES)
        if (ML_EVENT_TRIGGERED == frame_list[MessageIndex].frame_type) {
            /* if signals in the associated unconditional frame were not updated */
            l_u16 bitmap = (l_u16)1 << MessageIndex;
            if ((frame_updated & bitmap) == 0U) {
                (void)ml_DiscardFrame();
                return;
            }
            else {
                isEventTriggered = ML_TRUE;
            }
        }
#endif  /* HAS_EVENT_TRIGGERED_FRAMES */ 
        
        /* if dataBuffer is defined in frame_list[] for this Message Index */
        src = frame_list[MessageIndex].dataBuffer;
        if (src != (void *) 0) {
           /*
            * Copy message from data buffer 'dataBuffer' to LinFrameDataBuffer 
            */
            dest = &LinFrameDataBuffer[0];
            size = frame_list[MessageIndex].dataBufferSize;

            while (size != 0U) {
                size--;
                                    /*lint -esym(960, 17.4) -esym(961, 12.13) permitted by MISRA 2012 */
                *dest++ = *src++;   /*lint +esym(960, 17.4) +esym(961, 12.13) */
            }

            (void)ml_DataReady(ML_END_OF_TX_ENABLED);

#if defined(HAS_EVENT_TRIGGERED_FRAMES)
            /* Set flags if unconditional frame is transferred */
            if (ML_FALSE == isEventTriggered) {
                /*
                 * Set all attached flags to 1 since signals have just been copied to frame buffer
                 * for transmission ("considered to be transmitted" as per LIN spec)
                 * and application can update signals with a new values
                 */
                setAttachedFlags(MessageIndex);
            }
            /* else : For event-triggered frame flags will be set only after successful transfer */
#else
            setAttachedFlags(MessageIndex);
#endif  /* HAS_EVENT_TRIGGERED_FRAMES */ 
        }
        /* 
         * The dataBuffer is not defined for this Message Index in frame_list[]
         */
        else {
            (void)ml_DiscardFrame();

#if defined(HAS_EVENT_TRIGGERED_FRAMES)            
            isEventTriggered = ML_FALSE;
#endif  /* HAS_EVENT_TRIGGERED_FRAMES */            
        }
    }

#if LIN_VERSION >= 20
    else if (R_DIA == MessageIndex) {  /* if Slave Response diag. frame is requested */
        if (ml_DiagSlaveResponse() == ML_SUCCESS) {
            (void)ml_DataReady(ML_END_OF_TX_DISABLED);
        }
        else {
            /* Header is not yet discarded here so can provide a response
               for the header for application specific frames
            */
            (void)ml_DiscardFrame();
        }
    }
#endif /* LIN_VERSION >= 20 */
    /*
     * Unexpected message
     */
    else {
        (void)ml_DiscardFrame();
    }
}

/*
 ******************************************************************************
 * mlu_ErrorDetected
 *
 ******************************************************************************
 */
void mlu_ErrorDetected(ml_LinError Error)
{
#if LIN_VERSION >= 20

    ml_uint8 frameId;
    
    
    frameId = LinProtectedID & 0x3FU; /* mask parity bits to get FrameId */

    /*
     * Abort Diagnostic communication with corrupted Diagnostic request
     * Checked by LIN2.1 CT test case 13.2.2
     */
    if ( (0x3CU == frameId) /* MRF */
        && ((ml_erDataFraming == Error) || (ml_erCheckSum == Error)))
    {
        ml_DiagClearPendingResponse();  
    }

#endif

    /* ---- ml_erLinModuleReset -------------------------------------------- */
    if (ml_erLinModuleReset == Error) {
        /* non-recoverable failure has occurred in the LIN Module */
        /* switch to System Mode and reinitialize LIN module */
    }

    /* ---- ml_erDataFraming ----------------------------------------------- */
    else if (ml_erDataFraming == Error) {
#if LIN_VERSION < 20
        /* empty handler for LIN1.3 */
#elif defined(HAS_EVENT_TRIGGERED_FRAMES)
        if (ML_FALSE == isEventTriggered) {
            l_bool_wr_XXXResponse_ErrorXXX(1U);
            l_ifc_write_status_flags_i1(ML_IFC_ERROR_IN_RESPONSE);
            l_ifc_write_status_pid_i1(LinProtectedID);
        }
#else
        l_bool_wr_XXXResponse_ErrorXXX(1U);
        l_ifc_write_status_flags_i1(ML_IFC_ERROR_IN_RESPONSE);
        l_ifc_write_status_pid_i1(LinProtectedID);
#endif

    }

    /* ---- ml_erBreakDetected --------------------------------------------- */
    else if (ml_erBreakDetected == Error)  {
        /* Last frame response too short */
#if LIN_VERSION < 20
        /* empty handler for LIN1.3 */
#elif defined(HAS_EVENT_TRIGGERED_FRAMES)
        if (ML_FALSE == isEventTriggered) {
            l_bool_wr_XXXResponse_ErrorXXX(1U);
            l_ifc_write_status_flags_i1(ML_IFC_ERROR_IN_RESPONSE);
            l_ifc_write_status_pid_i1(LinProtectedID);
        }
#else
        l_bool_wr_XXXResponse_ErrorXXX(1U);
        l_ifc_write_status_flags_i1(ML_IFC_ERROR_IN_RESPONSE);
        l_ifc_write_status_pid_i1(LinProtectedID);
#endif
    }

    /* ---- ml_erCheckSum ------------------------------------------------- */
    else if (ml_erCheckSum == Error) {
#if LIN_VERSION < 20
        /* empty handler for LIN1.3 */
#elif defined(HAS_EVENT_TRIGGERED_FRAMES)
        if (ML_FALSE == isEventTriggered) {
            l_bool_wr_XXXResponse_ErrorXXX(1U);
            l_ifc_write_status_flags_i1(ML_IFC_ERROR_IN_RESPONSE);
            l_ifc_write_status_pid_i1(LinProtectedID);
        }
#else
        l_bool_wr_XXXResponse_ErrorXXX(1U);
        l_ifc_write_status_flags_i1(ML_IFC_ERROR_IN_RESPONSE);
        l_ifc_write_status_pid_i1(LinProtectedID);
#endif
    }

    /* ---- ml_erBit ||  ml_erStopBitTX ------------------------------------ */
    else if ((ml_erBit == Error) || (ml_erStopBitTX == Error)) {
#if LIN_VERSION < 20
        /* empty handler for LIN1.3 */
#elif defined(HAS_EVENT_TRIGGERED_FRAMES)
        if (ML_FALSE == isEventTriggered) {
            /*
             * Clear flag which indicates that response_error signal
             * is being transmitted
             */
            l_flg_clr_XXXResponse_ErrorXXX();

            l_bool_wr_XXXResponse_ErrorXXX(1U);
            l_ifc_write_status_flags_i1(ML_IFC_ERROR_IN_RESPONSE);
            l_ifc_write_status_pid_i1(LinProtectedID);
        }
#else
       /*
        * Clear flag which indicates that response_error signal
        * is being transmitted
        */
        l_flg_clr_XXXResponse_ErrorXXX();

        l_bool_wr_XXXResponse_ErrorXXX(1U);
        l_ifc_write_status_flags_i1(ML_IFC_ERROR_IN_RESPONSE);
        l_ifc_write_status_pid_i1(LinProtectedID);
#endif

    }

    /* ---- ml_erIdParity -------------------------------------------------- */
    else if (ml_erIdParity == Error) {
        /* do NOT set response_error bit, because error occurred in a header */
    }

    /* ---- default -------------------------------------------------------- */
    else {
       /* nothing to do yet */
    }

#if defined(HAS_EVENT_TRIGGERED_FRAMES)
    isEventTriggered = ML_FALSE;
#endif /* HAS_EVENT_TRIGGERED_FRAMES */
}

#if defined (HAS_LIN_AUTOADDRESSING)
/*
 ******************************************************************************
 * mlu_AutoAddressingStep event
 *
 * A step is sent by the LIN Module during the auto addressing
 ******************************************************************************
 */
__attribute__((weak)) void mlu_AutoAddressingStep(ml_uint8 StepNumber)
{
    (void)StepNumber;   /* unused parameter */
}
#endif /* HAS_LIN_AUTOADDRESSING */

/*
 ******************************************************************************
 * mlu_DataTransmitted event
 *
 * Data have been successfully transmitted 
 ******************************************************************************
 */
void mlu_DataTransmitted(void)
{
#if LIN_VERSION >= 20
    /* 
     * Successful transfer is set if a frame has been
     * transmitted/received without an error
     */
    l_ifc_write_status_flags_i1(ML_IFC_SUCCESSFUL_TRANSFER);
    l_ifc_write_status_pid_i1(LinProtectedID);


    /*
     * Response_Error flag is set when Response Error signal is copied to
     * frame buffer for transmission in the current frame. If an error occurred
     * during transmission, the flag is cleared in mlu_ErrorDetected handler.
     */
    if (l_flg_tst_XXXResponse_ErrorXXX() != 0U) {
        l_flg_clr_XXXResponse_ErrorXXX();
        
        /* response error has been successfully transmitted */
        l_bool_wr_XXXResponse_ErrorXXX(0U);
    }
#endif /* LIN_VERSION >= 20 */

#if defined(HAS_EVENT_TRIGGERED_FRAMES)
    if (midx_ < ML_NUMBER_OF_DYNAMIC_MESSAGES) {
        if (ML_TRUE == isEventTriggered) {
            setAttachedFlags(midx_);  /* Set all attached flags to 1 (signals successfully transmitted) */
        }
        /* Unconditional frame: replace message index of unconditional frame 
         * by the message index of associated event-triggered frame. 
         * If there is no associated event-triggered frame for this message
         * index, midx_ will contain 0xFF
         */
        else {
            midx_ = associatedFrames[midx_];
        }
        
        /* Here midx_ contains index of event-triggered frame or 0xFF */
        /* if associated event-triggered frame exists */
        if (midx_ < ML_NUMBER_OF_DYNAMIC_MESSAGES) {
            l_u16 bitmask = (l_u16)1 << midx_;  /* for midx_ frame ..       */
            frame_updated &= ~bitmask;          /* ..  clear updated flag   */
        }
        /* else : there is no associated frame for this message index */
    }
    
    isEventTriggered = ML_FALSE;
    midx_ = 0xFFU;  /* set to invalid index */
#endif /* HAS_EVENT_TRIGGERED_FRAMES */
}

/*
 ****************************************************************
 * set_attached_flags
 * 
 * Set all flags attached to the frame MessageIndex
 * Note that MessageIndex < ML_NUMBER_OF_DYNAMIC_MESSAGES
 ****************************************************************
 */
static void setAttachedFlags (const ml_MessageID MessageIndex)
{
    ml_uint8 volatile *dest;
    ml_uint16 size;


    dest = frame_list[MessageIndex].flagsBuffer;
    if (dest != (void *) 0) {
        size = frame_list[MessageIndex].flagsBufferSize;
        while (size != 0U) {
            size--;
                                /*lint -esym(960, 17.4) -esym(961, 12.13) permitted by MISRA 2012 */
            *dest++ = 0xFFU;    /* set all flags to '1' */
                                /*lint -esym(960, 17.4) -esym(961, 12.13) permitted by MISRA 2012 */
        }
    }
}

/* EOF */
