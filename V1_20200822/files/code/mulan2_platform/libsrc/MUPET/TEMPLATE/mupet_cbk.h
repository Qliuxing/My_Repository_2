/*
 * Copyright (C) 2013 Melexis N.V.
 *
 * MUPeT, Application Layer
 *        Callback (cbk) application dependent functions (MUPeT_APP_*())
 *
 *
 */


/*
 * NOTE: Must be included only in ONE .c-file (mupet.c)
 *       (except validation project)
 */

#ifndef MUPET_CBK_H_
#define MUPET_CBK_H_

#include <stdbool.h>     /* bool */
#include "mupet_private.h"


/* clear (pending) interrupt */
static INLINE void clearMUPeTInt(void)   {PEND =  CLR_MUPeT_IT;}
/* Enable interrupt */
static INLINE void enableMUPeTInt(void)  {MASK |=  EN_MUPeT_IT;}
static INLINE void disableMUPeTInt(void) {MASK &= ~EN_MUPeT_IT;}
/* Set interrupt priority */
static INLINE void setPriorityMUPeTInt(prio_t prio) {
    Prio_v.MUPeT_it = prio;
}

/******************************************************************************/
/*                A P P L I C A T I O N    D E P E N D E N T                  */
/*                    ONLY FOR MLX16107                                       */
#ifdef UNIT_TEST
    uint16 g_MUPeT_Wr_Data;
    uint16 g_MUPeT_Rd_Data;

    uint16 g_bWrStsAck;
    uint16 g_bRdStsAck;
    uint16 g_bITStsAck;

    MUPET_SessionId_t  g_MUPeT_SessionId;

#else
    #include <ioports.h>
    #include <sys_modes.h>
    #include <map_eeprom.h>
    #include "diag.h"
#endif

/* Application FW version - 8 bytes  */
/* See in "mupet.c" : static const uint8 MUPeT_APP_FirmwareVersion[8] =  */
#define MUPET_APP_FIRWMARE_VERSIOM_INIT  { '1', '6', '1', '0', '7', 'A', 'A', 'A' }

/*------------------------------------------------------------------------------
 *  Application dependent function
 *  call on enter in MUPeT_Isr()
 */
static INLINE void  MUPeT_APP_onEnterInIsr (void)
{
    /* Nothing */
}

/*------------------------------------------------------------------------------
 *  Application dependent function
 *  call on exit from MUPeT_Isr()
 */
static INLINE void  MUPeT_APP_onExitFromIsr (void)
{
    /* Nothing */
}


/*------------------------------------------------------------------------------
 *  Application dependent function - check session ID
 *
 *    Result: sessionKeyValid or sessionKeyInvalid.
 */
static INLINE checkSessionId_t MUPeT_APP_checkSessionId (void)
{
	checkSessionId_t  result = sessionIdIsDenied;

    if (  MUPET_GetSessionID() == g_MUPeT_SessionId ) {
        result = sessionIdIsAsserted;
    }

    return result;
}


/*------------------------------------------------------------------------------
 *  Application dependent function
 *  EEPROM write function
 *
 *  Return: wrAccessEnable or wrAccessDisable.
 */
static INLINE void  MUPeT_APP_EEPROM_Write ( uint16* address, uint16 data)
{
#ifndef UNIT_TEST
    /* Check access to EEPROM */
    while ((EEPROM & EE_BUSY) == EE_BUSY) {
      ; /* polling */
    }

    /* write enable EEPROM */
    CONTROL |= EE_WE;

    /* Safety checker.
     * EEPROM writing possible only in PTC mode.
     * This is a check from fail "call/jmp" program-counter
     * on this critical function.
     */
    if ( ! PTC_Sts_v.PTC_MODE ) {
        CONTROL &= ~EE_WE; /* write disable EEPROM */
        /* Don't return, let be H/W protection (write access to EEPROM with EE_WE=0) */
    }

    /* erase EEPROM word */
    EEPROM = EE_CTL_ERASE;
    *address = 0; /* actual erase */

    /* wait till erase finished */
    while ((EEPROM & EE_BUSY) == EE_BUSY) {
      ; /* polling */
    }

    /* write EEPROM word */
    EEPROM = EE_CTL_WRITE;
    *address = data; /* actual write */

    /* wait till write finished */
    while ((EEPROM & EE_BUSY) == EE_BUSY) {
      ; /* polling */
    }

    /* write disable EEPROM */
    CONTROL &= ~EE_WE;
#else
    (void)address;
    (void)data;
#endif
}

/* Write byte to EEPROM */
#ifndef UNIT_TEST
static void  Write_EEPROM_Byte ( uint8* address, uint8 data)
{
    uint16 * pWord = (uint16*)(((uint16)address) & ~0x1u);
    uint16   Word;

    if ( ((uint16)address) & 0x1u ) {
        Word = (*pWord & 0x00FF) | ( ((uint16)data) << 8 );   /* odd  */
    } else {
        Word = (*pWord & 0xFF00) | ( ((uint16)data)      );   /* even */
    }

    MUPeT_APP_EEPROM_Write (pWord, Word);
}
#endif

/* Super User */
/* Additional benefits - write to EEPROM independently of MemLock */
static uint16 s_SuperUser = 0;

/* Product function call ID */
enum {
    CALL_ID_Update_EE_CRC = 0x0001,
    CALL_ID_Unlock        = 0x023E
};

/* Key for Unlock product call */
enum { CALL_ID_KEY_Unlock = 0x4873 };

/*------------------------------------------------------------------------------
 *  Application dependent function
 *  call function from table
 */
static INLINE uint16  MUPeT_APP_CallIdFunction ( uint16 callId, uint16 data )
{
    uint16 result = 0;
#ifndef UNIT_TEST
    uint8  crc;

    switch ( callId )
    {
    case CALL_ID_Update_EE_CRC:
        crc = eepromCRC();                            /* Calculate EEPROM CRC */
        Write_EEPROM_Byte ( &ee.CRC8, crc );          /* Write CRC */
        break;

    case CALL_ID_Unlock:
        if ( CALL_ID_KEY_Unlock == data ) {
            s_SuperUser = 1;                          /* Set Super User Mode */
        } else {
            result = 1;                               /* Error Result */
        }
        break;

    default:
        result = 1;                                   /* Error Result */
        MUPeT_virtMemErrReg.s.bInvalidCmd = 1u;       /* Report command error */
        break;
    }
#else
    (void)callId;
    (void)data;
#endif
    return result;
}

/*------------------------------------------------------------------------------
 *  Application dependent function
 *  EEPROM protection
 *
 *  Return: wrAccessEnable or wrAccessDisable.
 */
static INLINE wrAccess_t  MUPeT_APP_EEPROM_WriteAccess (void)
{
    wrAccess_t result = wrAccessDisable;

#ifndef UNIT_TEST
        /* Write EEPROM is possible for shortWrite command  */
        if ( cmdShortWrite == MUPeT_state.s.cmdOpcode)
        {
            /* Add write protection check for EEPROM areas */
            if ( (0 != s_SuperUser) || (MEM_UNLOCK == ee.MemLock) )
            {
                /* Freeze application */
                enterAppFreeze();
                MUPeT_state.s.bNoAppliFreeze = 0;     /* Clear application not freeze state */
                result = wrAccessEnable;
            }
        }
#endif
    return result;
}

/*------------------------------------------------------------------------------
 *  Application dependent function
 *  Call after address check (only if mem_type != ADDR_INVALID)
 *
 *  Return: address type.
 */
static INLINE  mem_t MUPeT_APP_GetAddressType (mem_t mem_type)
{
    /* Check address - state.params.p1  */

    /* Additional security for  s_SuperUser variable */
    if ( (ADDR_RAM == mem_type) && ( ((addr_pnt_t)&s_SuperUser) == MUPeT_state.params.p1.addr ) )
    {
        /* Change type as ROM (read-only access */
        mem_type = ADDR_ROM;
    }

    return mem_type;
}

/*------------------------------------------------------------------------------
 *  Application dependent function
 *  is called after successful OpenSessiom command
 */
static INLINE openSessionAccess_t  MUPeT_APP_onOpenSession (void)
{
    if (0 != MUPeT_state.s.bNoAppliFreeze) {    /* check if application have to be frozen */
        /* Don't freeze application */
    }
    else {
        /* Freeze application */
#ifndef UNIT_TEST
        enterAppFreeze();
#endif
    }

    return openSessionEnable;
}

/*------------------------------------------------------------------------------
 *  Application dependent function
 *  get PTC data (read PTCW IO port)
 */
static INLINE uint16  MUPeT_APP_getPTC_data (void)
{
#ifdef UNIT_TEST
    return g_MUPeT_Wr_Data;
#else
    return (uint16)PTCW_DATA;
#endif
}

/*------------------------------------------------------------------------------
 *  Application dependent function
 *  set PTC data (write PTCR IO port)
 */
static INLINE void  MUPeT_APP_setPTC_data (uint16 data)
{
#ifdef UNIT_TEST
    g_MUPeT_Rd_Data = data;
#else
    PTCR_DATA = data;
#endif
}


/*------------------------------------------------------------------------------
 *  Application dependent function
 *  get interrupt status ( for WR, RD and IT )
 *  Return : 0 - no interrupt, !0 - interrupt.
 */
static INLINE uint16  MUPeT_APP_getPTC_WrStatus(void)
{
#ifdef UNIT_TEST
    return g_bWrStsAck;
#else
    return(uint16)(PTC_Sts_v.W_Ack);
#endif
}

static INLINE uint16  MUPeT_APP_getPTC_RdStatus(void)
{
#ifdef UNIT_TEST
    return g_bRdStsAck;
#else
    return(uint16)(PTC_Sts_v.R_Ack);
#endif
}

static INLINE uint16  MUPeT_APP_getPTC_ItStatus(void)
{
#ifdef UNIT_TEST
    return g_bITStsAck;
#else
    return(uint16)(PTC_Sts_v.IT_Ack);
#endif
}

/*------------------------------------------------------------------------------
 *  Set all interrupts bit that now have been set
 */
static INLINE void  MUPeT_APP_PTC_Ack(void) {
#ifdef UNIT_TEST
    if ( 0 != g_bWrStsAck  ) { g_bWrStsAck = 0; }
    if ( 0 != g_bRdStsAck  ) { g_bRdStsAck = 0; }
    if ( 0 != g_bITStsAck  ) { g_bITStsAck = 0; }
#else
    /* Clear setting '1' */
    /* PTC_STS = PTC_STS; */
    /* See JIRA MLXCOMP-17 - Volatile variable assigned to itself */
    uint8 temp =  PTC_STS;
    PTC_STS    = temp;
#endif
}

/*------------------------------------------------------------------------------
 *  Application dependent function
 *  is called after successful closeSessiom command
 */
static INLINE void  MUPeT_APP_onCloseSession ( bool bAppReset )
{
    /* If application was in freese, need restart */
    if ( bAppReset )
    {
#ifndef UNIT_TEST
        /* WARNING: The Function never returns */
        /* Set write acknowledge in this place and wait inactivity of PTCW bit */
        MUPeT_APP_PTC_Ack();
        while( 0 != MUPeT_APP_getPTC_WrStatus() ) {
            NOP();
        }

        resetApp();
#endif
    }

    /* Reset Super User */
    s_SuperUser = 0;
}

#endif /* #ifndef MUPET_CBK_H_ */

/* EOF */
