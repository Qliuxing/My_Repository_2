/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Generic SENT Low Level Routines Header
 *
 */

#ifndef SENT_ISR_H_
    #define SENT_ISR_H_

    /* --------------------------- */
    /* Public Defines              */
    /* --------------------------- */


    /* --------------------------- */
    /* Public Variables            */
    /* --------------------------- */


    /* --------------------------- */
    /* Public Function Definitions */
    /* --------------------------- */
    void SENT_ISR_FastCHempty(void) __attribute__ ((interrupt));
    void SENT_ISR_SlowCHempty(void) __attribute__ ((interrupt));
    void SENT_ISR_SlowCHprepDTA(void) __attribute__ ((interrupt));

#endif /* SENT_ISR_H_ */
