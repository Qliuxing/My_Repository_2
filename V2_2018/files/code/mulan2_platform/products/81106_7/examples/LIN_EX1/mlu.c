/*
 * Copyright (C) 2011-2013 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * LIN minimal implementation (non-conformant!) for test purposes
 *
 *  - only answers to LIN2.0 Product Id request
 *  - LIN2.0 response error is NOT implemented
 */

#include <syslib.h>
#include <lin.h>
#include "lin_cfg.h"
#include "led.h"

/* 
 * Type of response that will be send upon receiving 0x3D (SRF) header.
 * Additional non-diagnostic frames can be sent between 0x3C(MRF) and
 * 0x3D(SRF)
 */
typedef enum {
    respNoResponse = 0x00,
    respLinProductID,      /* answer to Read by Identifier (0) request  */
    respNegativeResponse,  /* answer to non-supported sub-functions request */
} SlaveResponseType;

volatile SlaveResponseType pendingSlaveResponse;


/* ----------------------------------------------------------------------------
 *  LIN init
 */
void LIN_init (void)
{
#if defined (DEBUG_IO)
    LED_Init();
    LED_Off();
#endif /* DEBUG_IO */

    ml_InitLinModule();

#if 0
    ml_SetDefaultBaudRate();  /* fixed baud rate defined by ML_BAUDRATE from <profile>.mk */
#else
    ml_SetAutoBaudRateMode(ML_ABR_ON_EACH_FRAME);
#endif

    /* Configure the Mlx4 software */
    (void)ml_SetOptions (1U,        /* IDStopBitLength = 1.5 Bit (Melexis LIN Master has 1.5 Tbit stop bit */
                    0U,             /* TXStopBitLength = 1 Bit */
                    ML_ENABLED,     /* StateChangeSignal */
                    ML_LIGHTSLEEP   /* SleepMode: lightsleep mode */
                   );
    (void)ml_SetSlewRate(ML_SLEWHIGH);

    pendingSlaveResponse = respNoResponse; /* there was no MRF yet */
    

#if (LIN_PIN_LOADER != 0)
    ml_SetLoaderNAD(MLX_INITIAL_NAD);   /* notify the loader about application's NAD */
#endif /* LIN_PIN_LOADER */

    ml_Connect();
}


/* ----------------------------------------------------------------------------
 *  LIN API event: MessageReceived (slave RX)
 */
void mlu_MessageReceived (ml_MessageID midx)
{
    if (midx == ML_MRF_INDEX)  {      /*------- Diagnostic MRF (assigned to FID=0x3C) --*/
        /*
         * A new MRF received (no matter to what NAD), so reset pending response
         */
        pendingSlaveResponse = respNoResponse; 
        
        /* PDU structure: */
        /* Request:  NAD   PCI    SID   D1   D2   D3   D4   D5  */
        /* Response: NAD   PCI   RSID   D1   D2   D3   D4   D5  */
        if ( ((LinFrameDataBuffer[0] == MLX_INITIAL_NAD) || (LinFrameDataBuffer[0] == 0x7F)) /* NAD or Broadcast .. */
           && (LinFrameDataBuffer[1] == 0x06) /* .. correct PCI */
           && (LinFrameDataBuffer[2] == 0xB2) /* .. SID is Read by Identifier */
        )
        {
            uint8  Q_ID;     /* Question */
            uint16 supp_id;
            uint16 func_id;

            Q_ID    =  LinFrameDataBuffer[3];
            supp_id = ((uint16)LinFrameDataBuffer[5] << 8) | LinFrameDataBuffer[4];
            func_id = ((uint16)LinFrameDataBuffer[7] << 8) | LinFrameDataBuffer[6];

            /* Supplier_ID and Function_ID match */
            if ( ((supp_id == MLX_SUPPLIER_ID) || (supp_id == 0x7FFF))
              && ((func_id == MLX_FUNCTION_ID) || (func_id == 0xFFFF)) ) {

                switch (Q_ID) { /* Question */
                    case 0x00: 	/* LIN Product Identification */
                        pendingSlaveResponse = respLinProductID;
                        break;

                    default:
                        pendingSlaveResponse = respNegativeResponse;
                        break;
                } /* switch Q_ID */
            }
            /* else : Supplier Id or Function Id doesn't match */
        }
        else {      /*  NAD and PCI don't match */ 
        }
    }
    else {              /* Message Index doesn't match any messages in application */
    }
}


/* ----------------------------------------------------------------------------
 *  LIN API event: Data Request (slave TX)
 */
void mlu_DataRequest (ml_MessageID midx)
{
    if (midx == ML_SRF_INDEX) {       /*---- Diagnostic SRF  (assigned to FID=0x3D) --*/
        
        switch (pendingSlaveResponse) {
            case respLinProductID:
                /* Fill the buffer with slave Response */
                /* Response format : NAD PCI RSID D1 D2 D3 D4 D5 */
                LinFrameDataBuffer[0] = MLX_INITIAL_NAD;
                LinFrameDataBuffer[1] = 0x06; /* PCI */
                LinFrameDataBuffer[2] = 0xF2; /* RSID = SID+0x40 = 0xb2+0x40 */

                LinFrameDataBuffer[3] = (uint8) (MLX_SUPPLIER_ID & 0x00FF);
                LinFrameDataBuffer[4] = (uint8) (MLX_SUPPLIER_ID >> 8);    
                LinFrameDataBuffer[5] = (uint8) (MLX_FUNCTION_ID & 0x00FF);
                LinFrameDataBuffer[6] = (uint8) (MLX_FUNCTION_ID >> 8);    
                LinFrameDataBuffer[7] = (uint8)  MLX_VARIANT_ID;           
        
                ml_DataReady(ML_END_OF_TX_DISABLED);     /* Signal to MLX4 */
                break;

            case respNegativeResponse:
                LinFrameDataBuffer[0] = MLX_INITIAL_NAD;
                LinFrameDataBuffer[1] = 0x03;  /* PCI */
                LinFrameDataBuffer[2] = 0x7F;  /* RSID = 0x7F  */ 

                LinFrameDataBuffer[3] = 0xB2;  /* Data1: Requested ID  */
                LinFrameDataBuffer[4] = 0x12;  /* Data2: Error code    */
                LinFrameDataBuffer[5] = 0xFF;  /* Data3                */
                LinFrameDataBuffer[6] = 0xFF;  /* Data4                */
                LinFrameDataBuffer[7] = 0xFF;  /* Data5                */
                
                ml_DataReady(ML_END_OF_TX_DISABLED);     /* Signal to MLX4 */
                break;

            case respNoResponse: /* SRF header received before MRF */
            default:
                ml_DiscardFrame();
                break;  
        }  /* switch (pendingSlaveResponse) */

        pendingSlaveResponse = respNoResponse;  /* reset pending response after processing */
    }
    else {
        ml_DiscardFrame(); /* Message ID is not for this application */
    }
}


/* ----------------------------------------------------------------------------
 *  LIN API event: mlu_ErrorDetected
 */
void mlu_ErrorDetected(ml_LinError Error)
{
    (void)Error;
    /* No error handler */
}


/* ----------------------------------------------------------------------------
 *  LIN API event: mlu_ApplicationStop
 */
ml_Status mlu_ApplicationStop(void)
{
	return ML_SUCCESS;  /* return that the application has stopped */
}


/* ----------------------------------------------------------------------------
 *  LIN API event: mlu_AutoAddressingStep
 */
void mlu_AutoAddressingStep(ml_uint8 StepNumber)
{
    (void)StepNumber;   /* not used */
}


/* ----------------------------------------------------------------------------
 *  LIN API event: mlu_DataTransmitted
 */
void mlu_DataTransmitted(void)
{
    /* empty */
}


/* ----------------------------------------------------------------------------
 *  LIN API event: mlu_LinSleepMode
 */
void mlu_LinSleepMode(ml_StateReason Reason)
{
    (void)Reason;   /* not used */

#if defined (DEBUG_IO)
    LED_Toggle();
#endif /* DEBUG_IO */

    ml_WakeUp();    /* generate wake-up for test purposes */
}


/* EOF */
