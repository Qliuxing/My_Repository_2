/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Generic SENT Low Level Routines
 *
 */
#include <ioports.h>
#include "sent_lowlvl.h"
#include "sent_api.h"


/* --------------------------------------------- */
/* Local Variables                               */
/* --------------------------------------------- */
uint16 iSlowChBuff[2];
uint16 iFastChBuff[2];
volatile uint16 iSentDMA[5];
uint8  chStatNibble;


/******************************************************
 *  Initialize the SENT IP block
 *  Arguments:
 *  - None
 *  Return:
 *  - None
 ******************************************************/
void SENT_LL_Config(void)
{
    iSentDMA[0]    = 0;
    iSentDMA[1]    = 0;
    iSentDMA[2]    = 0;
    iSentDMA[3]    = 0;
    iSentDMA[4]    = 0;
    iSlowChBuff[0] = 0;
    iSlowChBuff[1] = 0;
    chStatNibble   = 0;

    SENT_DBASE = (uint16)iSentDMA;
}

/******************************************************
 *  Check the Configuration of the SENT IP Module
 *  Arguments:
 *  - ptrError: pointer to the error
 *  Return:
 *  - 0: No Errors; 1: Error found
 ******************************************************/
#ifdef DEF_SENT_ERROR_POINTER
uint8 SENT_LL_CheckConfig(uint16* ptrError)
#else
uint8 SENT_LL_CheckConfig(void)
#endif
{
    uint8 chReturn;

    chReturn = 0U;

    if(SENT_DBASE != (uint16)iSentDMA)
    {
        #ifdef DEF_SENT_ERROR_POINTER
            *ptrError = (uint16)(&SENT_DBASE);
        #endif
        chReturn = 1;
    }

    return chReturn;
}

/* EOF */
