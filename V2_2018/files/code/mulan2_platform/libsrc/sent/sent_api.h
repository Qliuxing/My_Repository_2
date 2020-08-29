/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Generic SENT API Routines Header
 *
 */

#ifndef SENT_API_H_
    #define SENT_API_H_

    #include "sent_lowlvl.h"


    /* --------------------------- */
    /* Public Defines              */
    /* --------------------------- */
    #define DEF_CHANNEL0                (1U<<0)
    #define DEF_CHANNEL1                (1U<<1)

    #define DEF_SC_PRIO_PENDING         2
    #define DEF_SC_PRIO_INIT            1
    #define DEF_SC_PRIO_DELAYED         3


    /* --------------------------- */
    /* Public Variables            */
    /* --------------------------- */
    extern volatile uint8  chFCoverwriteEN;
    extern volatile uint16 iFC0overwrite;
    extern volatile uint16 iFC1overwrite;


    /* --------------------------- */
    /* Public Function Definitions */
    /* --------------------------- */
    void SENT_API_Config(uint8 chFCcfg);
    #ifdef DEF_SENT_ERROR_POINTER
        uint8 SENT_API_CheckConfig(uint8 chFCcfg, uint16 *ptrError);
    #else
        uint8 SENT_API_CheckConfig(uint8 chFCcfg);
    #endif

    /* Fast Channel Pointers */
    extern uint16* ptrAppFCh0;
    extern uint16* ptrAppFCh1;

    static __inline__ __attribute__ ((__always_inline__))
    void SENT_API_Set_CH0ptr(uint16* ptrFC0)
    {
        ptrAppFCh0 = ptrFC0;
    }
    static __inline__ __attribute__ ((__always_inline__))
    void SENT_API_Set_CH1ptr(uint16* ptrFC1)
    {
        ptrAppFCh1 = ptrFC1;
    }
    static __inline__ __attribute__ ((__always_inline__))
    uint16* SENT_API_Get_CH0ptr(void)
    {
        return ptrAppFCh0;
    }
    static __inline__ __attribute__ ((__always_inline__))
    uint16* SENT_API_Get_CH1ptr(void)
    {
        return ptrAppFCh1;
    }

    /* SENT Low Level Configuration */
    static __inline__ __attribute__ ((__always_inline__))
    void SENT_API_Config_HW(uint16 iCfg1, uint16 iCfg2, uint8 iCfg3, uint16 iCfgSpc)
    {
        SENT_LL_Config();
        SENT_LL_Cfg_Output(iCfg1, iCfg2, iCfg3, iCfgSpc);
    }
    #ifdef DEF_SENT_ERROR_POINTER
        static __inline__ __attribute__ ((__always_inline__))
        uint8 SENT_API_CheckConfig_HW(uint16 iCfg1, uint16 iCfg2, uint8 iCfg3, uint16 iCfgSpc, uint16* ptrError)
        {
            uint8 chReturn;
            chReturn  = SENT_LL_CheckConfig(ptrError);
            chReturn |= SENT_LL_CheckCfg_Output(iCfg1, iCfg2, iCfg3, iCfgSpc, ptrError);
            return chReturn;
        }
    #else
        static __inline__ __attribute__ ((__always_inline__))
        uint8 SENT_API_CheckConfig_HW(uint16 iCfg1, uint16 iCfg2, uint8 iCfg3, uint16 iCfgSpc)
        {
            uint8 chReturn;
            chReturn  = SENT_LL_CheckConfig();
            chReturn |= SENT_LL_CheckCfg_Output(iCfg1, iCfg2, iCfg3, iCfgSpc);
            return chReturn;
        }
    #endif

    /* Prepare Data Functions */
    void SENT_API_FCprepareData(void);
    uint8 SENT_API_GetSlowCHmsgNr(void);
    void SENT_API_GetSlowChDTAfromMem(uint8 chMsgNr);
    static __inline__ __attribute__ ((__always_inline__))
    void SENT_API_SCprepareData(void)
    {
        /* Prepare ID and Data for Next Slow Channel Message */
        SENT_API_GetSlowChDTAfromMem(SENT_API_GetSlowCHmsgNr());
    }

#endif /* SENT_API_H_ */
