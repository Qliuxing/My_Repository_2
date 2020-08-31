/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Generic SENT Structures
 *
 */

#ifndef SENT_STRUCTS_H_
    #define SENT_STRUCTS_H_

    /* -------------------------------- */
    /* Compressed SENT LowLvl Config    */
    /* -------------------------------- */
    union UNION_SENT_CFG_COMPR
    {
        uint8 chValue;

        struct STR_SENT_CFG_COMPR
        {
            unsigned FCconfig:3;
            unsigned InvSent:1;
            unsigned TickDiv:3;
            unsigned Pause:1;
        }parse;
    };

    /* -------------------------------- */
    /* Extended SENT LowLvl Config      */
    /* -------------------------------- */
    struct STR_SENT_CONFIG1
    {
        unsigned FrameLen:12;
        unsigned EnCRC:1;
        unsigned StatInCRC:1;
        unsigned EnFailSafe:1;
        unsigned FailSafeHigh:1;
    };

    struct STR_SENT_CONFIG2
    {
        unsigned TickDiv:14;
        unsigned SerCfg:2;
    };

    struct STR_SENT_CONFIG3
    {
        unsigned PullLen:4;
        unsigned PullCfg:2;
        unsigned CRClegacy:1;
        unsigned IniNibbleEn:1;
    };

    /* -------------------------------- */
    /* Slow Channel Message Data Full   */
    /* -------------------------------- */
    union SC_MSG_DTA
    {
        uint16 iValue;

        struct STR_SC_MSG_DTA
        {
            unsigned Data:13;
            unsigned Shift:1;
            unsigned DtaType:1;
            unsigned IDtype:1;
        }parse;
    };

#endif /* SENT_GEN_STRUCTS_H_ */
