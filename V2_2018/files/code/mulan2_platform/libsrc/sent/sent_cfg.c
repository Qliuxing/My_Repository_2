/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * 12130 common routines
 *
 */
#include <ioports.h>
#include <syslib.h>
#include <sent_cfgGEN.h>
#include <sent_cfgFC.h>
#include <sent_cbk.h>
#include "sent_cfg.h"
#include "sent_structs.h"


/* --------------------------------------------- */
/* Local Variables                               */
/* --------------------------------------------- */
#if DEF_SENT_CFG_TYPE == 0
union UNION_SENT_CFG_COMPR SENT_CMP_CFG;
#endif


/* --------------------------------------------- */
/* Local Constants                               */
/* --------------------------------------------- */


/* --------------------------------------------- */
/* Local Functions                               */
/* --------------------------------------------- */



/******************************************************
 *  Do SENT module configuration
 *  Arguments:
 *  -
 *  Return:
 *  -
 *  Input:
 *  -
 *  Output:
 *  -
 ******************************************************/
void SENT_CFG_Config(void)
{
#if DEF_SENT_CFG_TYPE == 0
    SENT_CMP_CFG.chValue = EEP_SENT_CONFIG_COMPR_BYTE;

    /* Initialize the Low Level SENT module */
    SENT_API_Config_HW((iSENTconfig1[SENT_CMP_CFG.parse.Pause]), iSENTconfig2[SENT_CMP_CFG.parse.TickDiv], iSENTconfig3[SENT_CMP_CFG.parse.InvSent], 0);

    /* Set Fast Channel Configuration (Pointers/Type/...) */
    #ifdef DEF_USE_FC_CFG_FC0_FC1_EE_PTR
        if((iFCpredefFCCFG[SENT_CMP_CFG.parse.FCconfig]&0x0F) == DEF_SENT_FC_CFG_FC0_FC1_EE_PTR)
        {
            SENT_API_Set_CH0ptr((uint16*)EEP_FC0_PTR);
            SENT_API_Set_CH1ptr((uint16*)EEP_FC1_PTR);
        }
        else
    #endif
    {
        SENT_API_Set_CH0ptr((uint16*)iFCpredefPTRch0[SENT_CMP_CFG.parse.FCconfig]);
        SENT_API_Set_CH1ptr((uint16*)iFCpredefPTRch1[SENT_CMP_CFG.parse.FCconfig]);
    }

    SENT_API_Config(iFCpredefFCCFG[SENT_CMP_CFG.parse.FCconfig]);
#elif DEF_SENT_CFG_TYPE == 1
    uint16 iTemp;

    /* Initialize the Low Level SENT module */
    iTemp = SENT_APP_GetConfig1(EEP_SENT_CONFIG1);
    SENT_API_Config_HW(iTemp, EEP_SENT_CONFIG2, EEP_SENT_CONFIG3, 0);

    /* Set Fast Channel Configuration (Pointers/Type/...) */
    SENT_API_Set_CH0ptr((uint16*)((EEP_FC_PTR_CH0&0x3FFF)<<1));
    SENT_API_Set_CH1ptr((uint16*)((EEP_FC_PTR_CH1&0x3FFF)<<1));

    SENT_API_Config(EEP_SENT_FC_CFG);
#endif
}

/******************************************************
 *  Check SENT module configuration
 *  Arguments:
 *  -
 *  Return:
 *  -
 *  Input:
 *  -
 *  Output:
 *  -
 ******************************************************/
#ifdef DEF_SENT_ERROR_POINTER
uint8 SENT_CFG_CheckConfig(uint16* ptrError)
#else
uint8 SENT_CFG_CheckConfig(void)
#endif
{
    uint16 iTemp;
    uint8 chReturn;

    chReturn = 0U;

#if DEF_SENT_CFG_TYPE == 0
    if(SENT_CMP_CFG.chValue != EEP_SENT_CONFIG_COMPR_BYTE)
    {
        #ifdef DEF_SENT_ERROR_POINTER
            *ptrError = (uint16)(&SENT_CMP_CFG);
        #endif
        chReturn = 1;
    }

    /* Check Fast Channel Configuration (Pointers/Type/...) */
    #ifdef DEF_SENT_ERROR_POINTER
        chReturn |= SENT_API_CheckConfig(iFCpredefFCCFG[SENT_CMP_CFG.parse.FCconfig], ptrError);
    #else
        chReturn |= SENT_API_CheckConfig(iFCpredefFCCFG[SENT_CMP_CFG.parse.FCconfig]);
    #endif

    #ifdef DEF_USE_FC_CFG_FC0_FC1_EE_PTR
        if((iFCpredefFCCFG[SENT_CMP_CFG.parse.FCconfig]&0x0F) == DEF_SENT_FC_CFG_FC0_FC1_EE_PTR)
        {
            if(SENT_API_Get_CH0ptr() != (uint16*)EEP_FC0_PTR)
            {
                #ifdef DEF_SENT_ERROR_POINTER
                    *ptrError = (uint16)(&EEP_FC0_PTR);
                #endif
                chReturn = 1;
            }
            if(SENT_API_Get_CH1ptr() != (uint16*)EEP_FC1_PTR)
            {
                #ifdef DEF_SENT_ERROR_POINTER
                    *ptrError = (uint16)(&EEP_FC1_PTR);
                #endif
                chReturn = 1;
            }
        }
        else
    #endif
    {
        if(SENT_API_Get_CH0ptr() != (uint16*)iFCpredefPTRch0[SENT_CMP_CFG.parse.FCconfig])
        {
            #ifdef DEF_SENT_ERROR_POINTER
                *ptrError = (uint16)(&iFCpredefPTRch0);
            #endif
            chReturn = 1;
        }
        if(SENT_API_Get_CH1ptr() != (uint16*)iFCpredefPTRch1[SENT_CMP_CFG.parse.FCconfig])
        {
            #ifdef DEF_SENT_ERROR_POINTER
                *ptrError = (uint16)(&iFCpredefPTRch1);
            #endif
            chReturn = 1;
        }
    }

    /* Check the Low Level SENT module */
    iTemp = SENT_APP_GetConfig1((iSENTconfig1[SENT_CMP_CFG.parse.Pause]));
    #ifdef DEF_SENT_ERROR_POINTER
        chReturn |= SENT_API_CheckConfig_HW(iTemp, iSENTconfig2[SENT_CMP_CFG.parse.TickDiv], iSENTconfig3[SENT_CMP_CFG.parse.InvSent], 0, ptrError);
    #else
        chReturn |= SENT_API_CheckConfig_HW(iTemp, iSENTconfig2[SENT_CMP_CFG.parse.TickDiv], iSENTconfig3[SENT_CMP_CFG.parse.InvSent], 0);
    #endif
#elif DEF_SENT_CFG_TYPE == 1
    /* Check Fast Channel Configuration (Pointers/Type/...) */
    #ifdef DEF_SENT_ERROR_POINTER
        chReturn |= SENT_API_CheckConfig(EEP_SENT_FC_CFG, ptrError);
    #else
        chReturn |= SENT_API_CheckConfig(EEP_SENT_FC_CFG);
    #endif

    if(SENT_API_Get_CH0ptr() != (uint16*)((EEP_FC_PTR_CH0&0x3FFF)<<1))
    {
        #ifdef DEF_SENT_ERROR_POINTER
            *ptrError = EEP_FC_PTR_CH0;
        #endif
        chReturn = 1;
    }
    if(SENT_API_Get_CH1ptr() != (uint16*)((EEP_FC_PTR_CH1&0x3FFF)<<1))
    {
        #ifdef DEF_SENT_ERROR_POINTER
            *ptrError = EEP_FC_PTR_CH1;
        #endif
        chReturn = 1;
    }

    /* Check the Low Level SENT module */
    iTemp = SENT_APP_GetConfig1(EEP_SENT_CONFIG1);
    #ifdef DEF_SENT_ERROR_POINTER
        chReturn |= SENT_API_CheckConfig_HW(iTemp, EEP_SENT_CONFIG2, EEP_SENT_CONFIG3, 0, ptrError);
    #else
        chReturn |= SENT_API_CheckConfig_HW(iTemp, EEP_SENT_CONFIG2, EEP_SENT_CONFIG3, 0);
    #endif
#endif

    return chReturn;
}

/******************************************************
 *  Do SENT module configuration
 *  Arguments:
 *  -
 *  Return:
 *  -
 *  Input:
 *  -
 *  Output:
 *  -
 ******************************************************/
void SENT_CFG_Enable(void)
{
    /* If necessary Fast Channel Data will be prepared in this function (app specific) */
    SENT_APP_beforeEnableSENToutput();

    /* Enable the SENT Output */
    SENT_LL_EnableOutput();

    /* Previous message is copied to RAM in Enable routine, now prepare next message */
    SENT_API_SCprepareData();
}

/* EOF */
