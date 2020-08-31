/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef ITC2_H_
#define ITC2_H_

#include <extintlib.h>


/* Connected to EXT0_IT */
extern  void TMR1_Capture_A_Interrupt (void);
extern  void TMR1_Capture_B_Interrupt (void);
extern  void TMR1_Compare_A_Interrupt (void);
extern  void TMR1_Compare_B_Interrupt (void);
extern  void TMR1_Overflow_Interrupt (void);
 
extern  void TMR3_Capture_A_Interrupt (void);
extern  void TMR3_Capture_B_Interrupt (void);
extern  void TMR3_Compare_A_Interrupt (void);
extern  void TMR3_Compare_B_Interrupt (void);
extern  void TMR3_Overflow_Interrupt (void);

/* Connected to EXT1_IT */
extern  void TMR2_Capture_A_Interrupt (void);
extern  void TMR2_Capture_B_Interrupt (void);
extern  void TMR2_Compare_A_Interrupt (void);
extern  void TMR2_Compare_B_Interrupt (void);
extern  void TMR2_Overflow_Interrupt (void);
 
extern  void TMR4_Capture_A_Interrupt (void);
extern  void TMR4_Capture_B_Interrupt (void);
extern  void TMR4_Compare_A_Interrupt (void);
extern  void TMR4_Compare_B_Interrupt (void);
extern  void TMR4_Overflow_Interrupt (void);

/* Connected to EXT2_IT */
extern  void PWM1_Compare_Interrupt (void);
extern  void PWM1_Counter_Interrupt (void);

extern  void PWM2_Compare_Interrupt (void);
extern  void PWM2_Counter_Interrupt (void);

extern  void PWM3_Compare_Interrupt (void);
extern  void PWM3_Counter_Interrupt (void);

extern  void PWM4_Compare_Interrupt (void);
extern  void PWM4_Counter_Interrupt (void);

extern  void PWM5_Compare_Interrupt (void);
extern  void PWM5_Counter_Interrupt (void);

extern  void PWM6_Compare_Interrupt (void);
extern  void PWM6_Counter_Interrupt (void);

extern  void PWM7_Compare_Interrupt (void);
extern  void PWM7_Counter_Interrupt (void);

extern  void PWM8_Compare_Interrupt (void);
extern  void PWM8_Counter_Interrupt (void);

/* Connected to EXT3_IT */
extern  void SPI1_RX_Interrupt (void);
extern  void SPI1_TX_Interrupt (void);

extern  void SPI2_RX_Interrupt (void);
extern  void SPI2_TX_Interrupt (void);

extern  void UART1_RR_Interrupt (void);
extern  void UART1_TR_Interrupt (void);
extern  void UART1_RS_Interrupt (void);
extern  void UART1_TS_Interrupt (void);
extern  void UART1_SB_Interrupt (void);
extern  void UART1_TE_Interrupt (void);

extern  void UART2_RR_Interrupt (void);
extern  void UART2_TR_Interrupt (void);
extern  void UART2_RS_Interrupt (void);
extern  void UART2_TS_Interrupt (void);
extern  void UART2_SB_Interrupt (void);
extern  void UART2_TE_Interrupt (void);

#endif /* ITC2_H_ */
