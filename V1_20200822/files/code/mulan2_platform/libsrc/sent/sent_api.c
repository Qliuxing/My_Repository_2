/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Generic SENT API Routines
 *
 */
#include <ioports.h>
#include <sent_cfgGEN.h>
#include <sent_cfgSC.h>
#include <sent_cbk.h>
#include "sent_api.h"
#include "sent_lowlvl.h"
#include "sent_structs.h"
#include <mathlib.h>            /* Included for _sfb() */


/* --------------------------------------------- */
/* Local Constants                               */
/* --------------------------------------------- */
const uint8 chSingleFastChMask[4] =
{
    0x00U,          /* No Data */
    0x07U,          /* 3 LS-Nibbles */
    0x0EU,          /* 3 MS-Nibbles */
    0x0FU           /* All Nibbles */
};

#ifdef DEF_COMPR_REP_FACT
    const uint8 chNIIlookup[8] =
    {
            0, 1, 2, 3, 4, 8, 16, 32
    };
#endif


/* --------------------------------------------- */
/* Local Defines                                 */
/* --------------------------------------------- */
#define DEF_REPMSG_CTR_DISABLED     (-3)
#define DEF_REPMSG_CTR_INIT         (-2)
#define DEF_REPMSG_CTR_DELAYED      (-1)
#define DEF_REPMSG_CTR_PENDING      (0)


/* --------------------------------------------- */
/* Local Variables                               */
/* --------------------------------------------- */
volatile uint8   chFCoverwriteEN;
volatile uint16  iFC0overwrite;
volatile uint16  iFC1overwrite;
int8    chRepMsgCTR[DEF_TOTAL_NR_SC_REP_MESSAGES];
volatile uint8   chNonRepMsgCTR;
uint8   chSecureCTR;
uint16* ptrAppFCh0;
uint16* ptrAppFCh1;
uint8   SENTfcCFG;
#define FC_CFG_CONFIG_SHIFT 0
#define FC_CFG_CONFIG_MASK  (0xF<<FC_CFG_CONFIG_SHIFT)
#define FC_CFG_FC0_SHIFT    4
#define FC_CFG_FC0_MASK     (0x3<<FC_CFG_FC0_SHIFT)
#define FC_CFG_FC1_SHIFT    6
#define FC_CFG_FC1_MASK     (0x3<<FC_CFG_FC1_SHIFT)


/* --------------------------------------------- */
/* Local Functions                               */
/* --------------------------------------------- */
int8 SENT_API_GetRepInterval(uint8 chRepMsgNr);


/******************************************************
 * Load the configuration of the SENT Module
 *  Arguments:
 *  - chFCcfg: Fast Channel Configuration Byte
 *  Return:
 *  - None
 ******************************************************/
void SENT_API_Config(uint8 chFCcfg)
{
    uint8 chTemp;

    /* Set Fast Channel Configuration Byte */
    SENTfcCFG           = chFCcfg;

    /* Initialize Variables */
    chFCoverwriteEN     = 0U;
    iFC0overwrite       = 0U;
    iFC1overwrite       = 0U;
    chNonRepMsgCTR      = 0U;
    chSecureCTR         = 0U;

    /* Calculate and Set Fast Channel mask */
    chTemp = chSingleFastChMask[(SENTfcCFG >> FC_CFG_FC0_SHIFT) & 0x03];
    chTemp <<= 4;
    chTemp |= chSingleFastChMask[(SENTfcCFG >> FC_CFG_FC1_SHIFT) & 0x03];
    SENT_LL_Mask(chTemp);

    /* Initialize the Rep Message CTR to correct value (0 or -1) */
    for(chTemp=0; chTemp<DEF_TOTAL_NR_SC_REP_MESSAGES; chTemp++)
    {
        if(SENT_API_GetRepInterval(chTemp) == DEF_REPMSG_CTR_DISABLED)
        {
            /* Not Used as Repetitive Message */
            chRepMsgCTR[chTemp] = DEF_REPMSG_CTR_DISABLED;
        }
        else
        {
            /* Used as Repetitive Message */
            chRepMsgCTR[chTemp] = DEF_REPMSG_CTR_INIT;
        }
    }
}

/******************************************************
 * Check SENT Module Configuration Registers and RAM
 *  Arguments:
 *  - chFCcfg: Fast Channel Configuration Byte
 *  - ptrError: pointer to the error
 *  Return:
 *  - 0: No Errors; 1: Error found
 ******************************************************/
#ifdef DEF_SENT_ERROR_POINTER
uint8 SENT_API_CheckConfig(uint8 chFCcfg, uint16 *ptrError)
#else
uint8 SENT_API_CheckConfig(uint8 chFCcfg)
#endif
{
    uint8 chReturn;
    uint8 chTemp;
    int8 chCurRepFact;
    int8 chExpFact;

    chReturn = 0U;

    /* Check Fast Channel Configuration Byte */
    if(SENTfcCFG != chFCcfg)
    {
        #ifdef DEF_SENT_ERROR_POINTER
            *ptrError = (uint16)(&SENTfcCFG);
        #endif
        chReturn = 1;
    }
    else
    {
        /* Calculate and Check Fast Channel mask */
        chTemp = chSingleFastChMask[(chFCcfg & FC_CFG_FC0_MASK) >> FC_CFG_FC0_SHIFT];
        chTemp <<= 4;
        chTemp |= chSingleFastChMask[(chFCcfg & FC_CFG_FC1_MASK) >> FC_CFG_FC1_SHIFT];
        #ifdef DEF_SENT_ERROR_POINTER
            chReturn |= SENT_LL_Mask_Check(chTemp, ptrError);
        #else
            chReturn |= SENT_LL_Mask_Check(chTemp);
        #endif

        /* Check the Rep Message CTR values */
        for(chTemp=0; chTemp<DEF_TOTAL_NR_SC_REP_MESSAGES; chTemp++)
        {
            chCurRepFact = chRepMsgCTR[chTemp];
            chExpFact = SENT_API_GetRepInterval(chTemp);

            if(((chExpFact != chCurRepFact) && (chExpFact == DEF_REPMSG_CTR_DISABLED)) || \
               ((chExpFact > DEF_REPMSG_CTR_DISABLED) && (chCurRepFact <= DEF_REPMSG_CTR_DISABLED)))
            {
                chReturn = 1;
                #ifdef DEF_SENT_ERROR_POINTER
                    #ifdef DEF_COMPR_REP_FACT
                        *ptrError = (uint16)(&(EEP_SC_REP_NII));
                    #else
                        *ptrError = (uint16)(&(EEP_SC_REP_INT));
                    #endif
                #endif
            }
        }
    }

    return chReturn;
}

/******************************************************
 * Fast Channel Empty Interrupt Handler
 *  Arguments:
 *  - None
 *  Return:
 *  - None
 ******************************************************/
void SENT_API_FCprepareData(void)
{
    uint16 iTemp;
    /* Copy Data to Buffers to be used in Fast Channel */
    /* Copy Data without checks because:
     * Channel 0: is always PTR type, only mask can be changed
     * Channel 1: if not PTR type data will be overwritten, if disabled mask will make sure data is not transmitted
     */
    if((chFCoverwriteEN & DEF_CHANNEL0) == 0)
    {
        iFastChBuff[0] = *ptrAppFCh0;
    }
    else
    {
        iFastChBuff[0] = iFC0overwrite;
    }

    if((chFCoverwriteEN & DEF_CHANNEL1) == 0)
    {
        iFastChBuff[1] = *ptrAppFCh1;
    }
    else
    {
        iFastChBuff[1] = iFC1overwrite;
    }

    /* Check which special Channel 0/1 configuration must be applied */
    switch((SENTfcCFG & FC_CFG_CONFIG_MASK) >> FC_CFG_CONFIG_SHIFT)
    {
#ifdef DEF_SENT_FC_CFG_CH0_ONLY
    case DEF_SENT_FC_CFG_CH0_ONLY:
        /* FC0 only with Standard Output */
        /* Channel 1 is disabled by mask, do nothing */
        break;
#endif
#ifdef DEF_SENT_FC_CFG_CH0_ONLY_4_3BIT
    case DEF_SENT_FC_CFG_CH0_ONLY_4_3BIT:
        /* FC0 Only with 4x3bit Nibbles */
        iTemp  = (iFastChBuff[0] & 0x0007) << 0;
        iTemp |= (iFastChBuff[0] & 0x0038) << 1;
        iTemp |= (iFastChBuff[0] & 0x01C0) << 2;
        iTemp |= (iFastChBuff[0] & 0x0E00) << 3;
        iFastChBuff[0]  = iTemp;
        break;
#endif
#ifdef DEF_SENT_FC_CFG_CH0_CH1_STD
    case DEF_SENT_FC_CFG_CH0_CH1_STD:
        /* FC0 and FC1 Pointer */
        /* Already done, do nothing */
        break;
#endif
#ifdef DEF_SENT_FC_CFG_CH1_SECURE_CTR
    case DEF_SENT_FC_CFG_CH1_SECURE_CTR:
        /* Secure Counter Setup */
        /* Code only usable in 3 Nibble Mode */
        iFastChBuff[1] = ~(iFastChBuff[0]);
        iFastChBuff[1] >>= 8U;
        iFastChBuff[1] &= 0x000FU;
        iFastChBuff[1] |= ((uint16)chSecureCTR)<<4;
        break;
#endif
#ifdef DEF_SENT_FC_CFG_CH1_INV
    case DEF_SENT_FC_CFG_CH1_INV:
        /* FC1 uses Inverted Output */
        iFastChBuff[1] = ~iFastChBuff[1];
        break;
#endif
#ifdef DEF_SENT_FC_CFG_CH1_REV_NIBBLES
    case DEF_SENT_FC_CFG_CH1_REV_NIBBLES:
        /* FC1 uses Reverse Nibbles Order */
        iFastChBuff[1] = ((iFastChBuff[1] & 0xF0F0) >> 4) | ((iFastChBuff[1] & 0x0F0F) << 4);
        iFastChBuff[1] = ((iFastChBuff[1] & 0xFF00) >> 8) | ((iFastChBuff[1] & 0x00FF) << 8);
        break;
#endif
#ifdef DEF_SENT_FC_CFG_CH1_REV_INV_NIBBL
    case DEF_SENT_FC_CFG_CH1_REV_INV_NIBBL:
       /* FC1 uses Inverted and Reverse Nibbles Order */
        iFastChBuff[1] = ~iFastChBuff[1];
       iFastChBuff[1] = ((iFastChBuff[1] & 0xF0F0) >> 4) | ((iFastChBuff[1] & 0x0F0F) << 4);
       iFastChBuff[1] = ((iFastChBuff[1] & 0xFF00) >> 8) | ((iFastChBuff[1] & 0x00FF) << 8);
       break;
#endif
    default:
        /* Incorrect Setting => do nothing => pointers used */
        break;
    }

    /* Increase Secure Counter for next use */
    chSecureCTR++;
}

/******************************************************
 * Get the Current Slow Channel Message Nr
 *  Arguments:
 *  - None
 *  Return:
 *  - The SC array message number
 *    0xFF: no message found
 ******************************************************/
uint8 SENT_API_GetSlowCHmsgNr(void)
{
#if DEF_TOTAL_NR_SC_REP_MESSAGES > 0
    int8  chPrio;
#endif
    uint8 chCTR;
    uint8 chReturn;
#if DEF_TOTAL_NR_EN_BITS > 16
    uint8 chTemp;
    uint8 chOffset;
#endif

    chReturn = 0xFFU;

#if DEF_TOTAL_NR_SC_REP_MESSAGES > 0
    chPrio = 0;

    /* Decrease Fast Rep Msg counters and check if a message needs to be sent */
    for(chCTR=0; chCTR<DEF_TOTAL_NR_SC_REP_MESSAGES; chCTR++)
    {
        /* Decrease counter if enabled */
#ifdef DEF_CHECK_DELAYED_SC_MSG
        if(chRepMsgCTR[chCTR] > DEF_REPMSG_CTR_DELAYED)
#else
        if(chRepMsgCTR[chCTR] > DEF_REPMSG_CTR_PENDING)
#endif
        {
            chRepMsgCTR[chCTR]--;
        }

        /* Check if Fast Repetitive Message to be sent */
#ifdef DEF_CHECK_DELAYED_SC_MSG
        if((chRepMsgCTR[chCTR]==DEF_REPMSG_CTR_DELAYED) && (chPrio < DEF_SC_PRIO_DELAYED))
        {
            chPrio   = DEF_SC_PRIO_DELAYED;
            chReturn = chCTR;
        }
        else
#endif
        if((chRepMsgCTR[chCTR]==DEF_REPMSG_CTR_INIT) && (chPrio < DEF_SC_PRIO_INIT))
        {
            chPrio   = DEF_SC_PRIO_INIT;
            chReturn = chCTR;
        }
        else if((chRepMsgCTR[chCTR]==DEF_REPMSG_CTR_PENDING) && (chPrio < DEF_SC_PRIO_PENDING))
        {
            chPrio   = DEF_SC_PRIO_PENDING;
            chReturn = chCTR;
        }
    }

    /* If a Fast Repetitive Message was found the corresponding GenMsgCTR is reset */
    if(chReturn != 0xFFU)
    {
        /* Reinitialize the Repetition Counter */
        chRepMsgCTR[chReturn] = SENT_API_GetRepInterval(chReturn);

        /* Get the correct message number (position in SC message array) */
        chReturn = chSCrepMsgArrayNr[chReturn];

        /* Check if a message was delayed */
#ifdef DEF_CHECK_DELAYED_SC_MSG
        if(chPrio >= DEF_SC_PRIO_DELAYED)
        {
            SENT_APP_onSCdelayedMsg(chReturn);
        }
#endif
    }
    else
#endif
    {
        /* Check for Other Slow CH Messages to be sent */
        for(chCTR=0; chCTR<DEF_TOTAL_NR_SC_MESSAGE; chCTR++)
        {
            /* Check slow channel enable type */
            switch(iSCmessageConfig[chNonRepMsgCTR][DEF_EN_CFG] & 0xC0)
            {
            case DEF_SCEN_DEFAULT_ON:
                /* Message is always enabled */
                chReturn = chNonRepMsgCTR;
                chCTR = DEF_TOTAL_NR_SC_MESSAGE; /* Stop Searching */
                break;
            case DEF_SCEN_STANDARD:
                /* Message is enabled by specific enable bit */
                #if DEF_TOTAL_NR_EN_BITS > 16
                    chTemp   = (iSCmessageConfig[chNonRepMsgCTR][DEF_EN_CFG]&0x3F) & 0x07;    /* Remainder for Divide by 8 = Position in Array*/
                    chOffset = (iSCmessageConfig[chNonRepMsgCTR][DEF_EN_CFG]&0x3F) >> 3;      /* Divide by 8 */

                    /* Check if Enable Bit is set (Multiple 16-bit arrays in EEPROM) */
                    if((EEP_SC_EN_BITS[chOffset] & _sfb(chTemp)) != 0U)         /* req: MLX14612SW-47 - SENT: Enable/disable of serial data messages */
                    {
                        chReturn = chNonRepMsgCTR;
                        chCTR = DEF_TOTAL_NR_SC_MESSAGE; /* Stop Searching */
                    }
                #else
                    /* Check if Enable Bit is set */
                    if((EEP_SC_EN_BITS[0] & _sfb(iSCmessageConfig[chNonRepMsgCTR][DEF_EN_CFG]&0x3F)) != 0U)  /* req: MLX14612SW-47 - SENT: Enable/disable of serial data messages */
                    {
                        chReturn = chNonRepMsgCTR;
                        chCTR = DEF_TOTAL_NR_SC_MESSAGE; /* Stop Searching */
                    }
                #endif
                break;
            default:
                /* Message is always disabled */
                break;
            }

            /* Increase NonRepetitive Message Counter, Reset if Max was reached */
            chNonRepMsgCTR++;
            if(chNonRepMsgCTR >= DEF_TOTAL_NR_SC_MESSAGE)
            {
                chNonRepMsgCTR = 0U;
            }
        }
    }

    return chReturn;
}

/******************************************************
 * Get the Correct Slow Channel Data and ID from Mem
 *  Arguments:
 *  - chMsgNr: The message number to find the data for
 *  Return:
 *  - None
 *  Globals:
 *  - iSlowChBuff[x]: The message data is copied to the
 *          slow channel buffer.
 ******************************************************/
void SENT_API_GetSlowChDTAfromMem(uint8 chMsgNr)
{
    union SC_MSG_DTA SCDTA;
    uint8 SCdtaCfg;
    uint8 SCidCfg;
#if DEF_TOTAL_NR_SC_MSG_DTA_LOC_EE_CMPR > 0
    uint8 chTemp;
#endif

    iSlowChBuff[1] = 0;

    /* Check Value of General Message Counter */
    if(chMsgNr < DEF_TOTAL_NR_SC_MESSAGE)
    {
        /* Get the Correct Configuration Word from ROM table */
        SCidCfg  = iSCmessageConfig[chMsgNr][DEF_ID_CFG];
        SCdtaCfg = iSCmessageConfig[chMsgNr][DEF_DTA_CFG];

        /* Get the Slow Channel ID */
        switch(SCidCfg & 0xC0)
        {
    #if DEF_TOTAL_NR_SC_MSG_ID_LOC_ROM > 0
        case DEF_SCID_LOC_ROM:
            /* ID in ROM with Offset iTemp */
            iSlowChBuff[1] = chSCromID[SCidCfg&0x3F];
            break;
    #endif
    #if DEF_TOTAL_NR_SC_MSG_ID_LOC_EE > 0
        case DEF_SCID_LOC_EE:
            /* ID in EEPROM with Offset iTemp */
            iSlowChBuff[1] = EEP_SC_ID[SCidCfg&0x3F];
            break;
    #endif
    #if DEF_TOTAL_NR_SC_MSG_ID_LOC_IN_DTA > 0
        case DEF_SCID_LOC_IN_DTA:
            /* ID is in DTA, get it from there later */
            /* Only supported for EE DTA */
            break;
    #endif
    #if DEF_TOTAL_NR_SC_MSG_ID_LOC_EE_CMPR > 0
        case DEF_SCID_LOC_EE_CMPR:
            /* ID in EEPROM with Offset iTemp and must be increased with (iTemp&0x0F) */
            iSlowChBuff[1] = EEP_SC_ID_CMPR[(SCidCfg>>4)&0x03] + (SCidCfg & 0x0F);
            break;
    #endif
        default:
            /* Should never be reached because this means something is wrong with ROM */
            break;
        }

        /* Get the Slow Channel DATA */
        switch(SCdtaCfg&0xC0)
        {
    #if DEF_TOTAL_NR_SC_MSG_DTA_LOC_EE>0
        case DEF_SCDTA_LOC_EE:
            /* Data in EEPROM with Offset iTemp */
            SCDTA.iValue = EEP_SC_DATA[SCdtaCfg&0x3F];
            #if DEF_TOTAL_NR_SC_MSG_ID_LOC_IN_DTA > 0
                if((SCidCfg&0xC0) == DEF_SCID_LOC_IN_DTA)
                {
                    /* Get ID from Data */
                    iSlowChBuff[1] = (uint8)(SCDTA.iValue >> 8) & 0xF0;
                    if((SCidCfg & 0x01U) != 0)
                    {
                        iSlowChBuff[1]  |= (uint8)(EEP_SC_DATA[(SCdtaCfg&0x3F)-1] >> 12) & 0x0F;
                        iSlowChBuff[1]++;
                    }
                    else
                    {
                        iSlowChBuff[1] >>= 4;
                        iSlowChBuff[1]  |= (uint8)(EEP_SC_DATA[(SCdtaCfg&0x3F)+1] >> 8) & 0xF0;
                    }
                    /* Remove ID from Data to allow standard parsing */
                    SCDTA.iValue &= 0x0FFF;
                }
            #endif
            break;
    #endif
    #if DEF_TOTAL_NR_SC_MSG_DTA_LOC_EE_4B>0
        case DEF_SCDTA_LOC_EE_4B:
            /* Data in EEPROM with Offset iTemp - Small Type (1-Nibble) */
            SCDTA.iValue   = (uint16)EEP_SC_DATA_4B[(SCdtaCfg&0x3F)/2];
            SCDTA.iValue >>= (SCdtaCfg&0x01)*4;
            SCDTA.iValue  &= 0x000F;
            break;
    #endif
    #if DEF_TOTAL_NR_SC_MSG_DTA_LOC_ROM>0
        case DEF_SCDTA_LOC_ROM:
            /* Data in ROM with Offset iTemp */
            SCDTA.iValue   = iSCromDTA[SCdtaCfg&0x3F];
            break;
    #endif
    #if DEF_TOTAL_NR_SC_MSG_DTA_LOC_EE_CMPR > 0
        case DEF_SCDTA_LOC_EE_CMPR:
            /* Data in EEPROM with Offset iTemp */
            chTemp = (SCdtaCfg&0x3F)+((SCdtaCfg&0x3F)/2);
            if((SCdtaCfg&0x01) != 0)
            {
                SCDTA.iValue  = ((uint16)EEP_SC_DATA_CMPR[chTemp])>>4;
                SCDTA.iValue &= 0x000F;
                SCDTA.iValue |= ((uint16)EEP_SC_DATA_CMPR[chTemp+1])<<4;
            }
            else
            {
                SCDTA.iValue  = (uint16)EEP_SC_DATA_CMPR[chTemp];
                SCDTA.iValue |= ((uint16)EEP_SC_DATA_CMPR[chTemp+1])<<8;
            }
            SCDTA.iValue &= 0x0FFF;
            break;
    #endif
        default:
            /* Should never be reached because this means something is wrong with ROM */
            break;
        }
    }
    else
    {
        /* ERROR: No valid message found */
        iSlowChBuff[1] = 0x01U;           /* Diagnostic Code */
        SCDTA.iValue = DEF_SCDTA_8BIT_ID | DEF_SCDTA_DATA | (DEF_ERR_DIAG_SC_NO_MESSAGE & 0x0FFF);
    }

    /* Handle Data - Check Data Type */
    if(SCDTA.parse.DtaType == DEF_DTA_PTR)
    {
        /* DATA holds a Pointer */
        iSlowChBuff[0] = *(uint16*)(SCDTA.parse.Data<<1U);
        if(SCDTA.parse.Shift != DEF_DTA_NO_SHIFT)
        {
            iSlowChBuff[0] >>= 4;
        }
    }
    else
    {
        /* DATA holds the SENT data */
        iSlowChBuff[0] = (uint16)SCDTA.parse.Data;
    }

    /* Put Slow Channel Message Data and ID in the Correct Buffers for Transmission */
    if(SCDTA.parse.IDtype == DEF_4BIT_SC)
    {
        if(SCDTA.parse.DtaType == DEF_DTA_DATA)
        {
            iSlowChBuff[0] |= (((uint16)iSlowChBuff[1]<<12)&0xF000);
        }
        iSlowChBuff[1] &= 0xF0;
        iSlowChBuff[1] |= (1U << 8);
    }
    else
    {
        iSlowChBuff[1] |= (0U << 8);
    }
}

/******************************************************
 * Get the Repetition Interval for a Specific Message
 * Arguments:
 * - chRepMsgNr: Offset in the Repetition Interval Array
 * Return:
 * - Repetition Interval -3(disabled), 2, 4, 6, 8, .. ,30
 ******************************************************/
int8 SENT_API_GetRepInterval(uint8 chRepMsgNr)
{
#if DEF_TOTAL_NR_SC_REP_MESSAGES > 0
    uint16 iInt;
    uint8 SCenCfg;
    uint16 iTemp;

    if(chRepMsgNr<DEF_TOTAL_NR_SC_REP_MESSAGES)
    {
        /* Get the correct enable bit configuration byte */
        SCenCfg = iSCmessageConfig[chSCrepMsgArrayNr[chRepMsgNr]][DEF_EN_CFG];
        #ifdef DEF_COMPR_REP_FACT
            /* Get the corresponding NII word out of EEPROM */
            iInt  = EEP_SC_REP_NII[(SCenCfg>>4)&0x03];
            /* Get the shift value from the configuration word */
            iTemp = SCenCfg & 0x0FU;
            /* Extract the NII value for the specific message */
            iTemp = (iInt>>iTemp)&0x07U;

            /* Check if Repetitive Message is Enabled, otherwise return -1 */
            if(iTemp != 0)
            {
                iTemp = (uint16)chNIIlookup[iTemp];             /* NII 0, 1, 2, 3, 4, 8, 16, 32 */

                /* Multiply by NRI */
                iTemp *= ((EEP_SC_REP_NRI&0x1F)+1);     /* NRI 1..32 */

                /* Limit the Repetition Factor to fit in int8 */
                if(iTemp>0x7FU)
                {
                    iTemp = 0x7FU;
                }
                return (int8)iTemp;
            }
        #else
            /* Get the correct Repetition Interval From EEPROM */
            iInt  = EEP_SC_REP_INT[chRepMsgNr/2];      /* req: MLX14612SW-48 - SENT: Sequence and repetition of serial data messages */
            iTemp = (chRepMsgNr&0x01U)*4U;
            iTemp = (iInt>>iTemp)&0x0FU;

            /* Check if Repetitive Message is Enabled, otherwise return -1 */
            if(iTemp != 0)
            {
                iTemp *= 2;               /* Multiply Repetition Interval by 2 (Range = 2-30)*/
                return (int8)iTemp;
            }
        #endif
    }
#endif

    return DEF_REPMSG_CTR_DISABLED;
}

/* EOF */
