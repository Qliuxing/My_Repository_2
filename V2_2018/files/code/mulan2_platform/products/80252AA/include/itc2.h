/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef ITC2_H_
#define ITC2_H_

/* ----------------------------------------------------------------------------
 * Enables SPI_IRQ interrupt
 *
 * SW priority: 1 (fixed)
 * MASK[1], PEND[1]
 */
#define SPI_IRQ_ENABLE()         \
do {                            \
    PEND  = (1 << 1);      		\
    MASK |= (1 << 1);       	\
} while(0)

/*
 * Disables (masks) SPI_IRQ interrupt
 */
#define SPI_IRQ_DISABLE()     \
	( MASK &= ~(1 << 1) )

/* ----------------------------------------------------------------------------
 * Enables TIMER1_IRQ interrupt
 *
 * SW priority: 2 (fixed)
 * MASK[3], PEND[3]
 */
#define TIMER1_IRQ_ENABLE()     \
do {                            \
    PEND  = (1 << 3);      		\
    MASK |= (1 << 3);       	\
} while(0)

/*
 * Disables (masks) TIMER1_IRQ interrupt
 */
#define TIMER1_IRQ_DISABLE()     \
	( MASK &= ~(1 << 3) )

/* ----------------------------------------------------------------------------
 * Enables TIMER2_IRQ interrupt
 *
 * SW priority: 5 (fixed)
 * MASK[4], PEND[4]
 */
#define TIMER2_IRQ_ENABLE()     \
do {                            \
    PEND  = (1 << 4);      		\
    MASK |= (1 << 4);       	\
} while(0)

/*
 * Disables (masks) TIMER2_IRQ interrupt
 */
#define TIMER2_IRQ_DISABLE()     \
	( MASK &= ~(1 << 4) )

/* ----------------------------------------------------------------------------
 * Enables IO_PWM_IRQ interrupt and sets its priority to 'prio'
 * prio: 3..6
 *
 * PRIO[13:12], MASK[11], PEND[11]
 */
#define IO_PWM_IRQ_ENABLE(prio)             \
do {                                        \
    PRIO =            (PRIO & ~(3 << 12))   \
            | ((((prio) - 3) & 3) << 12);   \
    PEND  = (1 << 11);                      \
    MASK |= (1 << 11);                      \
} while(0)

/*
 * Disables (mask) IO_PWM_IRQ interrupt
 */
#define IO_PWM_IRQ_DISABLE()   \
    ( MASK &= ~(1 << 11) )

/* ----------------------------------------------------------------------------
 * Enables DIAG_IRQ interrupt and sets its priority to 'prio'
 * prio: 3..6
 *
 * PRIO[15:14], MASK[12], PEND[12]
 */
#define DIAG_IRQ_ENABLE(prio)               \
do {                                        \
    PRIO =        (PRIO & ~(3 << 14))       \
        | ((((prio) - 3) & 3) << 14);       \
    PEND  = (1 << 12);                      \
    MASK |= (1 << 12);                      \
} while(0)

/*
 * Disables (mask) DIAG_IRQ interrupt
 */
#define DIAG_IRQ_DISABLE()   \
    ( MASK &= ~(1 << 12) )

/* Connected to SPI_IRQ */
extern void SPI_TxFrameNotification (void);
extern void SPI_RxFrameNotification (void);
extern void SPI_OverflowNotification (void);

/* Connected to TIMER1_IRQ */
extern void TMR1_Int1Notification (void);
extern void TMR1_Int2Notification (void);
extern void TMR1_Int3Notification (void);
extern void TMR1_Int4Notification (void);
extern void TMR1_Int5Notification (void);

/* Connected to TIMER2_IRQ */
extern void TMR2_Int1Notification (void);
extern void TMR2_Int2Notification (void);
extern void TMR2_Int3Notification (void);
extern void TMR2_Int4Notification (void);
extern void TMR2_Int5Notification (void);

/* Connected to IO_PWM_IRQ */
extern void IO_Ext0Notification (void);
extern void IO_Ext1Notification (void);
extern void IO_Ext2Notification (void);
extern void IO_Ext3Notification (void);
extern void IO_Ext4Notification (void);
extern void IO_Ext5Notification (void);
extern void PWMI_Notification (void);
extern void PWMA_Notification (void);

/* Connected to DIAG_IRQ */
extern void DIAG_OvercurrentNotification (void);
extern void DIAG_VgsErrorNotification (void);
extern void DIAG_VdsErrorNotification (void);
extern void DIAG_OvertempNotification (void);
extern void DIAG_VboostUvNotification  (void);
extern void DIAG_VregUvNotification (void);
extern void DIAG_VsupUvNotification (void);
extern void DIAG_VsupOvNotification (void);
extern void DIAG_CurrRegNotification (void);


#endif /* ITC2_H_ */
