/*
 * Copyright (C) 2013 Melexis N.V.
 *
 * Software Platform
 */

#ifndef MLX_EEP_MAP_H_
#define MLX_EEP_MAP_H_

#include <syslib.h>

/*
 * Trimming map
 */
extern volatile uint16 EEP_ANA_OUTB_PT50 __attribute__((nodp, addr(0x11A8)));  /* VDD, Bandgap, Bias trimming at 50degC, FT50      */
extern volatile uint16 EEP_ANA_OUTC_PT50 __attribute__((nodp, addr(0x11AA)));  /* PLL and RC Osc trimming at 50degC, FT50          */
extern volatile uint16 EEP_ANA_OUTD_PT50 __attribute__((nodp, addr(0x11AC)));  /* ADC references 1 and 2 trimming at 50degC, FT50  */
extern volatile uint16 EEP_ANA_OUTE_PT50 __attribute__((nodp, addr(0x11AE)));  /* ADC references 3 trimming at 50degC, FT50        */
extern volatile uint16 EEP_ANA_OUTI_PT50 __attribute__((nodp, addr(0x11CE)));  /* LIN pull-up, slew rate and VCM*/

extern volatile uint16 EEP_SAGAIN        __attribute__((nodp, addr(0x11DE)));  /* Gain of the shunt amplifier */
extern volatile uint16 EEP_CCM           __attribute__((nodp, addr(0x11DC)));  /* CMRR calibration coefficient */


extern volatile uint16 EEP_ITH0          __attribute__((nodp, addr(0x11EC)));  /* LIN auto-addressing current threshold 0 */
extern volatile uint16 EEP_ITH1          __attribute__((nodp, addr(0x11EA)));  /* LIN auto-addressing current threshold 1 */
extern volatile uint16 EEP_ITH2          __attribute__((nodp, addr(0x11E8)));  /* LIN auto-addressing current threshold 2 */
extern volatile uint16 EEP_ITH3          __attribute__((nodp, addr(0x11E6)));  /* LIN auto-addressing current threshold 3 */
extern volatile uint16 EEP_ITH4          __attribute__((nodp, addr(0x11E4)));  /* LIN auto-addressing current threshold 4 */

extern volatile uint16 EEP_CURRDAC_450uA_35   __attribute__((nodp, addr(0x11E2)));  /* Calibration value for 0.45mA Current DAC at 35 deg C */
extern volatile uint16 EEP_CURRDAC_2_05mA_35  __attribute__((nodp, addr(0x11E0)));  /* Calibration value for 2.05mA Current DAC at 35 deg C */

extern volatile uint16 EEP_CURRDAC_4mA_35 __attribute__((nodp, addr(0x11F0)));  /* Calibration value for 4mA Current DAC at 35 deg C */
extern volatile uint16 EEP_ISHUNT35       __attribute__((nodp, addr(0x11F4)));  /* Calibration value for Rshunt at 35 deg C */


extern volatile uint8  EEP_FLASH_ERASE_SIZE  __attribute__((nodp, addr(0x11B2))); /* Erase size in pages: 1 (or 0) for H11, 16 for H12 */
extern volatile uint16 EEP_TM_TR_LSW         __attribute__((nodp, addr(0x11B4))); /* Flash trimming bit [15:0] */
extern volatile uint16 EEP_TM_TR_MSW         __attribute__((nodp, addr(0x11B6))); /* Flash trimming bit [31:16] */

extern volatile uint16 EEP_mTempHigh      __attribute__((nodp, addr(0x11BC))); /* Raw ADC value of internal temperature sensor @125C (measured with 2.5V ADC reference voltage)*/
extern volatile uint16 EEP_mTempMid       __attribute__((nodp, addr(0x11BA))); /* Raw ADC value of internal temperature sensor @35C (measured with 2.5V ADC reference voltage)*/
extern volatile uint16 EEP_mTempLow       __attribute__((nodp, addr(0x11B8))); /* Raw ADC value of internal temperature sensor @-40C (measured with 2.5V ADC reference voltage)*/

extern volatile uint16 EEP_NVRAM2_TRIM         __attribute__((nodp, addr(0x11FE))); /* NVRAM2 trimming value */
extern volatile uint16 EEP_NVRAM1_TRIM_SAVE    __attribute__((nodp, addr(0x11F8))); /* NVRAM1 trimming value stored in NVRAM2*/
extern volatile uint16 EEP_NVRAM2_TRIM_CRC     __attribute__((nodp, addr(0x11F6))); /* NVRAM2 16bit CRC for trimming value area in NVRAM2*/

extern volatile uint16 EEP_NVRAM1_TRIM         __attribute__((nodp, addr(0x10FE))); /* NVRAM1 trimming value */
extern volatile uint16 EEP_NVRAM1_TRIM_CRC     __attribute__((nodp, addr(0x11FC))); /* NVRAM1 16bit CRC for trimming value area in NVRAM1*/

#endif /* MLX_EEP_MAP_H_ */

