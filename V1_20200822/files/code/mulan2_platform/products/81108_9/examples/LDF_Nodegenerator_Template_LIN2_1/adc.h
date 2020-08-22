/**********************************************
* Copyright (C) 2014 Melexis N.V.
*
* LDF_Nodegenerator_Template_LIN2_x
*
**********************************************/
/* History:
     Revision 1.0
       - Initial release
 **********************************************/

#ifndef ADC_H_
#define ADC_H_

/* ==========================================================================
 * Declaration public functions
 * ========================================================================== */
void initADC(void);
uint16 getADCVoltage(uint16 ADCChannelConfig);

#endif /* ADC_H_ */
