/*
 * Copyright (C) 2005-2015 Melexis N.V.
 *
 * MelexCM Software Platform
 *
 */

/*
 *  NOTES:
 *      1.  Startup code, LIN driver and loader itself should be linked into
 *          the first half of the Flash:
 *              - all functions should be defined with __MLX_TEXT__ attribute
 *              - all constants should be prefixed with  _mlx_,
 *                  e.g. _mlx_TableAddress
 *
 *      2.  This module should NOT use _initialized_ global and static variables!
 *          Such variables are linked into .data or .dp.data sections and their
 *          initialization lists are stored at load address (LMA) in the Flash.
 *          Since there is no control on the position of the load address, the
 *          linker might link it to second half of the Flash and thus
 *          initialization values will be overwritten by the loader during
 *          programming of the a new application. As a result, variables in .data
 *          sections will not be correctly initialized.
 *          Use uninitialized variables instead (will be linked to .bss section).
 */

#include <syslib.h>
#include <plib.h>

#include "lin.h"
#include "lin_internal.h"

#include "lin_nad.h"

#include <nvram.h>

#include "flashfunctions.h"
#include "flashupload_cfg.h"
#include "flashupload.h"

/* Software Errors : values for MLX16_error */
typedef enum {

    ddErTBL    = 0x10,   /* table does not exist            */
    ddErDATA   = 0x20,   /* too many data frames received   */
    ddErCCNT   = 0x30,   /* reserved: error in the CF frame counter */
    ddErFPROT  = 0x40,   /* reserved: unable to enter fast protocol */

#if (LDR_FLASH_WRITE_TEST != FLASH_TEST_NONE)
    ddErFLASH  = 0x50,   /* writing to flash failed         */
#endif /* LDR_FLASH_WRITE_TEST */

    ddErPCI    = 0xB0,   /* PCI not valid                   */
    ddErCFRAME = 0xC0,   /* too many CF frames              */
    ddErOP     = 0xD0,   /* incorrect operation mode        */
    ddErMODE   = 0xE0,   /* reserved: not in Flash Upload mode  */
    ddErNONE   = 0xF0,   /* no error                        */
} ml_MLX16_error_t;


/* Functions Prototypes */
static void ml_PrepareErrorResponse (ml_MLX16_error_t MLX16_error);
static uint8 ml_FlashUploadStatus (ml_MLX16_error_t MLX16_error);
static void ml_ReadData (ml_uint8 BufferIndex, ml_uint8 BufferSize);
static void ml_SendReadResponse (void);
static void ml_SendWriteResponse(uint16_t timeout);

static void ml_UpdateDataIndex (void);

static ml_uint16 ml_ldr_ReadFlashCRC16 (void);
static void ml_ldr_SendCrcResponse (ml_uint16 add_info);
static void ml_ldr_SendLinProdIDResponse (void);

/* Melexis Data Dump Commands */
/* 6 command bits (LSBs) and 2 parity bits (LIN ID parity) */
typedef enum {
    ddNop           = 0x80,
    ddRestart       = 0xC1,
    ddExit          = 0x42,     /* reserved */
    ddFastProt      = 0x03,
    ddSlowProt      = 0xC4,     /* reserved */
    ddWriteAdd      = 0x85,
    ddReadAdd       = 0x06,

    ddProtExtension = 0xD6,

#if (LDR_HAS_EEPROM_COMMANDS != 0)
    ddEeWrite       = 0x47,
    ddEeRead        = 0x08,
    ddEeErase       = 0x49,     /* reserved */
    ddEeEraseAll    = 0xCA,     /* reserved */
#endif /* LDR_HAS_EEPROM_COMMANDS */

    ddEeRestore     = 0x8B,     /* reserved */
    ddEeSave        = 0x4C,     /* reserved */

    ddWriteRam      = 0x0D,     /* reserved */
    ddReadRam       = 0x8E,     /* reserved */
    ddExecRam       = 0xCF,     /* reserved */

    ddTableExist    = 0x50,     /* reserved */
    ddWriteTable    = 0x11,     /* reserved */
    ddReadTable     = 0x92,     /* see Jira:MPT-472 */

    ddData          = 0xD3,
    ddWriteKey      = 0x14,     /* reserved */

    ddFlashPageErase = 0x55     /* reserved, see JIRA:MPT-182 */

} ml_DataDumpCommands;

/* Protocol extension commands */
/* This commands available with 'ddProtExtension' command only */
typedef enum {
    peReadFlashModify   = 0x00,
    peMarginModify      = 0x01

} ml_ProtocolExtension;

/* ReadFlash modify commands */
/* This modes available with 'peReadFlashModify' only */
typedef enum {
    rfmNormal           = 0x00,
    rfmCrcCalc          = 0x01

} ml_ReadFlashModify;

/* Flash Margin modify commands */
/* This modes available with 'peMarginModify' only */
typedef enum {
    mmMarginSetOffset   = 0x00

} ml_MarginModify;

#pragma space dp
/*
 * Loader Global Variables
 */
ml_uint8 ml_driver_mode;    /* LIN driver mode: kLinAppMode or kLinLoaderMode */

/* NodeStatus sent in Response frames
 * The "error bit" is set to 1 only in the error reply and in this case the following byte
 * gives additional information about the error 
 *
 * x x x x x x xx
 *  \ \ \ \ \ \  \
 *   \ \ \ \ \ \  Loader Status
 *    \ \ \ \ \ tbd
 *     \ \ \ \ tbd
 *      \ \ \ tbd
 *       \ \ tbd
 *        \ tbd
 *         error bit
 */
#define ML_ERROR_BIT    (1u << 7)

/* ML_FAST_BAUDRATE in kBaud */
#define ML_FAST_BAUDRATE_K    ML_FAST_BAUDRATE / 1000

/* Keep track of the last error received from the Mlx4 */
static ml_uint8 MLX4_error;

/* Current Operation (possible values are enum below) */
static ml_uint8 ddCurrentOp;

/* Protocol extension Current Operation */
static ml_uint8 peCurrentOp;

/* Protocol extension Current Value
 * Note: this variable also is used for switching to the Fast protocol */
static ml_uint8 peCurrentValue;

#if (LDR_FLASH_WRITE_TEST != FLASH_TEST_NONE)
static uint16_t flashWriteStatus;
#endif  /* LDR_FLASH_WRITE_TEST */

/* Global Variables for Flash Access (read and Write) */
static ml_uint16 ddDataAddress;     /* base address of the data for flash access (ends with x000_0000)  */
                                    /* ... complete address for RAM         */
static ml_uint16 ddAddressOffset;   /* offset to the real address from the base address */
static ml_uint16 ddDataSize;        /* size of the block to read/write          */
static ml_uint16 ddDataSizeRq;      /* requested size of the block to write         */
static ml_uint16 ddDataCounter;     /* byte being accessed relative to ddAddressOffset  */
static ml_uint8  ddFrameCounter;    /* counter to check the continuous frames       */

static ml_uint8 pendingAction;      /* Action that should be taken after Slave Response Frame */

#pragma space none

/* Supplier ID & Function ID for Loader mode */
static const uint16_t LDR_SUPPLIER_ID = 0x0013;
static const uint16_t LDR_FUNCTION_ID = 0xCAFE;

static const uint8_t _mlx_MsgEnterProgModeID = 0x33;
static const uint8_t _mlx_MsgLinProdID = 0x00;

/* Read by identifier broadcast frame: {NAD, 0x06, 0xB2, 0x00, 0xFF, 0x7F, 0xFF, 0xFF} */
static const uint16_t READ_BY_ID_BC[4] = {
    0x0600,
    0x00B2,
    0x7FFF,
    0xFFFF
};


typedef struct __attribute__((packed)) {
    uint8_t     dynamic_timeout_enabled;
} LoaderInfo_Type;

static const LoaderInfo_Type _mlx_loader_info = {
    .dynamic_timeout_enabled = 1
};


/* ----------------------------------------------------------------------------
 * Verifies if message in buffer is an "Read By ID" message
 *
 * \param *buffer          pointer to `LinFrameDataBuffer[]'
 * \param CheckWildcard    defines whether wildcards are supported
 *
 * \return 0:   message in the buffer is NOT ReadByID
 *         1:   message in the buffer is ReadByID
 *
 * Inputs: LIN_nad(Global), READ_BY_ID_BC, LDR_SUPPLIER_ID, LDR_FUNCTION_ID
 *
 */
__MLX_TEXT__ ml_bool ldr_isReadByIdMessage (const void *buffer, ml_bool CheckWildcard)
{
    uint16_t const *src = (uint16_t const *)buffer;

    if ( ( (src[0] == (READ_BY_ID_BC[0] | 0x7F )) ||    /* LIN_nad or wildcard (0x7F) */
           (src[0] == (READ_BY_ID_BC[0] | LIN_nad) )
         ) &&
         ( ((src[1]&0xFF) == READ_BY_ID_BC[1]) &&
           ( ( (src[2] == LDR_SUPPLIER_ID) &&           /* check FunctionID and ProductID */
               (src[3] == LDR_FUNCTION_ID)
             ) ||
             ( (CheckWildcard == ML_TRUE) &&
               (src[2] == READ_BY_ID_BC[2]) &&          /* check wildcards */
               (src[3] == READ_BY_ID_BC[3])
             )
           )
         )
       )
    {
        return ML_TRUE;
    }
    else
    {
        return ML_FALSE;
    }
}


/* ----------------------------------------------------------------------------
 * Do "ReadByID" message handling
 *
 * \param Id            identifier value of ReadById message
 */
__MLX_TEXT__ void ml_ldr_ReadByIdMessage (uint8_t Id)
{
    if (_mlx_MsgEnterProgModeID == Id) { /* EnterProgMode frame was requested */
#if defined (LDR_RESET_ON_ENTER_PROG_MODE)
        ml_ldr_SwitchToProgMode(ML_TRUE);
#else /* LDR_RESET_ON_ENTER_PROG_MODE */
        ml_ldr_SwitchToProgMode(ML_FALSE);
#endif /* LDR_RESET_ON_ENTER_PROG_MODE */
    }
    else if (_mlx_MsgLinProdID == Id) {  /* LIN product identification frame was requested */
        ml_ldr_SendLinProdIDResponse();
    }
}


#if defined (LDR_HAS_PROTECTION_KEY)
/* ----------------------------------------------------------------------------
 * Verifies if Loader is protected with key
 *
 * LIN Loader is available for all commands executing if at least one of two following cases is true:
 * 1. Protection key's most significant part is the same as less significant part
 * 2. Key into NVRAM is the same as protection key (key into NVRAM is correct)
 *
 * Otherwise, if protection key's high part isn't equal to low part and key into NVRAM is different from protection key,
 * then Loader is protected and only two Data Dump (SID=0xB4) commands can be executed: ddWriteKey and ddRestart.
 * Loader will answer with ddErOP error in the case of another commands.
 *
 * \param
 *
 * \return 0: Loader isn't protected (all commands are available)
 *         1: Loader is protected (ddWriteKey and ddRestart commands can be executed only)
 */
__MLX_TEXT__ uint8_t ldr_isProtectedByKey (void) {
#if LDR_PROTECTION_KEY_LENGTH == 32
    /* Protection key */
    uint16_t *key_flash_a = (uint16_t*)LDR_PROTECTION_KEY_ADDR_FLASH;
    uint16_t *key_flash_b = key_flash_a + 1;
    /* Key into NVRAM */
    uint16_t *key_nvram_a = (uint16_t*)LDR_UNLOCKING_KEY_ADDR_NVRAM;
    uint16_t *key_nvram_b = key_nvram_a + 1;
#elif LDR_PROTECTION_KEY_LENGTH == 64
    /* Protection key */
    uint32_t *key_flash_a = (uint32_t*)LDR_PROTECTION_KEY_ADDR_FLASH;
    uint32_t *key_flash_b = key_flash_a + 1;
    /* Key into NVRAM */
    uint32_t *key_nvram_a = (uint32_t*)LDR_UNLOCKING_KEY_ADDR_NVRAM;
    uint32_t *key_nvram_b = key_nvram_a + 1;
#else
#error "Incorrect LDR_PROTECTION_KEY_LENGTH value"
#endif /* LDR_PROTECTION_KEY_LENGTH */

    /* Compare protection key's high part with low part */
    if (*key_flash_a != *key_flash_b) {
        /* Verify if key into NVRAM is equal to protection key */
        if ((*key_flash_a == *key_nvram_a) && (*key_flash_b == *key_nvram_b)) {
            /* NVRAM key and protection key are the same; Loader isn't protected */
            return 0;
        }
        else {
            /* NVRAM key and protection key are different; Loader is protected */
            return 1;
        }
    }
    else {
        /* Protection key's low part is equal to high part; Loader isn't protected */
        return 0;
    }
}
#endif /* LDR_HAS_PROTECTION_KEY */


/* ----------------------------------------------------------------------------
 * Send positive response with Loader identifiers
 */
__MLX_TEXT__ void ml_ldr_SendLinProdIDResponse (void)
{
    /* Enable prog mode: sending of prepared data buffer w/o message request */
    (void)ml_ReleaseBufferProg(ML_ENABLED);

    /* Prepare the data to be sent */
    LinFrameDataBuffer[0] = LIN_nad;
    LinFrameDataBuffer[1] = 0x06;   /* PCI */
    LinFrameDataBuffer[2] = 0xF2;   /* SID + 0x40 */
    LinFrameDataBuffer[3] = (ml_uint8)(LDR_SUPPLIER_ID);
    LinFrameDataBuffer[4] = (ml_uint8)(LDR_SUPPLIER_ID >> 8);
    LinFrameDataBuffer[5] = (ml_uint8)(LDR_FUNCTION_ID);
    LinFrameDataBuffer[6] = (ml_uint8)(LDR_FUNCTION_ID >> 8);
#if defined (HAS_ROM_LOADER)
    LinFrameDataBuffer[7] = 0;
#else
    LinFrameDataBuffer[7] = 1;
#endif
    (void)ml_DataReady(ML_END_OF_TX_DISABLED);
}


/* ----------------------------------------------------------------------------
 * Prepare error response to in the LinFrameDataBuffer[]
 *
 * Globals:
 *  LinFrameDataBuffer[]: out, LIN frame buffer
 *  LIN_nad : in, LIN Node Address
 *  MLX4_error: in/out
 */
__MLX_TEXT__  static void ml_PrepareErrorResponse (ml_MLX16_error_t MLX16_error)
{
    LinFrameDataBuffer[0] = LIN_nad;    /* NAD */
    LinFrameDataBuffer[1] = 3;          /* PCI */
    LinFrameDataBuffer[2] = 0x7F;       /* RSID */
    LinFrameDataBuffer[3] = ML_ERROR_BIT | LDR_GetState();      /* NodeStatus */
    LinFrameDataBuffer[4] = (ml_uint8)MLX16_error | MLX4_error; /* Combine MLX16_error (4 MSBs) and MLX4_error (4 LSBs) */

    LinFrameDataBuffer[5] = 0xFF;       /* not used */
    LinFrameDataBuffer[6] = 0xFF;
    LinFrameDataBuffer[7] = 0xFF;

    /* Reset last error after reporting. If transmission of the status message
     * failed we will get another error which cause this failure. This _last_
     * detected error will be reported to master in next status frame.
     * Master aborts operation on any error.
     */
    MLX4_error = 0;  /* set to "No error" */
}


/* ----------------------------------------------------------------------------
 * Send the Status message or Error message as a reply depending on MLX16_error
 * and MLX4_error values
 *
 * Globals:
 *  LinFrameDataBuffer[] : out, LIN Frame buffer
 *
 * \note
 * 1. Assumes that SID of request is 0xB4
 */
__MLX_TEXT__  static uint8 ml_FlashUploadStatus (ml_MLX16_error_t MLX16_error)
{
    uint8 blReturn;

    if ((MLX16_error != ddErNONE) || (MLX4_error != 0)) {  /* MLX16 or MLX4 error */
        ml_PrepareErrorResponse(MLX16_error);
        blReturn = ML_FALSE;
    }
    else { /* no error, reply status */
        LinFrameDataBuffer[0] = LIN_nad;            /* NAD */
        LinFrameDataBuffer[1] = 2;                  /* PCI : length = 2 databytes (SID + status) */
        LinFrameDataBuffer[2] = 0xF4;               /* RSID = SID + 0x40 */
        LinFrameDataBuffer[3] = LDR_GetState();     /* NodeStatus : since no error, error bit is not set */

        LinFrameDataBuffer[4] = 0xFF;               /* not used */
        LinFrameDataBuffer[5] = 0xFF;
        LinFrameDataBuffer[6] = 0xFF;
        LinFrameDataBuffer[7] = 0xFF;
        blReturn = ML_TRUE;
    }

    (void)ml_DataReady(ML_END_OF_TX_DISABLED);      /* Signal to MLX4 that the data is ready */

    return ( blReturn );
}


/* ----------------------------------------------------------------------------
 * Copy `BufferSize` bytes of data to `LinFrameDataBuffer[]` buffer starting
 * at index `BufferIndex`:
 * - first `ddDataSize` bytes from address `ddDataAddress` are copied
 *   to `LinFrameDataBuffer[]`
 * - then the rest of the `LinFrameDataBuffer[]` buffer is filled with 0xFF
 *
 * \param BufferIndex   staring index for writing to `LinFrameDataBuffer[]`
 * \param BufferSize    number of bytes to be copied to `LinFrameDataBuffer[]`
 *
 * \return  none
 *
 * Input (global):
 *      ddDataAddress   Source data start address
 *      ddDataCounter   Source data offset from the start address
 *      ddDataSize      Number of bytes to copy from the source
 *
 * Output (global):
 *      LinFrameDataBuffer[]  LIN frame buffer
 *
 * Global variables used : ddDataAddress, ddDataCounter, ddDataSize, ddCurrentOp
 */
__MLX_TEXT__  static void ml_ReadData (ml_uint8 BufferIndex, ml_uint8 BufferSize)
{
    uint16_t size = BufferSize;
    uint8_t *src = (uint8_t *)(ddDataAddress + ddDataCounter);      /* read byte directly from memory */
    uint8_t *dst = (uint8_t *)(&LinFrameDataBuffer[BufferIndex]);


    do {
        if (ddDataCounter < ddDataSize) {   /* if there's something to send ..  */
            *dst++ = *src++;                /* .. take it from memory           */
        }
        else {                              /* no more data in memory .. */
            *dst++ = 0xFF;                  /* .. fill up the rest of the frame with 0xFF */
        }

        ddDataCounter += 1;
    } while (--size != 0);

    if (ddDataCounter >= ddDataSize) {      /* if all requested data were sent .. */
        (void)ml_ContFrame(ML_DISABLED);    /* .. clear the Continuous Frames flag for the Mlx4 */
    }
    /* else : not all data have been sent yet => Continuous Frames are still active */
}


/* ----------------------------------------------------------------------------
 * Send a response to a read request (read from flash, RAM or EEPROM)
 *
 * Input (global):
 *      MLX4_error      MLX4 errors
 *      ddDataSize      Number of bytes to copy from the source
 *      LIN_nad         Slave NAD
 *
 *  Output (global):
 *      LinFrameDataBuffer[]  LIN frame buffer
 *      ddFrameCounter        Continuous frame counter
 *
 * \note:
 *  1. Can be a Single Frame or a First Frame
 *      Single Frame format : NAD RPCI RSID NodeStatus Data1 Data2 Data3 Data4
 *      First Frame format  : NAD RPCI RLEN RSID NodeStatus Data1 Data2 Data3
 */
__MLX_TEXT__  static void ml_SendReadResponse(void)
{
    ml_uint16 ResponseLength;


    if (MLX4_error != 0) {
        ml_PrepareErrorResponse(ddErNONE); /* Error response */
    }
    else {  /* Prepare the Response */
        ResponseLength = ddDataSize + 2;    /* Length = flash data + status + SID */

        if (ddDataSize > 4) {   /* if more than 4 bytes are requested than will send using Continuous Frames .. */

            (void)ml_ContFrame(ML_ENABLED);/* signal to MLX4 that some Continuous Frames are coming */
                                /* Next frames will be TX frames (see ml_DiagRequest) */

            ddFrameCounter = 0; /* reset frame counter */

            /* Fill the buffer for the First Frame (FF) */
            LinFrameDataBuffer[0] = LIN_nad;                                            /* NAD */
            LinFrameDataBuffer[1] = 0x10 | (ml_uint8) ((ResponseLength & 0x0F00) >> 8); /* PCI = 0001 xxxx where xxxx is length/256 */
            LinFrameDataBuffer[2] = (ml_uint8) (ResponseLength & 0x00FF);               /* note : RLEN has to be < 4095 (spec)      */
            LinFrameDataBuffer[3] = 0xF4;                                               /* RSID =  SID + 0x40 */
            LinFrameDataBuffer[4] = LDR_GetState();                                     /* NodeStatus : since no error, error bit is not set */
            ml_ReadData(5, 3);    /* FF: copy to LinFrameDataBuffer[5] next 3 bytes from (ddDataAddress + ddDataCounter) */
        }
        else { /* Single Frame (SF) is enough to deliver 4 (or less) bytes */

            /* Fill the buffer */
            LinFrameDataBuffer[0] = LIN_nad;
            LinFrameDataBuffer[1] = (ml_uint8) ResponseLength;  /* RPCI = length */
            LinFrameDataBuffer[2] = 0xF4;                       /* RSID =  SID + 0x40 */
            LinFrameDataBuffer[3] = LDR_GetState();             /* NodeStatus : since no error, error bit is not set */

#if !defined (HAS_H12_LOADER_PROTOCOL)
            /*
             * Intercept direct loader state reading from Flash (a word @ 0xBF66) and
             * replace it by the loader state returned by LDR_GetState()
             * TODO: MPT should not read loader state directly
             */
            if ((ddDataAddress == 0xBF66) && (ddDataSize == 2)) {   /* if a _word_ reading from address 0xBF66 is requested .. */
                LinFrameDataBuffer[4] = LDR_GetState();             /* .. replace it by a state information (LSByte) */
                LinFrameDataBuffer[5] = 0;                          /* .. MSByte of the loader state is always 0 */
                LinFrameDataBuffer[6] = 0xFF;                       /* frame padding */
                LinFrameDataBuffer[7] = 0xFF;                       /* frame padding */
            }
            else {
                ml_ReadData(4, 4);  /* SF: copy to LinFrameDataBuffer[4] next 4 bytes from (ddDataAddress + ddDataCounter) */
            }
#else
            ml_ReadData(4, 4);      /* SF: copy to LinFrameDataBuffer[4] next 4 bytes from (ddDataAddress + ddDataCounter) */
#endif
        }
    }

    (void)ml_DataReady(ML_END_OF_TX_DISABLED);  /* Signal that the data is ready to send */
}


/* ----------------------------------------------------------------------------
 * Send a response to a write request (to flash, RAM or EEPROM)
 * Frame format : NAD PCI RSID NodeStatus BLK1 BLK0 TIM1 TIM0
 */
__MLX_TEXT__  static void ml_SendWriteResponse(uint16_t timeout)
{
    if (MLX4_error != 0) {
        ml_PrepareErrorResponse(ddErNONE); /* Error response */
    }
    else {
        /* Fill the buffer */
        LinFrameDataBuffer[0] = LIN_nad;                            /* NAD */
        LinFrameDataBuffer[1] = 0x06;                               /* PCI (Single Frame + Length) */
        LinFrameDataBuffer[2] = 0xF4;                               /* RSID = SID + 0x40 */
        LinFrameDataBuffer[3] = LDR_GetState();                     /* NodeStatus : since no error, error bit is not set */
        LinFrameDataBuffer[4] = (ml_uint8)(ddDataSize >> 8);        /* Size of next allowed block (MSB) */
        LinFrameDataBuffer[5] = (ml_uint8)(ddDataSize & 0x00FF);    /* Size of next allowed block (LSB) */
        LinFrameDataBuffer[6] = (ml_uint8)(timeout >> 8);           /* command execution time (MSB)     */
        LinFrameDataBuffer[7] = (ml_uint8)(timeout & 0xFF);         /* command execution time (LSB)     */
    }

    (void)ml_DataReady(ML_END_OF_TX_DISABLED);                      /* Signal to MLX4 that the data is ready */
}


/* ----------------------------------------------------------------------------
 * Update ddDataAddress, ddAddressOffset, ddDataSize, ddDataSizeRq and reset
 * ddDataCounter after a block has been written in flash.
 * Also read the next block.
 */
__MLX_TEXT__  static void ml_UpdateDataIndex (void)
{
    ddDataAddress += 128;           /* Get the next block */
    ddAddressOffset = 0;            /* Reset the address offset (0 since we start at the beginning of a next block) */
    ddDataCounter = 0;              /* Reset the data counter */
    ddDataSize = ddDataSizeRq;      /* Calculate the next size */
    if (ddDataSize > 128) {
        ddDataSize = 128;
    }
    /* else : size is correct */

    ddDataSizeRq -= ddDataSize;

    Flash_PageRead(ddDataAddress);  /* Read the flash block (128 bytes) to internal RAM buffer */
}


/* ----------------------------------------------------------------------------
 * This function is called by LIN ISR to notify flash loader about errors
 * detected by LinModule (MLX4)
 */
__MLX_TEXT__  void ml_ldr_ErrorDetected (ml_LinError Error)
{
    /*
     * Sometimes erShort error is detected by MLX4 during flash uploading due to
     * transition slow/fast mode. Ignore ml_erShort error during reprogramming
     */
    if (Error != ml_erShort) {
        MLX4_error = (ml_uint8)Error;  /* save as last detected MLX4 error */
    }
    /* else: ignore the error */

}


/* ----------------------------------------------------------------------------
 * This function calculates CRC on flash starting from 'ddDataAddress' to 'ddDataSize'
 *
 * Input (global):
 *      ddDataAddress   Source data start address
 *      ddDataSize      Number of bytes to copy from the source
 *
 * Output:
 *      CRC calculated on specified addresses
 *
 * Optimized implementation of CRC16 using the CCITT polynomial 1021
 * (X^16 + X^12 + X^5 + 1)
 *
 * The function is based on 'mathlib.h -> crc_ccit'.
 * Calling this function ones increase calculation speed instead of every byte
 * function calling from 'mathlib'.
 */
__MLX_TEXT__ static uint16 ml_ldr_ReadFlashCRC16 (void)
{
    const uint8 *data = (uint8 *)ddDataAddress;

    uint16 i;
    uint16 crc = 0xFFFF;

    for (i = 0; i < ddDataSize; i++) {
        crc  = (uint8)(crc >> 8) | (crc << 8);
        crc ^= *data;
        data++;
        crc ^= (uint8)(crc & 0xff) >> 4;
        crc ^= (crc << 8) << 4;
        crc ^= ((crc & 0xff) << 4) << 1;

        if ((i & 0x0FFF) == 0)
        {
            WDG_Manager();
        }
    }
    return crc;
}


/* ----------------------------------------------------------------------------
 * Prepare LinFrameDataBuffer[] with 'ml_ldr_ReadFlashCRC16' function result
 *
 * \param     add_info        additional information to write into `LinFrameDataBuffer[]'
 *
 * Globals: Same as for ml_ldr_ReadFlashCRC16
 */
__MLX_TEXT__ static void ml_ldr_SendCrcResponse (uint16 add_info)
{
    if (MLX4_error != 0) {
        ml_PrepareErrorResponse(ddErNONE); /* Error response */
    }
    else {

#if STANDALONE_LOADER != 1
        ml_Disconnect(); /* Disconnect Mlx4 from LIN bus; Preventing mlx4 unwanted interrupts */
#endif /* !STANDALONE_LOADER */
        /* CRC calculation function:
         * Global:            - ddDataAddress (address in the flash)
         *                    - ddDataSize
         * Return arguments:  - calculated CRC
         */
        uint16 flash_block_crc = ml_ldr_ReadFlashCRC16();

        /* ...wait... */

        /* Fill the buffer */
        LinFrameDataBuffer[0] = LIN_nad;
        LinFrameDataBuffer[1] = 0x06;           /* RPCI = length           */
        LinFrameDataBuffer[2] = 0xF4;           /* RSID =  SID + 0x40      */
        LinFrameDataBuffer[3] = 0xFF;           /* 0xFF means CRC is ready (reserved) */
        LinFrameDataBuffer[4] = (uint8) (flash_block_crc >> 8) & 0xFF;    /* MSB of CRC   */
        LinFrameDataBuffer[5] = (uint8) (flash_block_crc) & 0xFF;         /* LSB of CRC   */
        LinFrameDataBuffer[6] = (uint8) (add_info >> 8) & 0xFF;           /* MSB of word  */
        LinFrameDataBuffer[7] = (uint8) (add_info) & 0xFF;                /* LSB of word  */

#if STANDALONE_LOADER != 1
        ml_Connect();                 /* Connect Mlx4 to LIN bus; Calculation of CRC is over */
#endif /* !STANDALONE_LOADER */
    }
    (void)ml_DataReady(ML_DISABLED);  /* Signal that the data is ready to send               */
}


/* ----------------------------------------------------------------------------
 *  Switch to programming mode
 */
__MLX_TEXT__  void ml_ldr_SwitchToProgMode (ml_bool Reset)
{
    /*
     * Test if this operation is allowed with IsKey()
     */

    /*
     * Test is the application needs to be stopped
     * Note: mlu_ApplicationStop doesn't exits if LoaderState != 0
     */
    if ((LDR_GetState() == 0) && (mlu_ApplicationStop() != ML_SUCCESS)) { /* if application can not be stopped .. */

        /*
         * Stay in application mode (kLinAppMode)
         */

        ml_driver_mode = kLinAppMode; /* TODO: check this one */
        /*
         * Can be also used for LIN loader if binary compatibility is not required
         */
        (void)ml_ReleaseBufferProg(ML_DISABLED);    /* Disable prog mode, SID = 0xB2 */
    }
    else {
        MLX16_MASK_ALL_INT();           /* mask all interrupts */

        if (   (0 == LDR_GetState())
#if defined (LDR_RESET_ON_ENTER_PROG_MODE)
            && (bistResetInfo != C_CHIP_STATE_LOADER_PROG_RESET) /* if reset reason isn't EnterProgMode */
#endif /* LDR_RESET_ON_ENTER_PROG_MODE */
           )
        {   /* in state 0 .. */
            NVRAM_SaveAll();            /* save NVRAM; will be restored on next reset       */
            ENABLE_MLX4_INT();          /* enable only LIN interrupt */
        }
        /* else : for other loader state use LIN interrupt polling */

        ml_driver_mode  = kLinLoaderMode;   /* loader mode */
        ddCurrentOp = 0;                    /* reset ddCurrentOp state machine */
        Flash_InitDriver();

        /*
         * Can be also used for LIN loader if binary compatibility is not required
         */
        (void)ml_ReleaseBufferProg(ML_ENABLED); /* Enable prog mode: sending of prepared buffer w/o message request */

        if (Reset == ML_TRUE) {
#if defined (LDR_RESET_ON_ENTER_PROG_MODE)
            bistResetInfo = C_CHIP_STATE_LOADER_PROG_RESET;
            MLX4_RESET();           /* reset the Mlx4   */
            MLX16_RESET();          /* reset the Mlx16  */
            for (;;) {
                /* waiting for reset */
            }
#endif /* LDR_RESET_ON_ENTER_PROG_MODE */
        }
        else {
            /* Prepare the data to be sent */
            LinFrameDataBuffer[0] = LIN_nad;
            LinFrameDataBuffer[1] = 0x06;   /* PCI */
            LinFrameDataBuffer[2] = 0xF2;   /* SID + 0x40 */

            uint32_t version = ml_GetPlatformVersion();

            LinFrameDataBuffer[3] = (ml_uint8)(version >> 24);
            LinFrameDataBuffer[4] = (ml_uint8)(version >> 16);
            LinFrameDataBuffer[5] = (ml_uint8)(version >>  8);
            LinFrameDataBuffer[6] = (ml_uint8)version;

            LinFrameDataBuffer[7] = LDR_GetState();

            (void)ml_DataReady(ML_END_OF_TX_DISABLED);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Response Diagnostic Frame
 * This is entered when a SRF (0x7D) is sent without a MRF before, 
 * meaning this is a continuous frame (CF)
 */
__MLX_TEXT__  void ml_DiagRequest (void)
{
#if 0
    WDG_Manager();
#endif


    switch (pendingAction) {

#if STANDALONE_LOADER == 0
        case ddFastProt:
            (void)ml_ContFrame(ML_DISABLED);
            (void)ml_Disconnect();
            /* Set baudrate */
            (void)ml_SetFastBaudRate(peCurrentValue);
            /* Configure the Mlx4 software */
            (void)ml_SetOptions (1U,        /* IDStopBitLength = 1.5 Bit (Melexis LIN Master has 1.5 Tbit stop bit */
                            0U,             /* TXStopBitLength = 1 Bit */
                            ML_ENABLED,     /* StateChangeSignal */
                            ML_LIGHTSLEEP   /* SleepMode: lightsleep mode */
                           );
            (void)ml_SetSlewRate(ML_SLEWFAST);
            (void)ml_SwitchToFast(); /* Switch to fast protocol */
            pendingAction = 0;
            break;
#endif /* STANDALONE_LOADER */

        default:    /* Continuous Frame (CF) reading handling */
            if ((ddCurrentOp == ddReadAdd)
#if (LDR_HAS_EEPROM_COMMANDS != 0)
                    || (ddCurrentOp == ddEeRead)
#endif /* LDR_HAS_EEPROM_COMMANDS */
               )
            {
                /* Flash, Table or EEPROM Read Operation */
                if (ddDataCounter < ddDataSize) {       /* if there is still data to send .. */
                    ddFrameCounter += 1;

                    /* Fill the buffer : NAD PCI Data1 Data2 Data3 Data4 Data5 Data6 */
                    LinFrameDataBuffer[0] = LIN_nad;                            /* NAD */
                    LinFrameDataBuffer[1] = 0x20 | ((ddFrameCounter) & 0x0F);   /* RPCI */
                    ml_ReadData(2, 6);      /* copy next 6 bytes from ddDataAddress to buffer starting from LinFrameDataBuffer[2] */
                                            /* if less than 6 bytes left, fill with 0xFF */

                    (void)ml_DataReady(ML_END_OF_TX_DISABLED);
                }
                /* else : Nothing to send */
            }
            else {
                /* Mlx4 always generates evENDtx in Programming Mode.
                 * Loader couldn't recognise whether it is an error or not.
                 * Due to this - ignore this error */
                if (0) {
                    ml_FlashUploadStatus(ddErCFRAME);   /* error: too many CC frames */
                }
            }

            break;
    }  /* !switch (pendingAction) */
}


/* ----------------------------------------------------------------------------
 * ACT_DFR_DIA (Master Request Command Frame)
 * Request format :  NAD PCI SID  Data[0:4]
 * Response format : NAD PCI RSID Data[0:4]
 * Get the data received
 * byte 0 : NAD (LinFrameDataBuffer[0])
 * byte 1 : PCI (Protocol Control Information) (LinFrameDataBuffer[1])
 */
__MLX_TEXT__  void ml_DiagReceived (void)
{
    uint16_t i;                 /* iterations */

#if 0
    WDG_Manager();
#endif

    const ml_uint8 PCI = LinFrameDataBuffer[1];

    /* --- Consecutive Frame (CF) handler -----------------------------------
     *            [0] [1] [2] [3] [4] [5] [6] [7]
     * CF format: NAD PCI D0  D1  D2  D3  D4  D5
     *
     * \note:
     * CF is only used for the commands: ddWriteKey and ddData (operations
     * ddWriteAdd, ddEeWrite)
     */
    if ((PCI & 0xF0) == 0x20) {                                 /* if Consecutive Frame (CF) frame received ..  */
        ml_uint8 const * const Data = &LinFrameDataBuffer[2];   /* data start from byte 2 of the frame  */

#if defined (LDR_HAS_PROTECTION_KEY)
        /* if Loader isn't protected with key or permitted command is requested */
        if ( (0 == ldr_isProtectedByKey()) || (ddCurrentOp == ddWriteKey) ) {
#endif /* LDR_HAS_PROTECTION_KEY */
            if ((PCI & 0x0F) == (ddFrameCounter & 0x0F)) {  /* if local frame counter (4 bits) matches the counter in PCI .. */
                ddFrameCounter += 1;

                if (ddCurrentOp == ddWriteAdd) {
                    for (i = 0; i < 6; i++) {               /* Store the Data received */
                        if (ddDataCounter < ddDataSize) {
                            Flash_PageBufferFill(ddAddressOffset + ddDataCounter, Data[i]);
                        }
                        /* else : ignore padding data beyond the original message size (ddDataSize) */

                        ddDataCounter += 1;
                    }

                    /* If all data has been written to the buffer, write the flash
                     * This also means that this was the last Continuous Frame
                     */
                    if (ddDataCounter >= ddDataSize) {

#if (LDR_FLASH_WRITE_TEST != FLASH_TEST_NONE)
                        flashWriteStatus = Flash_PageWriteFiltered(ddDataAddress);
#else
                        (void)Flash_PageWriteFiltered(ddDataAddress);
#endif /* LDR_FLASH_WRITE_TEST */

#if (LDR_FLASH_WRITE_TEST != FLASH_TEST_NONE)
                        if (FLASH_ERR_NONE == flashWriteStatus) {
#endif /* LDR_FLASH_WRITE_TEST */
                            if (ddDataCounter < ddDataSizeRq) { /* if there is still some data to be written ..*/
                                ml_UpdateDataIndex();           /* .. update the indexes and counters */
                            }
                            else {                              /* Operation is done */
                                ddDataSize = 0;
                                ddDataSizeRq = 0;
                                /* ddCurrentOp = 0; */
                            }
#if (LDR_FLASH_WRITE_TEST != FLASH_TEST_NONE)
                        }
                        /* else : Writing to flash failed do not update any index.
                         * The error will be reported in the next status frame (ddNop)
                         */
#endif /* LDR_FLASH_WRITE_TEST */

                        (void)ml_ContFrame(ML_DISABLED);    /* signal to MLX4 that there are no more Continuous Frame after that */
                    }
                    /* else: message is not fully received yet  */
                }
#if (LDR_HAS_EEPROM_COMMANDS != 0)
                else if (ddCurrentOp == ddEeWrite) {
                    for (i = 0; i < 6; i++) {               /* Store the Data received */
                        if (ddDataCounter < ddDataSize) {
                            NVRAM_BufferFill(ddDataAddress + ddDataCounter, Data[i]);
                        }
                        /* else : ignore padding data beyond the original message size (ddDataSize) */

                        ddDataCounter += 1;
                    }

                    /* If all data has been written to the buffer, write the EEPROM
                     * This also means that this was the last Continuous Frame
                     */
                    if (ddDataCounter >= ddDataSize) {
                        NVRAM_SaveAll();                    /* Save the EEPROM */
                        ddDataSize = 0;
                        (void)ml_ContFrame(ML_DISABLED);    /* no more Continuous Frame (signal to MLX4) */
                    }
                    /* else: message is not fully received yet  */
                }
#endif /* LDR_HAS_EEPROM_COMMANDS */

#if defined (LDR_HAS_PROTECTION_KEY)
                else if (ddCurrentOp == ddWriteKey) {
                    /* Write data to EEPROM buffer */
#if LDR_PROTECTION_KEY_LENGTH == 64
                    /* Write 5 bytes (D4 - D0) to EEPROM buffer */
                    for (i = 0; i <= 4; i++) {
                        NVRAM_BufferFill(LDR_UNLOCKING_KEY_ADDR_NVRAM + (4 - i), Data[i]);
                    }
#else /* LDR_PROTECTION_KEY_LENGTH == 32 */
                    /* Write 4 less significant bytes (D3 - D0) to EEPROM buffer */
                    for (i = 1; i <= 4; i++) {
                        NVRAM_BufferFill(LDR_UNLOCKING_KEY_ADDR_NVRAM + (4 - i), Data[i]);
                    }
#endif /* LDR_PROTECTION_KEY_LENGTH */
                    NVRAM_SaveAll();                    /* Save the EEPROM */
                    (void)ml_ContFrame(ML_DISABLED);    /* no more Continuous Frame (signal to MLX4) */
                }
#endif /* LDR_HAS_PROTECTION_KEY */

                else {              /* unknown operation .. */
                    (void)ml_ContFrame(ML_DISABLED); /* .. signal to MLX4 that there are no more Continuous Frame after that */
                }
            }
            else {                  /* Error in the CF frame counter */
                ddCurrentOp = 0;    /* Cancel the current operation */
                (void)ml_ContFrame(ML_DISABLED); /* signal to MLX4 that there are no more Continuous Frame after that */
            }
#if defined (LDR_HAS_PROTECTION_KEY)
        }
        else {
            ml_FlashUploadStatus(ddErOP);  /* Send error status */
        }
#endif /* LDR_HAS_PROTECTION_KEY */
    }
    /* --- First Frame (FF) handler -------------------------------------------
     *            [0] [1] [2] [3] [4] [5] [6] [7]
     * FF format: NAD PCI LEN SID D0  D1  D2  D3
     *
     * \note:
     * FF is only used for the commands: ddWriteKey and ddData (operations
     * ddWriteAdd, ddEeWrite)
     */
    else if ((PCI & 0xF0) == 0x10) {    /* if First Frame (FF) frame received .. */
        ml_uint8 const * const Data = &LinFrameDataBuffer[4]; /* data start from byte 4 of the frame  */
        const ml_uint8 SID = LinFrameDataBuffer[3];           /* byte 3 : SID (Service Identifier)    */

#if defined (LDR_HAS_PROTECTION_KEY)
        /* if Loader isn't protected with key or permitted command is requested */
        if ( (0 == ldr_isProtectedByKey()) || (Data[0] == ddWriteKey) ) {
#endif /* LDR_HAS_PROTECTION_KEY */

            if (SID != 0xB4) {                  /* if this is not a Data Dump operation (SID = 0xB4)    */
                ml_FlashUploadStatus(ddErOP);   /* .. send error                                        */
            }
            else {                              /* Data Dump operation (SID = 0xB4) */
                if (Data[0] == ddData) {
                    ddFrameCounter = 1;         /* Data is being sent - reset the frame counter */

                    /* Check the preceding command */
                    if (ddCurrentOp == ddWriteAdd) {
                        /* Write data to Flash buffer */
                        Flash_PageBufferFill(ddAddressOffset,     Data[1]);
                        Flash_PageBufferFill(ddAddressOffset + 1, Data[2]);
                        Flash_PageBufferFill(ddAddressOffset + 2, Data[3]);
                        ddDataCounter = 3;

                        (void)ml_ContFrame(ML_ENABLED); /* signal to MLX4 that some Continuous Frames are coming */
                    }
#if (LDR_HAS_EEPROM_COMMANDS != 0)
                    else if (ddCurrentOp == ddEeWrite) {
                        /* Write data to EEPROM buffer */
                        NVRAM_BufferFill(ddDataAddress,     Data[1]);
                        NVRAM_BufferFill(ddDataAddress + 1, Data[2]);
                        NVRAM_BufferFill(ddDataAddress + 2, Data[3]);
                        ddDataCounter = 3;

                        (void)ml_ContFrame(ML_ENABLED); /* signal to MLX4 that some Continuous Frames are coming */
                    }
#endif /* LDR_HAS_EEPROM_COMMANDS */

                    else { /* There is a problem : what is the data for ? */
                        ml_FlashUploadStatus(ddErDATA); /* Send error status */
                    }
                }
#if defined (LDR_HAS_PROTECTION_KEY)
                else if (Data[0] == ddWriteKey) {
                    /* First Frame followed by one Continuous Frame */

                    ddFrameCounter = 1; /* Data is being sent - reset the frame counter */
#if LDR_PROTECTION_KEY_LENGTH == 64
                    /* Write most significant data bytes to EEPROM buffer */
                    for (i = 1; i <= 3; i++) {
                        NVRAM_BufferFill(LDR_UNLOCKING_KEY_ADDR_NVRAM + (8 - i), Data[i]);
                    }
#else /* LDR_PROTECTION_KEY_LENGTH == 32 */
                    /*
                     *  No data to be written in First Frame
                     */
#endif /* LDR_PROTECTION_KEY_LENGTH */
                    (void)ml_ContFrame(ML_ENABLED); /* signal to MLX4 that some Continuous Frames are coming */
                    ddCurrentOp = ddWriteKey;
                }
#endif /* LDR_HAS_PROTECTION_KEY */
                else {
                    ml_FlashUploadStatus(ddErOP); /* wrong operation requested */
                }
            }
#if defined (LDR_HAS_PROTECTION_KEY)
        }
        else {
            ml_FlashUploadStatus(ddErOP);  /* Send error status */
        }
#endif /* LDR_HAS_PROTECTION_KEY */
    }
    /* --- Single Frame (SF) -------------------------------------
     *            [0] [1] [2] [3] [4] [5] [6] [7]
     * SF format: NAD PCI SID D0  D1  D2  D3  D4
     */
    else if ((PCI & 0xF0) == 0x00) {        /* if Single Frame (SF) is received .. */
        (void)ml_ContFrame(ML_DISABLED);    /* signal to MLX4 that this is NOT Continuous Frame */

        ml_uint8 const * const Data = &LinFrameDataBuffer[3];   /* data start from byte 3 of the frame */
        const ml_uint16 MessageLength = PCI & 0x0F;             /* length */
        const ml_uint8 SID = LinFrameDataBuffer[2];             /* byte 2 : SID (Service Identifier) */

        if (SID != 0xB4) {                  /* if this is not a Data Dump operation (SID = 0xB4) .. */
            if (ldr_isReadByIdMessage(LinFrameDataBuffer, ML_TRUE)) {  /* check if ReadById (SID = 0xB2) */
                ml_ldr_ReadByIdMessage(Data[0]);
            }
            else {
                ml_FlashUploadStatus(ddErOP);   /* .. send error status */
            }
        }
        else {                              /* Data Dump operation (SID = 0xB4): process the data */

#if defined (LDR_HAS_PROTECTION_KEY)
            /* if Loader isn't protected with key or permitted command is requested */
            if ( (0 == ldr_isProtectedByKey()) || (Data[0] == ddRestart) ) {
#endif /* LDR_HAS_PROTECTION_KEY */

                /* Get the requested 'address' and the 'size'
                 * Not used for ddRestart, ddNop, ddFastProt, ddSlowProt, ddErazeAll, ddEeRestore, ddEeSave
                 * Special care for ddTableExist, ddWriteTable, ddReadTable
                 * Do not do that for ddData and for ddNop (ddDataAddress will be used)
                 */
                if ((Data[0] != ddData) && (Data[0] != ddNop)) {            /* if not Data/Status command(s) .. */
                    ddDataAddress = (((ml_uint16) Data[1]) << 8) | Data[2]; /* .. reload address .. */
                    ddDataSize    = (((ml_uint16) Data[3]) << 8) | Data[4]; /* .. and size */
                    ddDataCounter = 0;
                }
                /* else : do not update ddDataAddress, ddDataSize and ddDataCounter
                 *        for ddData or ddNop operations
                 */

                if ((Data[0] & 0x20 /* bit5 */) != 0)            /* if command is Read Flash (ddReadAdd) or Write Flash (ddWriteAdd) .. */
                {
                    if ((Data[0] & 0x30 /* bits 4-5 */) == 0x20) /* if command is Write Flash (Data[0] = xx10xxxx) */
                    {
                        /* Write Flash:
                         * Arguments : - ddDataAddress (address in the flash)
                         *             - ddDataSizeRq (size of the data to be written)
                         * Return arguments :
                         *  - ddBlockAddress : base address of the flash
                         *  - ddAddressOffset : offset to address the flash buffer
                         *  - ddBlockSizeRequest : block size that can be written at once
                         */
                        ddCurrentOp = ddWriteAdd;

                        /* Get the address and the size requested */
                        /* ddFlashBlockAddress = Data[0] & 0x0F; */ /* get the MSBs - not used for now */
                        ddAddressOffset = ddDataAddress & 0x007F;   /* Address offset */
                        ddDataAddress  &= 0xFF80;                   /* Flash Base Address (7 LSBs are 0) */

                        ddDataSizeRq = ddDataSize;                  /* memorize requested size */
                        ddDataSize   = 128 - ddAddressOffset;       /* max data chunk that can be written from the base address */

                        if (ddDataSize > ddDataSizeRq) {            /* if requested less than max ..    */
                            ddDataSize = ddDataSizeRq;              /* .. do a requested size           */
                        }
                        /* else : do max possible data chunk */

                        ml_SendWriteResponse(Flash_GetWriteTime(ddDataAddress));
                        Flash_PageRead(ddDataAddress);              /* Read the flash block (128 bytes) to internal RAM buffer */
                    }
                    else {
                        /* CRC calculation instead of Read Flash command   */
                        if ((ddCurrentOp == ddProtExtension) && \
                                (peCurrentOp == peReadFlashModify) && (peCurrentValue == rfmCrcCalc))
                        {
                            /* CRC calculation and response sending
                             * Global:            - ddDataAddress (address in the flash)
                             *                    - ddDataSize
                             */
                            ml_ldr_SendCrcResponse(ddDataSize);
                        }
                        else
                        {
                        /* ddFlashBlockAddress = Data[0] & 0x0F; */ /* get the MSBs - not used for now */
                            ddCurrentOp = ddReadAdd;
                            ml_SendReadResponse();                      /* Prepare the Response */
                        }
                    }
                }
                else {
                    switch(Data[0]) {
                        /* General Operations **************************************************************/
                        case ddRestart :
                            if (Data[1] == LDR_GetState()) {
                                /* skip reset if Loader's state is equal to required */
                            }
                            else {
                                MLX4_RESET();                       /* reset the Mlx4   */
                                MLX16_RESET();                      /* reset the Mlx16  */
                            }
                            break;

                        case ddNop :
                            /* DO NOT modify ddCurrentOp, as the operation might still be in process */
                            /* Return the status and eventually the block size and data bytes */
                            if (ddCurrentOp == ddWriteAdd) {
#if (LDR_FLASH_WRITE_TEST != FLASH_TEST_NONE)
                                if (FLASH_ERR_NONE == flashWriteStatus) {
#endif /* LDR_FLASH_WRITE_TEST */
                                    if (0 == ddDataSize) {          /* if nothing to request */
                                        ddCurrentOp = 0;            /* operation done */
                                        ml_SendWriteResponse(0);    /* response : block = 0, remain = 0 */
                                    }
                                    else {  /* still some data are expected from programming tool */
                                        /* Send the Block and Rest size */
                                        /* Fill the buffer and signal that the data is ready */
                                        /* ml_SendWriteResponse(0); */
                                        ml_FlashUploadStatus(ddErDATA);     /* MPT-613 */ /* TODO:check */
                                    }
#if (LDR_FLASH_WRITE_TEST != FLASH_TEST_NONE)
                                }
                                else { /* previous writing to flash failed */
                                    ml_FlashUploadStatus(ddErFLASH);
                                }
#endif /* LDR_FLASH_WRITE_TEST */
                            }
#if (LDR_HAS_EEPROM_COMMANDS != 0)
                            else if (ddCurrentOp == ddEeWrite) {
                                ddCurrentOp = 0;                /* operation done */
                                ml_SendWriteResponse(0);        /* Prepare the Response */
                            }
#endif /* LDR_HAS_EEPROM_COMMANDS */
                            else {  /* (ddCurrentOp == 0) */
                                ml_FlashUploadStatus(ddErNONE); /* Send Status only with no errors */
                            }
                            break;

#if STANDALONE_LOADER == 0
                        /* Fast Protocol */
                        case ddFastProt :
                            if (PCI == 3) { /* check if Data[1] is significant byte */
                                peCurrentValue = Data[1];
                                if (peCurrentValue > ML_FAST_BAUDRATE_K) { /* saturate requested baudrate[kBd] to MAX Baudrate */
                                    peCurrentValue = ML_FAST_BAUDRATE_K;
                                }
                                else if (peCurrentValue < ML_MIN_FAST_BAUDRATE_K) { /* check with MIN baudrate[kBd] */
                                    peCurrentValue = ML_MIN_FAST_BAUDRATE_K;
                                }
                            }
                            else {
                                peCurrentValue = ML_FAST_BAUDRATE_K; /* set MAX baudrate if Master doesn't support flexible baudrate */
                            }
                            /* Prepare S2M message */
                            LinFrameDataBuffer[0] = LIN_nad;            /* NAD */
                            LinFrameDataBuffer[1] = 3;                  /* PCI : length = 3 databytes (SID + status + baudrate) */
                            LinFrameDataBuffer[2] = 0xF4;               /* RSID = SID + 0x40 */
                            LinFrameDataBuffer[3] = LDR_GetState();     /* NodeStatus : since no error, error bit is not set */
                            LinFrameDataBuffer[4] = peCurrentValue;     /* Applied Fast Protocol baudrate */
                            LinFrameDataBuffer[5] = 0xFF;
                            LinFrameDataBuffer[6] = 0xFF;
                            LinFrameDataBuffer[7] = 0xFF;

                            (void)ml_DataReady(ML_END_OF_TX_DISABLED);  /* Signal to MLX4 that the data is ready */
                            (void)ml_ContFrame(ML_ENABLED);
                            pendingAction = ddFastProt;
                            break;
#endif /* STANDALONE_LOADER */

#if (LDR_HAS_EEPROM_COMMANDS != 0)
                        /* EEPROM Operations ***************************************************************/
                        case ddEeWrite :
                            ddCurrentOp = ddEeWrite;    /* Write to the EEPROM      */
                            NVRAM_LoadAll();            /* Load the EEPROM values   */
                            ml_SendWriteResponse(0);    /* Prepare the Response     */
                            break;

                        case ddEeRead :
                            ddCurrentOp = ddEeRead;     /* Read the EEPROM          */
                            NVRAM_LoadAll();            /* Load the EEPROM values   */
                            ml_SendReadResponse();      /* Prepare the Response     */
                            break;
#endif /* LDR_HAS_EEPROM_COMMANDS */

                        /* simple case with Table #8 (read-only) */
                        case ddReadTable :
                        {
                            uint16_t table_number = (ml_uint8)(ddDataAddress >> 8); /* get table number form Data[1] */

                            if (table_number != 8) {                                /* if table other than 8 is requested ..    */
                                ml_FlashUploadStatus(ddErTBL);                      /* .. send error status                     */
                            }
                            else {                                                  /* Error : table does not exist */
                                ddDataSize    = sizeof(_mlx_loader_info);           /* table size; TODO: size = MIN(real_table_size, requested_size) */
                                ddDataAddress = (ml_uint16)&_mlx_loader_info;       /* get the address of the table */
                                ddCurrentOp   = ddReadAdd;                          /* use the same operations to read tables as for the Flash => ddReadAdd */
                                ml_SendReadResponse();                              /* prepare the response */
                            }
                            break;
                        }

                        /* all operations longer than one frame *********************************************/
                        case ddData :   /* ddData for Single Frame (only 1, 2, 3 or 4 bytes to write) */

                            if (ddCurrentOp == ddWriteAdd) {    /* if previous command is ddWriteAdd (write Flash) */
#if !defined (HAS_H12_LOADER_PROTOCOL)
                               /*
                                * Intercept direct writing to loader state word (a word @ 0xBF66)
                                * and replace it:
                                *  0xBF66 = 0  ==>  loader_flags.app_enabled  = 1
                                *  0xBF66 = 1  ==>  loader_flags.app_disabled = 1
                                */
                                if (((ddDataAddress + ddAddressOffset)== 0xBF66)    /* if writing to address 0xBF66 is requested .. */
                                    && (ddDataSizeRq == 2))                         /* .. with data size of 2 bytes ..      */
                                {
                                    uint16_t new_state = Data[1];                   /* new loader state */

                                    switch (new_state) {
                                        case 0:     /* transition to state 0 ==> enable application */
                                        {
                                            uint16_t addr = (uint16_t)&loader_flags.app_enabled;
                                            Flash_PageRead(addr);                                   /* copy page to buffer starting from base address */
                                            Flash_PageBufferFill(addr & ML_FLASH_BUFFER_MASK, 1);   /* write to buffer using address offset */
                                                                                                    /* TODO: address masking is redundant; check if we can use address instead of offset in Flash_PageBufferFill */
#if (LDR_FLASH_WRITE_TEST != FLASH_TEST_NONE)
                                            flashWriteStatus = Flash_PageWrite(addr);   /* [unfiltered] write page back to the flash */
#else
                                            (void)Flash_PageWrite(addr);
#endif  /* LDR_FLASH_WRITE_TEST */

#if defined (DEBUG_LOADER_FLAGS)
                                            /* test after writing */
                                            if ((loader_flags.app_disabled != 0) || (loader_flags.app_enabled != 1))
                                            {
                                                while (1) {WDG_Manager(); }
                                            }
                                            /* else: writing was OK */

                                            if ((FL_CTRL0 & (FL_SBE | FL_DBE)) != 0)    /* if writing caused Flash error(s) .. */
                                            {
                                                while (1) {WDG_Manager(); }
                                            }
                                            /* else: no errors */
#endif /* DEBUG_LOADER_FLAGS */

                                        }
                                            break;

                                        case 1:     /* transition to state 1 ==> disable application */
                                        {
#if defined (DEBUG_LOADER_FLAGS)
                                            /* test before writing */
                                            if ((loader_flags.app_disabled != 0) || (loader_flags.app_enabled != 1)) {
                                                while (1) { WDG_Manager(); }
                                            }
                                            /* else: expected values */
#endif /* DEBUG_LOADER_FLAGS */
                                            uint16_t addr = (uint16_t)&loader_flags.app_disabled;
                                            Flash_PageRead(addr);                                   /* copy page to buffer starting from base address */
                                            Flash_PageBufferFill(addr & ML_FLASH_BUFFER_MASK, 1);   /* write to buffer using address offset */
#if (LDR_FLASH_WRITE_TEST != FLASH_TEST_NONE)
                                            flashWriteStatus = Flash_PageWrite(addr);   /* [unfiltered] write page into the flash */
#else
                                            (void)Flash_PageWrite(addr);
#endif  /* LDR_FLASH_WRITE_TEST */

#if defined (DEBUG_LOADER_FLAGS)
                                            /* test after writing */
                                            if ((loader_flags.app_disabled != 1) || (loader_flags.app_enabled != 1))
                                            {
                                                while (1) {WDG_Manager(); }
                                            }
                                            /* else: expected values */


                                            if ((FL_CTRL0 & (FL_SBE | FL_DBE)) != 0)    /* if writing caused Flash error(s) .. */
                                            {
                                                while (1) {WDG_Manager(); }
                                            }
                                            /* else: no errors after Flash write */
#endif /* DEBUG_LOADER_FLAGS */

                                        }
                                            break;

                                        default:
                                            /* Ignore other state transitions */
                                            break;
                                    }
                                }
                                else {
#endif /* HAS_H12_LOADER_PROTOCOL */
                                for (i = 0; i < (MessageLength - 2); i++) { /* don't count SID and command opcode */
                                    Flash_PageBufferFill(ddAddressOffset + i, Data[i+1]);
                                }

#if (LDR_FLASH_WRITE_TEST != FLASH_TEST_NONE)
                                flashWriteStatus = Flash_PageWriteFiltered(ddDataAddress);  /* write page into the flash */
#else
                                (void)Flash_PageWriteFiltered(ddDataAddress);
#endif  /* LDR_FLASH_WRITE_TEST */

#if !defined (HAS_H12_LOADER_PROTOCOL)
                                }
#endif
                            }
#if (LDR_HAS_EEPROM_COMMANDS != 0)
                            else if (ddCurrentOp == ddEeWrite) {
                                for (i = 0; i < (MessageLength - 2); i++) { /* don't count SID and command opcode */
                                    NVRAM_BufferFill(ddDataAddress + i, Data[i+1]);
                                }

                                NVRAM_SaveAll();
                            }
#endif /* LDR_HAS_EEPROM_COMMANDS */

                            else {                              /* There is a problem : what is the data for ?  */
                                ml_FlashUploadStatus(ddErDATA); /* Send error status                            */
                            }

                            /* Since it was a Single Frame, the operation has been completed */
                            ddDataSizeRq = 0;
                            ddDataSize = 0;
                            /* ddCurrentOp = 0; */
                            break;

                        /* --- Protocol extension block -------------------------------------
                         *            [0] [1] [2] [3] [4] [5] [6] [7]
                         * PE format: NAD PCI SID D0  D1  D2  D3  D4
                         *                         \   \   \   \   \
                         *                          \   \   \   X   X
                         *                           \   \   PE Value
                         *                            \   PE Code
                         *                             ddProtExtension = 0xD6
                         */
                        case ddProtExtension:
                            /* Set ddProtExtension as current operation */
                            ddCurrentOp = ddProtExtension;
                            /* Get protocol extension command CODE */
                            peCurrentOp = Data[1];

                            /* --------------------------------
                             Commands for Read Flash redefining
                             ---------------------------------- */
                            if (peCurrentOp == peReadFlashModify) {
                                /* Get protocol extension command VALUE */
                                peCurrentValue = Data[2];

                                switch (peCurrentValue) {
                                /* Normal ReadFlash command execution */
                                case rfmNormal:
                                    ddCurrentOp = 0;
                                    ml_FlashUploadStatus(ddErNONE); /* Send Status with no errors */
                                    break;

                                /* CRC calculation instead of ReadFlash command */
                                case rfmCrcCalc:
                                    ml_FlashUploadStatus(ddErNONE); /* Send Status with no errors */
                                    break;

                                /* Wrong protocol extension command VALUE sets ddErOp */
                                default:
                                    ddCurrentOp = 0;
                                    ml_FlashUploadStatus(ddErOP); /* Send Status with error: incorrect operation mode  */
                                }
                            }
                            /* --------------------------------
                             Flash Margin modify commands
                             ---------------------------------- */
                            else if (peCurrentOp == peMarginModify) {
                                /* Get protocol extension command VALUE */
                                peCurrentValue = Data[2];

                                switch (peCurrentValue) {
                                /* Set up signed offset to threshold for MardinRead */
                                case mmMarginSetOffset: {
                                    /* Get signed offset value */
                                    int16 offset_iref = (int8) Data[3];

                                    /* Change IREF by offset value in FLASHTRIMA register */
                                    uint16 saved_iref = Flash_IREF_Offset(offset_iref);

                                    /* Use this functionality for response generation */
                                    ddDataAddress = ML_FLASH_START_ADDRESS;
                                    ddDataSize = 0; /* No data to CRC */

                                    /* Used to generate response with old IREF value
                                     * No CRC calculation, 0xFFFF will be returned */
                                    ml_ldr_SendCrcResponse(saved_iref);
                                    break;
                                }
                                /* Wrong protocol extension command VALUE sets ddErOp */
                                default:
                                    ddCurrentOp = 0;
                                    ml_FlashUploadStatus(ddErOP); /* Send Status with error: incorrect operation mode  */
                                }
                            }
                            /* Wrong protocol extension command CODE sets ddErOp */
                            else {
                                ddCurrentOp = 0;
                                ml_FlashUploadStatus(ddErOP); /* Send Status with error: incorrect operation mode  */
                            }
                            break;

                        default :   /* no action */
                            ml_FlashUploadStatus(ddErOP);       /* Send error status */
                            break;
                    }
                }
#if defined (LDR_HAS_PROTECTION_KEY)
            }
            else {
                ml_FlashUploadStatus(ddErOP);  /* Send error status */
            }
#endif /* LDR_HAS_PROTECTION_KEY */
        }
    }
    else {                              /* PCI not valid .. */
        ml_FlashUploadStatus(ddErPCI);  /* .. send error status */
    }
}


/* EOF */
