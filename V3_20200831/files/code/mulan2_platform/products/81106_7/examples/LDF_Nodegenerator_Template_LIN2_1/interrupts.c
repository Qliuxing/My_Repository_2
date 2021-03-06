/**********************************************
* Copyright (C) 2009-2015 Melexis N.V.
*
* MLX8110x interrupt service routines
*
**********************************************/
/* History:
     Revision 1.0
       - Initial release
     Revision 2.0
       - Removed EXTx_IT interrupt service routines
     Revision 2.1
       - Added support for MLX4 monitoring
 **********************************************/

/* ==========================================================================
 * Includes
 * ========================================================================== */
#include <alib.h>
#include <plib.h>

/* LIN specific */
#include "lin_api.h"

#define TO_N_AS_LIN   (100 * 2)     /* x2x5 mS = 1000 mS */
static volatile uint8 tmrTL;

/* Counter for MLX4 monitoring */
extern uint16 g_u16Mlx4StateCheckCounter;

/****************************************************//**
* ADC Interrupt Service routine
********************************************************/
/*!
 */
void __interrupt__ ADC_IT(void){
	while (1) {
        ;
    }

} /* ADC_IT */

/****************************************************//**
* Timer Interrupt Service routine
********************************************************/
/*!
 */
void __interrupt__ TIMER_IT(void){

    /*
     * Note that 'tmrTL' is also accessed from LIN ISR:
     *   LIN_ISR -> mlu_MessageReceived -> ml_DiagMasterRequest -> ...
     *           ... -> mlu_TL_TimeoutStart -> tmrTL
     * To guarantee ATOMIC access to 'tmrTL' variable disable interrupts
     * by raising CPU priority
     */

    if (tmrTL != 0) {
        --tmrTL;
        if (tmrTL == 0) {
            ml_DiagClearPendingResponse();
        }
        /* else : in progress */
    }
    /* else : timeout is not started yet  */
    
    g_u16Mlx4StateCheckCounter++;   /* increase counter for MLX4 monitoring */

} /* TIMER_IT */

/****************************************************//**
* Function name: SOFT_IT
********************************************************/
/*!
   Software interrupt
 */
void __interrupt__ SOFT_IT(void){

	while (1) {
        ;
    }

} /* SOFT_IT */

/****************************************************//**
* Function name: STACK_IT
********************************************************/
/*!
   stack interrupt
 */
void __interrupt__ STACK_IT(void){
    while (1) {
        ;
    }

} /* STACK_IT */

/****************************************************//**
* Function name: PROT_ERR_IT
********************************************************/
/*!
   protection error interrupt
 */
void __interrupt__ PROT_ERR_IT(void){

	while (1) {
        ;
    }

} /* PROT_ERR_IT */

/****************************************************//**
* Function name: INV_AD_IT
********************************************************/
/*!
   invalid address error interrupt
 */
void __interrupt__ INV_AD_IT(void){

	while (1) {
        ;
    }

} /* INV_AD_IT */

/****************************************************//**
* Function name: PROG_ERR_IT
********************************************************/
/*!
   programm error interrupt
 */
void __interrupt__ PROG_ERR_IT(void){

	while (1) {
        ;
    }

} /* PROG_ERR_IT */

/****************************************************//**
* Function name: EXCHANGE_IT
********************************************************/
/*!
   exchange interrupt
 */
void __interrupt__ EXCHANGE_IT(void){

	while (1) {
        ;
    }

} /* EXCHANGE_IT */

/****************************************************//**
* Function name: TASK_RST_IT
********************************************************/
/*!
   task reset interrupt
 */
void __interrupt__ TASK_RST_IT(void){

	while (1) {
        ;
    }

} /* TASK_RST_IT */

/****************************************************//**
* Function name: WD_ATT_IT
********************************************************/
/*!
   watchdog attention interrupt
 */
void __interrupt__ WD_ATT_IT(void){

	while (1) {
        ;
    }

} /* WD_ATT_IT */

/****************************************************//**
* Function name: M4_MUTEX_IT
********************************************************/
/*!
   MLX4 mutex interrupt
 */
void __interrupt__ M4_MUTEX_IT(void){

	while (1) {
        ;
    }

} /* M4_MUTEX_IT */

/****************************************************//**
* Function name: EE_IT
********************************************************/
/*!
   eeprom interrupt
 */
void __interrupt__ EE_IT(void){

	while (1) {
        ;
    }

} /* EE_IT */

/*
 ******************************************************************************
 * Starts Transport Layer N_As_LIN timeout (1000 mS)
 * See LIN2.1 spec section 3.2.5 TIMING CONSTRAINTS
 ******************************************************************************
 */
void mlu_TL_TimeoutStart (void){

	tmrTL = TO_N_AS_LIN; /* MUST be compiled into atomic operation */

} /* mlu_TL_TimeoutStart */

