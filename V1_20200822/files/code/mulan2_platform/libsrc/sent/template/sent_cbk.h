/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Generic SENT Config
 *
 */

#ifndef SENT_CBK_H_
    #define SENT_CBK_H_

    #include <syslib.h>
    #include <sent_api.h>
    #include "eep.h"
    #include "common.h"


    #define SOFT_ISR_REQUEST()   ( VARIOUS_L |= SWI )

    /* ---------------------------------------------------------------------------- */
    /*  Called in the SENT configuration function just before enabling SENT output  */
    /* ---------------------------------------------------------------------------- */
    static INLINE void  SENT_APP_beforeEnableSENToutput (void)
    {
        /* 14612 no 0(initializing) may be output on SENT bus */

        /* Prepare valid Fast Channel Data */
        SENT_FCprepareData();

        /* Get correct status bits from application */
        chStatNibble = common_GetStatus();
    }

    /* ---------------------------------------------------------------------------- */
    /*  Called on enter Fast Channel Empty interrupt                                */
    /* ---------------------------------------------------------------------------- */
    static INLINE void  SENT_APP_onEnterFCemptyIsr(void)
    {
#if 0
        /* Read back check that the values of the SENT RAM DMA cells are matching
         * the values of the SENT hw IP internal flip-flops.
         */
        if( ((iSentDMA[0] != (uint16)(SENT_FC_STAT&0x000Fu)) || \
             (iSentDMA[1] != (uint16)SENT_FC0_DTA) || \
             (iSentDMA[2] != (uint16)SENT_FC1_DTA) )
        {
            /* Error: Data inconsistent */
            commom_SENT_DMAerr();
        }
#endif
    }

    /* ---------------------------------------------------------------------------- */
    /*  Called on exit Fast Channel Empty interrupt                                 */
    /* ---------------------------------------------------------------------------- */
    static INLINE void  SENT_APP_onExitFCemptyIsr (void)
    {
#if 0
        /* Check that the correct values (selected with the pointers) are written
         * in the RAM DMA locations. If not the same go to the fail safe state */
        /* req: MLX12130SW-35 - Read back from DMA cell once writen */
        if( (iSentDMA[0] != (uint16)chStatNibble) || \
            (iSentDMA[1] != iFastChBuff[0]) || \
            (iSentDMA[2] != iFastChBuff[1]) )
        {
            /* Error in DMA register */
            commom_SENT_DMAerr();
        }
#endif
    }

    /* ---------------------------------------------------------------------------- */
    /*  Called on enter Slow Channel Empty interrupt                                */
    /* ---------------------------------------------------------------------------- */
    static INLINE void  SENT_APP_onEnterSCemptyIsr(void)
    {
#if 0
        if( (iSentDMA[3] != (uint16)SENT_SC_DTA) || \
            (iSentDMA[4] != (uint16)(SENT_SC_ID&0x01FF)) )
        {
            /* Error: Data inconsistent */
            commom_SENT_DMAerr();
        }
#endif
    }

    /* ---------------------------------------------------------------------------- */
    /*  Called on exit Slow Channel Empty interrupt                                 */
    /* ---------------------------------------------------------------------------- */
    static INLINE void  SENT_APP_onExitSCemptyIsr (void)
    {
#if 0
        /* Check that the correct values (selected with the pointers) are written
         * in the RAM DMA locations. If not the same go to the fail safe state */
        if( (iSentDMA[3] != iSlowChBuff[0]) || \
            (iSentDMA[4] != iSlowChBuff[1]) )
        {
            /* Error in DMA register */
            commom_SENT_DMAerr();
        }
        SOFT_ISR_REQUEST();
#endif

        /* Prepare SENT slow channel data for new message */
        SENT_SCprepareData();
    }

    /* ---------------------------------------------------------------------------- */
    /*  Called to enter Prepare Slow Channel Data Function                          */
    /* ---------------------------------------------------------------------------- */
    static INLINE uint8  SENT_APP_onEnterSCprepData (void)
    {
        common_GetDiagCode();
        return 0;       /* Normal Slow Channel Handling will continue (if 1 no new message is prepared) */
    }

    /* ---------------------------------------------------------------------------- */
    /*  Called on exit ISR Prepare Slow Channel Data Function                      */
    /* ---------------------------------------------------------------------------- */
    static INLINE void  SENT_APP_onExitSCprepData (void)
    {
#if 0
        /* If diag code was not transmitted (8bit ID 0x01) then ATOMIC_CODE(or error flags with local copy) */
        if(iSlowChBuff[1] != 0x0001)
        {
            /* Diagnostic message is not sent at this time */
            /* req: MLX12130SW-41 - SENT slow channel : Diagnostic error reporting */
            ATOMIC_CODE(
                SS_diag0_bitmap |= SS_DIAGx_BITMAP_COPY[0];
                SS_diag1_bitmap |= SS_DIAGx_BITMAP_COPY[1];
                SS_diag2_bitmap |= SS_DIAGx_BITMAP_COPY[2];
                SS_diag3_bitmap |= SS_DIAGx_BITMAP_COPY[3];
            );
        }
#endif
    }

    /* ---------------------------------------------------------------------------- */
    /*  Called when a Delayed Slow Channel Message was detected                     */
    /* ---------------------------------------------------------------------------- */
    static INLINE void  SENT_APP_onSCdelayedMsg (uint8 chMsgNr)
    {
        /*ERROR_FLAGS |= F_DIAG_SENT_SLOW;*/
    }

    /* ---------------------------------------------------------------------------- */
    /*  Called configuring/checking SENT configuration registers                    */
    /* ---------------------------------------------------------------------------- */
    static INLINE uint16 SENT_APP_GetConfig1 (uint16 iEEval)
    {
        return iEEval;
    }

#endif /* SENT_CBK_H_ */
