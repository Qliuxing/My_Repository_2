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
extern volatile uint16 EEP_ANA_OUTA __attribute__((nodp, addr(0x11F0)));      /* ANA_OUTA */
extern volatile uint16 EEP_ANA_OUTG __attribute__((nodp, addr(0x11E8)));      /* ANA_OUTG */

extern volatile uint16 EEP_ANA_OUTB_PT35 __attribute__((nodp, addr(0x11A8))); /* VDD, Bandgap, Bias trimming at 50degC, FT50      */
extern volatile uint16 EEP_ANA_OUTC_PT35 __attribute__((nodp, addr(0x11AA))); /* PLL and RC Osc trimming at 50degC, FT50          */
extern volatile uint16 EEP_ANA_OUTD_PT35 __attribute__((nodp, addr(0x11AC))); /* ADC references 1 and 2 trimming at 50degC, FT50  */
extern volatile uint16 EEP_ANA_OUTE_PT35 __attribute__((nodp, addr(0x11AE))); /* ADC references 3 trimming at 50degC, FT50        */
extern volatile uint16 EEP_ANA_OUTF_PT35 __attribute__((nodp, addr(0x11E2))); /* reference for 8bit DAC trimming at 50degC - only low 5 bits are relevant; FT50 */

extern volatile uint8  EEP_FLASH_ERASE_SIZE  __attribute__((nodp, addr(0x11B2))); /* Erase size in pages: 1 (or 0) for H11, 16 for H12 */
extern volatile uint16 EEP_TM_TR_LSW         __attribute__((nodp, addr(0x11B4))); /* Flash trimming bit [15:0] */
extern volatile uint16 EEP_TM_TR_MSW         __attribute__((nodp, addr(0x11B6))); /* Flash trimming bit [31:16] */

#endif /* MLX_EEP_MAP_H_ */

