/*! \file		Diagnostic.h
 *  \brief		MLX81300 ErrorCodes handling
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-03-20
 *   
 * \version 	1.0 - preliminary
 *
 *
 * MELEXIS Microelectronic Integrated Systems
 * 
 * Copyright (C) 2012-2013 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * ****************************************************************************	*/

#ifndef ERRORCODES_H_
#define ERRORCODES_H_

#ifndef SYSLIB_H_
#include "syslib.h"
#endif /* SYSLIB_H_ */

#define C_ERR_NONE				0x00U
											/* Unsupported MLX16 IRQ's */
#define C_MLX16_IRQ_01			0x01U		/* STACK_IT (fatal.c) */
#define C_MLX16_IRQ_02			0x02U		/* PROT_ERR_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_03			0x03U		/* INV_AD_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_04			0x04U		/* PROG_ERR_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_05			0x05U		/* EXCHANGE_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_06			0x06U		/* TASK_RST_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_07			0x07U		/* WD_ATT_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_08			0x08U		/* M4_MUTEX_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_09			0x09U		/* M4_SHE_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_0A			0x0AU		/* TIMER_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_0B			0x0BU		/* ADC_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_0C			0x0CU		/* EE_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_0D			0x0DU		/* EXT0_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_0E			0x0EU		/* EXT1_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_0F			0x0FU		/* EXT2_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_10			0x10U		/* EXT3_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_11			0x11U		/* EXT4_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_12			0x12U		/* SOFT_IT (vectors-(no)lin.S) */
#define C_MLX16_MAIN_RET		0x18U		/* (low_level_init.c) */
#define C_MLX16_MAIN_FATAL		0x19U		/* (fatal.c) */
/* LIN Communication errors */
#define C_ERR_LIN_COMM			0x80U		/* 0x81: Short-Done
											 * 0x82: Crash/LIN Module reset
											 * 0x83: ID Parity error
											 * 0x84: Checksum error
											 * 0x85: Transmission (S2M) bit error
											 * 0x86: Data Framing
											 * 0x87: ID Framing (Stop-bit)
											 * 0x88: Synch field
											 * 0x89: Buffer locked (LIN message OVL)
											 * 0x8A: Short
											 * 0x8B: Time-out response
											 * 0x8C: Break detected
											 * 0x8F: Wake-up initialisation
											 * 0x90: Error during STOP bit transmission
											 */
/* Application errors */
#define C_ERR_APPL_UNDER_TEMP	0xA0U		/* Application: Under Temperature */
#define C_ERR_APPL_OVER_TEMP	0xA1U		/* Application: Over Temperature */
#define C_ERR_APPL_UNDER_VOLT	0xA2U		/* Application: Under Voltage */
#define C_ERR_APPL_OVER_VOLT	0xA3U		/* Application: Over Voltage */
#define C_ERR_APPL_SPI_INIT		0xA4U		/* Soft SPI Initialization failed */
#define C_ERR_APPL_SPI_NOT_INIT	0xA5U		/* Soft SPI not initialized */
#define C_ERR_APPL_SPI_READ		0xA6U		/* Soft SPI Read failure */
#define C_ERR_APPL_SPI_WRITE	0xA7U		/* Soft SPI Write failure */
#define C_ERR_APPL_STOP			0xA8U		/* Application stop */
/* LIN frame errors */
#define C_ERR_LIN2X_B0			0xB0U		/* LIN 2.x NAD Change error */
#define C_ERR_LIN2X_B1			0xB1U		/* LIN 2.x Assign MessageID to FrameID failed/unsupported */
#define C_ERR_LIN2X_B2			0xB2U		/* LIN 2.x Read by Identifier (unsupported ID) */
#define C_ERR_LIN2X_B3			0xB3U		/* n.a. */
#define C_ERR_LIN2X_B4			0xB4U		/* LIN 2.x Assign Variant-ID, HW-Ref and SW-Ref */
#define C_ERR_LIN2X_B5			0xB5U		/* LIN 2.x Auto Addressing failure */
#define C_ERR_LIN2X_B6			0xB6U		/* LIN 2.x Assign Group-address failure */
#define C_ERR_LIN2X_B7			0xB7U		/* LIN 2.x Assign Frame-ID range failure */
#define C_ERR_LIN2X_LINAA		0xB8U		/* LIN 2.x Non LIN-AA Diagnostic command during LIN-AA mode */
#define C_ERR_LIN2X_WRITE		0xB9U		/* LIN 2.x Write not allowed (not stop-mode) */
#define C_ERR_LIN_INV_PARAM		0xBAU		/* LIN Invalid Parameter */
#define C_ERR_LIN_INV_PLA_KEY	0xBBU		/* LIN Invalid Pla "key" */
#define C_ERR_LIN_INV_CMD		0xBCU		/* LIN Invalid Command Request */
#define C_ERR_LIN2X_CB			0xBDU		/* LIN 2.x Write by Identifier (unsupported ID, or incorrect) */
#define C_ERR_PWM_BUS_TIMEOUT	0xBEU		/* PWM Bus-timeout */
#define C_ERR_LIN_BUS_TIMEOUT	0xBFU		/* LIN Bus-timeout */
/* MLX NVRAM Page errors */
#define C_ERR_INV_USERPAGE_1	0xC0U		/* Invalid NVRAM User Page #1 (use User Page #2) */
#define C_ERR_INV_USERPAGE_2	0xC1U		/* Invalid NVRAM User Page #2 (use User Page #1) */
#define C_ERR_INV_USERPAGE_BOTH	0xC2U		/* Invalid NVRAM User Page #1 & #2 (using defaults) */
#define C_ERR_INV_NAD			0xC3U		/* Invalid Node Address */
#define C_ERR_INV_MLXPAGE_CRC1	0xC8U		/* Invalid MLX NVRAM CRC #1 */
#define C_ERR_INV_MLXPAGE_CRC2	0xC9U		/* Invalid MLX NVRAM CRC #2 */
#define C_ERR_INV_MLXPAGE_CRC3	0xCAU		/* Invalid MLX NVRAM CRC #3 */
#define C_ERR_INV_MLXPAGE_CRC4	0xCBU		/* Invalid MLX NVRAM CRC #4 */
#define C_ERR_INV_MLXPAGE_CRC5	0xCCU		/* Invalid MLX NVRAM CRC #5 (Recovered) */
#define C_ERR_NVRAM_MLX_CAL_GN	0xCFU		/* MLX Calibration Gain (Zero) */
/* Chip Diagnostics errors */
#define C_ERR_DIAG_OVER_CURRENT	0xD0U		/* Diagnostic: Over Current */
#define C_ERR_DIAG_OVER_TEMP	0xD1U		/* Diagnostic: Over Temperature */
#define C_ERR_DIAG_UNDER_VOLT	0xD2U		/* Diagnostic: Under Voltage */
#define C_ERR_DIAG_OVER_VOLT	0xD3U		/* Diagnostic: Over Voltage */
#define C_ERR_DIAG_VDS			0xD4U		/* Diagnostic: Vds (MMP161119-1) */
#define C_ERR_VREF				0xD5U		/* ADC VREF check (MMP170222-5) */
#define C_ERR_VDDA				0xD6U		/* ADC VDDA check (MMP170222-4) */
#define C_ERR_VDDD				0xD7U		/* ADC VDDD check (MMP170222-4) */
#define C_ERR_VIO_0				0xD8U		/* ADC VIO[0] check (MMP170302-1) */
#define C_ERR_VIO_1				0xD9U		/* ADC VIO[1] check (MMP170302-1) */
#define C_ERR_VIO_2				0xDAU		/* ADC VIO[2] check (MMP170302-1) */
#define C_ERR_VIO_3				0xDBU		/* ADC VIO[3] check (MMP170302-1) */
#define C_ERR_VIO_4				0xDCU		/* ADC VIO[4] check (MMP170302-1) */
#define C_ERR_VIO_5				0xDDU		/* ADC VIO[5] check (MMP170302-1) */
#define C_ERR_VIO_6				0xDEU		/* ADC VIO[6] check (MMP170302-1) */
#define C_ERR_VIO_7				0xDFU		/* ADC VIO[7] check (MMP170302-1) */
/* Electric Errors */
#define C_ERR_TESTMODE_0		0xE0U
#define C_ERR_TESTMODE_1		0xE1U
#define C_ERR_COIL_ZERO_CURRENT	0xE6U		/* Actuator coil zero current (open) */
#define C_ERR_COIL_OVER_CURRENT	0xE7U		/* Actuator coil over current (short) */
#define C_ERR_CHIP_TEMP_PROFILE	0xE8U		/* Chip temperature profile error */
#define C_ERR_PHASE_SHORT_GND	0xE9U		/* Motor driver phase to GND short */
#define C_ERR_SELFTEST_A		0xEAU		/* Self-Test: FET Shortage (Over Current) */
#define C_ERR_SELFTEST_B		0xEBU		/* Self-Test: Short with other Phase (Over Current) */
#define C_ERR_SELFTEST_C		0xECU		/* Self-Test: Open Phase */
#define C_ERR_SELFTEST_D		0xEDU		/* Self-Test: Phase (H) */
#define C_ERR_SELFTEST_E		0xEEU		/* Self-Test: Phase (L) */
#define C_ERR_SELFTEST_F		0xEFU		/* Self-Test: - */
/* Fatal errors */
#define C_ERR_FATAL				0xF0U		/* Fatal-Error */
#define C_ERR_WD_RST			0xF1U		/* (Digital) Watchdog-reset */
#define C_ERR_AWD_RST			0xF2U		/* Analogue Watchdog-reset */
#define C_ERR_WD_AWD_RST		0xF3U		/* Analogue & Digital Watchdog-reset */
#define C_ERR_NVRAM_PG11		0xF4U		/* NVRAM Page 1.1 Write error */
#define C_ERR_NVRAM_PG12		0xF5U		/* NVRAM Page 1.2 Write error */
#define C_ERR_NVRAM_PG21		0xF6U		/* NVRAM Page 2.1 Write error */
#define C_ERR_RAM				0xF7U		/* RAM Complete-test error */
#define C_ERR_RAM_BG			0xF8U		/* RAM Background test error */
#define C_ERR_FLASH_BG			0xF9U		/* Flash Background test error */
#define C_ERR_MLX4_RESTART		0xFAU		/* MLX4 have been restarted */
#define C_ERR_FLASH_CRC_FAIL	0xFBU		/* Flash CRC failure during HTOL/Burn-in */
#define C_ERR_IOREG				0xFCU		/* I/O-registers error */
#define C_ERR_FATAL_EMRUN		0xFEU		/* Fatal Emergency-run */

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
void ErrorLogInit( void);
void SetLastError( uint8 u8ErrorCode);
uint8 GetLastError( void);

#endif /* ERRORCODES_H_ */

/* EOF */
