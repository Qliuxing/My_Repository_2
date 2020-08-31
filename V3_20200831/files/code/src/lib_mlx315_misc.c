#include "Build.h"
#include <awdg.h>															/* Analogue Watchdog support */
#include "SPI_Debug.h"
#include "lib_mlx315_misc.h"

void MLX315_SystemInit(void)
{

	/* *********************************************************** */
	/* *** C. Initialise watch-dogs, both analogue and digital *** */
	/* *********************************************************** */
#if WATCHDOG == DISABLED
	WD_CTRL = 0u;															/* Disable digital Watch-dog */
	AWD_CTRL = 0x8000u;														/* Disable analogue Watch-dog */
#endif /* WATCHDOG == DISABLED */
#if WATCHDOG == ENABLED
	WD_T = WatchDog_PeriodOf100ms;											/* Initialise the (Digital) watch-dog comparator to 100ms */
	WD_CTRL = WatchDog_ModeTimer;											/* Define the mode and start the watchdog */
	awdg_init( AWDG_DIV_16, C_AWD_PERIOD_250MS);
	/* Maximum Watch-dog period */
#endif /* WATCHDOG == ENABLED */

	/* I/O ports  */
#if MCU_ASSP_MODE
	/* IO[5]:HALL, IO[4]:GND-HW connected, IO[3:0]:debug purpose,PMOS not disabled */
	ANA_OUTL |= ASSP;														/* ASSP-mode */
	
	ANA_OUTM = (IO5_OUTCFG_SOFT | IO4_OUTCFG_SOFT | IO3_OUTCFG_SOFT | 
				IO2_OUTCFG_SOFT | IO1_OUTCFG_SOFT | IO0_OUTCFG_SOFT);
	ANA_OUTN = ((uint16)0u << 5u) | ((uint16)0u << 4u) | ((uint16)0u << 3u) | 
				((uint16)0u << 2u) | ((uint16)0u << 1u) | ((uint16)0u << 0u);
	IO_DEB = ( IO5_DEBOUNCE_OFF | IO4_DEBOUNCE_OFF | IO3_DEBOUNCE_OFF | 
				IO2_DEBOUNCE_OFF | IO1_DEBOUNCE_OFF | IO0_DEBOUNCE_OFF);
	ANA_OUTF = ( IO3_ENA | IO2_ENA | IO1_ENA | IO0_ENA | 
				IO5_DIS_PMOS | IO4_DIS_PMOS );
	
#endif /* MCU_ASSP_MODE */


#if _SUPPORT_TESTMODE_OFF
	/* ******************************************** */
	/* *** D. (Optional) Chip test-mode disable *** */
	/* ******************************************** */
	CONTROL |= OUTA_WE;														/* Grant access to ANA_OUTx registers */
	ANA_OUTA |= TEST_MODE_DIS;												/* Disable test-mode */
	CONTROL &= ~OUTA_WE;
#endif /* _SUPPORT_TESTMODE_OFF */


#if _DEBUG
#if _DEBUG_SPI
	ANA_OUTM = (IO3_OUTCFG_SPI | IO2_OUTCFG_SPI | IO1_OUTCFG_SPI | IO0_OUTCFG_SPI);
	ANA_OUTF = (IO3_ENA | IO2_ENA | IO0_ENA);								/* IO[0] = MOSI, IO[2] = CLK, IO[3] = -CS */
	SpiDebugInit();
#else  /* _DEBUG_SPI */
#if MCU_ASSP_MODE
	DEBUG_INI_IO_ABC();
#else  /* MCU_ASSP_MODE */
	/* None-ASSP-mode: IO[4] and IO[5] can be used for debugging */
	DEBUG_SET_IO_AB_00();
#endif /* MCU_ASSP_MODE */
#endif /* _DEBUG_SPI */
#endif /* _DEBUG */

	/* Disable Driver */
	DRVCFG_DIS_UVWT();

	/* Interrupts */
	/* System Tick Timer - Core Timer  */
	PRIO &= ~((uint16)3u << 0u) ;												/* Set CoreTimer priority to 6 (3..6) */
	PRIO |= ((uint16)(6u - 3u) << 0u);
	PEND = CLR_TIMER_IT;
	MASK |= EN_TIMER_IT;														/* Enable Timer interrupt */
	
	/* BLDC motor Commutation/Stepper timer */
	XI0_PEND = CLR_T1_INT4;														/* Clear (potentially) Timer1 second level interrupts (T1_INT4) */
	XI0_MASK |= EN_T1_INT4;														/* Enable Timer1, CompareB (T1_INT4) */
	PRIO &= ~((uint16)3u << 6u);												/* Set Timer1 priority to 4 (3..6) */
	PRIO |= ((uint16)(4u - 3u) << 6u);									
	PEND = CLR_EXT0_IT;
	MASK |= EN_EXT0_IT;	

	/* HALL,diagnostic(OVT,OVC,UV,OV) interrupt */
	/* PRIO = (PRIO & ~(3U << 14)) | ((3U - 3U) << 14); */							/* EXT4_IT Priority: 3 (3..6) */
	PRIO &= ~((uint16)3u << 14u);												/* EXT4_IT Priority: 3 (3..6) */
	PRIO |= ((uint16)(3u - 3u) << 14u);
	PEND = CLR_EXT4_IT;
	MASK |= EN_EXT4_IT;															/* Enable Diagnostic Interrupt */
	
}

void MLX315_GotoSleep(void)
{
    /* MMP140812-4 - Begin */
	/* Put the chip in it's Sleep Mode by fulfilling following condition:
	 *                     _
	 * MLX16_HALTED ------| \
	 *  MLX4_HALTED ------|  \____ goto sleep
	 *     ADC_BUSY -----o|  /
	 *      NV_BUSY -----o|_/
	 *
	 * Internal power supplies will be switched off and chip can only be woken up by one
	 * of the following wake up sources:
	 * - LIN bus
	 * - IO3
	 * - Internal timer
	 * Wake up is done by resetting the chip. The source of the wake up can be found in ANA_INB.
	 * Note: In case between MLX4_HALT and MLX16_HALT a LIN-bus event happens, the chip will not
	 * enter SLEEP but HALT-mode. To allow a LIN-wakeup, the Analogue Watchdog is set to minimum
	 * period of 100us to allow a chip reset.
	 */
	IO_WU = 0;														/* Disable IO3 wake up */
	ANA_OUTG &= 0xFF9Eu;											/* Clear Internal WU delay and DIS_GTSM */
	MASK = 0;
	/* Go into sleep/halt */
	AWD_CTRL = ((uint16)3u << 8u) | 1u;								/* Set 1:1 prescaler and minimal period; AWD timeout will be 100 us (MMP140813-2) */
	MLX4_RESET();
	MLX16_HALT();													/* See MELEXIS doc */
	/* Chip should reset upon LIN bus changes */
	/* We should never make it to here, as a backup we add a chip reset */
	MLX16_RESET();
}
