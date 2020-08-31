/*
 * Copyright (C) 2005-2011 Melexis N.V.
 *
 * Software Platform
 *
 */

#include "lin.h"

ml_Status mlu_ApplicationStop(void)
{
    return ML_SUCCESS;      /* by default, return that the application has stopped */
}

#if STANDALONE_LOADER == 0
void mlu_AutoAddressingStep(ml_uint8 StepNumber)
{
    (void)StepNumber;       /* not used */
}

void mlu_DataRequest(ml_MessageID MessageIndex)
{
    (void)MessageIndex;     /* unused parameter */
    ml_DiscardFrame();
}

void mlu_DataTransmitted(void)
{
    /* empty */
}

void mlu_ErrorDetected(ml_LinError Error)
{
    (void)Error; /* not used */
}

void mlu_LinSleepMode(ml_StateReason Reason)
{
    (void)Reason;   /* not used */

    /* some action during entering sleep state */
}

void mlu_MessageReceived (ml_MessageID Index)
{
    (void)Index; /* not used */
}

#endif /* STANDALONE_LOADER */

/* EOF */
