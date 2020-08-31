/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Generic SENT Low Level Routines Header
 *
 */

#ifndef SENT_LOW_LEVEL_H_
    #define SENT_LOW_LEVEL_H_

    #include <sent_cfgGEN.h>


    /* --------------------------- */
    /* Public Defines              */
    /* --------------------------- */


    /* --------------------------- */
    /* Public Variables            */
    /* --------------------------- */
    extern uint16 iSlowChBuff[2];
    extern uint16 iFastChBuff[2];
    extern volatile uint16 iSentDMA[5];
    extern uint8  chStatNibble;


    /* --------------------------- */
    /* Public Function Definitions */
    /* --------------------------- */
    void SENT_LL_Config(void);
    #ifdef DEF_SENT_ERROR_POINTER
        uint8 SENT_LL_CheckConfig(uint16* ptrError);
    #else
        uint8 SENT_LL_CheckConfig(void);
    #endif

    static __inline__ __attribute__ ((__always_inline__))
    void SENT_LL_Cfg_Output(uint16 iCfg1, uint16 iCfg2, uint8 iCfg3, uint16 iCfgSpc)
    {
        SENT_CFG1 = iCfg1 & 0x3FFFu;
        SENT_CFG2 = iCfg2;
        SENT_CFG3 = iCfg3;
        #ifdef DEF_CONFIG_SPC_REGISTERS
            SENT_SPC_CFG = iCfgSpc;
        #endif
    }
#ifdef DEF_SENT_ERROR_POINTER
    static __inline__ __attribute__ ((__always_inline__))
    uint8 SENT_LL_CheckCfg_Output(uint16 iCfg1, uint16 iCfg2, uint8 iCfg3, uint16 iCfgSpc, uint16* ptrError)
#else
    static __inline__ __attribute__ ((__always_inline__))
    uint8 SENT_LL_CheckCfg_Output(uint16 iCfg1, uint16 iCfg2, uint8 iCfg3, uint16 iCfgSpc)
#endif
    {
        uint8 chReturn;

        chReturn = 0U;
        if((SENT_CFG1 & 0x3FFFu) != (iCfg1 & 0x3FFFu))
        {
            #ifdef DEF_SENT_ERROR_POINTER
                *ptrError = (uint16)(&(SENT_CFG1));
            #endif
            chReturn = 1;
        }
        if(SENT_CFG2 != iCfg2)
        {
            #ifdef DEF_SENT_ERROR_POINTER
                *ptrError = (uint16)(&(SENT_CFG2));
            #endif
            chReturn = 1;
        }
        if(SENT_CFG3 != iCfg3)
        {
            #ifdef DEF_SENT_ERROR_POINTER
                *ptrError = (uint16)(&(SENT_CFG3));
            #endif
            chReturn = 1;
        }
        #ifdef DEF_CONFIG_SPC_REGISTERS
            if(SENT_SPC_CFG != iCfgSpc)
            {
                #ifdef DEF_SENT_ERROR_POINTER
                    *ptrError = (uint16)(&(SENT_SPC_CFG));
                #endif
                chReturn = 1;
            }
        #endif
        return chReturn;
    }

    /* Fast Channel Mask */
    static __inline__ __attribute__ ((__always_inline__))
    void SENT_LL_Mask(uint8 chValue)
    {
        SENT_NIB_MASK = chValue;
    }
    #ifdef DEF_SENT_ERROR_POINTER
        static __inline__ __attribute__ ((__always_inline__))
        uint8 SENT_LL_Mask_Check(uint8 chValue, uint16* ptrError)
        {
            if(SENT_NIB_MASK != chValue)
            {
                *ptrError = (uint16)(&(SENT_NIB_MASK));
                return 1;
            }
            return 0;
        }
    #else
        static __inline__ __attribute__ ((__always_inline__))
        uint8 SENT_LL_Mask_Check(uint8 chValue)
        {
            if(SENT_NIB_MASK != chValue)
            {
                return 1;
            }
            return 0;
        }
    #endif

    static __inline__ __attribute__ ((__always_inline__))
    void SENT_LL_ChangeFCdata(void)
    {
        /* Update Status Nibble with chStat Bits */
        iSentDMA[0] = (uint16)chStatNibble;
        /* Update Fast Channel Data with iFastChBuff[] */
        iSentDMA[1] = iFastChBuff[0];
        iSentDMA[2] = iFastChBuff[1];
    }

    static __inline__ __attribute__ ((__always_inline__))
    void SENT_LL_ChangeSCdata(void)
    {
        iSentDMA[3] = iSlowChBuff[0];
        iSentDMA[4] = iSlowChBuff[1];
    }

    static __inline__ __attribute__ ((__always_inline__))
    void SENT_LL_EnableOutput(void)
    {
        /* Copy the Correct Fast Channel Data to DMA buffer */
        SENT_LL_ChangeFCdata();
        /* Copy the Previously Prepared ID and Data */
        SENT_LL_ChangeSCdata();
        /* Enable Output */
        SENT_CFG1 |= SENT_EN;
    }

    static __inline__ __attribute__ ((__always_inline__))
    void SENT_LL_DisableOutput(void)
    {
        SENT_CFG1 &= ~SENT_EN;
    }

    static __inline__ __attribute__ ((__always_inline__))
    void SENT_LL_DisableSlowChannel(void)
    {
        SENT_CFG2 &= ~(3U<<14U);
        SENT_CFG2 |= 1U<<14U;
    }

#endif /* SENT_LOW_LEVEL_H_ */
