/*
 * Test application
 * 
 * Test frames are defined in busMaster.lin (LIN Commander script) and
 * shall be sent using LIN Commander/USB LIN Master.
 *
 * Copyright (C) 2007 Melexis N.V.
 */

#include "lin_api.h"
#include <plib.h>       /* product libs */

/*
 * Application version
 */
#define __APP_VERSION_MAJOR__      1UL
#define __APP_VERSION_MINOR__      0UL
#define __APP_VERSION_REVISION__   3UL

const uint32 application_version __attribute__((used, section(".app_version"))) = 
    (__APP_VERSION_MAJOR__) |
    (__APP_VERSION_MINOR__ << 8) |
    (__APP_VERSION_REVISION__ << 16);


/*
 * Serial Number definition
 */
#define ML_SERIAL_NUMBER  0xFEEDBEEFul      /* serial number */

static l_u8 br_level;


/*
 *****************************************************************************
 *  Main
 *****************************************************************************
 */
int main (void)
{
    l_sys_init();
    l_ifc_init_i1();
    l_ifc_connect_i1();

    for (;;) {
         WDG_Manager();     /* Restart watchdog */
         
         if (l_flg_tst_doorFrontLeft_brightLevel()) {
             l_flg_clr_doorFrontLeft_brightLevel();
             
             l_u8 new_br_level = l_u8_rd_doorFrontLeft_brightLevel();   /* get brightness level */
             if (new_br_level != br_level) {                            /* if different from previous value .. */
                br_level = new_br_level;
                l_u8_wr_doorFrontLeft_brightFeedback(new_br_level);     /* .. send it back in event-triggered frame */
             }
             /* else: same brightness is requested */
         }
    }
    return 0;
}


/*
 *****************************************************************************
 *  ld_serial_number_callout
 *
 * Call out from LIN driver to get Serial Number of the device
 *****************************************************************************
 */
void ld_serial_number_callout (l_u8 data[4])
{
    data[0] = (l_u8)ML_SERIAL_NUMBER;           /* LSB */
    data[1] = (l_u8)(ML_SERIAL_NUMBER >> 8);
    data[2] = (l_u8)(ML_SERIAL_NUMBER >> 16);
    data[3] = (l_u8)(ML_SERIAL_NUMBER >> 24);   /* MSB */
}


/*
 *****************************************************************************
 *  LIN API event: mlu_ApplicationStop
 *****************************************************************************
 */
ml_Status mlu_ApplicationStop(void)
{
	return ML_SUCCESS;  /* return that the application has stopped */
}

/* EOF */
