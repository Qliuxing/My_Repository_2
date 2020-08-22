/*
 * Copyright (C) 2005-2013 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * LIN conformance test application (for IHR LIN1.3 test suite)
 *
 */

#include <syslib.h>
#include <plib.h>       /* product libs */

#include <lin.h>

#include "hal.h"
#include "config.h"

/*
 * Application version
 */
const uint32 application_version __attribute__((section(".app_version"))) = 
    (__APP_VERSION_MAJOR__) |
    (__APP_VERSION_MINOR__ << 8) |
    (__APP_VERSION_REVISION__ << 16);

/*
 * Local functions
 */
static void hw_init(void);
static void LIN_init(void);

/*
 * Global variables
 */
volatile ml_uint8 value = STAT_SLAVE_DEF;

/*
 ******************************************************************************
 *    MAIN
 ******************************************************************************
 */
int main (void)
{
    hw_init();
    LIN_init();

    while (1) {	
        WDG_Manager();  /* Restart watchdog */

        NOP();
        NOP();
	}

    return 0;
}

/*
 ******************************************************************************
 *  Hardware init
 ******************************************************************************
 */
static void hw_init(void)
{
    LED2_OFF();
}

extern void ml_LinInit(void);
/*
 ******************************************************************************
 *  LIN init
 *
 ******************************************************************************
 */
static void LIN_init(void)
{
#if 0
    ml_InitLinModule();
    ml_SetDefaultBaudRate(); /* baudrate is defined by ML_BAUDRATE platform's variable */

    /* Configure the Mlx4 software
	 * IDStopBitLength : 0 (default)/1/2/3 -> 1 / 1.5 / 2 / 2.5 stop bits
	 * TXStopBitLength : 0 (default)/1 -> 1 stop bit / 2
	 * StateChangeSignal : enabled (default)/disabled
	 * SleepMode:
     *        0 : lightsleep, no timeout / 1 (default) : lightsleep, timeout
	 *        2 : deepsleep,  no timeout / 3 :           deepsleep, timeout
     * SlewRate: ML_SLEWHIGH=20kbps/ML_SLEWLOW=10kbps/ML_SLEWFAST=max (fast protocol)
     */
    ml_SetOptions( 0, 0, ML_ENABLED, 1, ML_SLEWHIGH );
#endif
    ml_LinInit();
    ml_Connect();
}


/*
 ******************************************************************************
 *  LIN API event: MessageReceived (slave RX)
 ******************************************************************************
 */
void mlu_MessageReceived(ml_MessageID midx)
{
    LED2_OFF();

    switch (midx) {
        case FRM_MASTER:            /* FRM_MASTER (alias FRM_MASTER4, FRM_M2S_2A): slave RX - 4 bytes */
        case FRM_M2S_2A:
        case FRM_MASTER2:           /* FRM_MASTER2: slave RX 2 bytes */
            value = LinFrameDataBuffer[0];
            break;

        default:
            break;
    }
}

/*
 ******************************************************************************
 *  LIN API event: Data Request (slave TX)
 ******************************************************************************
 */
void mlu_DataRequest (ml_MessageID midx)
{
    LED2_OFF();
    
    switch (midx) {
        case FRM_SLAVE:                             /* FRM_SLAVE  - slave TX - 2 bytes */
            LinFrameDataBuffer[0] = value;
            LinFrameDataBuffer[1] = 0xFF;           /* report Lin error in this byte */
            ml_DataReady(ML_END_OF_TX_DISABLED);
            break;

        case FRM_SLAVE4:                            /* FRM_SLAVE4 (alias) - slave TX - 4 bytes */
            LinFrameDataBuffer[0] = 0xFF;           /* fill data according to STAT_SLAVE_BET in *.tpd file */
            LinFrameDataBuffer[1] = 0xFF;
            LinFrameDataBuffer[2] = 0xFF;
            LinFrameDataBuffer[3] = 0xFC;
            ml_DataReady(ML_END_OF_TX_DISABLED);
            break;

        case FRM_SLAVE8:                            /* FRM_SLAVE8 - slave TX - 8 bytes */
            LinFrameDataBuffer[0] = 0x00;
            LinFrameDataBuffer[1] = 0x01;
            LinFrameDataBuffer[2] = 0x02;
            LinFrameDataBuffer[3] = 0x03;
            LinFrameDataBuffer[4] = 0x04;
            LinFrameDataBuffer[5] = 0x05;
            LinFrameDataBuffer[6] = 0x06;
            LinFrameDataBuffer[7] = 0x07;
            ml_DataReady(ML_END_OF_TX_DISABLED);
            break;

        case FRM_S2M_13:                            /* FRM_S2M_13 - slave TX - 2 bytes */
        case FRM_S2M_14:                            /* FRM_S2M_14 - slave TX - 2 bytes */
        case FRM_S2M_15:                            /* FRM_S2M_15 - slave TX - 2 bytes */
            LinFrameDataBuffer[0] = midx;
            LinFrameDataBuffer[1] = 0x55;
            ml_DataReady(ML_END_OF_TX_DISABLED);
            break;
        
        default:
            ml_DiscardFrame();                      /* message idx is not supported */
            break;
    }
}

/*
 ******************************************************************************
 *  LIN API event: mlu_ErrorDetected
 ******************************************************************************
 */
void mlu_ErrorDetected(ml_LinError Error)
{
    switch (Error) {
        case ml_erLinModuleReset:
            /* non-recoverable failure has occurred in the LIN Module
             * should switch to System Mode and reinitialize LIN module
             */
            break;

        case ml_erSynchField:
            NOP(); /* nothing to do yet */
            break;

         case ml_erIdParity:
            NOP(); /* nothing to do yet */
            break;

        case ml_erDataFraming:
            NOP(); /* nothing to do yet */
            break;

        case ml_erCheckSum:
            NOP(); /* nothing to do yet */
            break;

        case ml_erTimeOutResponse:
            LED2_ON();
            break;

        case ml_erBit:
        case ml_erStopBitTX:
            NOP(); /* nothing to do yet */
            break;

        default:
            /* nothing to do yet */
            break;
    }
}


/*
 ******************************************************************************
 *  LIN API event: mlu_ApplicationStop
 ******************************************************************************
 */
ml_Status mlu_ApplicationStop(void)
{
	return ML_SUCCESS;  /* return that the application has stopped */
}

/*
 ******************************************************************************
 *  LIN API event: mlu_AutoAddressingStep
 ******************************************************************************
 */
void mlu_AutoAddressingStep(ml_uint8 StepNumber)
{
    (void)StepNumber;   /* unused parameter */
}

/*
 ******************************************************************************
 *  LIN API event: mlu_DataTransmitted
 ******************************************************************************
 */
void mlu_DataTransmitted(void)
{

}


/*
 ******************************************************************************
 *  LIN API event: mlu_LinSleepMode
 ******************************************************************************
 */
void mlu_LinSleepMode(ml_StateReason Reason)
{
    (void)Reason;   /* not used */
}

/* EOF */
