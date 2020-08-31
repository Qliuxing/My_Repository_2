/*
 * Copyright (C) 2005-2015 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef MLX16_LIN_H_
#define MLX16_LIN_H_

#include <typelib.h>
#include "lincst.h"         /* MLX4 constants */
#include "lin_baudrate.h"

/* 
 * STOP bit transmission error (erStopBitTX) is generated by LIN ISR (mlx16)
 * based on erTXCOL MLX4 error and mostly required for SAE J2602.
 * Range of error codes reported by MLX4 is 0x00-0x0F
 */
#define mlx16_erStopBitTX     0x10    /* Error during STOP bit transmission */

/* Type Definitions ----------------------------------------------------------------    */
/* ml_uint8 */
typedef unsigned char ml_uint8;

/* ml_uint16 */
typedef unsigned int ml_uint16;

/* ml_bool */
typedef unsigned char ml_bool;

/* ml_Status */
typedef unsigned char ml_Status;

/* ml_LinError */
typedef enum {
    ml_erShortDone       = erSHORTDONE,
    ml_erLinModuleReset  = erCRASH,
    ml_erIdParity        = erIDPAR,
    ml_erCheckSum        = erCKSUM,
    ml_erBit             = erTXCOL,
    ml_erDataFraming     = erRX,
    ml_erIdFraming       = erIDSTOP,
    ml_erSynchField      = erSYNC,
    ml_erBufferLocked    = erRXOVR,
    ml_erShort           = erSHORT,
    ml_erTimeOutResponse = erTORESP,
    ml_erBreakDetected   = erBRFRM,
    ml_erWakeUpInit      = erWKUPINIT,
    ml_erStopBitTX       = mlx16_erStopBitTX   /* Error during STOP bit transmission */
} ml_LinError;

/* ml_CheckSumType : Regular checksum (LIN v1.3) or Enhanced checksum (LIN v2.0) */
typedef enum {
    ml_chk13 = chk13,
    ml_chk20 = chk20
} ml_CheckSumType;

/* ml_LinState */
typedef enum {
    ml_stINIT         = stINIT,
    ml_stDISCONNECTED = stDISC,
    ml_stACTIVE       = stACT,
    ml_stSLEEP        = stSLEEP,
    ml_stWAKEUP       = stWKUP,
    ml_stSHORT        = stSHORT,
    ml_stINVALID      = (ml_uint16)-1
} ml_LinState;


/* ml_StateReason : different causes of a LIN state change as reported with the mlu_LinSleepMode event */
typedef enum {
    ml_reasonMaster          = slMST,
    ml_reasonCommand         = slAPP,
    ml_reasonWakeup          = slWKUP,
    ml_reasonWakeupAbort     = slWKUPabort,
    ml_reasonWakeupResponse  = slWKUPresp,
    ml_reasonWakeupBreak     = slWKUPbreak,
    ml_reasonTimeOut         = slTO,
    ml_reasonTimeOutDominant = slTODOM
} ml_StateReason;

/* ml_AutoAddressingMode */
typedef unsigned char ml_AutoAddressingMode;

/* ml_MessageID */
typedef unsigned char ml_MessageID;

/* ml_FrameID */
typedef unsigned char ml_FrameID;

/* LIN API Data ----------------------------------------------------------------------  */
#pragma space dp  /* shared RAM is in dp area on MULAN2 */

/*
 * Shared RAM MLX4/MLX16 interface
 */
extern volatile ml_uint8  LinProtectedID;

#pragma space none


/* LIN Frame Buffer (private MLX16) */
extern ml_uint8 LinFrameDataBuffer[8] __attribute__((dp, aligned(2)));

/* LIN API Commands ----------------------------------------------------------------    */
/* Software and Flash Loader version */
extern ml_uint16 ml_LinModuleVersion(void);

/* Bus Timing */
extern ml_Status ml_SetBaudRate(ml_uint8 caPresc, ml_uint8 caBaud);
extern ml_Status ml_SetAutoBaudRateMode(ml_uint8 Mode);
extern ml_uint16 ml_GetBaudRate(void);

/* Software and Hardware options */
extern ml_Status    ml_SetSlewRate(ml_uint16 SlewRate);
extern ml_Status    ml_SetOptions(ml_uint8 IDStopBitLength, ml_uint8 TXStopBitLength, ml_bool EnableStateChangeEvent, ml_bool SleepMode);

/* Task Control and Task Status*/
extern void         ml_LinInit(void);
extern ml_Status    ml_InitLinModule(void);
extern ml_Status    ml_Connect(void);
extern ml_Status    ml_Disconnect(void);
extern ml_Status    ml_GotoSleep(void);
extern ml_Status    ml_WakeUp(void);
extern ml_Status    ml_SwitchToFast(void);

/*
 * This command retrieves the current state of the LIN Module.
 *
 * Preconditions:
 *  1. The ml_GetState shall be called atomically from the background main loop
 *     because it could create event racing between MLX4 and MLX16 (see details
 *     in PLTF-733 on Jira)
 *
 * Input:
 *      None
 *
 * Output:
 *      AxxO SSSS
 *       \  \  \
 *        \  \  \
 *         \  \  current state
 *          \  event overflow
 *           bus activity
 *      Current state of the LIN module: ml_stDISCONNECTED, ml_stACTIVE,
 *      ml_stSLEEP, ml_stWAKEUP. The value ml_stINVALID is returned in case
 *      the state of the LIN module can not be retrieved.
 */
extern ml_LinState  ml_GetState(ml_uint8 bits_to_reset);
extern volatile ml_uint8 LinStatus;
#define ML_LIN_BUS_ACTIVITY     (1U << 0)    /* Getting LIN bus activity bit */
#define ML_LIN_BUFFER_NOT_FREE  (1U << 1)
#define ML_LIN_CMD_OVERFLOW     (1U << 3)    /* Mask for getting LIN overflow event bit */

/* defines for bits_to_reset */
#define ML_CLR_BUS_ACT_AND_OVF   0
#define ML_CLR_LIN_CMD_OVERFLOW  1
#define ML_CLR_LIN_BUS_ACTIVITY  8
#define ML_NOT_CLEAR             9

/* Change Loader NAD */
extern ml_Status    ml_SetLoaderNAD(ml_uint8 Nad);
extern ml_uint8     ml_GetLoaderNAD(void);


/* Message Management */
/*
 * Note: These functions shall be called in Disconnected state only
 */
extern ml_Status    ml_AssignFrameToMessageID(ml_MessageID MessageIndex, ml_FrameID FrameID);
extern ml_Status    ml_EnableMessage(ml_MessageID MessageIndex);
extern ml_Status    ml_DisableMessage(ml_MessageID MessageIndex);

/* Data Transfer */
extern ml_Status    ml_DiscardFrame(void);
extern ml_Status    ml_ContFrame(ml_bool blEnable);
extern ml_Status    ml_ReleaseBufferProg(ml_bool blProgMode);
extern ml_Status    ml_DataReady(ml_bool DataTransmittedEvent);

/* Auto-addressing */
extern ml_Status    ml_AutoAddressingConfig(ml_AutoAddressingMode AutoAddressingMode);

/*
 * User functions prototypes
 * These function shall be defined in user application
 */
#if defined (HAS_LIN_AUTOADDRESSING)
extern void mlu_AutoAddressingStep(ml_uint8 StepNumber);
#endif /* HAS_LIN_AUTOADDRESSING */

extern void mlu_MessageReceived(ml_MessageID MessageIndex);
extern void mlu_DataRequest(ml_MessageID MessageIndex);

extern void mlu_DataTransmitted(void);
extern void mlu_ErrorDetected(ml_LinError Error);
extern void mlu_LinSleepMode(ml_StateReason Reason);
#pragma GCC poison mlu_LinStateChanged   /* use mlu_LinSleepMode instead */


/* Flash reprogramming */
extern ml_Status mlu_ApplicationStop(void);

/*
 * LIN Interrupt handler
 *
 * Notes:
 *  1. SW Platform provides default LIN Interrupt handler (ml_LinInterruptHandler) which calls
 *     ml_GetLinEventData and then ml_ProccessLinEvent.
 *  2. In multithreaded (RTOS) environment the processing of LIN event (call to ml_ProccessLinEvent)
 *     can be moved to a dedicated thread/task. In this case application should provide own
 *     ml_LinInterruptHandler which should contain a call to ml_GetLinEventData and IPC signaling
 *     to LIN processing task which eventually calls ml_ProccessLinEvent.
 */
void ml_LinInterruptHandler (void);
void linit (void); /* alias of ml_LinInterruptHandler for compatibility with old vector files */

void ml_GetLinEventData (void);     /* Copies LIN event data from the shared memory to the private structures */
void ml_ProccessLinEvent (void);    /* Processes LIN event and invokes mlu_ call-back functions */

#endif /* MLX16_LIN_H_ */
