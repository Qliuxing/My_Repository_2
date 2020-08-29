/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef MMU_API_H_
#define MMU_API_H_

#include <ioports.h>

#define MMU_INTERRUPT_MASK      EN_EXT6_IT  /* MMU interrupt is connected to EXT6_IT */
#define MMU_PEND_PORT           PEND
#define MMU_MASK_PORT           MASK


typedef void (* Task_Type) (uint16_t);


/** ---------------------------------------------------------------------------
 * Get stack pointer register (S)
 *
 * \param       none
 *
 * \returns     Stack pointer value (S register)
 */
inline __attribute__((always_inline))
static uint16_t MMU_GetStackPointer (void)
{
    uint16_t sp;

    __asm__ __volatile__ (
        "mov %[dest], S" "\n\t"
        : [dest] "=r"(sp)
        :
    );

    return sp;
}


/** ---------------------------------------------------------------------------
 * Initializes and starts MMU
 *
 * \param   None
 *
 * \return  None
 *
 * \note
 * 1. MMU interrupt is also enabled by this function
 *
 */
inline __attribute__((always_inline))
static void MMU_Init (void)
{
    MMU_1 = MMU_EN1;
    MMU_2 = MMU_EN2;

    MMU_PEND_PORT  = MMU_INTERRUPT_MASK;    /* clear any pending requests .. */
    MMU_MASK_PORT |= MMU_INTERRUPT_MASK;    /* enable MMU interrupt */
}


/** ---------------------------------------------------------------------------
 * Configures MMU and executes function func with taskid credentials. Defines
 * stack limit for function func as stk_size words (stack elements).
 *
 * \note
 *  1. The purpose of the volatile modifier used with local variabels
 *     saved_task_ids and saved_stk_limits is to help mlx16 compiler with
 *     optimization, i.e. do not keep on stack locals after exiting the inline
 *     function. The volatile modifier should be removed when compiler will be
 *     mature enough to handle this itself.
 *
 * \param taskid    Bitmask of the task id bit (shall be MMU_TASK1_ID
 *                  or MMU_TASK2_ID)
 * \param func      Pointer to the function to be called
 * \param stk_size	Stack limit for the function func in words (stack elements)
 *
 * \return None
 */
inline __attribute__((always_inline))
static void MMU_RunAs (const uint16_t taskid, void * const func, uint16_t arg, const uint16_t stk_size)
{
    volatile  uint16_t saved_task_ids;
    volatile  uint16_t saved_stk_limits;


    saved_task_ids = MMU_TASK_IDS;
    MMU_TASK_IDS   = taskid;

    saved_stk_limits = MMU_STACK_LIMITS;

    uint16_t low_lmt = MMU_GetStackPointer() / 2;

    MMU_STACK_LIMITS = (low_lmt << 8) | (low_lmt + stk_size);

    ((Task_Type)func)(arg);

    MMU_STACK_LIMITS = saved_stk_limits;
    MMU_TASK_IDS     = saved_task_ids;

}


#endif /* MMU_API_H_ */
