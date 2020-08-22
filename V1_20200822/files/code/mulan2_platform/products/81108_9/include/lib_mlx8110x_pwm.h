/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef LIB_MLX8110X_PWM_H_
#define LIB_MLX8110X_PWM_H_

#include "ioports.h"

/* PWM1_LV0 .. Register: LV_OUTOD .. Bit 8 */
#define PWM1_LV0 (1u<<8)

/* PWM2_LV1 .. Register: LV_OUTOD .. Bit 9 */
#define PWM2_LV1 (1u<<9)

/* PWM3_LV2 .. Register: LV_OUTOD .. Bit 10 */
#define PWM3_LV2 (1u<<10)

/* PWM4_LV3 .. Register: LV_OUTOD .. Bit 11 */
#define PWM4_LV3 (1u<<11)

/* PWM1_LV4 .. Register: LV_OUTOD .. Bit 12 */
#define PWM1_LV4 (1u<<12)

/* PWM2_LV5 .. Register: LV_OUTOD .. Bit 13 */
#define PWM2_LV5 (1u<<13)

/* PWM3_LV6 .. Register: LV_OUTOD .. Bit 14 */
#define PWM3_LV6 (1u<<14)

/* PWM4_LV7 .. Register: LV_OUTOD .. Bit 15 */
#define PWM4_LV7 (1u<<15)


/* PWM1_HV0 .. Register: HV_OUTOD .. Bit 8 */
#define PWM1_HV0 (1u<<8)
/* PWM2_HV1 .. Register: HV_OUTOD .. Bit 9 */
#define PWM2_HV1 (1u<<9)
/* PWM3_HV2 .. Register: HV_OUTOD .. Bit 10 */
#define PWM3_HV2 (1u<<10)
/* PWM4_HV3 .. Register: HV_OUTOD .. Bit 11 */
#define PWM4_HV3 (1u<<11)

/** Connect PWM-Block to PWM1 to LV0
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM0, Bit positions [8]
 */
#define PWM1_CONNECT_LV0()        LV_OUTOD |= (PWM1_LV0)

/** Disconnect PWM-Block PWM1 from LV0
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM0, Bit positions [8]
 */
#define PWM1_DISCONNECT_LV0()        LV_OUTOD &= (uint16)~(PWM1_LV0)

/** Connect PWM-Block to PWM2 to LV1
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM1, Bit positions [9]
 */
#define PWM2_CONNECT_LV1()        LV_OUTOD |= (PWM2_LV1)

/** Disconnect PWM-Block PWM2 from LV1
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM1, Bit positions [9]
 */
#define PWM2_DISCONNECT_LV1()        LV_OUTOD &= (uint16)~(PWM2_LV1)

/** Connect PWM-Block to PWM3 to LV2
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM2, Bit positions [10]
 */
#define PWM3_CONNECT_LV2()        LV_OUTOD |= (PWM3_LV2)

/** Disconnect PWM-Block PWM3 from LV2
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM2, Bit positions [10]
 */
#define PWM3_DISCONNECT_LV2()        LV_OUTOD &= (uint16)~(PWM3_LV2)

/** Connect PWM-Block to PWM4 to LV3
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM3, Bit positions [11]
 */
#define PWM4_CONNECT_LV3()        LV_OUTOD |= (PWM4_LV3)

/** Disconnect PWM-Block PWM4 from LV3
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM3, Bit positions [11]
 */
#define PWM4_DISCONNECT_LV3()        LV_OUTOD &= (uint16)~(PWM4_LV3)


/** Connect PWM-Block to PWM1 to LV4
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM4, Bit positions [12]
 */
#define PWM1_CONNECT_LV4()        LV_OUTOD |= (PWM1_LV4)

/** Disconnect PWM-Block PWM1 from LV4
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM4, Bit positions [12]
 */
#define PWM1_DISCONNECT_LV4()        LV_OUTOD &= (uint16)~(PWM1_LV4)

/** Connect PWM-Block to PWM2 to LV5
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM5, Bit positions [13]
 */
#define PWM2_CONNECT_LV5()        LV_OUTOD |= (PWM2_LV5)

/** Disconnect PWM-Block PWM2 from LV5
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM5, Bit positions [13]
 */
#define PWM2_DISCONNECT_LV5()        LV_OUTOD &= (uint16)~(PWM2_LV5)

/** Connect PWM-Block to PWM3 to LV6
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM6, Bit positions [14]
 */
#define PWM3_CONNECT_LV6()        LV_OUTOD |= (PWM3_LV6)

/** Disconnect PWM-Block PWM3 from LV6
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM6, Bit positions [14]
 */
#define PWM3_DISCONNECT_LV6()        LV_OUTOD &= (uint16)~(PWM3_LV6)

/** Connect PWM-Block to PWM4 to LV7
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM7, Bit positions [15]
 */
#define PWM4_CONNECT_LV7()        LV_OUTOD |= (PWM4_LV7)

/** Disconnect PWM-Block PWM4 from LV7
 * @param
 * @return void
 * Register: LV_OUTOD, Bit LVENPWM7, Bit positions [15]
 */
#define PWM4_DISCONNECT_LV7()        LV_OUTOD &= (uint16)~(PWM4_LV7)


/** Connect PWM-Block to PWM1 to HV0
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVENPWM0, Bit positions [8]
 */
#define PWM1_CONNECT_HV0()        HV_OUTOD |= (PWM1_HV0)

/** Disconnect PWM-Block PWM1 from HV0
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVENPWM0, Bit positions [8]
 */
#define PWM1_DISCONNECT_HV0()        HV_OUTOD &= (uint16)~(PWM1_HV0)

/** Connect PWM-Block to PWM2 to HV1
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVENPWM1, Bit positions [9]
 */
#define PWM2_CONNECT_HV1()        HV_OUTOD |= (PWM2_HV1)

/** Disconnect PWM-Block PWM2 from HV1
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVENPWM1, Bit positions [9]
 */
#define PWM2_DISCONNECT_HV1()        HV_OUTOD &= (uint16)~(PWM2_HV1)

/** Connect PWM-Block to PWM3 to HV2
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVENPWM2, Bit positions [10]
 */
#define PWM3_CONNECT_HV2()        HV_OUTOD |= (PWM3_HV2)

/** Disconnect PWM-Block PWM3 from HV2
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVENPWM2, Bit positions [10]
 */
#define PWM3_DISCONNECT_HV2()        HV_OUTOD &= (uint16)~(PWM3_HV2)

/** Connect PWM-Block to PWM4 to HV3
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVENPWM3, Bit positions [11]
 */
#define PWM4_CONNECT_HV3()        HV_OUTOD |= (PWM4_HV3)

/** Disconnect PWM-Block PWM4 from HV3
 * @param
 * @return void
 * Register: HV_OUTOD, Bit HVENPWM3, Bit positions [11]
 */
#define PWM4_DISCONNECT_HV3()        HV_OUTOD &= (uint16)~(PWM4_HV3)


#endif /* LIB_MLX8110X_PWM_H_ */
