/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef LIB_MLX8110X_LIN_SPECIALMODE_H_
#define LIB_MLX8110X_LIN_SPECIALMODE_H_

#include "ioports.h"

/* LIN_EN .. Register: LIN_XCFG .. Bit 0-1 */
#define LIN_EN_X_CLEAR (uint16)~(3u << 0)
#define LIN_EN_XPRO             (1u << 0)
#define LIN_EN_XPHY             (1u << 1)

/* LIN_DISTERM .. Register: LIN_XCFG .. Bit 2 */
#define LIN_DISTERM             (1u << 2)

/* LIN_XOUT_INV .. Register: LIN_XCFG .. Bit 3 */
#define LIN_XOUT_INV            (1u << 3)

/* BYPASS .. Register: LIN_XCFG .. Bit 4 */
#define BYPASS                  (1u << 4)

/* HSM .. Register: LIN_XCFG .. Bit 5 */
#define HSM                     (1u << 5)

/* LSM .. Register: LIN_XCFG .. Bit 6 */
#define LSM                     (1u << 6)

/* SLEEPB_LIN .. Register: LIN_XCFG .. Bit 7 */
#define SLEEPB_LIN              (1u << 7)
                                
/* LIN_XPRO_ACT .. Register: LIN_XCFG .. Bit 8 */                                
#define LIN_XPRO_ACT            (1u << 8)

/* LIN_XPHY_ACT .. Register: LIN_XCFG .. Bit 9 */
#define LIN_XPHY_ACT            (1u << 9)
                                
/* LIN_XKEY_VALUE .. Register: LIN_XKEY .. Bit 0-15 */                                
#define LIN_XKEY_VALUE          0x5F0Au

/* LIN_XKEY0 .. Register: LIN_XKEY .. Bit 0 */ 
#define LIN_XKEY0               (1u << 0)
                                
/* ENLINRX .. Register: LV_TMR .. Bit 8 */                                 
#define ENLINRX                 (1u << 8)

/* ENLINTX .. Register: LV_TMR .. Bit 9 */
#define ENLINTX                 (1u << 9)

/* HV_LV_INTERFACE .. Register: LV_TMR .. Bit 10 */
#define HV_LV_INTERFACE         (1u << 10)

/** LIN with external protocol
 * @param
 * @return void
 * Register: LIN_XCFG, Bit LIN_EN_XPRO, Bit positions [0]
 */
#define LIN_ENABLE_EXT_PROTOCOL() LIN_XCFG = ((LIN_XCFG & LIN_EN_X_CLEAR) | LIN_EN_XPRO)

/** LIN with external physical layer
 * @param
 * @return void
 * Register: LIN_XCFG, Bit LIN_EN_XPHY, Bit positions [1]
 */
#define LIN_ENABLE_EXT_PHYSICAL() LIN_XCFG = ((LIN_XCFG & LIN_EN_X_CLEAR) | LIN_EN_XPHY)

/** LIN disable external stuff
 * @param
 * @return void
 * Register: LIN_XCFG, Bit LIN_EN_X_CLEAR, Bit positions [1:0]
 */
#define LIN_DISABLE_EXTERNAL() LIN_XCFG &= (LIN_EN_X_CLEAR)

/** LIN disable pull-up termination
 * @param
 * @return void
 * Register: LIN_XCFG, Bit LIN_DISTERM, Bit positions [2]
 */
#define LIN_DISABLE_TERMINATION() LIN_XCFG |= (LIN_DISTERM)

/** LIN enable pull-up termination
 * @param
 * @return void
 * Register: LIN_XCFG, Bit LIN_DISTERM, Bit positions [2]
 */
#define LIN_ENABLE_TERMINATION() LIN_XCFG &= (uint16)~(LIN_DISTERM)

/** LIN invert-signal at HV2 or LV2
 * @param
 * @return void
 * Register: LIN_XCFG, Bit LIN_XOUT_INV, Bit positions [3]
 */
#define LIN_INVERT_OUT() LIN_XCFG |= (LIN_XOUT_INV)

/** LIN normal-signal at HV2 or LV2
 * @param
 * @return void
 * Register: LIN_XCFG, Bit LIN_XOUT_INV, Bit positions [3]
 */
#define LIN_NORMAL_OUT() LIN_XCFG &= (uint16)~(LIN_XOUT_INV)

/** LIN enable bypass
 * @param
 * @return void
 * Register: LIN_XCFG, Bit BYPASS, Bit positions [4]
 */
#define LIN_ENABLE_BYPASS() LIN_XCFG |= (BYPASS)

/** LIN disable bypass
 * @param
 * @return void
 * Register: LIN_XCFG, Bit BYPASS, Bit positions [4]
 */
#define LIN_DISABLE_BYPASS() LIN_XCFG &= (uint16)~(BYPASS)

/** LIN enable highspeed mode
 * @param
 * @return void
 * Register: LIN_XCFG, Bit HSM, Bit positions [5]
 */
#define LIN_SET_HSM() LIN_XCFG |= (HSM)

/** LIN enable normal mode
 * @param
 * @return void
 * Register: LIN_XCFG, Bit HSM,LSM, Bit positions [6:5]
 */
#define LIN_SET_NM() LIN_XCFG &= (uint16)~((HSM|LSM))

/** LIN enable lowspeed mode
 * @param
 * @return void
 * Register: LIN_XCFG, Bit LSM, Bit positions [6]
 */
#define LIN_SET_LSM() LIN_XCFG |= (LSM)

/** LIN enable sleepb
 * @param
 * @return void
 * Register: LIN_XCFG, Bit SLEEPB_LIN, Bit positions [7]
 */
#define LIN_ENABLE_SLEEPB() LIN_XCFG |= (SLEEPB_LIN)

/** LIN disable sleepb
 * @param
 * @return void
 * Register: LIN_XCFG, Bit SLEEPB_LIN, Bit positions [7]
 */
#define LIN_DISABLE_SLEEPB() LIN_XCFG &= (uint16)~(SLEEPB_LIN)

/** LIN - read-back external protocol mode
 * @param
 * @return uint16 (1=set, 0=not set)
 * Register: LIN_XCFG, Bit LIN_XPRO_ACT, Bit positions [8]
 */
#define LIN_GET_XPRO_STAT() ((LIN_XCFG & LIN_XPRO_ACT) >> 8)

/** LIN - read-back external physical mode
 * @param
 * @return uint16 (1=set, 0=not set)
 * Register: LIN_XCFG, Bit LIN_XPHY_ACT, Bit positions [9]
 */
#define LIN_GET_XPHY_STAT() ((LIN_XCFG & LIN_XPHY_ACT) >> 9)

/** LIN - set XKEY-register
 * @param
 * @return void
 * Register: LIN_XKEY, Bit LIN_XKEYx, Bit positions [15:0]
 */
#define LIN_SET_XKEY() LIN_XKEY = LIN_XKEY_VALUE

/** LIN - reset XKEY-register
 * @param
 * @return void
 * Register: LIN_XKEY, Bit LIN_XKEYx, Bit positions [15:0]
 */
#define LIN_RESET_XKEY() LIN_XKEY = 0

/** LIN - check for valid key
 * @param
 * @return uint16 (1=valid, 0=not valid)
 * Register: LIN_XKEY, Bit LIN_XKEY0, Bit positions [0]
 */
#define LIN_GET_XKEY_STAT() ((LIN_XKEY & LIN_XKEY0) >> 0)

/** LIN enable LV interface
 * @param
 * @return void
 * Register: LV_TMR, Bit HV_LV_INTERFACE, Bit positions [10]
 */
#define LIN_ENABLE_LV_INTERFACE() LV_TMR &= (uint16)~(HV_LV_INTERFACE)

/** LIN enable HV interface
 * @param
 * @return void
 * Register: LV_TMR, Bit HV_LV_INTERFACE, Bit positions [10]
 */
#define LIN_ENABLE_HV_INTERFACE() LV_TMR |= (HV_LV_INTERFACE)

/** LIN enable RX (HV3 or LV3)
 * @param
 * @return void
 * Register: LV_TMR, Bit ENLINRX, Bit positions [8]
 */
#define LIN_ENABLE_RX() LV_TMR |= (ENLINRX)

/** LIN disable RX (HV3 or LV3)
 * @param
 * @return void
 * Register: LV_TMR, Bit ENLINRX, Bit positions [8]
 */
#define LIN_DISABLE_RX() LV_TMR &= (uint16)~(ENLINRX)

/** LIN enable TX (HV2 or LV2)
 * @param
 * @return void
 * Register: LV_TMR, Bit ENLINTX, Bit positions [9]
 */
#define LIN_ENABLE_TX() LV_TMR |= (ENLINTX)

/** LIN disable TX (HV2 or LV2)
 * @param
 * @return void
 * Register: LV_TMR, Bit ENLINTX, Bit positions [9]
 */
#define LIN_DISABLE_TX() LV_TMR &= (uint16)~(ENLINTX)

#endif /* LIB_MLX8110X_LIN_SPECIALMODE_H_ */
