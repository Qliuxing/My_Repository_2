/*
 * Copyright (C) 2005-2015 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 *  NOTES:
 *      1.  All functions in this module should be defined with __MLX_TEXT__
 *          attribute. Thus they will be linked in first half of the Flash.
 *
 *      2.  This module should NOT use _initialized_ global and static variables!
 *          Such variables are linked into .data or .dp.data sections and their
 *          initialization lists are stored at load address (LMA) in the Flash.
 *          Since there is no control on the position of the load address, the
 *          linker might link it to second half of the Flash and thus
 *          initialization values will be overwritten by the loader during
 *          programming of the a new application. As a result, variables in .data
 *          sections will not be correctly initialized.
 *          Use uninitialized variables instead (will be linked to .bss section).
 */

#include <syslib.h>
#include "lin.h"
#include "lin_internal.h"
#include "mark.h"

#if (LIN_PIN_LOADER != 0)
#include "flashupload.h"
#include "lin_nad.h"
#endif /* LIN_PIN_LOADER */


/* Generate warnings for debug options */
#if defined (DEBUG_HAS_MLX4_EVENT_BUFFER)
# warning "DEBUG_HAS_MLX4_EVENT_BUFFER is enabled"
#endif

#if defined (DEBUG_HAS_MLX4_EVENT_SCOPE_MARKER)
# warning "DEBUG_HAS_MLX4_EVENT_SCOPE_MARKER is enabled"
#endif

#if defined (DEBUG_HAS_SLEEP_STATE_MARKER)
# warning "DEBUG_HAS_SLEEP_STATE_MARKER is enabled"
#endif

#if defined (DEBUG_HAS_MLX4_EVENT_BUFFER)
/*
 * To enable event buffer, specify `-DDEBUG_HAS_MLX4_EVENT_BUFFER'
 * in the Config.mk
 */

#define BUF_SIZE    64
#define BUF_MASK    (BUF_SIZE - 1)

#pragma space nodp
static uint16_t events[BUF_SIZE];
static uint8_t idx = 0;
#pragma space none

/* Verify configuration */
#if (BUF_SIZE < 2) || (BUF_SIZE & BUF_MASK)
#error "BUF_SIZE shall be in a powers of 2, i.e. 8, 16, 32 etc"
#endif

#endif /* DEBUG_HAS_MLX4_EVENT_BUFFER */

volatile ml_uint8 LinStatus;

#pragma space dp

ml_uint8  LinFrameDataBuffer[8] __attribute__((aligned(2)));    /* copy of shared LinFrame buffer   */
ml_uint16 LinMessage;                                           /* copy of shared LinMess word      */

#pragma space none

#if (LIN_PIN_LOADER != 0)
ml_uint8  LIN_nad __attribute__((dp));  /* Global value. Current value of NAD for "Enter to Program Mode" */

/*
 * Store NAD info (NAD value and security key) in fixed ram memory.
 * This NAD is accessed from Application, System and LINLoader.
 */
volatile ST_FIXED_RAM_NAD stFixedRamNAD __attribute__ ((section(".ram_lin_fixed")));
#endif /* LIN_PIN_LOADER */

/* LINPrescaler and LINBaud values, that will be used in ml_GetBaudRate */
volatile ml_uint8 LINBaud  = 100;
volatile ml_uint16 LINPresc = 0;

extern ml_Status    ml_ReleaseBuffer(void);
/* ----------------------------------------------------------------------------
 * Get LIN event data from MLX4
 *
 * Copies LIN event data from the shared memory to the private structures
 */
__MLX_TEXT__  void ml_GetLinEventData (void)
{
    uint16_t LinCommand;


    SLVIT = 0xAAU;                      /* Disable all slave interrupts                 */
    LinMessage = LinMess;               /* Get the message from the shared memory area  */

#if defined (DEBUG_HAS_MLX4_EVENT_SCOPE_MARKER)
    IO_EXTIO = IO4_OUT | IO4_ENABLE;  /* IO4=1 */
#endif

#if defined (DEBUG_HAS_MLX4_EVENT_BUFFER)
    events[idx & BUF_MASK] = LinMessage;
    idx++;
#endif

    CLEAR_MLX4_INT();                   /* Clear pending M4_SHEM interrupt */

    LinCommand = LinMessage & 0x000FU;  /* get the command: LinCommand = LinMessage[3:0]    */

#if defined (_DEBUG_LIN_MARK)
    markv(1, 0x8A00, LinCommand);       /* Mark instruction : display the command received */
#endif /* _DEBUG_LIN_MARK */

    if (evMESSrcvd == LinCommand)  {    /* if message received then copy LinFrame to LinFrameDataBuffer (4 words) */
        /*
         * LinFrame and LinFrameDataBuffer buffers are aligned to word,
         * so can copy a word at a time.
         */
        uint16_t const *src = (uint16_t const *)LinFrame;
        uint16_t *dst       = (uint16_t *)LinFrameDataBuffer;

        *dst++ = *src++;    /* copy 4 words => 8 bytes */
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;

        (void)ml_ReleaseBuffer();           /* release LinFrame[] buffer for MLX4;
                                             * data in LinFrameDataBuffer[] is still available
                                             * since they're updated only at the the beginning
                                             * of the LIN ISR
                                             */
    }
    /* else : LinFrame (shared) is not copied to LinFrameDataBuffer (mlx16 private) */

    SLVCMD = 0x42U;                     /* Do the handshake and let the Mlx4 go */
}

/* ----------------------------------------------------------------------------
 * Process LIN event
 */
__MLX_TEXT__  void ml_ProccessLinEvent (void)
{
    ml_LinError Error;
    uint16_t LinCommand;

    LinCommand = LinMessage & 0x000FU;  /* get the command: LinCommand = LinMessage[3:0]    */

#if STANDALONE_LOADER == 1

    if (ml_driver_mode != kLinLoaderMode) { /* --- application mode ---------------------------- */
        switch(LinCommand) {
            case evMESSrcvd :/* Message received (data is available in the buffer) */
                if ( ldr_isReadByIdMessage(LinFrameDataBuffer, ML_TRUE) ) { /* check if we received a Read By ID for the loader */
                    ml_ldr_ReadByIdMessage(LinFrameDataBuffer[3]);
                }
                else {
                    /* ignore message */
                }
                break;

            default:
                /* Events evERR, evNONE, evMESSrqst and evENDtx are ignored */
                break;
        }
    }
    else { /* --- loader mode --------------------------------------------------------- */
        switch(LinCommand)
        {
            case evERR :    /* error detected by the LIN task */
                /* Get the error type
                 * Errors erCKSUM, erTXCOL, erCRASHPLL and erCRASH are reported in stadalone mode
                 */
                Error = (LinMessage >> 4) & 0x000F;
                ml_ldr_ErrorDetected(Error); /* notify flash loader about error */
                break;

            case evMESSrcvd :/* Message received (data is available in the buffer) */
                if ( (LinFrameDataBuffer[0] == LIN_nad)
                     || (LinFrameDataBuffer[0] == 0x7F /* wildcard */ )) {
                    ml_DiagReceived();  /* notify loader */
                }
                else {
                    /* ignore message */
                }
                break;

            case evENDtx :  /* Transmit done (without any collision) */
                ml_DiagRequest();
                break;
                                    
            default :   /* Ignore other events */
                break;
        }
    }
#else /* standard LIN handler */

#if defined (HAS_LIN_AUTOADDRESSING)
    if ( LinCommand == evCOOLAUTO )    /* Cooling Auto-Addressing pulse */
    {
        ml_uint8 CoolingPulse = (ml_uint8) ((LinMessage >> 4) & 0x000F);
        mlu_AutoAddressingStep( CoolingPulse);
    }
    else
    {

#endif /* HAS_LIN_AUTOADDRESSING */
    ml_MessageID LinID = (LinMessage >> 8) & 0x003F; /* get the LinID: LinID = LinMessage[13:8] */

#if (LIN_PIN_LOADER != 0)
    if (ml_driver_mode != kLinLoaderMode) {  /* --- Application mode --------------------------- */
        if (LDR_GetState() == 0) {
#endif
            switch(LinCommand) {
                case evSTCH :   /* state of the LIN task has changed */
                    {
                    /*
                     * LinMessage[15:12]    State change reason
                     * LinMessage[11:8]     Old state
                     * LinMessage[7:4]      New state
                     * LinMessage[3:0]      event code
                     */
                    ml_uint16 NewState;

                    NewState = LinMessage & 0x00F0U;
                    if ((stSLEEP << 4) == NewState) {           /* signal to application only transition to SLEEP state */

#if defined (DEBUG_HAS_SLEEP_STATE_MARKER)
                    IO_EXTIO = IO4_OUT | IO4_ENABLE;            /* IO4 = 1 */
#endif

                        ml_StateReason Reason = (ml_StateReason)((LinMessage >> 12) & 0x000FU);    /* decode the reason of SLEEP */
                        mlu_LinSleepMode(Reason);
                    }
                    /* else: skip signaling other transitions */
                    }
                    break;

                case evERR :    /* error detected by the LIN task */
                    /* get the error type */
                    Error = (LinMessage >> 4) & 0x000F;

                    if (ml_erBit == Error) {
                        ml_uint8  crashedBit;
                        ml_uint8  crashedByte;


                       /*
                        * Get additional parameters in case of collision error
                        * LinMessage[8:11]  Crashed Bit
                        *                   0: stop bit of the previous byte
                        *                   1: start bit of current byte
                        *                   2-9: data bits
                        *                   10: stop bit of the current byte
                        *
                        * LinMessage[12:15] Crashed Byte
                        *                   0-8: data byte number
                        *                   15: collision before start of TX
                        */
                        crashedBit = (ml_uint8) ((LinMessage >> 8) & 0x000F);
                        crashedByte = (ml_uint8) ((LinMessage >> 12) & 0x000F);

                        /* If collision was during TX of STOP bit,
                         * report it as a separate error (SAE J2602)
                         */
                        if ( ((crashedBit == 0) || (crashedBit > 9)) && (crashedByte != 15) ) {
                            Error = ml_erStopBitTX;
                        }
                        /* else : collision was during TX of the data bits */
                    }
                    else if (ml_erBreakDetected == Error) {
                        ml_uint8 nbytes; /* number of received bytes */

                        nbytes = (LinMessage >> 12) & 0x000F;  /* get LINmess+3 parameter */
                        if(nbytes == 0)
                        {
                            /* TODO also check if ID byte = 0, otherwise it is a real stop bit error */
                            break;
                        }
                    }
                    else if (ml_erLinModuleReset == Error)  {
                        ml_uint8 subcode;

                        subcode = (LinMessage >> 8) & 0x000F;  /* get LINmess+2 parameter */

                        if (subcode == erCRASHTX) {
                            /* Propagation delay error : TX/RX propagation can not
                             * be calculated by LIN module due to collision at start bit
                             * of the own response. Both dominant and recessive collisions
                             * could be the reason of this error
                             */
                            (void)ml_Disconnect();  /* for safety and clarity (MLX4 is already in disconnected state) */
                            (void)ml_Connect();
                            Error = ml_erBit; /* map this error to "collision error" for application */
                        }
                    }
                    /* else :  Nothing for other errors */

                    mlu_ErrorDetected(Error); /* notify application about error */
                    break;

                case evMESSrcvd :/* Message received (data is available in the buffer) */

#if defined (DEBUG_HAS_SLEEP_STATE_MARKER)
                    IO_EXTIO = IO4_ENABLE;  /* IO4 = 0 */
#endif
                    LINPresc = LinMess;     /* Copy LIN prescaller |XXXX|XXXX|PRES|XXXX| */
                    LINBaud  = LinMess2;    /* Copy LIN baud (divider) */

#if (LIN_PIN_LOADER != 0)
                    if (LinID != D_DIA) {  /* fast check if it is not MRF frame */
                        mlu_MessageReceived(LinID); /* notify application */
                    }
                    else if ( ldr_isReadByIdMessage(LinFrameDataBuffer, ML_FALSE) ) { /* check if we received a Read By ID for the loader */
                        ml_ldr_ReadByIdMessage(LinFrameDataBuffer[3]);
                    }
                    else {
                        mlu_MessageReceived(D_DIA); /* notify application */
                    }
#else
                    mlu_MessageReceived(LinID); /* notify application */
#endif /* LIN_PIN_LOADER */                    
                    break;

                case evMESSrqst :/* message ID received, TX identified, request data */

#if defined (DEBUG_HAS_SLEEP_STATE_MARKER)
                    IO_EXTIO = IO4_ENABLE;  /* IO4 = 0 */
#endif

                    LINPresc = LinMess;     /* Copy LIN prescaller |XXXX|XXXX|PRES|XXXX| */
                    LINBaud  = LinMess2;    /* Copy LIN baud (divider) */

                    mlu_DataRequest(LinID);
                    break;

                case evENDtx :  /* Transmit done (without any collision) */
                    mlu_DataTransmitted();
                    break;

                default:    /* Ignore other events */
                    break;
            }  /* ! switch */
#if (LIN_PIN_LOADER != 0)
        }
        /*
         * state != 0 => flash reprogramming states
         */
        else {
            switch(LinCommand) {
                case evERR :    /* error detected by the LIN task */
                    /*
                     * There is no recovering from critical error erCRASH, erCRASHTX
                     * A reset should be applied to recover from such errors
                     */
                    Error = (LinMessage >> 4) & 0x000F; /* get the error type */
                    ml_ldr_ErrorDetected(Error);        /* notify flash loader about error */
                    break;

                case evMESSrcvd :/* Message received (data is available in the buffer) */

                    LINPresc = LinMess;     /* Copy LIN prescaller |XXXX|XXXX|PRES|XXXX| */
                    LINBaud  = LinMess2;    /* Copy LIN baud (divider) */

                    if (LinID == D_DIA) {
                        if ( ldr_isReadByIdMessage(LinFrameDataBuffer, ML_TRUE) ) { /* check if we received a Read By ID for the loader */
                            ml_ldr_ReadByIdMessage(LinFrameDataBuffer[3]);
                        }
                    }
                    else {
                        /* ignore other messages */
                    }
                    break;

                case evMESSrqst :/* ID received, TX identified, request data */

                    LINPresc = LinMess;     /* Copy LIN prescaller |XXXX|XXXX|PRES|XXXX| */
                    LINBaud  = LinMess2;    /* Copy LIN baud (divider) */

                    (void)ml_DiscardFrame();   /* other master requests are not expected here */
                    break;

                default :   /* Command not relevant in this mode */
                    /*
                     * Events : evNONE, evSTCH, evENDtx, evIDrcvd and evCOOLAUTO
                     * are ignored in loader mode
                     */
                    break;
            }
        }
    }
    else {
         /* --- Loader mode -------------------------------------------------- */
         switch(LinCommand) {
             case evERR :    /* error detected by the LIN task */
                 Error = (LinMessage >> 4) & 0x000F; /* get the error type */
                 ml_ldr_ErrorDetected(Error); /* notify flash loader about error */
                 break;

             case evMESSrcvd : /* Message received (data is available in the buffer) */
                 if ( (LinID == D_DIA) /* MRF diag frame */
                      && (   (LinFrameDataBuffer[0] == LIN_nad)
                          || (LinFrameDataBuffer[0] == 0x7F /* wildcard */ )))
                 {
                     ml_DiagReceived();  /* notify loader */
                 }
                 else {
                     /* ignore message */
                 }
                 break;

             case evMESSrqst :   /* message ID received, TX identified, request data */
                 (void)ml_DiscardFrame();   /* other requests are not expected in the loader mode */
                 break;

             case evENDtx :  /* Transmit done (without any collision) */
                 ml_DiagRequest();
                 break;

             default :
                 /*
                  * Events : evNONE, evSTCH, evIDrcvd and evCOOLAUTO
                  * are ignored in loader mode
                  */
                 break;
         } /* ! switch */
    }
#endif /* LIN_PIN_LOADER */
#if defined (HAS_LIN_AUTOADDRESSING)
    }
#endif /* HAS_LIN_AUTOADDRESSING */
#endif /* STANDALONE_LOADER */

    SLVIT = 0xABU;  /* Enable Event interrupt (SLVIT[8] = 1) (get ready for the next interrupt) */

#if defined (DEBUG_HAS_MLX4_EVENT_SCOPE_MARKER)
    IO_EXTIO = IO4_ENABLE;  /* IO4 = 0 */
#endif /* DEBUG_HAS_MLX4_EVENT_SCOPE_MARKER */

}

#if (LIN_PIN_LOADER != 0)

/* ----------------------------------------------------------------------------
 * To set NAD for Loader (by default value is MLX_NAD).
 */
__MLX_TEXT__ ml_Status ml_SetLoaderNAD(ml_uint8 Nad)
{
    stFixedRamNAD.nad = Nad;
    stFixedRamNAD.key = _mlx_NAD_Security_Key;
    LIN_nad = Nad;
    return ML_SUCCESS;
}

/* ----------------------------------------------------------------------------
 * To get NAD of Loader.
 */
__MLX_TEXT__ ml_uint8 ml_GetLoaderNAD(void)
{
    return LIN_nad;
}

#endif /* LIN_PIN_LOADER */

/* EOF */
