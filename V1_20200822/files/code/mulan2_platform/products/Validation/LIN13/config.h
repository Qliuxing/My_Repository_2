/*
 * Copyright (C) 2005-2012 Melexis N.V.
 *
 * Software Platform
 *
 */
#ifndef CONFIG_H_
#define CONFIG_H_

/* 
 ******************************************************************************
 *               Values that CAN be changed by the user
 ******************************************************************************
 */

/*
 * Application version
 */
#define __APP_VERSION_MAJOR__      1UL
#define __APP_VERSION_MINOR__      3UL
#define __APP_VERSION_REVISION__   1UL

/* 
 * ----------------------------------------------------------------------------
 *              LIN configuration
 * ----------------------------------------------------------------------------
 */

#define     STAT_SLAVE_DEF      0xE0    /* { 0xE0, } DataByte 0: from slave in default state */

/*--- Application frames -----------------------------------------------------------------------------*/
#define     FRM_MASTER          0x21    /* Master command frame: Slave RX - Subscriber (alias FRM_MASTER4) */
#define     FRM_SLAVE           0x01    /* Slave status frame: Slave TX - Publisher */
#define     FRM_MASTER2         0x04    /* ID of subscribed 2 byte frame */
#define     FRM_SLAVE2          0x01    /* ID of published 2 byte frame */
#define     FRM_SLAVE4          0x20    /* ID of published 4 byte frame */
#define     FRM_SLAVE8          0x32    /* ID of published 8 byte frame */
#define     FRM_SLAVE_BET       0x20    /* Frame ID used for bit error testing -- 4 bytes */

/* Frames for test case 4.1.2_IUTspecific (TC_401.TDF) */
#define     FRM_S2M_13          0x13    
#define     FRM_S2M_14          0x14
#define     FRM_S2M_15          0x15
#define     FRM_M2S_2A          0x2A

#endif /* CONFIG_H_ */
