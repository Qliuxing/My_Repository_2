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


/* Connected to EXT2_IT */
extern  void PWM1_Compare_Interrupt (void);
extern  void PWM1_Counter_Interrupt (void);

extern  void PWM2_Compare_Interrupt (void);
extern  void PWM2_Counter_Interrupt (void);

extern  void PWM3_Compare_Interrupt (void);
extern  void PWM3_Counter_Interrupt (void);

extern  void PWM4_Compare_Interrupt (void);
extern  void PWM4_Counter_Interrupt (void);

/* Connected to EXT3_IT */
extern  void SPI1_RX_Interrupt (void);
extern  void SPI1_TX_Interrupt (void);

/* Connected to EXT4_IT */
extern  void LV0_Interrupt (void);
extern  void LV1_Interrupt (void);
extern  void LV2_Interrupt (void);
extern  void LV3_Interrupt (void);
extern  void LV4_Interrupt (void);
extern  void LV5_Interrupt (void);
extern  void LV6_Interrupt (void);
extern  void LV7_Interrupt (void);

extern  void HV0_Interrupt (void);
extern  void HV1_Interrupt (void);
extern  void HV2_Interrupt (void);
extern  void HV3_Interrupt (void);

//extern  void Unused_Interrupt (void);

extern  void OV_VS_Interrupt (void);
extern  void UV_VS_Interrupt (void);

extern  void OVT_Interrupt (void);

#endif /* ITC2_H_ */
