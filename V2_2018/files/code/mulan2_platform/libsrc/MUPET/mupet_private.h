/*
 * Copyright (C) 2013 Melexis N.V.
 *
 * MUPeT, Application Layer.
 *        Internal typedef-s.
 *
 */

#ifndef MUPET_PRIVATE_H_
#define MUPET_PRIVATE_H_

#include <typelib.h>
#include <syslib.h>
#include <mupet.h>

/* User types */
typedef enum { openSessionEnable = 0,     openSessionDisable }        openSessionAccess_t;
typedef enum { wrAccessEnable = 0,        wrAccessDisable }           wrAccess_t;
typedef enum { sessionIdIsAsserted = 0,   sessionIdIsDenied }         checkSessionId_t;
typedef enum { closeSessionWithReset = 0, closeSessionWithoutReset }  closeSessionReset_t;

/* Address pointer type, for MLX16 - 16bits address */
typedef uint16  addr_pnt_t;

/* Firmware MUPeT version */
#define FIRMWARE_MUPET_VERSION     ( (MUPET_SW_MAJOR_VERSION << 8) | MUPET_SW_MINOR_VERSION  )

/*
 * Address types
 */
typedef enum {
    ADDR_INVALID = 0,
    ADDR_RAM,
    ADDR_EEPROM,
    ADDR_IO,
    ADDR_ROM,
    ADDR_VirtMem
} mem_t;


/* Commands opcode (see p.4 ) */
typedef enum {

    cmdSetBase                  = 0xD,
    cmdGetBase                  = 0xE,

    cmdShortRead                = 0x4,
    cmdShortWrite               = 0x5,
    cmdRepeatRead               = 0xB,

    cmdBurstRead                = 0xC,
    cmdBurstWrite               = 0x7,

    cmdByteWrite                = 0x9,

    cmdCall                     = 0xA,

    cmdReverved_1h              = 0x1,               /* Reserved */
    cmdReverved_2h              = 0x2,
    cmdReverved_3h              = 0x3,
    cmdReverved_6h              = 0x6,
    cmdReverved_8h              = 0x8,
    cmdReverved_Fh              = 0xF,

    cmdNoCommand                = 0x0               /* Don't used */

} cmdOpcode_t;

#define SESSION_ID_UNKNOWN  0x00000000L

typedef enum { cCloseSession=0, cOpenSession=3, cResetApp=6 } sessionType_t;


/* command executable state */
typedef enum {
    FirstWord = 0,         /* 1-st word of message frame */

    SecondWord,            /* 2-nd word     ----//----   */

    WritingOr3rdWord,      /* Writing or 3-rd word       */


    /* All reading state must be latterly */
    /* i.e RaadingXXX >= ReadingWord      */
    ReadingWord,           /* Reading current word       */

    ReadingChSum,          /* Reading Check Sum          */

    ReadingLowAddr,        /* Reading Low addres         */

    ReadingHighAddr        /* Reading High address       */

} cmdState_t;



/**** Global variable ****/

extern addr_pnt_t   MUPeT_pointers[4]   __attribute__((dp));     /* Pointers table */


typedef struct
{
    union {

        struct {
        	/* Little bits first */
            uint16  params       : 12;    /* Parameters              */
            uint16  opcode       : 4;     /* Opcode, see cmdOpcode_t */
        } s;

        uint16 u;

    } cmd;

    struct {                                 /* Command's parameters : */
        union {
            addr_pnt_t  addr;                /*  address or            */
            uint16      sessionLowID;        /*    session Low ID      */
        }p1;

        union {
            uint8        counter;            /* read/write counter  or */
            uint8        base;               /*       base number      */
        }p2;
    } params;

    /* Internal flags */
    struct {
        uint8           bSessionCmd    : 1; /* Flag this is session cmd */
        uint8           cmdState       : 3; /* State,  see cmdState_t   */
        volatile uint8  bOpenedSession : 1; /* Flag of session opened   */
        volatile uint8  bNoAppliFreeze : 1; /* No Application Freeze    */
    } s;

} state_t ;

extern state_t MUPeT_state          __attribute__((dp));


/* Virtual memory status registers */
/*  - error code */
typedef union {
    struct {
        /* little end first */
        uint8  bWrWithMemlock : 1;     /* Write Attempt with MemLock byte set */
        uint8  bInvalidSeq : 1;        /* Invalid PTCR/PTCW sequence          */
        uint8  bInvalidCmd : 1;        /* Invalid Command                     */
        uint8  bInvalidSession : 1;    /* Invalid Session                     */
    } s;

    uint8 u;

} ErrRegister_t ;

/* Virtual memory status registers */
extern ErrRegister_t MUPeT_virtMemErrReg    __attribute__((dp));

/* Check sum */
extern uint16 MUPeT_checkSum        __attribute__((dp));

/* DECODE MESSAGE */


#define GETW_CMD(w)               ( (((uint16)(w)) >> 12 ) & 0x000Fu )
#define GETW_SESSION_TYPE(w)      ( (((uint16)(w)) >> 9  ) & 0x0007u )
#define GETW_NOAPPLIFREEZE(w)     ( (((uint16)(w)) >> 8  ) & 0x0001u )
#define GETW_BASE(w)              ( (((uint16)(w)) >> 10 ) & 0x0003u )
#define GETW_OFFSET(w)            ( (((uint16)(w)) >> 0  ) & 0x03FFu )
#define GETW_CALL_OFFSET(w)       ( (((uint16)(w)) >> 0  ) & 0x007Fu )
#define GETW_CALL_N(w)            ( (((uint16)(w)) >> 7  ) & 0x0007u )
#define GETW_ADDR_H(w)            ( (((uint16)(w)) >> 0  ) & 0x000Fu )
#define GETW_ADDR_L(w)            ( (((uint16)(w)) >> 0  )           )
#define GETW_N(w)                 ( (((uint16)(w)) >> 4  ) & 0x00FFu )
#define GETW_CALLID(w)            ( (((uint16)(w)) >> 0  ) & 0x0FFFu )
#define GETW_DATA(w)              ( (((uint16)(w)) >> 0  )           )

#define GETP_CMD()                GETW_CMD          (MUPeT_APP_getPTC_data())
#define GETP_SESSION_TYPE()       GETW_SESSION_TYPE (MUPeT_APP_getPTC_data())
#define GETP_NOAPPLIFREEZED()     GETW_NOAPPLIFREEZE(MUPeT_APP_getPTC_data())
#define GETP_BASE()               GETW_BASE         (MUPeT_APP_getPTC_data())
#define GETP_OFFSET()             GETW_OFFSET       (MUPeT_APP_getPTC_data())
#define GETP_CALL_OFFSET()        GETW_CALL_OFFSET  (MUPeT_APP_getPTC_data())
#define GETP_CALL_N()             GETW_CALL_N       (MUPeT_APP_getPTC_data())
#define GETP_ADDR_H()             GETW_ADDR_H       (MUPeT_APP_getPTC_data())
#define GETP_ADDR_L()             GETW_ADDR_L       (MUPeT_APP_getPTC_data())
#define GETP_N()                  GETW_N            (MUPeT_APP_getPTC_data())
#define GETP_CALLID()             GETW_CALLID       (MUPeT_APP_getPTC_data())
#define GETP_DATA()               GETW_DATA         (MUPeT_APP_getPTC_data())



#define READ_DATA(data)         (  MUPeT_APP_setPTC_data(data) )


/* Linker symbols */
extern uint16 _ram_start;
extern uint16 _ram_end;         /* last address + 1 */

extern uint16 _eeprom_start;
extern uint16 _eeprom_end;

extern uint16 _system_io_start;
extern uint16 _system_io_end;

extern uint16 _user_io_start;
extern uint16 _user_io_end;

extern uint16 _rom_start;
extern uint16 _rom_end;


#define RAM_START_ADDR          ((addr_pnt_t)&_ram_start)
#define RAM_END_ADDR            ((addr_pnt_t)&_ram_end)

#define EEP_START_ADDR          ((addr_pnt_t)&_eeprom_start)
#define EEP_END_ADDR            ((addr_pnt_t)&_eeprom_end)

#define SYSTEM_IO_START_ADDR    ((addr_pnt_t)&_system_io_start)
#define SYSTEM_IO_END_ADDR      ((addr_pnt_t)&_system_io_end)

#define USER_IO_START_ADDR      ((addr_pnt_t)&_user_io_start)
#define USER_IO_END_ADDR        ((addr_pnt_t)&_user_io_end)

#define ROM_START_ADDR          ((addr_pnt_t)&_rom_start)
#define ROM_END_ADDR            ((addr_pnt_t)&_rom_end)


/* Limits of virtual address */
#define VIRT_MEM_START_ADDR       (addr_pnt_t)0xFF00u
#define VIRT_MEM_END_ADDR         (VIRT_MEM_START_ADDR + (1u+1u+4u)*2u)     /* 6 words */

/* Default pointers */
#define PTR_IO           USER_IO_START_ADDR
#define PTR_ROM_FLASH    ROM_START_ADDR
#define PTR_RAM          RAM_START_ADDR
#define PTR_EEPROM_NVM   EEP_START_ADDR
#define PTR_VIRT_MEM     VIRT_MEM_START_ADDR


#endif /* MUPET_PRIVATE_H_ */

/* EOF */

