/*
 * Copyright (C) 2011-2015 Melexis N.V.
 *
 * Software Platform
 *
 */

/*
 * LIN test example
 *
 * Notes:
 *  1. Generates PWM waveform on pin PWM1 with frequency equal to FPLL/1000 =
 *     = 20MHz/1000 = 20 KHz (i.e. period is 50us) to check RC oscillator
 *     trimming and PLL activation/stability. Duty cycle is 25%.
 *
 *  2. Set unique NAD and blink signal on IO.
 *
 */

#include <syslib.h>
#include <plib.h>       /* product libs */
#include <pwmlib.h>

#include "nvram.h"
#include <lin.h>

/*
 * Local functions
 */
static void hw_init(void);
static void LIN_init (void);


/* IO4 and IO5 */
#define  IO_INIT()            (IO_EXTIO  =  0x11)
#define  IO_FAIL_ON()         (IO_EXTIO |=  0x20) /* IO4 */
#define  IO_FAIL_OFF()        (IO_EXTIO &= ~0x20)
#define  IO_STOP_ON()         (IO_EXTIO |=  0x02) /* IO5 */
#define  IO_STOP_OFF()        (IO_EXTIO &= ~0x02)
#define  IO_FAIL_TOGGLE()     (IO_EXTIO ^=  0x20)
#define  IO_STOP_TOGGLE()     (IO_EXTIO ^=  0x02)


/*
 *
 */
int main (void)
{
    hw_init();
    LIN_init();

    while (1) {         /* idle loop */

        WDG_Manager();  /* Restart watchdog */

        MSEC_DELAY(1000/UNIQUE_NAD);
        IO_FAIL_TOGGLE();
        IO_STOP_TOGGLE();
    }
    return 0;
}


/* ----------------------------------------------------------------------------
 *  Hardware init
 */
static void hw_init (void)
{
    /* IO init */
    /* setup IO4, IO5 as output */
    IO_INIT();

    /* setup initial value for output port IO0 and IO1 */
    IO_FAIL_ON();
    IO_STOP_OFF();
}


/* ----------------------------------------------------------------------------
 *  LIN init
 */
void LIN_init (void)
{
    (void)ml_InitLinModule();

    ml_SetDefaultBaudRate();  /* baudrate is defined by ML_BAUDRATE platform's variable */

    /* Configure the Mlx4 software
     *   IDStopBitLength : 0 (default)/1/2/3 -> 1 / 1.5 / 2 / 2.5 stop bits
     *   TXStopBitLength : 0 (default)/1     -> 1 / 2 stop bits
     *   StateChangeSignal : enabled (default)/disabled
     *   SleepMode: ML_LIGHTSLEEP
     */
    (void)ml_SetOptions (1U,        /* IDStopBitLength = 1.5 Bit (Melexis LIN Master has 1.5 Tbit stop bit */
                    0U,             /* TXStopBitLength = 1 Bit */
                    ML_ENABLED,     /* StateChangeSignal */
                    ML_LIGHTSLEEP   /* SleepMode: lightsleep mode */
                   );
    (void)ml_SetSlewRate(ML_SLEWHIGH);

    /* Set unique NAD for Loader */
    (void)ml_SetLoaderNAD(UNIQUE_NAD);

    (void)ml_Connect();
}


/* ----------------------------------------------------------------------------
 *  LIN API event: MessageReceived (slave RX)
 */
void mlu_MessageReceived (ml_MessageID midx)
{
    (void)midx;
}


/* ----------------------------------------------------------------------------
 *  LIN API event: Data Request (slave TX)
 */
void mlu_DataRequest (ml_MessageID midx)
{
    (void)midx;
    ml_DiscardFrame();
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
}


ml_Status    mlu_CheckPowerSupply(void)
{

    return ML_FAILURE;
}


/* EOF */
