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

#define C_ERR_NONE				0x00
											/* Unsupported MLX16 IRQ's */
#define C_MLX16_IRQ_01			0x01		/* STACK_IT (fatal.c) */
#define C_MLX16_IRQ_02			0x02		/* PROT_ERR_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_03			0x03		/* INV_AD_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_04			0x04		/* PROG_ERR_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_05			0x05		/* EXCHANGE_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_06			0x06		/* TASK_RST_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_07			0x07		/* WD_ATT_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_08			0x08		/* M4_MUTEX_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_09			0x09		/* M4_SHE_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_0A			0x0A		/* TIMER_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_0B			0x0B		/* ADC_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_0C			0x0C		/* EE_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_0D			0x0D		/* EXT0_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_0E			0x0E		/* EXT1_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_0F			0x0F		/* EXT2_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_10			0x10		/* EXT3_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_11			0x11		/* EXT4_IT (vectors-(no)lin.S) */
#define C_MLX16_IRQ_12			0x12		/* SOFT_IT (vectors-(no)lin.S) */
#define C_MLX16_MAIN_RET		0x18		/* (low_level_init.c) */
#define C_MLX16_MAIN_FATAL		0x19		/* (fatal.c) */

#define C_ERR_LIN_COMM			0x80

#define C_ERR_APPL_UNDER_TEMP	0xA0		/* Application: Under Temperature */
#define C_ERR_APPL_OVER_TEMP	0xA1		/* Application: Over Temperature */
#define C_ERR_APPL_UNDER_VOLT	0xA2		/* Application: Under Voltage */
#define C_ERR_APPL_OVER_VOLT	0xA3		/* Application: Over Voltage */
#define C_ERR_APPL_SPI_INIT		0xA4		/* Soft SPI Initialization failed */
#define C_ERR_APPL_SPI_NOT_INIT	0xA5		/* Soft SPI not initialized */
#define C_ERR_APPL_SPI_READ		0xA6		/* Soft SPI Read failure */
#define C_ERR_APPL_SPI_WRITE	0xA7		/* Soft SPI Write failure */
#define C_ERR_APPL_STOP			0xA8		/* Application stop */

#define C_ERR_LIN2X_B0			0xB0		/* LIN 2.x NAD Change error */
#define C_ERR_LIN2X_B1			0xB1		/* LIN 2.x Assign MessageID to FrameID failed/unsupported */
#define C_ERR_LIN2X_B2			0xB2		/* LIN 2.x Read by Identifier (unsupported ID) */
#define C_ERR_LIN2X_B3			0xB3		/* n.a. */
#define C_ERR_LIN2X_B4			0xB4		/* LIN 2.x Assign Variant-ID, HW-Ref and SW-Ref */
#define C_ERR_LIN2X_B5			0xB5		/* LIN 2.x Auto Addressing failure */
#define C_ERR_LIN2X_B6			0xB6		/* LIN 2.x Assign Group-address failure (MMP150125-1) */
#define C_ERR_LIN2X_B7			0xB7		/* LIN 2.x Assign Frame-ID range failure */
#define C_ERR_LIN2X_LINAA		0xB8		/* LIN 2.x Non LIN-AA Diagnostic command during LIN-AA mode */
#define C_ERR_LIN2X_WRITE		0xB9		/* LIN 2.x Write not allowed (not stop-mode) */
#define C_ERR_LIN_INV_PARAM		0xBA		/* LIN Invalid Parameter */
#define C_ERR_LIN_INV_PLA_KEY	0xBB		/* LIN Invalid Pla "key" */
#define C_ERR_LIN_INV_CMD		0xBC		/* LIN Invalid Command Request */
#define C_ERR_LIN2X_CB			0xBD		/* LIN 2.x Write by Identifier (unsupported ID, or incorrect) */
#define C_ERR_PWM_BUS_TIMEOUT	0xBE		/* PWM Bus-timeout */
#define C_ERR_LIN_BUS_TIMEOUT	0xBF		/* LIN Bus-timeout */

#define C_ERR_INV_USERPAGE_1	0xC0		/* Invalid NVRAM User Page #1 (use User Page #2) */
#define C_ERR_INV_USERPAGE_2	0xC1		/* Invalid NVRAM User Page #2 (use User Page #1) */
#define C_ERR_INV_USERPAGE_BOTH	0xC2		/* Invalid NVRAM User Page #1 & #2 (using defaults) */
#define C_ERR_INV_NAD			0xC3		/* Invalid Node Address */
#define C_ERR_INV_MLXPAGE_CRC1	0xC8		/* Invalid MLX NVRAM CRC #1 */
#define C_ERR_INV_MLXPAGE_CRC2	0xC9		/* Invalid MLX NVRAM CRC #2 */
#define C_ERR_INV_MLXPAGE_CRC3	0xCA		/* Invalid MLX NVRAM CRC #3 */
#define C_ERR_INV_MLXPAGE_CRC4	0xCB		/* Invalid MLX NVRAM CRC #4 */
#define C_ERR_INV_MLXPAGE_CRC5	0xCC		/* Invalid MLX NVRAM CRC #5 (Recovered) */
#define C_ERR_NVRAM_MLX_CAL_GN	0xCF		/* MLX Calibration Gain (Zero) */

#define C_ERR_DIAG_OVER_CURRENT	0xD0		/* Diagnostic: Over Current */
#define C_ERR_DIAG_OVER_TEMP	0xD1		/* Diagnostic: Over Temperature */
#define C_ERR_DIAG_UNDER_VOLT	0xD2		/* Diagnostic: Under Voltage */
#define C_ERR_DIAG_OVER_VOLT	0xD3		/* Diagnostic: Over Voltage */

#define C_ERR_TESTMODE_0		0xE0
#define C_ERR_TESTMODE_1		0xE1
#define C_ERR_COIL_ZERO_CURRENT	0xE6		/* Actuator coil zero current (open) */
#define C_ERR_COIL_OVER_CURRENT	0xE7		/* Actuator coil over current (short) */
#define C_ERR_CHIP_TEMP_PROFILE	0xE8		/* Chip temperature profile error */
#define C_ERR_PHASE_SHORT_GND	0xE9		/* Motor driver phase to GND short */
#define C_ERR_SELFTEST_A		0xEA		/* Self-Test: FET Shortage (Over Current) */
#define C_ERR_SELFTEST_B		0xEB		/* Self-Test: Short with other Phase (Over Current) */
#define C_ERR_SELFTEST_C		0xEC		/* Self-Test: Open Phase */
#define C_ERR_SELFTEST_D		0xED		/* Self-Test: Phase (H) */
#define C_ERR_SELFTEST_E		0xEE		/* Self-Test: Phase (L) */
#define C_ERR_SELFTEST_F		0xEF		/* Self-Test: - */

#define C_ERR_FATAL				0xF0		/* Fatal-Error */
#define C_ERR_WD_RST			0xF1		/* (Digital) Watchdog-reset */
#define C_ERR_AWD_RST			0xF2		/* Analogue Watchdog-reset */
#define C_ERR_WD_AWD_RST		0xF3		/* Analogue & Digital Watchdog-reset */
#define C_ERR_NVRAM_PG11		0xF4		/* NVRAM Page 1.1 Write error */
#define C_ERR_NVRAM_PG12		0xF5		/* NVRAM Page 1.2 Write error */
#define C_ERR_NVRAM_PG21		0xF6		/* NVRAM Page 2.1 Write error */
#define C_ERR_RAM				0xF7		/* RAM Complete-test error */
#define C_ERR_RAM_BG			0xF8		/* RAM Background test error */
#define C_ERR_FLASH_BG			0xF9		/* Flash Background test error */
#define C_ERR_MLX4_RESTART		0xFA		/* MLX4 have been restarted */
#define C_ERR_FLASH_CRC_FAIL	0xFB		/* Flash CRC failure during HTOL/Burn-in */
#define C_ERR_IOREG				0xFC		/* I/O-registers error */
#define C_ERR_FATAL_EMRUN		0xFE		/* Fatal Emergency-run */

/* ****************************************************************************	*
 *	P u b l i c   f u n c t i o n s												*
 * ****************************************************************************	*/
void ErrorLogInit( void);
void SetLastError( uint8 u8ErrorCode);
uint8 GetLastError( void);

#endif /* ERRORCODES_H_ */

/* EOF */
