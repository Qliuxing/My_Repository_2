/*
 * Copyright (C) 2008-2015 Melexis N.V.
 *
 * Software Platform
 *
 *
 *
 *
 * Note: NVRAM can only be accessed at even addresses as 16-bits words.
 */

#ifndef LIB_MLX81315_NVRAM_H_
#define LIB_MLX81315_NVRAM_H_

/*
 * Trimming map
 * from file TrimvalsMapNVSRAM_V3p0.xls
 */
/* NVRAM - Melexis Page (Page 2.2) */
typedef struct _MLX_CALIBRATION_PARAMS
{
	/* MLX Production data */
	uint16 EE_CRC1_RES;															/* 0x1180: LSB: CRC1: 00-3F; MSB: Reserved */
	uint16 EE_HW_SW_ID;															/* 0x1182: LSB: HW-ID, MSB: SW-ID */
	uint16 EEDATA_04;															/* 0x1184: MLX Production data */
	uint16 EEDATA_06;															/* 0x1186: MLX Production data */
	uint16 EE_CHIPID_00;														/* 0x1188: Chip ID, nibble-based */
	uint16 EE_CHIPID_02;														/* 0x118A: Chip ID */
	uint16 EE_CHIPID_04;														/* 0x118C: Chip ID */
	uint16 EE_CHIPID_06;														/* 0x118E: Chip ID */
	uint16 EEDATA_10;						  									/* 0x1190: MLX Production data */
	uint16 EEDATA_12;							 								/* 0x1192: MLX Production data */
	uint16 EEDATA_14;							 								/* 0x1194: MLX Production data */
	uint16 EEDATA_16;							 								/* 0x1196: MLX Production data */
	uint16 EEDATA_18;															/* 0x1198: MLX Production data */
	uint16 EEDATA_1A;		  													/* 0x119A: MLX Production data */
	uint16 EEDATA_1C;								   							/* 0x119C: MLX Production data */
	uint16 EEDATA_1E;				  											/* 0x119E: MLX Production data */
	uint16 EEDATA_20;										   					/* 0x11A0: MLX Production data */
	uint16 EEDATA_22;											  				/* 0x11A2: MLX Production data */
	uint16 EEDATA_24;							 	   							/* 0x11A4: MLX Production data */
	uint16 EEDATA_26;												   			/* 0x11A6: MLX Production data */
	uint16 EE_ANA_OUTB;															/* 0x11A8: Trim value for ANA_OUTB */
	uint16 EE_ANA_OUTC;															/* 0x11AA: Trim value for ANA_OUTC */
	uint16 EE_ANA_OUTD;															/* 0x11AC: Trim value for ANA_OUTD */
	uint16 EE_ANA_OUTE;															/* 0x11AE: Trim value for ANA_OUTE */
	uint16 EEDATA_30;															/* 0x11B0: Reserved */
	uint16 EEDATA_32;															/* 0x11B2: Reserved */
	uint16 EE_TM_TR_LSW;														/* 0x11B4: TM_TR[15:00] */
	uint16 EE_TM_TR_MSW;														/* 0x11B6: TM_TR[31:16] */
	uint16 EE_mTempLow;															/* 0x11B8: Raw temperature ADC measurement at TempHigh (-40 degrees Celsius) */
	uint16 EE_mTempMid;															/* 0x11BA: Raw temperature ADC measurement at TempMid  ( 35 degrees Celsius) */
	uint16 EE_mTempHigh;														/* 0x11BC: Raw temperature ADC measurement at TempLow  (125 degrees Celsius) */
	uint16 EEDATA_3E;															/* 0x11BE: MLX Production data - Reserved */
	/* MLX Chip calibration data: See also MLX81315 Calibration doc. */
	uint16 EE_CRC2_OCLOCK10K;													/* 0x11C0: LSB: CRC2 40-5D; MSB: OClock10K_User_EE */
	uint16 EE_SCLOCK10K_P_N;													/* 0x11C2: LSB: SClock10K_p_User_EE; MSB: SClock10K_n_User_EE */
	uint16 EE_OCLOCK_SCLOCK_P;													/* 0x11C4: LSB: OClock_User_EE; MSB: SClock_p_User_EE */
	uint16 EE_SCLOCK_N;															/* 0x11C6: LSB: SClock_n_User_EE; MSB: Reserved */
	uint16 EEDATA_48;															/* 0x11C8: MLX Calibration data - Reserved*/
	uint16 EE_TGAINCAL_RES;														/* 0x11CA: LSB: Temperature Gain; MSB: Reserved */
	uint16 EE_ADC_OFFS_GAIN_CAL;												/* 0x11CC: LSB: (int8) ADCOFFSCAL (Divided ADC channel offset), MSB: ADCGAINCAL (Divided ADC channel gain) */
	uint16 EE_V_OFFS_GAIN_CAL;													/* 0x11CE: LSB: (int8) VOFFSCAL (supply sensor filter offset), MSB: VGAINCAL (supply sensor filter gain) */
	uint16 EE_C_GAINS_CAL;														/* 0x11D0: LSB: CGAINCAL_FLT (Current sensor filter gain), MSB: CGAINCAL (Current sensor direct gain) */
	 int16 EE_C_OFFS_FLT_CAL;													/* 0x11D2: COFFSCAL_FLT (Current sensor filter offset) */
	 int16 EE_C_OFFS_CAL;														/* 0x11D4: COFFSCAL (Current sensor direct offset)  */
	 int16 EE_AA_GAIN_CAL;														/* 0x11D6: AAGAINCAL (Auto addressing gain calibration) */
	 int16 EE_AA_SDMCM;															/* 0x11D8: Common Mode Rejection factor */
	 int16 EE_AA_OCM;															/* 0x11DA: Common Mode Offset */
	 int16 EE_AA_ODM;															/* 0x11DC: Differential Mode Offset */
	/* Other chip values */
	uint16 EE_CRC3_RES;															/* 0x11DE: LSB: CRC3: 5E-75; MSB: Reserved */
	uint16 EE_ANA_OUTF_PT50;													/* 0x11E0: Trim value for ANA_OUTF, PT50, bit[4:0] */
	uint16 EE_ANA_OUTF_FT50;													/* 0x11E2: Trim value for ANA_OUTF, FT50, bit[4:0] */
	uint16 EEDATA_64;															/* 0x11E4: MLX - Reserved */
	uint16 EEDATA_66;															/* 0x11E6: MLX - Reserved */
	uint16 EEDATA_68;															/* 0x11E8: MLX - Reserved */
	uint16 EEDATA_6A;															/* 0x11EA: MLX - Reserved */
	uint16 EE_ANA_OUTH_112_205;													/* 0x11EC: ANA_OUTH LSB for LIN-AA current 1.12mA (LSB) and 2.05mA (MSB) */
	uint16 EE_ANA_OUTH_350_800;													/* 0x11EE: ANA_OUTH LSB for LIN-AA current 3.50mA (LSB) and 8.00mA (MSB) */
	uint16 EE_ANA_OUTA;															/* 0x11F0: Trim value for ANA_OUTA */
	uint16 EE_MLX81315CA_1;														/* 0x11F2: MLX81315CA Features */
	uint16 EE_MLX81315CA_2;														/* 0x11F4: MLX81315CA Features(2) */
	/* Flash/NVRAM Trimming */
	uint16 EE_CRC4_RES;															/* 0x11F6: LSB: CRC4: 76-7F; MSB:  Reserved */
	uint16 EE_NVSRAM1_TRIM;														/* 0x11F8: NVSRAM1 Trim value */
	uint16 EE_FLASH_TRIM_LSW;													/* 0x11FA: Flash-trim (LSW) */
	uint16 EE_FLASH_TRIM_MSW;													/* 0x11FC: Flash-trim (MSW) */
	uint16 EE_NVSRAM2_TRIM;														/* 0x11FE: NVSRAM2 Trim value */
} MLX_CALIBRATION_PARAMS, *PMLX_CALIBRATION_PARAMS;

extern volatile MLX_CALIBRATION_PARAMS CalibrationParams __attribute((nodp, addr(0x1180)));

#define EE_GLAA			(CalibrationParams.EE_AA_GAIN_CAL)
#define EE_ODMAA		(CalibrationParams.EE_AA_ODM)
#define EE_OCMAA		(CalibrationParams.EE_AA_OCM)
#define EE_GDMCMAA		(CalibrationParams.EE_AA_SDMCM)
#define EE_OMCURR		(CalibrationParams.EE_C_OFFS_FLT_CAL)
#define EE_GMCURR		((CalibrationParams.EE_C_GAINS_CAL & 0xFF) + 256)
#define EE_OVOLTAGE		((int16)((CalibrationParams.EE_V_OFFS_GAIN_CAL & 0xFF) << 8) >> 8)	/* 8-bit signed */
#define EE_GVOLTAGE		(CalibrationParams.EE_V_OFFS_GAIN_CAL >> 8)
#define EE_OADC			((int16)((CalibrationParams.EE_ADC_OFFS_GAIN_CAL & 0xFF) << 8) >> 8)	/* 8-bit signed */
#define EE_GADC			(CalibrationParams.EE_ADC_OFFS_GAIN_CAL >> 8)
#define EE_LOWTEMP		(CalibrationParams.EE_mTempLow)
#define EE_OTEMP		(CalibrationParams.EE_mTempMid)
#define EE_HIGHTEMP		(CalibrationParams.EE_mTempHigh)
#define EE_GTEMP		(CalibrationParams.EE_TGAINCAL_RES & 0x00FF)
#define EE_MIDTEMP		35
#define EE_FEAT			(CalibrationParams.EE_MLX81300CA_1)

#define EE_OCLOCK		((int16)((CalibrationParams.EE_OCLOCK_SCLOCK_P & 0xFF) << 8) >> 8)	/* 8-bit signed */
#define EE_GPCLOCK		((int16)  CalibrationParams.EE_OCLOCK_SCLOCK_P >> 8)				/* 8-bit signed */
#define EE_GNCLOCK		((int16)((CalibrationParams.EE_SCLOCK_N & 0xFF) << 8) >> 8)			/* 8-bit signed */

#define C_GTEMP_DIV		256														/* Temperature gain divider (MMP110822-2) */
#define C_GVOLTAGE_DIV	64														/* Supply gain (filter) divider */
#define C_GMCURR_DIV	128														/* Motor Current gain (filter) divider */

extern volatile uint16 EEP_ANA_OUTA __attribute((nodp, addr(0x11F0)));			/* ANA_OUTA */
extern volatile uint16 EEP_ANA_OUTG __attribute((nodp, addr(0x11E8)));			/* ANA_OUTG */

/* --- Probing Test calibration --- */
extern volatile uint16 EEP_ANA_OUTB_PT35 __attribute((nodp, addr(0x11A8)));		/* VDD, Bandgap, Bias trimming at 50degC, FT50      */
extern volatile uint16 EEP_ANA_OUTC_PT35 __attribute((nodp, addr(0x11AA)));		/* PLL and RC Osc trimming at 50degC, FT50          */
extern volatile uint16 EEP_ANA_OUTD_PT35 __attribute((nodp, addr(0x11AC)));		/* ADC references 1 and 2 trimming at 50degC, FT50  */
extern volatile uint16 EEP_ANA_OUTE_PT35 __attribute((nodp, addr(0x11AE)));		/* ADC references 3 trimming at 50degC, FT50        */
/*extern volatile uint16 EEP_ANA_OUTF_PT35 __attribute((nodp, addr(0x11E2)));*/	/* reference for 8bit DAC trimming at 50degC - only low 5 bits are relevant; FT50 */

extern volatile uint8  EEP_FLASH_ERASE_SIZE __attribute((nodp, addr(0x11B2)));  /* Erase size in pages: 1 (or 0) for H11, 16 for H12 */
extern volatile uint16 EEP_TM_TR_LSW        __attribute((nodp, addr(0x11B4)));  /* Flash trimming bit [15:0] */
extern volatile uint16 EEP_TM_TR_MSW        __attribute((nodp, addr(0x11B6)));  /* Flash trimming bit [31:16] */

#define BGN_NVRAM1_PAGE1_ADDRESS      0x1000U									/* Begin address NVRAM1 Page1 (uint16) */
#define END_NVRAM1_PAGE1_ADDRESS      0x107FU									/* Last address NVRAM1 Page1  (uint16) */

#define BGN_NVRAM1_PAGE2_ADDRESS      0x1080U									/* Begin address NVRAM1 Page2 (uint16) */
#define END_NVRAM1_PAGE2_ADDRESS      0x10FBU									/* Last address NVRAM1 Page2  (uint16) */

#define BGN_MLX_PATCH_ADDR            0x10ECU									/* Begin address ROM Patch vector table (uint16) */
#define END_MLX_PATCH_ADDR            0x10FBU									/* Last address ROM Patch vector table  (uint16) */

#define BGN_NVRAM2_PAGE1_ADDRESS      0x1100U									/* Begin address NVRAM2 Page1 (uint16) */
#define END_NVRAM2_PAGE1_ADDRESS      0x117FU									/* Last address NVRAM2 Page1 (uint16) */

#define BGN_MLX_CALIB_ADDRESS_AREA1   0x1180U									/* Begin address Melexis Calibration Page, Area #1 */
#define END_MLX_CALIB_ADDRESS_AREA1   0x11BFU									/* Last address Melexis Calibration Page, Area #1 */
#define BGN_MLX_CALIB_ADDRESS_AREA2   0x11C0U									/* Begin address Melexis Calibration Page, Area #2 */
#define END_MLX_CALIB_ADDRESS_AREA2   0x11DDU									/* Last address Melexis Calibration Page, Area #2 */
#define BGN_MLX_CALIB_ADDRESS_AREA3   0x11DEU									/* Begin address Melexis Calibration Page, Area #3 */
#define END_MLX_CALIB_ADDRESS_AREA3   0x11F5U									/* Last address Melexis Calibration Page, Area #3 */
#define BGN_MLX_CALIB_ADDRESS_AREA4   0x11F6U									/* Begin address Melexis Calibration Page, Area #4 */
#define END_MLX_CALIB_ADDRESS_AREA4   0x11FFU									/* Last address Melexis Calibration Page, Area #4 */
#define BGN_MLX_CALIB_ADDRESS_AREA5   0x10FCU									/* Begin address Melexis Calibration Page, Area #5 */
#define END_MLX_CALIB_ADDRESS_AREA5   0x10FFU									/* Last address Melexis Calibration Page, Area #5 */

/* nvram_CalcCRC()
 * const uint16_t *pu16BeginAddress: Start-address (16-bit aligned)
 * const uint16_t u16Length: Length (in 16-bit words)
 *
 * returns a 8-bit (extended to 16-bit) CRC (Sum with carry) calculation over the specified area.
 */
__MLX_TEXT__ static __inline__ uint16 nvram_CalcCRC( const uint16_t *pu16BeginAddress, const uint16_t u16Length)
{
    uint16 u16Result;
    uint16 u16Result2;  /* Clobbering of the register */
    uint16 u16Result3;  /* Clobbering of the register */
    __asm__ __volatile__
    (
        "mov    A, #0\n\t"              /* Clear CRC */
        "clrb   ML.7\n\t"               /* Clear CY */
        "adc    A, [%[start]++]\n\t"           /* Add value to CRC */
        "djnz   %[len], .-2\n\t"
        "adc    AL, AH\n\t"             /* Add CRC-H to CRC-L, including CY */
        "adc    AL, #0\n\t"
        "usex   A"                      /* Unsigned extend */
        : "=a" (u16Result), "=x" (u16Result2), "=y" (u16Result3)
        : [start] "y" (pu16BeginAddress), [len] "x" (u16Length)
    );
    return ( u16Result );
} /* End of nvram_CalcCRC() */

#endif /* LIB_MLX81315_NVRAM_H_ */
