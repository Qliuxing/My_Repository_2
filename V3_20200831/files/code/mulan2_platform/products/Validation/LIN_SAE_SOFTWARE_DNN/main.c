/*
 * Copyright (C) 2005-2013 Melexis N.V.
 *
 * MelexCM Software Platform
 *
 */

/*
 ******************************************************************************
 * Title : SAE J2602 example
 *
 * GLOSSARY:
 *
 *  FID  - Frame ID
 *  MRF  - Diagnostic Mater Request Frame (FID = 0x3C)
 *  SRF  - Diagnostic Slave Response Frame (FID = 0x3D)
 ******************************************************************************
 */

#include <syslib.h>
#include <plib.h>       /* product libs */
#include <syslib.h>
#include "lincore.h"

/*
 * Application version
 */
#define __APP_VERSION_MAJOR__      1UL
#define __APP_VERSION_MINOR__      0UL
#define __APP_VERSION_REVISION__   3UL

const uint32 application_version __attribute__((section(".app_version"))) = 
    (__APP_VERSION_MAJOR__) |
    (__APP_VERSION_MINOR__ << 8) |
    (__APP_VERSION_REVISION__ << 16);

/*
 * Serial Number definition
 */
#define ML_SERIAL_NUMBER  0xFEEDBEEFul      /* serial number */

/*
 * SAE_apinfo contains APINFO[0:4] Application Information Filed
 * of the SAE J2602 Status Byte 
 *  
 * Note that APINFO4 (bit4) shall be used to indicate when the application 
 * requires attention from Master Device, e.g. needs to be configured
 */
static ml_uint8 SAE_apinfo;


/*
 * Local functions
 */
static void hw_init(void);


/*
 ******************************************************************************
 *    MAIN
 ******************************************************************************
 */
int main(void)
{
    hw_init();
    ml_SAE_LinInitModule(MLX_INITIAL_NAD);
    ml_Connect();

    SAE_apinfo = _BV(4); /* application requiers attention from Master */

    for (;;) {	
    	WDG_Manager();
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

}


/*
 ******************************************************************************
 * SAE API event: SAE Reset frame has been received
 *
 * Parameters:
 *  dest: ML_TARGETED_RESET or ML_BROADCAST_RESET
 *     
 * Upon receipt of the Reset (Targeted or Broadcast) command, the slave device
 * shall cause an internal reset of operational variables to occur, e.g. mode
 * control  variables, communication error counters, input source
 * re-initialization, output device reinitialization.
 *
 * NB!!! Slave shall not alter any previously configured application
 * level configuration information stored in non-volatile memory.
 * The Reset operation shall not cause the slave to destructively alter
 * any LIN configuration data or addresses stored in non-volatile memory.
 * Also, the Reset operation shall not cause any configuration parameters
 * in the LIN Data Link device (LIN module/UART) to be altered. Upon conclusion
 * of the Reset operation, the slave device shall remain configured, and shall
 * assume a state consistent with a power-on initialization, with exception
 * of the LIN configuration information.
 ******************************************************************************
 */
ml_ResetStatus mlu_SAE_Reset (ml_ResetDestination dest) {
    

    if (ML_TARGETED_RESET == dest) {
        /* Action for Targeted reset ... */
    }
    else {
        /* Action for Broadcast reset ... */
    }

    /* Some common action here ... */
    SAE_apinfo = 0;

    return ML_RESET_ACCEPTED;
}

/*
 ******************************************************************************
 *  SAE API event: User Application message has been received
 ******************************************************************************
 */
void mlu_SAE_MessageReceived (ml_MessageID midx)
{
    switch (midx) {
        case msg_SUB1:                                  /* received M2S frame, 8 bytes */
            if (   (LinFrameDataBuffer[0] == 'M')
                && (LinFrameDataBuffer[1] == 'E')
                && (LinFrameDataBuffer[2] == 'L')
                && (LinFrameDataBuffer[3] == 'E')
                && (LinFrameDataBuffer[4] == 'X')
                && (LinFrameDataBuffer[5] == 'I')
                && (LinFrameDataBuffer[6] == 'S') )
            {
                /* Placeholder for the Frame Hamdler ... */
            }
            break;

        case msg_SUB2:                                  /* received M2S frame, 2 bytes */
            (void)LinFrameDataBuffer[0];                /* process data from the frame */
            (void)LinFrameDataBuffer[1];


        default:                                        /* received unkown M2S frame */
            break;

    }
}


/*
 ******************************************************************************
 * SAE API event: User Application message has been requested
 *
 * NOTE ! : LinFrameDataBuffer[0] shall not be used, since
 *          it will be overwritten by SAE J2602 Status Byte
 ******************************************************************************
 */
void mlu_SAE_DataRequest (ml_MessageID midx)
{
    switch (midx) {
        case msg_PUB1:                      /*  TX, 2 bytes */

            /* skip LinFrameDataBuffer[0] since contains SAE J2602 Status Byte */
            LinFrameDataBuffer[1] = 0xFF;  /* TX some data; unused bits SHALL be
                                            * set to recessive (1) level
                                            */
            ml_SAE_DataReady(SAE_apinfo);
            break;

        case msg_PUB2:                      /*  TX, 2 bytes */

            /* skip LinFrameDataBuffer[0] since contains SAE J2602 Status Byte */
            LinFrameDataBuffer[1] = 0x11;  /* TX some data; unused bits SHALL be
                                            * set to recessive (1) level
                                            */
            ml_SAE_DataReady(SAE_apinfo);
            break;

        default:
            ml_DiscardFrame();              /* unknown message index */
            break;

    }
}

/*
 ******************************************************************************
 * SAE API event: User Application message has been transmitted
 ******************************************************************************
 */
void mlu_SAE_DataTransmitted (void)
{
    /* 
     * This notification can be used to clear after transmission
     * bits in application information field APINFO[3:0]
     */
}


/*
 ******************************************************************************
 * Melexis LIN API event: mlu_ApplicationStop
 *
 * This is request from Flash Loader to stop the application,
 * e.g. save data and disable all interrupts except LIN ISR.
 ******************************************************************************
 */
ml_Status mlu_ApplicationStop(void)
{
	return ML_SUCCESS;  /* return that the application has stopped */
}

/*
 ******************************************************************************
 * Melexis LIN API event: mlu_AutoAddressingStep
 ******************************************************************************
 */
void mlu_AutoAddressingStep(ml_uint8 StepNumber)
{
    (void)StepNumber;   /* unused parameter */
}

/*
 *****************************************************************************
 *  ld_serial_number_callout
 *
 * Call out from LIN driver to get Serial Number of the device
 *****************************************************************************
 */
void ld_serial_number_callout (ml_uint8 data[4])
{
    data[0] = (ml_uint8)ML_SERIAL_NUMBER;           /* LSB */
    data[1] = (ml_uint8)(ML_SERIAL_NUMBER >> 8);
    data[2] = (ml_uint8)(ML_SERIAL_NUMBER >> 16);
    data[3] = (ml_uint8)(ML_SERIAL_NUMBER >> 24);   /* MSB */
}

/* EOF */
