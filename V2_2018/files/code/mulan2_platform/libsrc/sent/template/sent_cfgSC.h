/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Generic SENT API Config
 *
 */

#ifndef _SENT_CFG_SC_H_
    #define _SENT_CFG_SC_H_

    #include <sent_def.h>
    #include "globals.h"

    /* ----------------------------------------- */
    /* Use Compressed Repetition Factor Config   */
    /* ----------------------------------------- */
    /*#define DEF_COMPR_REP_FACT                      1*/


    /* ----------------------------------------- */
    /* Do Checking for Delayed Slow Channel MSG  */
    /* ----------------------------------------- */
    #define DEF_CHECK_DELAYED_SC_MSG                1


    /* ----------------------------------------- */
    /* Total Number Slow Channel MSG             */
    /* ----------------------------------------- */
    #define DEF_TOTAL_NR_EN_BITS                    16U

    #define DEF_TOTAL_NR_SC_MESSAGE                 24U
    #define DEF_TOTAL_NR_SC_REP_MESSAGES            3U

    #define DEF_TOTAL_NR_SC_MSG_DTA_LOC_EE          17U
    #define DEF_TOTAL_NR_SC_MSG_DTA_LOC_EE_4B       1U
    #define DEF_TOTAL_NR_SC_MSG_DTA_LOC_ROM         6U

    #define DEF_TOTAL_NR_SC_MSG_ID_LOC_EE           0U
    #define DEF_TOTAL_NR_SC_MSG_ID_LOC_ROM          8U
    #define DEF_TOTAL_NR_SC_MSG_ID_LOC_IN_DTA       16U


    /* -------------------------------- */
    /* Slow Channel MSG Configuration   */
    /* -------------------------------- */
    const uint8 iSCmessageConfig[DEF_TOTAL_NR_SC_MESSAGE][3] =       /* req: MLX14612SW-45 - SENT: Slow channel messages */
    {
        {DEF_SCDTA_LOC_ROM   | 0  , DEF_SCID_LOC_ROM    | 0  , 0 },  /* 01 - Diag Code */ /* REP */
        {DEF_SCDTA_LOC_EE_4B | 1  , DEF_SCID_LOC_ROM    | 1  , 1 },  /* 03 - Sensor Type */
        {DEF_SCDTA_LOC_EE    | 0  , DEF_SCID_LOC_ROM    | 2  , 2 },  /* 04 - Manufact Type */
        {DEF_SCDTA_LOC_ROM   | 1  , DEF_SCID_LOC_ROM    | 3  , 3 },  /* 05 - Manufact Code */
        {DEF_SCDTA_LOC_ROM   | 2  , DEF_SCID_LOC_ROM    | 4  , 4 },  /* 06 - SENT Rev */
        {DEF_SCDTA_LOC_EE    | 1  , DEF_SCID_LOC_IN_DTA | 0  , 7 },  /* Prog00 - 07 - FC1 X1 */
        {DEF_SCDTA_LOC_EE    | 2  , DEF_SCID_LOC_IN_DTA | 1  , 8 },  /* Prog01 - 08 - FC1 X2 */
        {DEF_SCDTA_LOC_EE    | 3  , DEF_SCID_LOC_IN_DTA | 2  , 9 },  /* Prog02 - 09 - FC1 Y1 */
        {DEF_SCDTA_LOC_EE    | 4  , DEF_SCID_LOC_IN_DTA | 3  , 10},  /* Prog03 - 0A - FC1 Y2 */
        {DEF_SCDTA_LOC_EE    | 5  , DEF_SCID_LOC_IN_DTA | 4  , 11},  /* Prog04 - 0B - FC2 X1 */
        {DEF_SCDTA_LOC_EE    | 6  , DEF_SCID_LOC_IN_DTA | 5  , 11},  /* Prog05 - 0C - FC2 X2 */
        {DEF_SCDTA_LOC_EE    | 7  , DEF_SCID_LOC_IN_DTA | 6  , 12},  /* Prog06 - 0D - FC2 Y1 */
        {DEF_SCDTA_LOC_EE    | 8  , DEF_SCID_LOC_IN_DTA | 7  , 12},  /* Prog07 - 0E - FC2 Y2 */
        {DEF_SCDTA_LOC_ROM   | 3  , DEF_SCID_LOC_ROM    | 5  , 5 },  /* 10 - NTC */        /* REP */
        {DEF_SCDTA_LOC_ROM   | 4  , DEF_SCID_LOC_ROM    | 6  , 6 },  /* 23 - Tpress */     /* REP */
        {DEF_SCDTA_LOC_EE    | 9  , DEF_SCID_LOC_IN_DTA | 8  , 13},  /* Prog08 - 90 - OEM Code #1 */
        {DEF_SCDTA_LOC_EE    | 10 , DEF_SCID_LOC_IN_DTA | 9  , 13},  /* Prog09 - 91 - OEM Code #2 */
        {DEF_SCDTA_LOC_EE    | 11 , DEF_SCID_LOC_IN_DTA | 10 , 13},  /* Prog10 - 92 - OEM Code #3 */
        {DEF_SCDTA_LOC_EE    | 12 , DEF_SCID_LOC_IN_DTA | 11 , 13},  /* Prog11 - 93 - OEM Code #4 */
        {DEF_SCDTA_LOC_EE    | 13 , DEF_SCID_LOC_IN_DTA | 12 , 14},  /* Prog12 - 94 - OEM Code #5 */
        {DEF_SCDTA_LOC_EE    | 14 , DEF_SCID_LOC_IN_DTA | 13 , 14},  /* Prog13 - 95 - OEM Code #6 */
        {DEF_SCDTA_LOC_EE    | 15 , DEF_SCID_LOC_IN_DTA | 14 , 14},  /* Prog14 - 96 - OEM Code #7 */
        {DEF_SCDTA_LOC_EE    | 16 , DEF_SCID_LOC_IN_DTA | 15 , 14},  /* Prog15 - 97 - OEM Code #8 */
        {DEF_SCDTA_LOC_ROM   | 5  , DEF_SCID_LOC_ROM    | 7  , 15}  /* E1 - BIST Sequence Code */
    };


    /* -------------------------------- */
    /* Slow Channel ROM IDs             */
    /* -------------------------------- */
    const uint8 chSCromID[DEF_TOTAL_NR_SC_MSG_ID_LOC_ROM] =
    {
        0x01, 0x03, 0x04, 0x05, 0x06, 0x10, 0x23, 0xE1
    };


    /* -------------------------------- */
    /* Slow Channel ROM DATA            */
    /* -------------------------------- */
    const uint16 iSCromDTA[DEF_TOTAL_NR_SC_MSG_DTA_LOC_ROM] =
    {
        DEF_SCDTA_8BIT_ID | DEF_SCDTA_PTR  | 0x0003,    /* 01 - Diag Code */
        DEF_SCDTA_8BIT_ID | DEF_SCDTA_DATA | 0x0003,    /* 05 - Manufact Code */
        DEF_SCDTA_8BIT_ID | DEF_SCDTA_DATA | 0x0003,    /* 06 - SENT Rev */
        DEF_SCDTA_8BIT_ID | DEF_SCDTA_PTR  | 0x0004,    /* 10 - T NTC */
        DEF_SCDTA_8BIT_ID | DEF_SCDTA_PTR  | 0x0005,    /* 23 - T Press */
        DEF_SCDTA_8BIT_ID | DEF_SCDTA_PTR  | 0x0002     /* E1 - Extended BIST Sequence Code */
    };


    /* -------------------------------- */
    /* Repetitive Message Array Numbers */
    /* -------------------------------- */
    const uint8 chSCrepMsgArrayNr[DEF_TOTAL_NR_SC_REP_MESSAGES] =
    {
        0U,
        13U,
        14U
    };


    /* -------------------------------- */
    /* Slow Channel EEPROM locations    */
    /* -------------------------------- */
    extern volatile uint16 EEP_SC_EN_BITS[1]                                __attribute__((ep, addr(0x1042U)));     /* 16bits   0x1042U -> 0x1043U */
    #if DEF_TOTAL_NR_SC_REP_MESSAGES > 0
        extern volatile uint8  EEP_SC_REP_INT[2]                            __attribute__((ep, addr(0x1044U)));     /* 3nibbles 0x1044U -> 0x1045U */
    #endif
    #if DEF_TOTAL_NR_SC_MSG_DTA_LOC_EE_4B > 0
        extern volatile uint8  EEP_SC_DATA_4B[1]                            __attribute__((ep, addr(0x1045U)));     /* 1nibbles 0x1045U -> 0x1045U */
    #endif
    #if DEF_TOTAL_NR_SC_MSG_DTA_LOC_EE > 0
        extern volatile uint16 EEP_SC_DATA[DEF_TOTAL_NR_SC_MSG_DTA_LOC_EE]  __attribute__((ep, addr(0x1046U)));     /* 17-MSG   0x1046U -> 0x1067U */
    #endif
    #if DEF_TOTAL_NR_SC_MSG_ID_LOC_EE > 0
        extern volatile uint8  EEP_SC_ID[DEF_TOTAL_NR_SC_MSG_ID_LOC_EE]     __attribute__((ep, addr(0x1028U)));     /* 4-IDs    0x1038U -> 0x103BU */
    #endif


    /* -------------------------------- */
    /* SENT SC Error Diagnostic Codes   */
    /* -------------------------------- */
    #define DEF_ERR_DIAG_SC_NO_MESSAGE              0x0F02U

#endif /* _SENT_CFG_SC_H_ */
