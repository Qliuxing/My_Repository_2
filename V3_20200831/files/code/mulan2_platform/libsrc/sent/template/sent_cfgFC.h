/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Generic SENT Config
 *
 */

#ifndef SENT_CFG_FC_H_
    #define SENT_CFG_FC_H_

    #include "sent_def.h"
    #include "sent_cfgGEN.h"
    #include "globals.h"


    /* -------------------------------- */
    /* EEPROM MAP                       */
    /* -------------------------------- */
    #if DEF_SENT_CFG_TYPE == 0
        extern volatile uint8 EEP_SENT_CONFIG_COMPR_BYTE                __attribute__((ep, addr(0x1040U)));
    #elif DEF_SENT_CFG_TYPE == 1
        extern volatile uint16 EEP_SENT_CONFIG1                         __attribute__((ep, addr(0x1000U)));
        extern volatile struct STR_SENT_CONFIG1 EEP_SENT_CONFIG1bits    __attribute__((ep, addr(0x1000U)));
        extern volatile uint16 EEP_SENT_CONFIG2                         __attribute__((ep, addr(0x1002U)));
        extern volatile struct STR_SENT_CONFIG2 EEP_SENT_CONFIG2bits    __attribute__((ep, addr(0x1002U)));
        extern volatile uint8  EEP_SENT_CONFIG3                         __attribute__((ep, addr(0x1004U)));
        extern volatile struct STR_SENT_CONFIG3 EEP_SENT_CONFIG3bits    __attribute__((ep, addr(0x1004U)));
        extern volatile uint8  EEP_SENT_FC_CFG                          __attribute__((ep, addr(0x1005U)));
        extern volatile union  SENT_FC_CFG      EEP_SENT_FC_CFGbits     __attribute__((ep, addr(0x1005U)));

        /* -------------------------------- */
        /* Fast Channel Configuration       */
        /* -------------------------------- */
        extern volatile uint16 EEP_FC_PTR_CH0                           __attribute__((ep, addr(0x1006U)));
        extern volatile uint16 EEP_FC_PTR_CH1                           __attribute__((ep, addr(0x1008U)));
    #endif

    #ifdef DEF_SENT_FC_CFG_FC0_FC1_EE_PTR
        extern volatile uint16 EEP_FC0_PTR          __attribute__((ep, addr(0x1068U)));
        extern volatile uint16 EEP_FC1_PTR          __attribute__((ep, addr(0x106AU)));
    #endif


    /* -------------------------------- */
    /* Fast Channel Configuration       */
    /* -------------------------------- */
    #if DEF_SENT_CFG_TYPE == 0
        const uint16 iSENTconfig1[2] =      /* req: MLX14612SW-43 - SENT: Pause pulse */
        {
            (0xF000 | SENT_FRAME_LEN(0)),   /* No Pause pulse is used */
            (0xF000 | SENT_FRAME_LEN(282))  /* Pause pulse is included so message length=282 */
        };
        const uint16 iSENTconfig2[8] =      /* req: MLX14612SW-44 - SENT: Slow channel format */
        {
            (0xC000 | SENT_TICK(1)),        /* 1 us */
            (0xC000 | SENT_TICK(2)),        /* 2 us */
            (0xC000 | SENT_TICK(3)),        /* 3 us */
            (0xC000 | SENT_TICK(4)),        /* 4 us */
            (0xC000 | SENT_TICK(6)),        /* 6 us */
            (0xC000 | SENT_TICK(10)),       /* 10 us */
            (0xC000 | SENT_TICK(12)),       /* 12 us */
            (0xC000 | SENT_TICK(16))        /* 16 us */
        };
        const uint8 iSENTconfig3[2] =       /* req: MLX14612SW-49 - SENT: "Inversion" of SENT output */
        {
            0x25,       /* Standard SENT output */
            0x35        /* Inverted SENT output */
        };
        const uint16 iSENTspcCfg = 0x0003;

        const uint8 iFCpredefFCCFG[8] =
        {
            ((0x01<<4) | (0x02<<6) | DEF_SENT_FC_CFG_CH1_REV_NIBBLES),  /* CH0=Pressure, CH1=TempNTC */         /* req: MLX14612SW-38 - SENT: fast data channel formats  */
            ((0x01<<4) | (0x01<<6) | DEF_SENT_FC_CFG_CH1_SECURE_CTR),   /* CH0=Pressure, CH1=SecureCTR */       /* req: MLX14612SW-38 - SENT: fast data channel formats  */
            ((0x03<<4) | (0x00<<6) | DEF_SENT_FC_CFG_CH0_ONLY_4_3BIT),  /* CH0=Pressure 4x3bit, CH1=Disabled */ /* req: MLX14612SW-38 - SENT: fast data channel formats  */
            ((0x01<<4) | (0x02<<6) | DEF_SENT_FC_CFG_CH1_REV_NIBBLES),  /* CH0=Pressure, CH1=TempPress */       /* req: MLX14612SW-38 - SENT: extra fast channel configurations */
                                                                                                                /* req: MLX14612SW-85 - SENT: extra fast channel configurations */
            ((0x01<<4) | (0x01<<6) | DEF_SENT_FC_CFG_CH1_INV),          /* CH0=Pressure, CH1=Inverted CH0 */    /* req: MLX14612SW-38 - SENT: extra fast channel configurations */
            ((0x03<<4) | (0x03<<6) | DEF_SENT_FC_CFG_CH1_REV_NIBBLES),  /* CH0=Vsup, CH1=Vdig */                /* req: MLX14612SW-38 - SENT: extra fast channel configurations */
                                                                                                                /* req: MLX14612SW-85 - SENT: extra fast channel configurations */
            ((0x03<<4) | (0x03<<6) | DEF_SENT_FC_CFG_CH1_REV_NIBBLES),  /* CH0=SP, CH1=SN */                    /* req: MLX14612SW-38 - SENT: extra fast channel configurations */
                                                                                                                /* req: MLX14612SW-85 - SENT: extra fast channel configurations */
            ((0x03<<4) | (0x03<<6) | DEF_SENT_FC_CFG_FC0_FC1_EE_PTR)    /* EE Pointers */                       /* req: MLX14612SW-39 - SENT: extra fast data channel format */
        };
        const uint16 iFCpredefPTRch0[8] =
        {
            0x000C,     /* CH0=Pressure, CH1=TempNTC */
            0x000C,     /* CH0=Pressure, CH1=SecureCTR */
            0x000C,     /* CH0=Pressure 4x3bit, CH1=Disabled */
            0x000C,     /* CH0=Pressure, CH1=TempPress */
            0x000C,     /* CH0=Pressure, CH1=Inverted CH0 */
            0x0012,     /* CH0=Vsup, CH1=Vdig */
            0x0010,     /* CH0=SP, CH1=SN */
            0x0000      /* EE Pointers */
        };
        const uint16 iFCpredefPTRch1[8] =
        {
            0x0008,     /* CH0=Pressure, CH1=TempNTC */
            0x0000,     /* CH0=Pressure, CH1=SecureCTR */
            0x0000,     /* CH0=Pressure 4x3bit, CH1=Disabled */
            0x000A,     /* CH0=Pressure, CH1=TempPress */
            0x000C,     /* CH0=Pressure, CH1=Inverted Pressure */
            0x0014,     /* CH0=Vsup, CH1=Vdig */
            0x000E,     /* CH0=SP, CH1=SN */
            0x0000      /* EE Pointers */
        };
    #elif DEF_SENT_CFG_TYPE == 1


    #endif

#endif /* _SENT_CFG_APP_H_ */
