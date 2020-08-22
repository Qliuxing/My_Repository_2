/*
 * Copyright (C) 2005-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef TIMERLIB_H_
#define TIMERLIB_H_

#include <syslib.h>

/*
 * Timer clock dividers 
 * TIMx_DIV[15:14]
 */
#define TIMER_DIV_1             0
#define TIMER_DIV_16            (1u << 14)
#define TIMER_DIV_256           (2u << 14)

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

#define TIMER1_DIN_0            (1u << 6)   /*! timer 1 PWM mode selector bit 0 */
#define TIMER1_DIN_1            (1u << 7)   /*! timer 1 PWM mode selector bit 1 */
/*
 *  AUTOLOAD MODE
 *
 *           Fck
 *  Ftcnt = -----
 *           DIV
 *
 *           Ftcnt
 *  Fint4 = --------
 *           CMPB+1
 */

#define __TIMER_AUTOLOAD_INIT(U, DIV, CMPB)                             \
do  {                                                                   \
    TIMER##U##_REGA = 0;                                                \
    TIMER##U##_REGB = (CMPB);                                           \
    TIMER##U##_CTRL = (DIV) | TIMER_MODE_AUTOLOAD | TIMER1_START_BIT;   \
}   while(0)

#define TIMER1_AUTOLOAD_INIT(DIV, CMPB) \
    __TIMER_AUTOLOAD_INIT(1, DIV, CMPB)

#define TIMER2_AUTOLOAD_INIT(DIV, CMPB) \
    __TIMER_AUTOLOAD_INIT(2, DIV, CMPB)


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

#define __TIMER_COMPARE_INIT(U, DIV, CMPA, CMPB)                        \
do  {                                                                   \
    TIMER##U##_REGA = (CMPA);                                           \
    TIMER##U##_REGB = (CMPB);                                           \
    TIMER##U##_CTRL = (DIV) | TIMER_MODE_COMPARE | TIMER1_START_BIT;    \
}   while(0)

#define TIMER1_COMPARE_INIT(DIV, CMPA, CMPB)    \
    __TIMER_COMPARE_INIT(1, DIV, CMPA, CMPB)

#define TIMER2_COMPARE_INIT(DIV, CMPA, CMPB)    \
    __TIMER_COMPARE_INIT(2, DIV, CMPA, CMPB)


/*
 *  DUAL CAPTURE MODE
 *
 *           Fck
 *  Ftcnt = -----
 *           DIV
 */

#define __TIMER_CAPTURE_INIT(U, DIV, EDGA, EDGB)        \
do  {                                                   \
    TIMER##U##_REGA = 0;                                \
    TIMER##U##_REGB = 0;                                \
    TIMER##U##_CTRL = TIMER_MODE_CAPTURE                \
         | (EDGA) | (EDGB) | (DIV) | TIMER1_START_BIT;  \
}   while(0)

#define TIMER1_CAPTURE_INIT(DIV, EDGA, EDGB)    \
    __TIMER_CAPTURE_INIT(1, DIV, EDGA, EDGB)

#define TIMER2_CAPTURE_INIT(DIV, EDGA, EDGB)    \
    __TIMER_CAPTURE_INIT(2, DIV, EDGA, EDGB)

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
    TIMER##U##_REGA = (CMPA);                               \
    TIMER##U##_REGB = 0;                                    \
    TIMER##U##_CTRL = TIMER_MODE_CAPTURE_COMPARE            \
        | (EDGB) | (DIV) | TIMER1_START_BIT;                \
}   while(0)

#define TIMER1_CAPTURE_COMPARE_INIT(DIV, CMPA, EDGB)    \
    __TIMER_CAPTURE_COMPARE_INIT(1, DIV, CMPA, EDGB)

#define TIMER2_CAPTURE_COMPARE_INIT(DIV, CMPA, EDGB)    \
    __TIMER_CAPTURE_COMPARE_INIT(2, DIV, CMPA, EDGB)

/*
 *  TIMER PWM MODE
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

#define __TIMER_PWM_INIT(U, DIV, CMPA, CMPB)                        \
do  {                                                                   \
    TIMER##U##_REGA = (CMPA);                                           \
    TIMER##U##_REGB = (CMPB);                                           \
    TIMER##U##_CTRL = (DIV) | TIMER_MODE_PWM | TIMER1_DIN_1 | TIMER1_START_BIT;    \
}   while(0)

#define TIMER1_PWM_INIT(DIV, CMPA, CMPB)    \
    __TIMER_PWM_INIT(1, DIV, CMPA, CMPB)

#define TIMER2_PWM_INIT(DIV, CMPA, CMPB)    \
    __TIMER_PWM_INIT(2, DIV, CMPA, CMPB)

/*
 *  CONTROL
 */

#define __TIMER_STOP(U)     (TIMER##U##_CTRL &= ~TIMER1_START_BIT)
#define TIMER1_STOP()       __TIMER_STOP(1)
#define TIMER2_STOP()       __TIMER_STOP(2)

#define __TIMER_START(U)    (TIMER##U##_CTRL |= TIMER1_START_BIT)
#define TIMER1_START()      __TIMER_START(1)
#define TIMER2_START()      __TIMER_START(2)

#define __TIMER_CNT(U)      (TIMER##U##_CNT)  
#define TIMER1()            __TIMER_CNT(1)
#define TIMER2()            __TIMER_CNT(2)

#endif /* TIMERLIB_H_ */
