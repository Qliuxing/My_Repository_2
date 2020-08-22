/*
 * Copyright (C) 2005-2015 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef EXTTIMERLIB_H_
#define EXTTIMERLIB_H_

#include "timerlib.h"

/*
 * PWM Output Types
 * TIMx_DIN[7:6]
 */
#define TIMER_PWM_TYPE_NORM     (1<<6)
#define TIMER_PWM_TYPE_INV      (2<<6)

/*
 *  PWM MODE
 *
 *          Fck       1
 *  Fpwm = ----- * --------
 *          DIV     CMPB+1
 *
 *           CMPB - CMPA
 *  DCpwm = -------------
 *             PERIOD
 */

#define __TIMER_PWM_INIT(U, DIV, DIN, CMPA, CMPB)       \
do  {                                                   \
    TMR##U##_CTRL = 1;                                  \
    TMR##U##_REGA = (CMPA);                             \
    TMR##U##_REGB = (CMPB);                             \
    TMR##U##_CTRL = (DIV) | DIN | TIMER_MODE_PWM | 3;   \
}   while(0)

#define TIMER1_PWM_INIT(DIV, DIN, CMPA, CMPB) \
    __TIMER_PWM_INIT(1, DIV, DIN, CMPA, CMPB)

#define TIMER2_PWM_INIT(DIV, DIN, CMPA, CMPB) \
    __TIMER_PWM_INIT(2, DIV, DIN, CMPA, CMPB)

#define TIMER3_PWM_INIT(DIV, DIN, CMPA, CMPB) \
    __TIMER_PWM_INIT(3, DIV, DIN, CMPA, CMPB)

#define TIMER4_PWM_INIT(DIV, DIN, CMPA, CMPB) \
    __TIMER_PWM_INIT(4, DIV, DIN, CMPA, CMPB)
 
#endif /* EXTTIMERLIB_H_ */
