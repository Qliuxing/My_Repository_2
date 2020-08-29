/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Generic SENT Interrupt Routines
 *
 */
#include <ioports.h>
#include "sent_ISR.h"
#include "sent_lowlvl.h"
#include "sent_api.h"
#include <sent_cfgGEN.h>
#include <sent_cbk.h>


/******************************************************
 * Slow Channel Empty Interrupt Handler
 ******************************************************/
void SENT_ISR_FastCHempty(void)
{
    SENT_APP_onEnterFCemptyIsr();

    /* Prepare SENT fast channel data for the new frame */
    SENT_API_FCprepareData();

    /* Update the Data for Fast Channel Generation (and Status Bits) */
    SENT_LL_ChangeFCdata();

    SENT_APP_onExitFCemptyIsr();
}

/******************************************************
 * Slow Channel Empty Interrupt Handler
 ******************************************************/
void SENT_ISR_SlowCHempty(void)
{
    SENT_APP_onEnterSCemptyIsr();

    /* Copy the Previously Prepared ID and Data */
    SENT_LL_ChangeSCdata();

    SENT_APP_onExitSCemptyIsr();
}

/******************************************************
 * Slow Channel Prepare Data Software Interrupt Handler
 ******************************************************/
void SENT_ISR_SlowCHprepDTA(void)
{
    if(SENT_APP_onEnterSCprepData() == 0)
    {
        /* Prepare SENT slow channel data for new message */
        SENT_API_SCprepareData();
    }
    SENT_APP_onExitSCprepData();
}

/* EOF */
