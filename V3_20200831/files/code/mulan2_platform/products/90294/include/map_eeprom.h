/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * MLX90294_dev
 *
 */

#ifndef MAP_EEPROM_H_
#define MAP_EEPROM_H_

#include <typelib.h>


/* Pack struct's content without alignment */
#define __PACKED_ATTR   __attribute__ ((packed))


/* ATN: File content was copied from 90294/product/system/EEPROM. */
/*      Handle correction after auto-generation from Doc-file  */

typedef  union __PACKED_ATTR {
        uint8 u;
        struct __PACKED_ATTR {
            /*little end first */
            uint8 OCL_DirPol: 1;
            uint8 OCL_SpeedPol: 1;
            uint8 DirPol_Analog: 1;
            uint8 SpeedPol_Analog: 1;
            uint8 OCL_ReversePol: 1;
            uint8 OShap_OutPol: 1;
            uint8        : 2;
        } s;
} EE_POLARITY_t ;

typedef  union __PACKED_ATTR {
     uint8 u;
     struct __PACKED_ATTR {
         /*little end first */
         uint8 CAMCRANKB: 1;
         uint8 EnableStandStill : 1;
         uint8 DisableCompReassign : 1;
         uint8 HystRatioMin : 2;
         /*uint8 EnableSanityChecker : 1;*/ /* Was removed */
         uint8 : 1;
         uint8 : 2;
     } s;
} EE_CONTROL_t ;

typedef  union __PACKED_ATTR {
     uint8 u;
     struct __PACKED_ATTR {
         /*little end first */
         uint8 TPO_ONLY: 1;
         uint8 StandStill : 1;
         uint8 : 5;
         uint8 FixDelta : 1;
     } s;
} EE_CAM_CONTROL_t ;

typedef  union __PACKED_ATTR {
      uint16 u;
      struct __PACKED_ATTR {
          /*little end first */
          uint16 WITHDIRECTION: 1;
          uint16 AgcOverloadMargin: 2;
          uint16 DeltaAmpliRatio: 1;
          uint16 FilterN: 3;
          uint16 : 1;
          uint16 FilterAmpli: 4;
          uint16 ToothDurLockDir: 4;
      } s;
} EE_CRANK_CONTROL_t ;

typedef  union __PACKED_ATTR {
     uint16 u;
     struct __PACKED_ATTR {
         /* little end first */
         uint8 StUp_MATCH       : 1;
         uint8 StUp_WDBIST      : 1;
         uint8 StUp_ROMCHECKSUM : 1;
         uint8 StUp_EECRC       : 1;
         uint8 StUp_VanaMoni    : 1;
         uint8 StUp_VddMoni     : 1;
         uint8 StUp_Temperature : 1;
         uint8 StUp_ADCTEST     : 1;

         uint8 RnTm_ROMCHECKSUM : 1;
         uint8 RnTm_EECRC       : 1;
         uint8 RnTm_VanaMoni    : 1;
         uint8 RnTm_VddMoni     : 1;
         uint8 RnTm_Temperature : 1;
         uint8 RnTm_ADCTEST     : 1;
         uint8 RnTm_ADCClip     : 1;
         uint8 RnTm_AmpliTooLow : 1;
     } s;
} EE_DIAG_SET_t ;

typedef struct __PACKED_ATTR {

    union {
        uint16 u;
        struct __PACKED_ATTR {
            /* little end first */
            uint16 OSCTRIM: 7;
            uint16 VANATRIM: 4;
            uint16 ITRIM: 5;
        } s;
    } ANAOUTA ; /* 00h .. 01h */

    union __PACKED_ATTR {
        uint16 u;
        struct __PACKED_ATTR {
            /* little end first */
            uint16 OUTCURRENT : 3;
            uint16 LNA_GAIN_SPEED : 3;
            uint16 LNA_GAIN_DIR : 3;
            uint16 : 7;
        } s;
    } ANAOUTB ; /* 02h .. 03h */

    union __PACKED_ATTR {
        uint16 u;
        struct __PACKED_ATTR {
            /*little end first */
            uint16 TEST_EXTRACURRENT: 2;
            uint16 ANA_OUTC_FREE: 3;
            uint16 : 6;
            uint16 VDIGMODE: 2;
            uint16 BG2TRIM: 3;
        } s;
    } ANAOUTC ; /* 04h .. 05h */

    union __PACKED_ATTR {
        uint16 u;
        struct __PACKED_ATTR {
            /* little end first */
            uint16 TIMA_TIMBClkMuxSel: 1;
            uint16 ADCLOWPOWER: 1;
            uint16 HE_LEFT: 1;
            uint16 HE_SINGLE_config: 1;
            uint16 TriAxis3HEB: 1;
            uint16 PLATEALLOFF    : 1;
            uint16 PLATETEST      : 1; /* PLATE_SINGLE */
            uint16 PLATE_RIGHTPOL : 1;
            uint16  : 6;
            uint16 HE_SINGLE_ctrl: 1;
        } s;
    } ANALOGCFG ; /* 06h .. 07 */

    union __PACKED_ATTR {
        uint8 u;
        struct __PACKED_ATTR {
            /* little end first */
            uint8 Dig_TO_Sel_Test: 5;
            uint8 TestMuxSel_Test: 3;
        } s;
    } TEST_2_L ; /* 08h */

    union __PACKED_ATTR {
        uint8 u;
        struct __PACKED_ATTR {
            /* little end first */
            uint8 TestMuxSel_Test_3   : 1;
            uint8 : 4;
            uint8 OutDisable_portmap  : 1;
            uint8 TestEnable_portmap  : 1;
            uint8 VdigOnTestB_portmap : 1;
        } s;
    } TEST_2_H ; /* 09h */

    union __PACKED_ATTR {
        uint8 u;
        struct __PACKED_ATTR {
            /* little end first */
            uint8 Dig_TO_Sel_Out: 5;
            uint8 TestMuxSel_Out: 3;
        } s;
    } TEST_3_L ; /* 0Ah */

    uint8 Reserved_0Bh; /* 0Bh */

    union __PACKED_ATTR {
        uint16 u;
        struct __PACKED_ATTR {
            /* little end first */
            uint8 TIMA: 7;
            uint8     : 1;
            uint8 TIMB: 7;
            uint8     : 1;
        } s;
    } ADCTIMEOUT ; /* 0Ch .. 0Dh */

    union __PACKED_ATTR {
        uint8 u;
        struct __PACKED_ATTR {
            /*little end first */
            uint8 Sys_freq_CKTRIM: 6;
            uint8 Slow_freq_6MHz_3MHzB: 1;
            uint8 Enable_Slow_clock  : 1;
        } s;
    } FREQ_CONTROL ; /* 0Eh */

    EE_POLARITY_t POLARITY ; /* 0Fh */

    uint8  S_HE [6];  /* 10h .. 15h */

    uint8  Bmin;         /* 16h */
    uint8  BminDebounce; /* 17h */

    uint8  S_MAG [6];  /* 18h .. 1Dh */

    uint16 TS_Slope ; /* 1Eh .. 1Fh */

    uint16 TS_Offset; /* 20h .. 21h */

    uint8  S_T35_SPEED; /* 22h */

    uint8  HYSTRATIO; /* 23h */

    EE_CONTROL_t CONTROL ; /* 24h */

    uint8   DYN2STD;      /* 25h */

    union __PACKED_ATTR {

        /* Only for Cam */
        struct __PACKED_ATTR {

             uint16   R; /* 26h .. 27h */

             uint16   F; /* 28h .. 29h */

             uint8    H; /* 2Ah */

             uint8    NteethLearn; /* 2Bh */
             uint8    NteethWD;    /* 2Ch */
             uint8    NteethDynamic; /* 2Dh */

             EE_CAM_CONTROL_t CONTROL ; /* 2Eh */

             uint8    TPOTimeOut;  /* 2Fh */

             uint8     Eps [10]; /* 30h .. 39h */

             uint16   BzOffset0; /* 3Ah .. 3Bh */

             union __PACKED_ATTR {
                 uint16 u;
                 struct __PACKED_ATTR {
                     /* little end first */
                     uint16 Cold: 8;
                     uint16 Hot : 8;
                 } s;
             } OffsetSlope ; /* 3Ch .. 3Dh */

             uint16    Ampli135mT; /* 3Eh .. 3F */

        } cm ; /* cm = cam */

        /* Only for Crank */

        struct __PACKED_ATTR {

            union __PACKED_ATTR {
                uint16 u;
                struct __PACKED_ATTR {
                    /*little end first */
                    uint16 Forward : 7;
                    uint16         : 1;
                    uint16 Reverse : 7;
                    uint16         : 1;
                } s;
            } PulseWidth ; /* 26h .. 27h */

            uint8   S_HE_DIR [6]; /* 28h .. 2Dh */

            uint8   MinToothCountPer100ms; /* 2Eh */

            uint8   DIRSKEW; /* 2Fh */

            uint8   T_35_DIR; /* 30h */

            uint8   NteethLearn; /* 31h */

            EE_CRANK_CONTROL_t CONTROL ; /* 32h .. 33h */

            uint8   T_LEARNTIMEOUT; /* 34h */

            uint8   reserved_35h_to_3Fh[11]; /* 35 .. 3Fh */

        } cr ; /* cr = crank */

    } sh; /* sh = shared */

    uint8    PatchCode [26]; /* 40h .. 59h */

    uint8    USERID [6]; /* 5Ah .. 5Fh */

    uint8    MLXID [6]; /* 60h .. 65h */

    uint16   PATCH0_A; /* 66h .. 67h */
    uint16   PATCH0_I; /* 68h .. 69h */
    uint16   PATCH1_A; /* 6Ah .. 6Bh */
    uint16   PATCH1_I; /* 6Ch .. 6Dh */
    uint16   PATCH2_A; /* 6Eh .. 6Fh */
    uint16   PATCH2_I; /* 70h .. 71h */
    uint16   PATCH3_A; /* 72h .. 73h */
    uint16   PATCH3_I; /* 74h .. 75h */

    union __PACKED_ATTR {
        uint8 u;
        struct __PACKED_ATTR {
            /*little end first */
            uint8 NOMARCH              : 1;
            uint8 NOWDBIST             : 1;
            uint8 DELAYED_STARTUP      : 1;
            uint8                      : 5;
        } s;
    } USERCFG ; /* 76h */

    uint8    MemLock; /* 77h */

    EE_DIAG_SET_t  DIAG_SET_DisRep ; /* 78h .. 79 h */

    uint16   ROMCHKSUM; /* 7Ah .. 7B */

    uint8    CRC8; /* 7Ch */
    uint8    Free; /* 7Dh */
    uint16   CRC_DISABLE; /* 7Eh */

} ee_t ;

extern ee_t ee __attribute((ep, aligned(2)));

#define T_DYN2STD_MIN_ms  100

#define M_EEPROM_CRC8     0x3131

#define MEM_UNLOCK        0x39

/* Default parameters, if ZERO value */
#define EE_DEFAULT_TIMA                 16
#define EE_DEFAULT_TIMB                 16
#define EE_Dyn2STD                      20 /* 2 sec */
#define EE_CAM_NTeethLearn               3
#define EE_CAM_NTeethWD                 10
#define EE_CAM_NTeethDynamic            30
#define EE_CAM_TPO_Timeout              20 /* 2 sec */
#define EE_CRANK_NTeethLearn             3
#define EE_CRANK_FORWARD                45 /* us */
#define EE_CRANK_REVERSE                90 /* us */
#define EE_CRANK_MIN_ToothCountPer100ms 18
#define EE_CRANK_T_LEARNTIMEOUT         20 /* 2 sec */


#endif /* MAP_EEPROM_H_ */
/* EOF */
