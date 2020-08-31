/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * SENT Application routines header
 *
 */

#ifndef __SENT_CFG_1BYTE_H_
    #define __SENT_CFG_1BYTE_H_

    #include "sent_api.h"
    #include "sent_structs.h"

    /* --------------------------- */
    /* SW component version        */
    /* --------------------------- */
    #define SENT_CFG_SW_MAJOR_VERSION       0
    #define SENT_CFG_SW_MINOR_VERSION       1
    #define SENT_CFG_SW_PATCH_VERSION       1


    /* --------------------------- */
    /* Public Defines              */
    /* --------------------------- */
    #define DEF_CHANNEL0                (1U<<0)
    #define DEF_CHANNEL1                (1U<<1)


    /* --------------------------- */
    /* Public Variables            */
    /* --------------------------- */
    #if DEF_SENT_CFG_TYPE == 0
        extern union UNION_SENT_CFG_COMPR SENT_CMP_CFG;
    #endif

    /* --------------------------- */
    /* Public Function Definitions */
    /* --------------------------- */
    void SENT_CFG_Config(void);
    #ifdef DEF_SENT_ERROR_POINTER
        uint8 SENT_CFG_CheckConfig(uint16* ptrError);
    #else
        uint8 SENT_CFG_CheckConfig(void);
    #endif

    static __inline__ __attribute__ ((__always_inline__))
    uint8 SENT_CFG_GetFCdtaId(void)
    {
    #if DEF_SENT_CFG_TYPE == 0
        return (uint8)SENT_CMP_CFG.parse.FCconfig;
    #elif DEF_SENT_CFG_TYPE == 1
        return 0;
    #endif
    }

    void SENT_CFG_Enable(void);

    static __inline__ __attribute__ ((__always_inline__))
    void SENT_CFG_Disable(void)
    {
        SENT_LL_DisableOutput();
    }
#endif /* __SENT_APP_H_ */
