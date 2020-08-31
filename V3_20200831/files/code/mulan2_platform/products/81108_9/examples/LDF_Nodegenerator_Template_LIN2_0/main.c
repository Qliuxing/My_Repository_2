/**********************************************
* Copyright (C) 2009-2015 Melexis N.V.
*
* LDF_Nodegenerator_Template_LIN2_x
*
**********************************************/
/* History:
     Revision 1.0
       - Initial release
     Revision 1.1
       - Added support for MLX4 monitoring
       - Added function GoToSleep()
 **********************************************/
/*
 ******************************************************************************
 *
 * Content: Example code for using the LDF node generator with LIN2.x
 *
 *
 * ATTENTION: Please enable "#define HAS_SAVE_CONFIGURATION_SERVICE" in file lin_api.h
 *            after building the LIN driver (gmake_drv) and before building the
 *            application(gmake all)!!
 *******************************************************************************
 */

/* ==========================================================================
 * Includes
 * ========================================================================== */
#include <alib.h>
#include <plib.h>
#include <nvram.h>
#include "lin_api.h"
#include "adc.h"

/* ----------------------------------------------------------------------------
 * Application version
 */
#define __APP_VERSION_MAJOR__      1UL
#define __APP_VERSION_MINOR__      1UL
#define __APP_VERSION_REVISION__   0UL

const uint32 application_version  __attribute__((section(".app_version"))) =
    (__APP_VERSION_MAJOR__) |
    (__APP_VERSION_MINOR__ << 8) |
    (__APP_VERSION_REVISION__ << 16);

/*
 * Serial Number is stored in NVRAM memory - here ram only
 */
ml_uint8 ml_SerialNumber[4];                                                                                             /* 4 bytes LSB..MSB */

/*--- LIN Product Identification --------------------------------------*/
ml_uint8 data_config[1 + ML_NUMBER_OF_DYNAMIC_MESSAGES] __attribute__((aligned(2))) = ML_NODE_CONFIGURATION_INITIALIZER; /* NAD + ML_NUMBER_OF_DYNAMIC_MESSAGES */
ml_uint8 length = ML_NUMBER_OF_DYNAMIC_MESSAGES + 1;

/* Counter for MLX4 monitoring */
uint16 g_u16Mlx4StateCheckCounter = 0;

/*--- Global Variables --------*/
#define NVRAM1_PAGE1_FIRST_ADDRESS   0x1000
#define NVRAM2_PAGE1_FIRST_ADDRESS   0x1100

#define NV_ADDR_VALID_CONF_PATTERN   (NVRAM2_PAGE1_FIRST_ADDRESS)
#define NV_ADDR_LIN_CONF             (ml_uint16)(NVRAM2_PAGE1_FIRST_ADDRESS + 2)

#define NVRAM_CONFIG_VALID           0x5A5A
extern volatile uint16 NV_valid_conf_pattern __attribute((nodp, addr(NV_ADDR_VALID_CONF_PATTERN)));
extern volatile uint16 NV_LIN_conf[1 + ML_NUMBER_OF_DYNAMIC_MESSAGES] __attribute((nodp, addr(NV_ADDR_LIN_CONF)));

/*
 * Module's functions
 */
static void lin_status_handler (void);
static void init_HW (void);
void initMulan2Timer(void);
void initWatchdog(void);
void GoToSleep(void);

/****************************************************//**
* Function name: main
********************************************************/
int main (void)
{
    /* init hardware */
    init_HW();

    /* init LIN  */
    l_sys_init();
    l_ifc_init_i1();

    /* Check if there is a valid LIN configuration stored in the NVRAM */
    if (NV_valid_conf_pattern == NVRAM_CONFIG_VALID) {
        for (ml_uint8 i = 0; i < length; i++) {
            data_config[i] = (ml_uint8)(NV_LIN_conf[i]);
        }
    }

    ld_set_configuration(0, data_config, length);

    for (;;) {
        /* Restart watchdog */
        WDG_Manager();

        if (l_flg_tst_SigPWM_HV0_DutyCycle()) {
            l_flg_clr_SigPWM_HV0_DutyCycle();
            /* set duty cycle for PWM channel HV0*/
            PWM1_HT = (uint8)(~(l_u8_rd_SigPWM_HV0_DutyCycle()));
            PWM1_LT = 0x00;
        }
        if (l_flg_tst_SigPWM_HV1_DutyCycle()) {
            l_flg_clr_SigPWM_HV1_DutyCycle();
            /* set duty cycle for PWM channel HV1*/
            PWM2_HT = (uint8)(~(l_u8_rd_SigPWM_HV1_DutyCycle()));
            PWM2_LT = 0x00;
        }

        if (l_flg_tst_SigPWM_HV2_DutyCycle()) {
            l_flg_clr_SigPWM_HV2_DutyCycle();
            /* set duty cycle for PWM channel HV2*/
            PWM3_HT = (uint8)(~(l_u8_rd_SigPWM_HV2_DutyCycle()));
            PWM3_LT = 0x00;
        }

        if (l_flg_tst_SigADCIntVS()) {
            l_flg_clr_SigADCIntVS();
            l_u8_wr_SigADCIntVS((getADCVoltage(ADC_SIN_VS_FOURTEENTH | ADC_SREF_2V5) >> 2) & 0x00FF);
        }

        lin_status_handler();
    }

    return 0;
} /* main */

/*
 *****************************************************************************
 * lin_status_handler
 *
 * Handles status information from LIN driver : bus activity, save
 * configuration request etc
 *****************************************************************************
 */
static void lin_status_handler (void)
{
    l_u16 tmp_u16;

    tmp_u16 = l_ifc_read_status_i1();

    if (tmp_u16 & ML_IFC_SAVE_CONFIGURATION) {
        unsigned int i; /* iterator */

        ld_read_configuration_i1(data_config,&length);

        for (i = 0; i < length; i++) {
            NV_LIN_conf[i] = (ml_uint16)(data_config[i]);
        }

        NV_valid_conf_pattern = NVRAM_CONFIG_VALID;  /* Set the pattern for a valid LIN configuration in the NVRAM */

        NVRAM_SavePage(NVRAM2_PAGE1);                /* Save the SRAM content into the NVRAM */

    }
    /* else : ignore othe status flags */

    /* monitor the MLX4 LIN controller */
    if (tmp_u16 & ML_IFC_BUS_ACTIVITY) {
        g_u16Mlx4StateCheckCounter = 0;
    } 
    
    if (g_u16Mlx4StateCheckCounter > 250){   /* check MLX4 status after 250 x 5ms without LIN activity */

        g_u16Mlx4StateCheckCounter = 0;

        DISABLE_MLX4_INT();

        if ( ml_GetState(ML_CLR_LIN_BUS_ACTIVITY) == ml_stINVALID ) {   /* in case the MLX4 status is invalid, switch to sleep mode */
      		  GoToSleep();  /* switch to sleep mode */
    	  }
        ENABLE_MLX4_INT();
    };


} /* lin_status_handler */

/****************************************************//**
* Function name: init_HW
********************************************************/
static void init_HW (void)
{
    initMulan2Timer(); /* init the Mulan 2 core timer */
    initWatchdog();    /* init the analog watchdog timing */
    initADC();         /* init the ADC */

    /* configure HV0,HV1,HV2 as 18mA current outputs and HV3 as open drain output */
    HV_SET_CURRENTSOURCES(HV0CURRENT_18mA,HV1CURRENT_18mA,HV2CURRENT_18mA,HV3CURRENT_ODMODE);

    /*
     * #define PWM1_INIT(M, N, PER, LT, HT, CMP, ECI, EPI, MODE)
     *
     *             Fck
     * Fpwm = -------------- ,
     *              N
     *         M * 2  * PERIOD
     *
     *   Fck = system frequency 24 MHz
     *   M = [1..16] predivider, PSCL[7:4] = M-1
     *   N = [0..11] predivider, PSCL[3:0] = N
     *   PERIOD = [1..0xFFFF] period, PER[0:15] = PERIOD-1
     *
     *   LT[15:0] = Lower threshold register -  (lower threshold < high threshold)
     *   HT[15:0] = Higher threshold register -  (higher threshold < PERIOD)
     *   CMP[15:0] - Comparator threshold register
     *   ECI  = 0/1  - disable/enable compare interrupt generation
     *   EPI  = 0/1  - disable/enable interrupt when PWM counter gets equal 0
     *   MODE = 0/1  - independent/mirror mode
     *
     */

    /* Set PWM Frequency to 294Hz@8bit resolution (M=10,N=5,PER=0xFF)*/
    PWM1_INIT(10,5,0x00FF,0,0xFF,0,0,0,0);
    PWM2_INIT(10,5,0x00FF,0,0xFF,0,0,0,0);
    PWM3_INIT(10,5,0x00FF,0,0xFF,0,0,0,0);

    PWM1_CONNECT_HV0();               /* connect PWM block 1 to I/O pin HV0 */
    PWM2_CONNECT_HV1();               /* connect PWM block 2 to I/O pin HV1 */
    PWM3_CONNECT_HV2();               /* connect PWM block 3 to I/O pin HV2 */

} /* init_HW */

/****************************************************//**
* Function name: initMulan2Timer
********************************************************/
/*!

 */
void initMulan2Timer(void){
    /* init Timer for time base 5ms */
    CORE_TIMER_INIT(5000);

    /* Enable Timer interrupt with priority 5 */
    CORE_TIMER_INT_ENABLE(5);

} /* initMulan2Timer */

/****************************************************//**
* Function name: initWatchdog
********************************************************/
/*!

 */
void initWatchdog(void){

    /* initialize analog watchdog to 100ms */
    /* awdg_init (uint8 prescaler, uint8 timer) */
    /* AWD_timeout = 1/10 kHz * Prescaler *  Timer */
    /* 100ms = 0.0001s * 4 * 250 */
    awdg_init(AWDG_DIV_4,250);

} /* initWatchdog */

/*
 *****************************************************************************
 *  ld_serial_number_callout
 *
 * Call out from LIN driver to get Serial Number of the device
 *****************************************************************************
 */
void ld_serial_number_callout (l_u8 data[4])
{
    data[0] = ml_SerialNumber[0];   /* LSB */
    data[1] = ml_SerialNumber[1];
    data[2] = ml_SerialNumber[2];
    data[3] = ml_SerialNumber[3];   /* MSB */
} /* ld_serial_number_callout */

#if defined (HAS_READ_BY_ID_CALLOUT)
/*
 *****************************************************************************
 *  ld_read_by_id_callout
 *
 *  Call out from LIN driver to serve user defined read_by_ID requests
 *  Return values:
 *      LD_NEGATIVE_RESPONSE
 *      LD_POSITIVE_RESPONSE
 *      LD_NO_RESPONSE
 *
 *****************************************************************************
 */
l_u8 ld_read_by_id_callout (l_u8 id, l_u8* data)
{
    /*
     * id values are in range from 0x20 to 0x3F (32 to 63),
     * their meanings are application dependent
     */
    ml_uint16 vers;

    vers = ml_LinModuleVersion();

    if (id == 0x20) {
        data[0] = __APP_VERSION_MAJOR__;
        data[1] = __APP_VERSION_MINOR__;
        data[2] = __APP_VERSION_REVISION__;
        data[3] = (ml_uint8) (vers); /* LSB: LIN mlx4 firmware version */;
        data[4] = (ml_uint8) (vers >> 8); /* MSB: Flash Loader version */;
        return LD_POSITIVE_RESPONSE;
    }else if (id == 0x3F) {
        return LD_NO_RESPONSE;
    }else {
        return LD_NEGATIVE_RESPONSE;
    }
} /* ld_read_by_id_callout */
#endif /* HAS_READ_BY_ID_CALLOUT */

/*
 *****************************************************************************
 *  l_ifc_sleep_entered_callout
 *
 *
 *
 *****************************************************************************
 */
void l_ifc_sleep_entered_callout(ml_StateReason Reason)
{
    switch (Reason) {
      /*
       * "Go-to-Sleep" frame has been received from Master
       */
      case ml_reasonMaster:

        GoToSleep();  /* switch to sleep mode */
        break;

      /*
       * LIN cluster is still inactive after 3 wake-up pulses
       *
       */
      case ml_reasonWakeupAbort:
      /*
       * LIN bus was inactive for 4 seconds without receiving an explicit
       * "Go-to-Sleep frame". This can be considered as a failure of the Master or
       * PHY layer. Slave can enter limp-home mode.
       */
      case ml_reasonTimeOut:
      case ml_reasonTimeOutDominant:
        /*
         * 1. Debugger
         * The following two commands are a workaround to avoid a goto sleep after 4s of LIN bus inactivity
         * using the debugger. They must be commented in case the debugger is not used and the
         * device should switch into sleep mode after 4s of LIN bus inactivity.
         */
        (void)ml_Disconnect();
        (void)ml_Connect();

        /*
         * 2. Real application
         * The following commands have to be enabled in a real application to
         * switch the device into sleep mode after 4s of LIN bus inactivity.
         */
//		  GoToSleep();  /* switch to sleep mode */
        break;

      default:
        break;
    } /* switch */

} /* l_ifc_sleep_entered_callout */

/****************************************************//**
* Function name: mlu_ApplicationStop
********************************************************/
/*!
   This notification is called when a diagnostic command to enter Programming mode is received by
   the LIN boot Loader firmware. Upon receiving this notification, the user application shall stop
   execution of background main loop and return ML_SUCCESS status if it is possible to switch to
   Flash Loader firmware right now. Note that during servicing Flash Loader requests all interrupts
   except LIN will be disabled. If application decides that it is not possible to stop right now, then
   ML_FAILURE status shall be returned. In this case the request to enter Programming Mode will
   be discarded.
 */
ml_Status mlu_ApplicationStop(void)
{
    MLX16_MASK_ALL_INT(); /* disable all interrupts */
    ENABLE_MLX4_INT();    /* enable the LIN interrupt */

    ADC_STOP_SEQUENCE();  /* stop ADC sequence */

    /* check if a NVRAM operation is ongoing and wait until it's finished */
    while ((NV_CTRL & NV_BUSY) != 0) {
        WDG_Manager();    /* executed the watchdog manager */
    }

    return ML_SUCCESS;    /* return that the application has stopped */
} /* mlu_ApplicationStop */

/****************************************************//**
* Function name: GoToSleep
********************************************************/
/*!
   Switch the MLX8110x into sleep mode
 */
void GoToSleep(void)
{
	mlu_ApplicationStop(); /* stop the application */

	MLX4_RESET();          		/* stop the MLX4 */
    MLX8110x_USEC_DELAY(500);	/* wait 500us */
	MLX16_HALT();          		/* stop the MLX16 */
	while (1) {
		  ;
	}
} /* GoToSleep */

/* EOF */
