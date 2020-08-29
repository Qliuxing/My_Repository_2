/*
 * Copyright (C) 2013 Melexis N.V.
 *
 * MUPeT, Application Layer
 *
 *  Implementation according with document:
 *
 *  "Firmware Specification of MUPeT (PTC)"
 *  Version 1.3 7-May-12 (by MPO).
 *
 *  ( http://cvs.bevaix.elex.be/cgi-bin/cvsweb/~checkout~/
 *                design/90370/product/system/FirmwareSpec_MUPeT.docx?rev=1.1 )
 *
 * For portable to a project need change only MUPeT_APP_* functions.
 *
 */

#include "mupet.h"
#include <mupet_cfg.h>   /* Application configuration */
#include <mupet_cbk.h>   /* Application callback function */
#include <stdbool.h>     /* bool */

/**** Global variable ****/
addr_pnt_t      MUPeT_pointers[4];                /* Pointers table */
state_t         MUPeT_state;                      /* State */
ErrRegister_t   MUPeT_virtMemErrReg;              /* Virtual memory status registers */
uint16          MUPeT_checkSum ;                  /* Check sum */

MUPET_SessionId_t     MUPET_SessionId;           /* Session Id */

/* Module's functions */
static void     MUPeT_ExecuteCommand (void);
static uint16   MUPeT_ReadWord (void);
static void     MUPeT_WriteWord (void);
static mem_t    MUPeT_GetAddressType (void);
static uint16   MUPeT_ReadVirtualMemWord(void);
static void     MUPeT_setErrParseCmd (void);


/* Application FW version - 8 bytes */
static const uint8 MUPeT_APP_FirmwareVersion[8] = MUPET_APP_FIRWMARE_VERSIOM_INIT ;


/* Call function types, from 0 to 7 parameters */
typedef uint16(*pCallFunc_0)(void);
typedef uint16(*pCallFunc_1)(uint16);
typedef uint16(*pCallFunc_2)(uint16, uint16);
typedef uint16(*pCallFunc_3)(uint16, uint16, uint16);
typedef uint16(*pCallFunc_4)(uint16, uint16, uint16, uint16);
typedef uint16(*pCallFunc_5)(uint16, uint16, uint16, uint16, uint16);
typedef uint16(*pCallFunc_6)(uint16, uint16, uint16, uint16, uint16, uint16);
typedef uint16(*pCallFunc_7)(uint16, uint16, uint16, uint16, uint16, uint16, uint16);


/*-----------------------------------------------------------------------------
 *  MUPeT initialization function
 *
 *  Notes:
 *     - only reset internal variables,
 *     - DON'T enable/disable any interrupt;
 */
void MUPET_Init (void)
{
    MUPeT_pointers[0] = PTR_IO;          /* Pointer table initialize */
    MUPeT_pointers[1] = PTR_RAM;
    MUPeT_pointers[2] = PTR_EEPROM_NVM;
    MUPeT_pointers[3] = PTR_VIRT_MEM;

    MUPeT_virtMemErrReg.u = 0;                  /* Virtual register error code */

    MUPeT_state.s.bOpenedSession = 0;           /* Session is closed */

    MUPeT_state.s.bNoAppliFreeze = 1u;          /* Application isn't frozen */

    MUPeT_state.cmd.s.opcode = cmdNoCommand;    /* No command */
    MUPeT_state.s.cmdState  = FirstWord;        /* Start executable from 1st word */

    MUPeT_checkSum = 0;                         /* check sum of burst message */

    MUPET_SessionId = SESSION_ID_UNKNOWN;
}

/*-----------------------------------------------------------------------------
 *  MUPeT function is opened session
 *
 *  Notes:
 *
 *    Result: 0 - closed, !0 - opened.
 */
uint16  MUPET_IsOpenedSession (void)
{
    return MUPeT_state.s.bOpenedSession;
}

/*-----------------------------------------------------------------------------
 *  MUPeT function is freeze application
 *
 *  Notes:
 *
 *    Result: 0 - noFreeze, !0 - freeze.
 */
uint16  MUPET_IsAppliFreeze (void)
{
    return ! MUPeT_state.s.bNoAppliFreeze;
}

/*-----------------------------------------------------------------------------
 *  MUPeT function gets current session ID
 *
 *  Notes:
 *
 *    Result: SessionID
 */
MUPET_SessionId_t  MUPET_GetSessionID (void) {

    return MUPET_SessionId;
}


/*-----------------------------------------------------------------------------
 *  MUPeT mode handler
 *
 *  Notes:
 *    - call application function MUPeT_disableInterrupts();
 *    - processing one of interrupt - Write, Read, IT ;
 *    - call function executable command;
 *    - call application function MUPeT_restoreInterrupts()
 *
 */
void MUPET_Isr (void) __attribute ((interrupt));
void MUPET_Isr (void)
{
    MUPeT_APP_onEnterInIsr();

    /* Check interrupts flags */
    if ( ( MUPeT_APP_getPTC_WrStatus() != 0 ) ||        /* Write or  */
         ( MUPeT_APP_getPTC_RdStatus() != 0 ) ||        /* Read      */
         ( MUPeT_APP_getPTC_ItStatus() != 0 ) )         /* IT        */
    {
        MUPeT_ExecuteCommand ();
    }

    MUPeT_APP_PTC_Ack();                                /* clear all interrupts bits */

    MUPeT_APP_onExitFromIsr();
}


/*-----------------------------------------------------------------------------
 *  MUPeT execute command
 *
 *  Notes:
 *      - parse command, change command state,
 *
 */
static void MUPeT_ExecuteCommand (void)
{
    bool bReadState = false;

    if ((MUPeT_state.s.cmdState  >=  ReadingWord )) { /* See commend in cmdState_t */
        bReadState = true;
    }

    /* Read's state need execute only on PTCR */
    if ( ((MUPeT_APP_getPTC_RdStatus() != 0) && (false == bReadState)) ||
         ((MUPeT_APP_getPTC_RdStatus() == 0) && (true  == bReadState)))
    {
        if ( (cmdRepeatRead == MUPeT_state.cmd.s.opcode) &&  (true == bReadState) ) {
            /* New PTCW or PTCI events are reset  "cmdRepeatRead" */
            MUPeT_state.s.cmdState = FirstWord;
        }
        else {
            if ( MUPET_IsOpenedSession() ) {
            	MUPeT_setErrParseCmd();            /* Error */
            }
            return;   /* EXIT */                   /* Need nothing to do */
        }
    }

    if ( FirstWord == MUPeT_state.s.cmdState ) {

        /* Execute new command */
        MUPeT_state.cmd.u = GETP_DATA();

        if ( MUPeT_APP_getPTC_ItStatus() != 0 ) {
            /* Session control command */
            MUPeT_state.s.bSessionCmd = 1;
        }
        else {
            MUPeT_state.s.bSessionCmd = 0;
        }
    }
    else {
        if ( MUPeT_APP_getPTC_ItStatus() != 0 ) {
            /* Interrupt current command and start session */
            MUPeT_setErrParseCmd();
            MUPeT_state.s.bSessionCmd = 1;
            MUPeT_state.cmd.u = GETP_DATA();
        }
    }

    /* Session command */
    if ( 0 != MUPeT_state.s.bSessionCmd ) {

        if (FirstWord == MUPeT_state.s.cmdState) {
            /* Command parameters is already stored in "MUPeT_state.cmd" structure */
            MUPeT_state.s.cmdState = SecondWord;
        }
        else if ( SecondWord == MUPeT_state.s.cmdState ) {
            /* Low word Session ID */
            MUPeT_state.params.p1.sessionLowID = GETP_DATA();
            MUPeT_state.s.cmdState = WritingOr3rdWord;
        }
        else if ( WritingOr3rdWord == MUPeT_state.s.cmdState ) {

            /* Session ID <= (High + Low) Words */
            MUPET_SessionId_t SessionId  = ((MUPET_SessionId_t)(GETP_DATA())) << 16;
                              SessionId |= MUPeT_state.params.p1.sessionLowID;
            /* Session Type */
            sessionType_t  sessionType =  GETW_SESSION_TYPE(MUPeT_state.cmd.u);

            if ( cOpenSession == sessionType ) { /* Opening of Session */

            	/* Set new session ID */
            	MUPET_SessionId = SessionId;

                if ( sessionIdIsAsserted == MUPeT_APP_checkSessionId() ) {

                    MUPeT_state.s.bNoAppliFreeze = GETW_NOAPPLIFREEZE(MUPeT_state.cmd.u);

                    /* TODO - others session parameters */

                    if ( openSessionEnable == MUPeT_APP_onOpenSession() ) {

                        MUPeT_state.s.bOpenedSession = 1;
                    }
                }
                else {
                    /* Invalid Session ID */
                    MUPeT_virtMemErrReg.s.bInvalidSession = 1;
                }
            }
            else if ( (cCloseSession == sessionType) || (cResetApp == sessionType) ) {

                if ( MUPET_IsOpenedSession() ) {
                    /* Only if session was opened early */

                    /* Check with open session ID */
                    if ( SessionId == MUPET_GetSessionID() ) {
                        MUPeT_state.s.bOpenedSession = 0;
                        /* With or without reset */
                        MUPeT_APP_onCloseSession( (cResetApp == sessionType) );
                    }
                    else {
                        /* Invalid Session ID */
                        MUPeT_virtMemErrReg.s.bInvalidSession = 1;
                    }
                }
            }
            else {
                MUPeT_setErrParseCmd();
            }

            /* Close command */
            MUPeT_state.s.cmdState = FirstWord;
        }
        else {
            MUPeT_setErrParseCmd();
        }

        return; /* EXIT */
    }


    /* Skip others command if session is closed */
    if ( ! MUPET_IsOpenedSession() ) {
        return; /* EXIT */
    }

    switch (MUPeT_state.cmd.s.opcode)
    {
        case   cmdShortRead:    /* break */
        case   cmdShortWrite:
        //case   cmdByteWrite:   /* TODO */

            if (FirstWord == MUPeT_state.s.cmdState) {
                /* Store address, offset word-address */
                MUPeT_state.params.p1.addr =
                		MUPeT_pointers [GETP_BASE()] + (GETP_OFFSET() << 1);

                if ( cmdShortRead == MUPeT_state.cmd.s.opcode ) {
                       /*** READ  ***/
                    /* Put data to the PTCR port, data will be read by master during next PTCR access */
                    READ_DATA(MUPeT_ReadWord());                 /* Read word */
                    MUPeT_state.s.cmdState = ReadingWord;        /* Next state */
                }
                else { /*** WRITE ***/
                    MUPeT_state.s.cmdState = WritingOr3rdWord;   /* Next state */
                }
            }
            else if ( WritingOr3rdWord == MUPeT_state.s.cmdState ) {
                /* Write word */
                MUPeT_WriteWord();
                MUPeT_state.s.cmdState = FirstWord;              /* Command has executed */
            }
            else if ( ReadingWord == MUPeT_state.s.cmdState ) {
                /* Data word was read from PTCR */
                MUPeT_state.s.cmdState = FirstWord;              /* Command has been executed */
            }
            else {
                MUPeT_setErrParseCmd();                          /* Report command sequence error */
            }
            break;


        case   cmdSetBase: /* break */
        case   cmdGetBase:

            if ( FirstWord == MUPeT_state.s.cmdState ) {
                /* High part of address is not used */

                /* Store number of base */
                MUPeT_state.params.p2.base  = (uint8)(GETP_BASE());

                /* Next state */
                if ( cmdSetBase == MUPeT_state.cmd.s.opcode ) {
                    MUPeT_state.s.cmdState = SecondWord;
                }
                else { /* cmdGetBase */
                    /* Put addr[15:0] to the PTCR port, data will be read by master during next PTCR access */
                    READ_DATA ( MUPeT_pointers[MUPeT_state.params.p2.base] );
                    MUPeT_state.s.cmdState = ReadingLowAddr;
                }
            }
            else if ( SecondWord == MUPeT_state.s.cmdState ) {
                /* Store low part of address */
                MUPeT_state.params.p1.addr = GETP_ADDR_L();

                /* Change base */
                MUPeT_pointers[MUPeT_state.params.p2.base] = MUPeT_state.params.p1.addr;
                MUPeT_state.s.cmdState = FirstWord;              /* Command has executed */
            }
            else if ( ReadingLowAddr == MUPeT_state.s.cmdState ) {
                /* Put addr[19:16] to the PTCR port, data will be read by master during next PTCR access */
                READ_DATA (0); /* High part of address is not used */
                MUPeT_state.s.cmdState = ReadingHighAddr;
            }
            else if ( ReadingHighAddr == MUPeT_state.s.cmdState ) {
                /* All data were read already */
                MUPeT_state.s.cmdState = FirstWord;              /* Command has executed */
            }
            else {
                MUPeT_setErrParseCmd();                          /* Report command sequence error */
            }

            break;


        case   cmdBurstRead:    /* break */
        case   cmdBurstWrite:   /* break */
        case   cmdRepeatRead:

            if ( FirstWord == MUPeT_state.s.cmdState ) {
                /* High part of address is not used */

                /* Store counter of read words */
                MUPeT_state.params.p2.counter  = (uint8)GETP_N();

                /* Next state */
                MUPeT_state.s.cmdState = SecondWord;
            }
            else if ( SecondWord == MUPeT_state.s.cmdState ) {
                /* Store low part of address */
                MUPeT_state.params.p1.addr = GETP_ADDR_L();

                /* TODO: Check address range */

                if ( (cmdBurstRead == MUPeT_state.cmd.s.opcode) ||
                        (cmdRepeatRead == MUPeT_state.cmd.s.opcode))  {
                    uint16 data = MUPeT_ReadWord();              /* Read first word */
                    READ_DATA(data);
                    MUPeT_checkSum = data;                       /* Init check sum */
                    /* Counter is not updated for first word, because frame count = N+1  */
                    MUPeT_state.s.cmdState = ReadingWord;
                }
                else { /* cmdBurstWrite */
                    MUPeT_checkSum = 0;                          /* Init check sum */
                    /* Counter is not updated for first word, because frame count = N+1  */
                    MUPeT_state.s.cmdState = WritingOr3rdWord;
                }
            }
            else if ( ReadingWord == MUPeT_state.s.cmdState ) {
                /* Counter */
                if ( (MUPeT_state.params.p2.counter != 0) ||
                        (cmdRepeatRead == MUPeT_state.cmd.s.opcode) ) {

                    if ( cmdRepeatRead != MUPeT_state.cmd.s.opcode ) {

                        MUPeT_state.params.p2.counter -- ;

                        /* Update address */
                        MUPeT_state.params.p1.addr += 2;
                    }

                    /* Read current word */
                    uint16 data = MUPeT_ReadWord();
                    READ_DATA (data);
                    MUPeT_checkSum += data;

                    /* Doesn't change state */
                    /* In case of "cmdRepeatRead" reading will finish after received */
                    /* either PTCW or PTCI event */

                }
                else {
                    /* Read check sum */
                    READ_DATA (MUPeT_checkSum);

                    /* Next state */
                    MUPeT_state.s.cmdState = ReadingChSum;
                }
            }
            else if ( WritingOr3rdWord == MUPeT_state.s.cmdState ) {

                MUPeT_WriteWord();

                /* Calculate check sum */
                MUPeT_checkSum += GETP_DATA();

                /* Counter */
                if ( MUPeT_state.params.p2.counter != 0 ) {
                    MUPeT_state.params.p2.counter -- ;
                }
                else {
                    /* Read check sum */
                    READ_DATA (MUPeT_checkSum);
                    MUPeT_state.s.cmdState = ReadingChSum;
                }

                /* Auto-increment of address */
                MUPeT_state.params.p1.addr += 2 ;
            }
            else if ( ReadingChSum == MUPeT_state.s.cmdState ) {
                /* Checksum word was read from PTCR */
                MUPeT_state.s.cmdState = ReadingLowAddr;         /* Command has executed */

                /* TODO - init address Low */
            }
            else if ( ReadingLowAddr == MUPeT_state.s.cmdState ) {

                MUPeT_state.s.cmdState = ReadingHighAddr;

                /* TODO - init address High */
            }
            else if ( ReadingHighAddr == MUPeT_state.s.cmdState ) {

                MUPeT_state.s.cmdState = FirstWord;              /* Command has executed */
            }
            else {
                MUPeT_setErrParseCmd();                          /* Report command sequence error */
            }
            break;


        case cmdCall:
            if ( FirstWord == MUPeT_state.s.cmdState ) {

                /* Address on call contents */
                MUPeT_state.params.p1.addr =
                		MUPeT_pointers [GETP_BASE()] + (GETP_CALL_OFFSET() << 1);

                /* TODO - check address of call contents */

                /* High part of address is not used */
                uint16 FuncResult = MUPeT_state.cmd.u; /* Default return in case of error */
                uint16 FuncAddr = *( (uint16*)(MUPeT_state.params.p1.addr + 2) );

                switch ( GETP_CALL_N() ) {
                case 0:
                    FuncResult = ((pCallFunc_0)FuncAddr)();
                    break;
                case 1:
                    FuncResult = ((pCallFunc_1)FuncAddr)( *( (uint16*)(MUPeT_state.params.p1.addr + 4) ) );
                    break;
                case 2:
                    FuncResult = ((pCallFunc_2)FuncAddr)( *( (uint16*)(MUPeT_state.params.p1.addr + 4) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 6) ));
                    break;
                case 3:
                    FuncResult = ((pCallFunc_3)FuncAddr)( *( (uint16*)(MUPeT_state.params.p1.addr + 4) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 6) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 8) ));
                    break;
                case 4:
                    FuncResult = ((pCallFunc_4)FuncAddr)( *( (uint16*)(MUPeT_state.params.p1.addr + 4) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 6) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 8) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 10) ));
                    break;
                case 5:
                    FuncResult = ((pCallFunc_5)FuncAddr)( *( (uint16*)(MUPeT_state.params.p1.addr + 4) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 6) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 8) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 10) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 12) ));
                    break;
                case 6:
                    FuncResult = ((pCallFunc_6)FuncAddr)( *( (uint16*)(MUPeT_state.params.p1.addr + 4) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 6) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 8) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 10) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 12) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 14) ));
                    break;
                case 7:
                    FuncResult = ((pCallFunc_7)FuncAddr)( *( (uint16*)(MUPeT_state.params.p1.addr + 4) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 6) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 8) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 10) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 12) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 14) ),
                                                          *( (uint16*)(MUPeT_state.params.p1.addr + 16) ));
                    break;

                default:
                    MUPeT_setErrParseCmd();                          /* Report command sequence error */
                    break;
                }

                /* Next state - reading return */
                READ_DATA (FuncResult);
                MUPeT_state.s.cmdState = ReadingWord;
            }
            else if ( ReadingWord == MUPeT_state.s.cmdState ) {
                /* Result was read from PTCR */
                MUPeT_state.s.cmdState = FirstWord;              /* Command has been executed */
            }
            else {
                MUPeT_setErrParseCmd();                          /* Report command sequence error */
            }
            break;

        default:
            /* Error  command */
            MUPeT_virtMemErrReg.s.bInvalidCmd = 1;
            break;

    }   /* end of switch */

}   /* End of MUPeT_ExecuteCommand() */


/*-----------------------------------------------------------------------------
 *  MUPeT read word
 *
 *  Notes:
 *      - return word, if case invalid address - only return 0.
 */
static uint16 MUPeT_ReadWord (void)
{
    uint16  data;
    mem_t mem_type = MUPeT_GetAddressType();

    /* Check address */
    if ( ADDR_INVALID == mem_type )
    {
        /* Nothing */
        MUPeT_virtMemErrReg.s.bInvalidCmd = 1u;                  /* Report command error */
        /* Return PTCW command instruction */
        data = MUPeT_state.cmd.u;
    }
    else if ( ADDR_VirtMem == mem_type )
    {
        /* Read virtual memory */
        data = MUPeT_ReadVirtualMemWord ();
    }
    else
    {
        /* Read word */
        data = *((uint16*)(MUPeT_state.params.p1.addr));
    }

    return data;
}

/*-----------------------------------------------------------------------------
 *  MUPeT write word
 *
 *  Notes:
 *
 */
static void MUPeT_WriteWord (void)
{
    mem_t mem_type = MUPeT_GetAddressType();

    if ( (ADDR_INVALID == mem_type) || (ADDR_ROM == mem_type) || ( ADDR_VirtMem == mem_type ) )
    {
        /* Invalid address */
        MUPeT_virtMemErrReg.s.bInvalidCmd = 1u;       /* Report command error */
    }
    else if ( ADDR_EEPROM == mem_type ) {
        /* Check EEPROM protection */
        if ( wrAccessEnable == MUPeT_APP_EEPROM_WriteAccess() ) {
            /* Write to EEPROM */
            MUPeT_APP_EEPROM_Write ( (uint16*)MUPeT_state.params.p1.addr, GETP_DATA());
        }
        else {
            /* Error of memory lock */
            MUPeT_virtMemErrReg.s.bWrWithMemlock = 1u;
        }
    }
    else {
        /* Common write*/
        *((uint16*)(MUPeT_state.params.p1.addr)) = GETP_DATA();
    }
}

/*-----------------------------------------------------------------------------
 * Function read virtual memory word.
 *
 *  Notes:
 *      - return word, if case invalid address - only return 0.
 */
static uint16   MUPeT_ReadVirtualMemWord(void)
{
    uint16 data = 0;
    uint16 offset = (MUPeT_state.params.p1.addr - VIRT_MEM_START_ADDR) >> 1;

    if ( 0 == offset ) {
        data = MUPeT_virtMemErrReg.u;

        /* After reading reset all errors */
        MUPeT_virtMemErrReg.u = 0;
    }
    else if ( 1u == offset ) {
        data = FIRMWARE_MUPET_VERSION;
    }
    else if ( (2u <= offset) && (offset <= 5u) )  {
        data = ( *((uint16*)((uint8*)MUPeT_APP_FirmwareVersion + ((offset - 2u) << 1))) );
    }
    else {
        /* Nothing */
    }

    return data;
}

/*-----------------------------------------------------------------------------
 * Identifies memory type to which address type.
 */
static mem_t MUPeT_GetAddressType (void)
{
    mem_t mem_type;

    /* Identify the memory type: RAM, ROM, IO or EEPROM */
    if (MUPeT_state.params.p1.addr < RAM_END_ADDR) {                /* if RAM area requested .. */
        mem_type = ADDR_RAM;
    }
    else if ((MUPeT_state.params.p1.addr >= EEP_START_ADDR) &&
             (MUPeT_state.params.p1.addr < EEP_END_ADDR)) {         /* if EEPROM area requested .. */
        mem_type = ADDR_EEPROM;
    }
    else if ((MUPeT_state.params.p1.addr >= SYSTEM_IO_START_ADDR) &&
             (MUPeT_state.params.p1.addr < SYSTEM_IO_END_ADDR)) {   /* if system IO area requested .. */
        mem_type = ADDR_IO;
    }
    else if ((MUPeT_state.params.p1.addr >= USER_IO_START_ADDR) &&
             (MUPeT_state.params.p1.addr < USER_IO_END_ADDR)) {     /* if user IO area requested .. */
        mem_type = ADDR_IO;
    }
    else if ((MUPeT_state.params.p1.addr >= ROM_START_ADDR) &&
             (MUPeT_state.params.p1.addr < ROM_END_ADDR)) {         /* if ROM area requested .. */
        mem_type = ADDR_ROM;
    }
    else if ((MUPeT_state.params.p1.addr >= VIRT_MEM_START_ADDR) &&
             (MUPeT_state.params.p1.addr < VIRT_MEM_END_ADDR)) {    /* if Virtual address area requested .. */
        mem_type = ADDR_VirtMem;
    }
    else {
        mem_type = ADDR_INVALID;
    }

    /* Check on even address */
    if ((MUPeT_state.params.p1.addr & 1u) != 0) {
        mem_type = ADDR_INVALID;
    }

    /* Additional application address check */
    if ( ADDR_INVALID != mem_type  ) {
        mem_type = MUPeT_APP_GetAddressType (mem_type);
    }

    return mem_type;
}

static void MUPeT_setErrParseCmd (void)
{
    MUPeT_virtMemErrReg.s.bInvalidSeq = 1u;
    MUPeT_state.s.cmdState = FirstWord;                             /* Command has executed */
}


/* EOF */
