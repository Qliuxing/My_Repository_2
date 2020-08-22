/*
 * Copyright (C) 2008-2012 Melexis N.V.
 *
 * Software Platform
 * Product/board specific functions
 *
 */

#ifndef LIB_MLX81150_H_
#define LIB_MLX81150_H_

#include "awdg.h"

/*
 * 2011-06-10: Added definitions from rfo
 */

/* switch PS on */
#define PS_ON()                   ANA_OUTG |=  0x0001

/* addional Makros for PWM-Port */
#define ENA_OPWM1_2()             ANA_OUTK |=  0x0400        /* from OPWM[1:2] PWMMODE=1 */
#define ENA_PWM1_2()              ANA_OUTK &= ~0x0400        /* from PWM[1:2] PWMMODE=0 */
#define PWM1_ON()                 ANA_OUTG |=  0x0002
#define PWM2_ON()                 ANA_OUTG |=  0x0004
#define PWM1_2_ON()               ANA_OUTG |=  0x0006
#define PWM1_OFF()                ANA_OUTG &= ~0x0002
#define PWM2_OFF()                ANA_OUTG &= ~0x0004
#define PWM1_2_OFF()              ANA_OUTG &= ~0x0006
#define SET_PWM1()                ANA_OUTK |=  0x0800
#define RESET_PWM1()              ANA_OUTK &= ~0x0800
#define SET_PWM2()                ANA_OUTK |=  0x1000
#define RESET_PWM2()              ANA_OUTK &= ~0x1000
#define SET_PWM1_2()              ANA_OUTK |=  0x1800
#define RESET_PWM1_2()            ANA_OUTK &= ~0x1800

/* additional Makros for Relay-Port */
/* PWM3 == REL1, PWM4 == REL1 -> Attention: Switching inverted */
#define ENA_OREL1_2()             ANA_OUTK |=  0x2000       /* from OREL[1:2] PWMMODE=1 */
#define ENA_PWM3_4()              ANA_OUTK &= ~0x2000       /* from PWM[3:4] RELMODE=0 */
#define SET_OREL1()               ANA_OUTK &= ~0x4000       /* set=logical high (inverted with relay) */
#define RESET_OREL1()             ANA_OUTK |=  0x4000       /* reset=logical low (inverted with relay) */
#define SET_OREL2()               ANA_OUTK &= ~0x8000       /* set=logical high (inverted with relay) */
#define RESET_OREL2()             ANA_OUTK |=  0x8000       /* reset=logical low (inverted with relay) */
#define SET_OREL1_2()             ANA_OUTK &= ~0xC000       /* set=logical high (inverted with relay) */
#define RESET_OREL1_2()           ANA_OUTK |=  0xC000       /* reset=logical low (inverted with relay) */
#define DISA_FLYBACK_DIODES()     ANA_OUTG |=  0x0018       /* Disable freewheel diode */
#define DISA_FLYBACK_DIODE_REL1() ANA_OUTG |=  0x0008       /* Disable freewheel diode */
#define DISA_FLYBACK_DIODE_REL2() ANA_OUTG |=  0x0010       /* Disable freewheel diode */
#define ENA_FLYBACK_DIODES()      ANA_OUTG &= ~0x0018       /* Enable freewheel diode */
#define ENA_FLYBACK_DIODE_REL1()  ANA_OUTG &= ~0x0008       /* Enable freewheel diode */
#define ENA_FLYBACK_DIODE_REL2()  ANA_OUTG &= ~0x0010       /* Enable freewheel diode */

/* definitions for PWM- and Relay-Port together */
#define ENA_ALL_PWM()             ANA_OUTK &= ~0x2400       /* from PWM[1:2] PWMMODE=0 and from PWM[3:4] RELMODE=0 */
#define DISA_ALL_PWM()            ANA_OUTK |=  0x2400

/* Defines for TimerBlock */
#define T1_INA_TC1()  ANA_OUTI &= 0xFFFC
#define T1_INA_TC2()  ANA_OUTI |= 0x0001
#define T1_INA_KEY0() ANA_OUTI |= 0x0002
#define T1_INA_KEY1() ANA_OUTI |= 0x0003
#define T1_INB_TC2()  ANA_OUTI &= 0xFFF3
#define T1_INB_TC1()  ANA_OUTI |= 0x0004
#define T1_INB_KEY1() ANA_OUTI |= 0x0008
#define T1_INB_KEY0() ANA_OUTI |= 0x000C
#define T2_INA_KEY0() ANA_OUTI &= 0xFFCF
#define T2_INA_KEY1() ANA_OUTI |= 0x0010
#define T2_INA_TC1()  ANA_OUTI |= 0x0020
#define T2_INA_TC2()  ANA_OUTI |= 0x0030
#define T2_INB_KEY1() ANA_OUTI &= 0xFF3F
#define T2_INB_KEY0() ANA_OUTI |= 0x0040
#define T2_INB_TC2()  ANA_OUTI |= 0x0080
#define T2_INB_TC1()  ANA_OUTI |= 0x00C0


/* ----------------------------------------------------------------------------
 * Function is call in the idle loop of flash loader
 */
__MLX_TEXT__  static INLINE  void WDG_Manager (void)
{
    awdg_restart();
}

#endif /* LIB_MLX81150_H_ */
