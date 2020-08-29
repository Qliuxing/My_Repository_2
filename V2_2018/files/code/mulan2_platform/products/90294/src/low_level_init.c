/*
 * Copyright (C) 2008-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <syslib.h>
#include <string.h> /* memcpy */
#include <mmc16_io.h>
#include <ioports.h>
#include <map_eeprom.h>


/*----------------------------------------------------------------------------*/
/*            Software reset undefined User Port Map                          */
static INLINE void reset_X_portMapRegs (void)
{
    ANALOG_BUFFERS_ADC_MIN_MAX_CONF = 0;
    OUTCTRL__OUT_DRV_CTRL = 0;
    SVT_COMP_STACKAT_UNMASK_CTRL = 0;
    OUT_STATUS = 0;
    OUT_CTRL = 0;
    dBz_FSM_INT_CTRL = 0;

    ADCMAX_RESET();
    ADCMIN_RESET();

    SUPVISOR_TIMEOUT = 0;

    SPEED_HYST = 0;
    SPEED_THRESH = 0;
    SpeedGDIDO_v.A = 0;
    SpeedGDIDO_v.B = 0; /* Save */

    DIR_THRESH   = 0;
    DirGDIDO_v.A   = 0;
    DirGDIDO_v.B   = 0; /* Save */

    POLARITIES_CTRL = 0;

    OShap_CKTRIM = 0;

    ADC_ANALOG_CFG = 0;

    PTCW_DATA = 0;

    ANA_SETTLE = 0;

    OSHAP_P_WIDTH = 0;

    /* Add new port registers */
}

void _low_level_init (void)
{
    /* Software reset User Port Map */
    /* Need to correct digital simulation */
    /* May be delete */
    reset_X_portMapRegs();

    /* Activate EEPROM and wait 15us */
    CONTROL2 |= EE_ACTIVE ;
    DELAY_US(15 + 3 /* 20% */);   /* EEPROM can not be accessed during first 15us + 20% */

    /* --- Trimming --------------------------------------------------------- */
    CONTROL  =   OUTA_WE | OUTB_WE | OUTC_WE;  /* grant access to ANA_OUTx registers */

    ANA_OUTA = ee.ANAOUTA.u;
    ANA_OUTB = ee.ANAOUTB.u;
    ANA_OUTC = ee.ANAOUTC.u;

    /* Direction enable in analog, JIRA MLX16107-5 */
    if ( 0 != ee.CONTROL.s.CAMCRANKB )
    {
        /* Cam */
        AnaOutB_IO_v.DIR_EN = 0;
    }
    else
    {
        /* Crank */
        AnaOutB_IO_v.DIR_EN = ee.sh.cr.CONTROL.s.WITHDIRECTION;
    }

    CONTROL &= ~(OUTA_WE | OUTB_WE | OUTC_WE);

    /* --- Patches ---------------------------------------------------------- */
    PATCH0_I =  ee.PATCH0_I;
    PATCH0_A =  ee.PATCH0_A;
    PATCH1_I =  ee.PATCH1_I;
    PATCH1_A =  ee.PATCH1_A;
    PATCH2_I =  ee.PATCH2_I;
    PATCH2_A =  ee.PATCH2_A;
    PATCH3_I =  ee.PATCH3_I;
    PATCH3_A =  ee.PATCH3_A;

    /*
     * Calibrate 1MHz internal clock using CK_TRIM divider
     * This 1MHz clock is used by 15-bit core timer, watchdog and EEPROM
     */
    /* Fixed JIRA16107-23 */
    CK_TRIM       = ee.FREQ_CONTROL.s.Sys_freq_CKTRIM; /*DEF_CK_TRIM*/;
    OShap_CKTRIM  = ee.FREQ_CONTROL.s.Sys_freq_CKTRIM;

    /* TEST OUTPUT */
    Test5_v.Test2_WE = 1;

    Test2_v.TESTMUXSEL_0_2_TEST = ee.TEST_2_L.s.Dig_TO_Sel_Test;
    Test2_v.TESTMUXSEL_0_2_TEST = ee.TEST_2_L.s.TestMuxSel_Test;
    Test2_v.TESTMUXSEL_3_TEST   = ee.TEST_2_H.s.TestMuxSel_Test_3;
    Test2_v.OUT_DISABLE_portmap = ee.TEST_2_H.s.OutDisable_portmap;
    Test2_v.VdigOnTestB_portmap = ee.TEST_2_H.s.VdigOnTestB_portmap;
    if ( 0 == ee.TEST_2_H.s.VdigOnTestB_portmap )
    {
        Test2_v.TEST_ENABLE_portmap = ee.TEST_2_H.s.TestEnable_portmap;
    }

    Test5_v.Test2_WE = 0;

    Test5_v.Test3_WE = 1;

    Test3_v.Dig_TO_SEL_OUT      = ee.TEST_3_L.s.Dig_TO_Sel_Out;
    Test3_v.TESTMUXSEL_OUT      = ee.TEST_3_L.s.TestMuxSel_Out;

    Test5_v.Test3_WE = 0;
}

/* EOF */
