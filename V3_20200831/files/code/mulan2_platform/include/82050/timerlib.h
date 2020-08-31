/*
 * Copyright (C) 2005-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef TIMERLIB_H_
#define TIMERLIB_H_

#include "syslib.h"

/*
 * Timer clock dividers 
 * TIMx_DIV[15:14]
 */
#define TIMER_DIV_1             0
#define TIMER_DIV_16            0x4000
#define TIMER_DIV_256           0x8000

/*
 * Timer modes
 * TIMx_MODE[13:11]
 */
#define TIMER_MODE_AUTOLOAD         0x0000
#define TIMER_MODE_COMPARE          0x0800
#define TIMER_MODE_CAPTURE          0x1000
#define TIMER_MODE_CAPTURE_COMPARE  0x1800
#define TIMER_MODE_PULSE            0x2000
#define TIMER_MODE_DEBOUNCER        0x2800
#define TIMER_MODE_PWM              0x3000
#define TIMER_MODE_UART             0x3800

/*
 * Rising or falling edge detection
 * TIMx_EDG2[5:4] -- channel B
 * TIMx_EDG2[3:2] -- channel B
 */
#define TIMER_EDGB_NO           0x0000
#define TIMER_EDGB_BOTH         0x0030
#define TIMER_EDGB_RISE         0x0020
#define TIMER_EDGB_FALL         0x0010
#define TIMER_EDGA_NO           0x0000
#define TIMER_EDGA_BOTH         0x000c
#define TIMER_EDGA_RISE         0x0008
#define TIMER_EDGA_FALL         0x0004

/*
 *  AUTOLOAD MODE
 *
 *           Fck
 *  Ftcnt = -----
 *           DIV
 *
 *           Ftcnt      Ftcnt
 *  Fint4 = -------- = --------
 *           CMPB+1     PERIOD
 */

#define __TIMER_AUTOLOAD_INIT(U, DIV, CMPB)             \
do  {                                                   \
    TMR##U##_CTRL = 1;                                  \
    TMR##U##_REGA = 0;                                  \
    TMR##U##_REGB = (CMPB);                             \
    TMR##U##_CTRL = (DIV) | TIMER_MODE_AUTOLOAD | 3;    \
}   while(0)

#define TIMER1_AUTOLOAD_INIT(DIV, CMPB) \
    __TIMER_AUTOLOAD_INIT(1, DIV, CMPB)

#define TIMER2_AUTOLOAD_INIT(DIV, CMPB) \
    __TIMER_AUTOLOAD_INIT(2, DIV, CMPB)

#define TIMER3_AUTOLOAD_INIT(DIV, CMPB) \
    __TIMER_AUTOLOAD_INIT(3, DIV, CMPB)

#define TIMER4_AUTOLOAD_INIT(DIV, CMPB) \
    __TIMER_AUTOLOAD_INIT(4, DIV, CMPB)


/*
 *  DUAL COMPARE MODE
 *
 *           Fck
 *  Ftcnt = -----
 *           DIV
 *
 *           Ftcnt
 *  Fint2 = --------
 *           CMPA+1
 *
 *           Ftcnt
 *  Fint4 = --------
 *           CMPB+1
 */

#define __TIMER_COMPARE_INIT(U, DIV, CMPA, CMPB)    \
do  {                                               \
    TMR##U##_CTRL = 1;                              \
    TMR##U##_REGA = (CMPA);                         \
    TMR##U##_REGB = (CMPB);                         \
    TMR##U##_CTRL = (DIV) | TIMER_MODE_COMPARE | 3; \
}   while(0)

#define TIMER1_COMPARE_INIT(DIV, CMPA, CMPB)    \
    __TIMER_COMPARE_INIT(1, DIV, CMPA, CMPB)

#define TIMER2_COMPARE_INIT(DIV, CMPA, CMPB)    \
    __TIMER_COMPARE_INIT(2, DIV, CMPA, CMPB)

#define TIMER3_COMPARE_INIT(DIV, CMPA, CMPB)    \
    __TIMER_COMPARE_INIT(3, DIV, CMPA, CMPB)

#define TIMER4_COMPARE_INIT(DIV, CMPA, CMPB)    \
    __TIMER_COMPARE_INIT(4, DIV, CMPA, CMPB)

/*
 *  DUAL CAPTURE MODE
 *
 *           Fck
 *  Ftcnt = -----
 *           DIV
 */

#define __TIMER_CAPTURE_INIT(U, DIV, EDGA, EDGB)            \
do  {                                                       \
    TMR##U##_CTRL = 1;                                      \
    TMR##U##_REGA = 0;                                      \
    TMR##U##_REGB = 0;                                      \
    TMR##U##_CTRL = TIMER_MODE_CAPTURE                      \
         | (EDGA) | (EDGB) | (DIV) | 3;                     \
}   while(0)

#define TIMER1_CAPTURE_INIT(DIV, EDGA, EDGB)    \
    __TIMER_CAPTURE_INIT(1, DIV, EDGA, EDGB)

#define TIMER2_CAPTURE_INIT(DIV, EDGA, EDGB)    \
    __TIMER_CAPTURE_INIT(2, DIV, EDGA, EDGB)

#define TIMER3_CAPTURE_INIT(DIV, EDGA, EDGB)    \
    __TIMER_CAPTURE_INIT(3, DIV, EDGA, EDGB)

#define TIMER4_CAPTURE_INIT(DIV, EDGA, EDGB)    \
    __TIMER_CAPTURE_INIT(4, DIV, EDGA, EDGB)

/*
 *  CAPTURE COMPARE MODE
 *
 *           Fck
 *  Ftcnt = -----
 *           DIV
 *
 *           Ftcnt
 *  Fint2 = --------
 *           CMPA+1
 */

#define __TIMER_CAPTURE_COMPARE_INIT(U, DIV, CMPA, EDGB)    \
do  {                                                       \
    TMR##U##_CTRL = 1;                                      \
    TMR##U##_REGA = (CMPA);                                 \
    TMR##U##_REGB = 0;                                      \
    TMR##U##_CTRL = TIMER_MODE_CAPTURE_COMPARE              \
        | (EDGB) | (DIV) | 3;                               \
}   while(0)

#define TIMER1_CAPTURE_COMPARE_INIT(DIV, CMPA, EDGB)    \
    __TIMER_CAPTURE_COMPARE_INIT(1, DIV, CMPA, EDGB)

#define TIMER2_CAPTURE_COMPARE_INIT(DIV, CMPA, EDGB)    \
    __TIMER_CAPTURE_COMPARE_INIT(2, DIV, CMPA, EDGB)

#define TIMER3_CAPTURE_COMPARE_INIT(DIV, CMPA, EDGB)    \
    __TIMER_CAPTURE_COMPARE_INIT(3, DIV, CMPA, EDGB)

#define TIMER4_CAPTURE_COMPARE_INIT(DIV, CMPA, EDGB)    \
    __TIMER_CAPTURE_COMPARE_INIT(4, DIV, CMPA, EDGB)

/*
 *  CONTROL
 */

#define __TIMER_STOP(U)     (TMR##U##_CTRL &= ~(1u << 1))
#define TIMER1_STOP()       __TIMER_STOP(1)
#define TIMER2_STOP()       __TIMER_STOP(2)
#define TIMER3_STOP()       __TIMER_STOP(3)
#define TIMER4_STOP()       __TIMER_STOP(4)

#define __TIMER_START(U)    (TMR##U##_CTRL |= (1u << 1))
#define TIMER1_START()      __TIMER_START(1)
#define TIMER2_START()      __TIMER_START(2)
#define TIMER3_START()      __TIMER_START(3)
#define TIMER4_START()      __TIMER_START(4)

#define __TIMER_CNT(U)      (TMR##U##_CNT)  
#define TIMER1()            __TIMER_CNT(1)
#define TIMER2()            __TIMER_CNT(2)
#define TIMER3()            __TIMER_CNT(3)
#define TIMER4()            __TIMER_CNT(4)

/*
 *  INTERRUPT CONTROL
 */
#define __XI0_INT_ENABLE(n)     \
do  {                           \
    XI0_PEND  = 1u << (n);      \
    XI0_MASK |= 1u << (n);      \
} while(0)

#define __XI0_INT_DISABLE(n)    \
    ( XI0_MASK &= ~(1u << (n)) )

#define __XI1_INT_ENABLE(n)     \
do  {                           \
    XI1_PEND  = 1u << (n);      \
    XI1_MASK |= 1u << (n);      \
} while(0)

#define __XI1_INT_DISABLE(n)    \
    ( XI1_MASK &= ~(1u << (n)) )

/*
 * INT1 :   TMR<U>_Capture_A_Interrupt
 * INT2 :   TMR<U>_Compare_A_Interrupt
 * INT3 :   TMR<U>_Overflow_Interrupt
 * INT4 :   TMR<U>_Compare_B_Interrupt
 * INT5 :   TMR<U>_Capture_B_Interrupt
 */
#define TIMER1_INT1_ENABLE()    __XI0_INT_ENABLE(9)
#define TIMER1_INT5_ENABLE()    __XI0_INT_ENABLE(8)
#define TIMER1_INT2_ENABLE()    __XI0_INT_ENABLE(7)
#define TIMER1_INT4_ENABLE()    __XI0_INT_ENABLE(6)
#define TIMER1_INT3_ENABLE()    __XI0_INT_ENABLE(5)

#define TIMER3_INT1_ENABLE()    __XI0_INT_ENABLE(4)
#define TIMER3_INT5_ENABLE()    __XI0_INT_ENABLE(3)
#define TIMER3_INT2_ENABLE()    __XI0_INT_ENABLE(2)
#define TIMER3_INT4_ENABLE()    __XI0_INT_ENABLE(1)
#define TIMER3_INT3_ENABLE()    __XI0_INT_ENABLE(0)


#define TIMER2_INT1_ENABLE()    __XI1_INT_ENABLE(9)
#define TIMER2_INT5_ENABLE()    __XI1_INT_ENABLE(8)
#define TIMER2_INT2_ENABLE()    __XI1_INT_ENABLE(7)
#define TIMER2_INT4_ENABLE()    __XI1_INT_ENABLE(6)
#define TIMER2_INT3_ENABLE()    __XI1_INT_ENABLE(5)

#define TIMER4_INT1_ENABLE()    __XI1_INT_ENABLE(4)
#define TIMER4_INT5_ENABLE()    __XI1_INT_ENABLE(3)
#define TIMER4_INT2_ENABLE()    __XI1_INT_ENABLE(2)
#define TIMER4_INT4_ENABLE()    __XI1_INT_ENABLE(1)
#define TIMER4_INT3_ENABLE()    __XI1_INT_ENABLE(0)


#define TIMER1_INT1_DISABLE()   __XI0_INT_DISABLE(9)
#define TIMER1_INT5_DISABLE()   __XI0_INT_DISABLE(8)
#define TIMER1_INT2_DISABLE()   __XI0_INT_DISABLE(7)
#define TIMER1_INT4_DISABLE()   __XI0_INT_DISABLE(6)
#define TIMER1_INT3_DISABLE()   __XI0_INT_DISABLE(5)

#define TIMER3_INT1_DISABLE()   __XI0_INT_DISABLE(4)
#define TIMER3_INT5_DISABLE()   __XI0_INT_DISABLE(3)
#define TIMER3_INT2_DISABLE()   __XI0_INT_DISABLE(2)
#define TIMER3_INT4_DISABLE()   __XI0_INT_DISABLE(1)
#define TIMER3_INT3_DISABLE()   __XI0_INT_DISABLE(0)

#define TIMER2_INT1_DISABLE()   __XI1_INT_DISABLE(9)
#define TIMER2_INT5_DISABLE()   __XI1_INT_DISABLE(8)
#define TIMER2_INT2_DISABLE()   __XI1_INT_DISABLE(7)
#define TIMER2_INT4_DISABLE()   __XI1_INT_DISABLE(6)
#define TIMER2_INT3_DISABLE()   __XI1_INT_DISABLE(5)

#define TIMER4_INT1_DISABLE()   __XI1_INT_DISABLE(4)
#define TIMER4_INT5_DISABLE()   __XI1_INT_DISABLE(3)
#define TIMER4_INT2_DISABLE()   __XI1_INT_DISABLE(2)
#define TIMER4_INT4_DISABLE()   __XI1_INT_DISABLE(1)
#define TIMER4_INT3_DISABLE()   __XI1_INT_DISABLE(0)


/* Source number to source name mapping */
#define TIMER1_INT_OVF_ENABLE       TIMER1_INT3_ENABLE /* all modes */
#define TIMER1_INT_RELOAD_ENABLE    TIMER1_INT4_ENABLE /* reload */
#define TIMER1_INT_CAPA_ENABLE      TIMER1_INT1_ENABLE /* dual-capture */
#define TIMER1_INT_OVRA_ENABLE      TIMER1_INT2_ENABLE
#define TIMER1_INT_OVRB_ENABLE      TIMER1_INT4_ENABLE
#define TIMER1_INT_CAPB_ENABLE      TIMER1_INT5_ENABLE
#define TIMER1_INT_CMPA_ENABLE      TIMER1_INT2_ENABLE /* dual-compare */
#define TIMER1_INT_CMPB_ENABLE      TIMER1_INT4_ENABLE
#define TIMER1_INT_CMPA_ENABLE      TIMER1_INT2_ENABLE /* compare and capture */
#define TIMER1_INT_EDGF_ENABLE      TIMER1_INT4_ENABLE /* debouncer */
#define TIMER1_INT_EDGR_ENABLE      TIMER1_INT5_ENABLE
#define TIMER1_INT_PWMA_ENABLE      TIMER1_INT2_ENABLE /* pwm */
#define TIMER1_INT_PWMB_ENABLE      TIMER1_INT4_ENABLE

#define TIMER2_INT_OVF_ENABLE       TIMER2_INT3_ENABLE /* all modes */
#define TIMER2_INT_RELOAD_ENABLE    TIMER2_INT4_ENABLE /* reload */
#define TIMER2_INT_CAPA_ENABLE      TIMER2_INT1_ENABLE /* dual-capture */
#define TIMER2_INT_OVRA_ENABLE      TIMER2_INT2_ENABLE
#define TIMER2_INT_OVRB_ENABLE      TIMER2_INT4_ENABLE
#define TIMER2_INT_CAPB_ENABLE      TIMER2_INT5_ENABLE
#define TIMER2_INT_CMPA_ENABLE      TIMER2_INT2_ENABLE /* dual-compare */
#define TIMER2_INT_CMPB_ENABLE      TIMER2_INT4_ENABLE
#define TIMER2_INT_CMPA_ENABLE      TIMER2_INT2_ENABLE /* compare and capture */
#define TIMER2_INT_EDGF_ENABLE      TIMER2_INT4_ENABLE /* debouncer */
#define TIMER2_INT_EDGR_ENABLE      TIMER2_INT5_ENABLE
#define TIMER2_INT_PWMA_ENABLE      TIMER2_INT2_ENABLE /* pwm */
#define TIMER2_INT_PWMB_ENABLE      TIMER2_INT4_ENABLE

#define TIMER3_INT_OVF_ENABLE       TIMER3_INT3_ENABLE /* all modes */
#define TIMER3_INT_RELOAD_ENABLE    TIMER3_INT4_ENABLE /* reload */
#define TIMER3_INT_CAPA_ENABLE      TIMER3_INT1_ENABLE /* dual-capture */
#define TIMER3_INT_OVRA_ENABLE      TIMER3_INT2_ENABLE
#define TIMER3_INT_OVRB_ENABLE      TIMER3_INT4_ENABLE
#define TIMER3_INT_CAPB_ENABLE      TIMER3_INT5_ENABLE
#define TIMER3_INT_CMPA_ENABLE      TIMER3_INT2_ENABLE /* dual-compare */
#define TIMER3_INT_CMPB_ENABLE      TIMER3_INT4_ENABLE
#define TIMER3_INT_CMPA_ENABLE      TIMER3_INT2_ENABLE /* compare and capture */
#define TIMER3_INT_EDGF_ENABLE      TIMER3_INT4_ENABLE /* debouncer */
#define TIMER3_INT_EDGR_ENABLE      TIMER3_INT5_ENABLE
#define TIMER3_INT_PWMA_ENABLE      TIMER3_INT2_ENABLE /* pwm */
#define TIMER3_INT_PWMB_ENABLE      TIMER3_INT4_ENABLE

#define TIMER4_INT_OVF_ENABLE       TIMER4_INT3_ENABLE /* all modes */
#define TIMER4_INT_RELOAD_ENABLE    TIMER4_INT4_ENABLE /* reload */
#define TIMER4_INT_CAPA_ENABLE      TIMER4_INT1_ENABLE /* dual-capture */
#define TIMER4_INT_OVRA_ENABLE      TIMER4_INT2_ENABLE
#define TIMER4_INT_OVRB_ENABLE      TIMER4_INT4_ENABLE
#define TIMER4_INT_CAPB_ENABLE      TIMER4_INT5_ENABLE
#define TIMER4_INT_CMPA_ENABLE      TIMER4_INT2_ENABLE /* dual-compare */
#define TIMER4_INT_CMPB_ENABLE      TIMER4_INT4_ENABLE
#define TIMER4_INT_CMPA_ENABLE      TIMER4_INT2_ENABLE /* compare and capture */
#define TIMER4_INT_EDGF_ENABLE      TIMER4_INT4_ENABLE /* debouncer */
#define TIMER4_INT_EDGR_ENABLE      TIMER4_INT5_ENABLE
#define TIMER4_INT_PWMA_ENABLE      TIMER4_INT2_ENABLE /* pwm */
#define TIMER4_INT_PWMB_ENABLE      TIMER4_INT4_ENABLE

#define TIMER1_INT_OVF_DISABLE      TIMER1_INT3_DISABLE /* all modes */
#define TIMER1_INT_RELOAD_DISABLE   TIMER1_INT4_DISABLE /* reload */
#define TIMER1_INT_CAPA_DISABLE     TIMER1_INT1_DISABLE /* dual-capture */
#define TIMER1_INT_OVRA_DISABLE     TIMER1_INT2_DISABLE
#define TIMER1_INT_OVRB_DISABLE     TIMER1_INT4_DISABLE
#define TIMER1_INT_CAPB_DISABLE     TIMER1_INT5_DISABLE
#define TIMER1_INT_CMPA_DISABLE     TIMER1_INT2_DISABLE /* dual-compare */
#define TIMER1_INT_CMPB_DISABLE     TIMER1_INT4_DISABLE
#define TIMER1_INT_CMPA_DISABLE     TIMER1_INT2_DISABLE /* compare and capture */
#define TIMER1_INT_EDGF_DISABLE     TIMER1_INT4_DISABLE /* debouncer */
#define TIMER1_INT_EDGR_DISABLE     TIMER1_INT5_DISABLE
#define TIMER1_INT_PWMA_DISABLE     TIMER1_INT2_DISABLE /* pwm */
#define TIMER1_INT_PWMB_DISABLE     TIMER1_INT4_DISABLE

#define TIMER2_INT_OVF_DISABLE      TIMER2_INT3_DISABLE /* all modes */
#define TIMER2_INT_RELOAD_DISABLE   TIMER2_INT4_DISABLE /* reload */
#define TIMER2_INT_CAPA_DISABLE     TIMER2_INT1_DISABLE /* dual-capture */
#define TIMER2_INT_OVRA_DISABLE     TIMER2_INT2_DISABLE
#define TIMER2_INT_OVRB_DISABLE     TIMER2_INT4_DISABLE
#define TIMER2_INT_CAPB_DISABLE     TIMER2_INT5_DISABLE
#define TIMER2_INT_CMPA_DISABLE     TIMER2_INT2_DISABLE /* dual-compare */
#define TIMER2_INT_CMPB_DISABLE     TIMER2_INT4_DISABLE
#define TIMER2_INT_CMPA_DISABLE     TIMER2_INT2_DISABLE /* compare and capture */
#define TIMER2_INT_EDGF_DISABLE     TIMER2_INT4_DISABLE /* debouncer */
#define TIMER2_INT_EDGR_DISABLE     TIMER2_INT5_DISABLE
#define TIMER2_INT_PWMA_DISABLE     TIMER2_INT2_DISABLE /* pwm */
#define TIMER2_INT_PWMB_DISABLE     TIMER2_INT4_DISABLE

#define TIMER3_INT_OVF_DISABLE      TIMER3_INT3_DISABLE /* all modes */
#define TIMER3_INT_RELOAD_DISABLE   TIMER3_INT4_DISABLE /* reload */
#define TIMER3_INT_CAPA_DISABLE     TIMER3_INT1_DISABLE /* dual-capture */
#define TIMER3_INT_OVRA_DISABLE     TIMER3_INT2_DISABLE
#define TIMER3_INT_OVRB_DISABLE     TIMER3_INT4_DISABLE
#define TIMER3_INT_CAPB_DISABLE     TIMER3_INT5_DISABLE
#define TIMER3_INT_CMPA_DISABLE     TIMER3_INT2_DISABLE /* dual-compare */
#define TIMER3_INT_CMPB_DISABLE     TIMER3_INT4_DISABLE
#define TIMER3_INT_CMPA_DISABLE     TIMER3_INT2_DISABLE /* compare and capture */
#define TIMER3_INT_EDGF_DISABLE     TIMER3_INT4_DISABLE /* debouncer */
#define TIMER3_INT_EDGR_DISABLE     TIMER3_INT5_DISABLE
#define TIMER3_INT_PWMA_DISABLE     TIMER3_INT2_DISABLE /* pwm */
#define TIMER3_INT_PWMB_DISABLE     TIMER3_INT4_DISABLE

#define TIMER4_INT_OVF_DISABLE      TIMER4_INT3_DISABLE /* all modes */
#define TIMER4_INT_RELOAD_DISABLE   TIMER4_INT4_DISABLE /* reload */
#define TIMER4_INT_CAPA_DISABLE     TIMER4_INT1_DISABLE /* dual-capture */
#define TIMER4_INT_OVRA_DISABLE     TIMER4_INT2_DISABLE
#define TIMER4_INT_OVRB_DISABLE     TIMER4_INT4_DISABLE
#define TIMER4_INT_CAPB_DISABLE     TIMER4_INT5_DISABLE
#define TIMER4_INT_CMPA_DISABLE     TIMER4_INT2_DISABLE /* dual-compare */
#define TIMER4_INT_CMPB_DISABLE     TIMER4_INT4_DISABLE
#define TIMER4_INT_CMPA_DISABLE     TIMER4_INT2_DISABLE /* compare and capture */
#define TIMER4_INT_EDGF_DISABLE     TIMER4_INT4_DISABLE /* debouncer */
#define TIMER4_INT_EDGR_DISABLE     TIMER4_INT5_DISABLE
#define TIMER4_INT_PWMA_DISABLE     TIMER4_INT2_DISABLE /* pwm */
#define TIMER4_INT_PWMB_DISABLE     TIMER4_INT4_DISABLE

 
#endif /* TIMERLIB_H_ */
