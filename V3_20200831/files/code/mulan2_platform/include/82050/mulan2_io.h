/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef MULAN2_IO_H_
#define MULAN2_IO_H_

/* This file should be only included from <ioports.h>, never directly */
#ifndef IOPORTS_H_
#error "Include <ioports.h> instead of this file."
#endif


/* -------------------------------------------------------------------------
 * System service to switch to System mode (product specific)
 */
#if defined (__WITH_ITC_BUG_WORKAROUND__)

#define __SYS_ENTER_PROTECTED_MODE  __asm__ __volatile__ (          \
                                                    "mov R, #0\n\t" \
                                                    "call fp0:0x60" \
                                    )
#else

#define __SYS_ENTER_PROTECTED_MODE  __asm__ __volatile__ (          \
                                                    "call fp0:0x60" \
                                    )

#endif /* __WITH_ITC_BUG_WORKAROUND__ */


/*
 * IO ports common to all MULAN2 based product
 *
 * Notes:
 * Based on MULAN2 portsmap 20081126, stu
 */

/*
 * I/O port definition macros
 * PORTB    : port in io: page with byte access (io: page length is 64 bytes)
 * PORTW    : port in io: page with word access (io: page length is 64 bytes)
 * PORTBBIT : byte-size port in io: page with bit access (only first 32 bytes
 *            of io: page could have bit access
 * FARPORTB : port outside of io: page with byte access
 * FARPORTW : port outside of io: page with word access
 */
#ifdef __ASSEMBLER__

/* --- Definition for assembler source file ----------------------- */
#define PORTBBIT( name, address ) \
.set name, address

#define PORTWBIT( name, address ) \
.set name, address

#define PORTB( name, address ) \
.set name, address

#define PORTW( name, address ) \
.set name, address

#define FARPORTB( name, address ) \
.set name, address

#define FARPORTW( name, address ) \
.set name, address

#else /* ! __ASSEMBLER__ */

/* --- Definition for C source file ------------------------------- */
#include "typelib.h"

/* Compiler version check */
#if ((__MLX16_GCC_MAJOR__ == 1) && (__MLX16_GCC_MINOR__ >= 5)) || (__MLX16_GCC_MAJOR__ > 1)

/*
 * MLX16_GCC >= 1.5 has bit_access attribute
 */
#define PORTBBIT( name, address ) \
extern volatile uint8 name __attribute__((io, bit_access, addr(address)));

#define PORTWBIT( name, address ) \
extern volatile uint16 name __attribute__((io, bit_access, addr(address)));

#else

#define PORTBBIT( name, address ) \
extern volatile uint8 name __attribute__((io, addr(address)));

#endif /* Compiler version check */

#define PORTB( name, address ) \
extern volatile uint8 name __attribute__((io, addr(address)));

#define PORTW( name, address ) \
extern volatile uint16 name __attribute__((io, addr(address)));

#define FARPORTB( name, address ) \
extern volatile uint8 name __attribute__((nodp, addr(address)));

#define FARPORTW( name, address ) \
extern volatile uint16 name __attribute__((nodp, addr(address)));

#endif /* !__ASSEMBLER__ */


/* =============================================================================
 *  System ports (accessed only in system mode of CPU)
 * =============================================================================
 */

/* --------------------------------------------------------------------------- */
FARPORTB    (CONTROL, 0x2000)

#define     WD_BOOT     (1u << 7)   /* read-only */
#define     EE_WE       (1u << 6)
#define     OUTC_WE     (1u << 5)
#define     OUTB_WE     (1u << 4)
#define     OUTA_WE     (1u << 3)
#define     MUTEX_SHE   (1u << 2)
#define     HALT        (1u << 1)   /* write-only */
#define     M4_RB       (1u << 0)

/* --------------------------------------------------------------------------- */
FARPORTB    (EEPROM, 0x2001)

#define     EE_BUSY     (1u << 7)   /* read-only */
#define     EE_CPTEST   (1u << 6)
#define     EE_VEE1     (1u << 5)
#define     EE_VEE0     (1u << 4)
#define     EE_TEST     (1u << 3)
#define     EE_DMA      (1u << 2)

#define     EE_CTL_MASK         (3u << 0)
#define     EE_CTL_WRITE        (0u << 0)
#define     EE_CTL_ERASE        (1u << 0)
#define     EE_CTL_BLOCK_WRITE  (2u << 0)
#define     EE_CTL_BLOCK_ERASE  (3u << 0)

/* --------------------------------------------------------------------------- */
/* MLX4/MLX16 shared RAM configuration */
FARPORTW    (SHRAM,   0x2002)
FARPORTB    (SHRAM_L, 0x2002)
FARPORTB    (SHRAM_H, 0x2003)

/* --------------------------------------------------------------------------- */
/* Interrupt priorities */
FARPORTW    (PRIO, 0x2004)

/* --------------------------------------------------------------------------- */
/* Interrupt masking
 * 0: Disabled, 1: Enabled
 */
FARPORTW    (MASK, 0x2006)

#define     EN_SOFT_IT          (1u << 13)
#define     EN_EXT4_IT          (1u << 12)
#define     EN_EXT3_IT          (1u << 11)
#define     EN_EXT2_IT          (1u << 10)
#define     EN_EXT1_IT          (1u << 9)
#define     EN_EXT0_IT          (1u << 8)
#define     EN_EE_IT            (1u << 7)
#define     EN_ADC_IT           (1u << 6)
#define     EN_TIMER_IT         (1u << 5)
#define     EN_M4_SHE_IT        (1u << 4)
#define     EN_M4_MUTEX_IT      (1u << 3)
#define     EN_WD_ATT_IT        (1u << 2)
#define     EN_TASK_RST_IT      (1u << 1)
#define     EN_EXCHANGE_IT      (1u << 0)

/* --------------------------------------------------------------------------- */
/* Pending interrupt flags
 * 0: No change, 1: Clear pending
 */
FARPORTW    (PEND, 0x2008)

#define     CLR_SOFT_IT         (1u << 13)
#define     CLR_EXT4_IT         (1u << 12)
#define     CLR_EXT3_IT         (1u << 11)
#define     CLR_EXT2_IT         (1u << 10)
#define     CLR_EXT1_IT         (1u << 9)
#define     CLR_EXT0_IT         (1u << 8)
#define     CLR_EE_IT           (1u << 7)
#define     CLR_ADC_IT          (1u << 6)
#define     CLR_TIMER_IT        (1u << 5)
#define     CLR_M4_SHE_IT       (1u << 4)
#define     CLR_M4_MUTEX_IT     (1u << 3)
#define     CLR_WD_ATT_IT       (1u << 2)
#define     CLR_TASK_RST_IT     (1u << 1)
#define     CLR_EXCHANGE_IT     (1u << 0)
    
/* --------------------------------------------------------------------------- */
FARPORTW    (M4IF,   0x200A)
FARPORTB    (SLVCMD, 0x200A)
FARPORTB    (SLVIT,  0x200B)

/* --------------------------------------------------------------------------- */
/* Patch 0 jump instruction
 */
FARPORTW    (PATCH0_I, 0x200C)

/* --------------------------------------------------------------------------- */
/* Patch 1 patch jump instruction
 */
FARPORTW    (PATCH1_I, 0x200E)

/* --------------------------------------------------------------------------- */
/* Patch 2 patch jump instruction
 */
FARPORTW    (PATCH2_I, 0x2010)

/* --------------------------------------------------------------------------- */
/* Patch 3 patch jump instruction
 */
FARPORTW    (PATCH3_I, 0x2012)

/* --------------------------------------------------------------------------- */
/* Patch 0 address/enable
 */
FARPORTW    (PATCH0_A, 0x2014)

#define     PATCH0_EN   (1u << 0)

/* --------------------------------------------------------------------------- */
/* Patch 1 address/enable
 */
FARPORTW    (PATCH1_A, 0x2016)

#define     PATCH1_EN   (1u << 0)

/* --------------------------------------------------------------------------- */
/* Patch 2 address/enable
 */
FARPORTW    (PATCH2_A, 0x2018)

#define     PATCH2_EN   (1u << 0)

/* --------------------------------------------------------------------------- */
/* Patch 3 address/enable
 */
FARPORTW    (PATCH3_A, 0x201A)

#define     PATCH3_EN   (1u << 0)

/* --------------------------------------------------------------------------- */
FARPORTW    (ANA_OUTA,   0x201C)    /* NB! write access to ANA_OUTA port is protected by OUTA_WE bit in CONTROL port */
FARPORTB    (ANA_OUTA_L, 0x201C)
FARPORTB    (ANA_OUTA_H, 0x201D)

#define		TEST_MODE_DIS		(1u << 7)	/* 0: Test mode enabled; 1: Test mode disabled */

/* --------------------------------------------------------------------------- */
FARPORTW    (ANA_OUTB,   0x201E)    /* NB! write access to ANA_OUTB port is protected by OUTB_WE bit in CONTROL port */
FARPORTB    (ANA_OUTB_L, 0x201E)
FARPORTB    (ANA_OUTB_H, 0x201F)

/* --------------------------------------------------------------------------- */
FARPORTW    (ANA_OUTC,   0x2020)    /* NB! write access to ANA_OUTC port is protected by OUTC_WE bit in CONTROL port */
FARPORTB    (ANA_OUTC_L, 0x2020)
FARPORTB    (ANA_OUTC_H, 0x2021)
/* bit 15 is reserved                                           */
/* bits [14:8] TR_RCO Analog trim bits for 1MHz RC oscillator   */
/* bits [7:0] TR_PLL  Analog trim bits for PLL                  */

/* --------------------------------------------------------------------------- */
FARPORTW    (ANA_TEST,   0x2022)
FARPORTB    (ANA_TEST_L, 0x2022)
FARPORTB    (ANA_TEST_H, 0x2023)

/* --------------------------------------------------------------------------- */
FARPORTW    (NV_CTRL, 0x2024)
#define     NV_SEL              (1u << 15)  /* 0: selects EEPROM1; 1: selects EEPROM2 for Store / Recall operation */
/* bit 14 reserved */
/* bit 13 reserved */
/* bit 12 reserved */
/* bit 11 reserved */
#define     NV2_MEM_ALLC        (1u << 10)  /* 0: enables write in both pages; 1: only one page can be written  */
#define     NV2_MEM_SEL         (1u <<  9)  /* Selection between upper (1) and lower page (0) if MEM_ALLC = 1  */
/* bit 8 reserved */
/* bit 7 reserved */
/* bit 6 reserved */
/* bit 5 reserved */
#define     NV1_MEM_ALLC        (1u << 4)   /* 0: enables write in both pages; 1: only one page can be written  */
#define     NV1_MEM_SEL         (1u << 3)   /* Selection between upper (1) and lower (0) page if MEM_ALLC = 1 */
/* bit 2 reserved */
#define     NV_CONF_MASK        (3u << 0)
#define     NV_CONF_SRAM_MODE   (0u << 0)
#define     NV_CONF_RECALL      (1u << 0)   /* 01: Recall (NV Area -> SRAM) */
#define     NV_CONF_STORE       (3u << 0)   /* 11: Store (SRAM -> NV Area)  */

#define     NV_SRAMWR           (1u << 1)   /* read only: 1=SRAM was written (NVRAM != SRAM) */
#define     NV_BUSY             (1u << 0)   /* read only: NVRAM is being updated */


/* --------------------------------------------------------------------------- */
FARPORTW    (FL_CTRL0,   0x2026)

#define     FL_ODD_WR           (1u << 15)  /* Enable write of page latches to all odd pages */
#define     FL_EVEN_WR          (1u << 14)  /* Enable write of page latches to all even pages */
#define     FL_FAST_ERA         (1u << 13)  /* Enable erase of all pages (flash erase) */
#define     FL_DIS_ALL          (1u << 12)  /* Disable all wordlines (clear to 0) */
#define     FL_SET_ALL          (1u << 11)  /* Enable all wordlines (set to 1) */
#define     FL_DMA              (1u << 10)  /* According to Address LSB, data or ECC bits only accessed
                                             * ABUS[0] = 1: Only ECC bits read / written (data unchanged)
                                             * ABUS[0] = 0: Only data bits read / written (ECC unchanged)
                                             */
#define     MLX4_RELOC          (1u << 9)   /* If set, relocate Mlx4 code fetch into another flash area     */
                                            /* Can change only when Mlx4 is at reset                        */
#define     FL_ERA_T2           (1u << 8)   /* Selects for Erase time counter between 5ms and 40ms          */
#define     FL_ERA_T1           (1u << 7)   /* If FL_ERA_T2 = 1 : 40ms / 80ms / 120ms / 160ms               */
#define     FL_ERA_T0           (1u << 6)   /* If FL_ERA_T2 = 0 : 5ms / 10ms / 15ms / 20ms                  */
#define     FL_BIT_VFY          (1u << 5)
#define     FL_WRERA_SEL        (1u << 4)   /* 1: select page write command     */
                                            /* 0: select page erased command    */
#define     FL_WRERA_EN         (1u << 3)   /* Enable Write or Erase (selected by FL_WRERA_SEL bit)         */
#define     FL_DBE              (1u << 2)   /* Double Error Detected in Flash read (keep 1 until clear)     */
#define     FL_SBE              (1u << 1)   /* Single Error Corrected in Flash read (keep 1 until clear)    */
#define     FL_DETECT           (1u << 0)   /* read-only: 0 if no flash present (only ROM/XROM) / 1 if there is a flash */


/* --------------------------------------------------------------------------- */
FARPORTB    (FLCTRL1, 0x2028)
#define     FL_FULL             (1u << 7)   /* Set test mode to set all bitlines to 1 */
#define     FL_TEST             (1u << 6)   /* Flash internal register outputs available on DOUT */
#define     FL_TRIM             (1u << 5)   /* 1: enables trim mode */
#define     FL_SANEG            (1u << 4)   /* writing of the latches */
#define     FL_EXT_CLK_EN       (1u << 3)   /* Enable external reference */
#define     FL_CP_LOAD          (1u << 2)   /* Enable internal load for test on charge pump high volt */
#define     FL_CP_MEAS          (1u << 1)   /* Enable measurement of charge pump */
#define     FL_CP               (1u << 0)   /* Start (if set) or stop positive and negative Charge Pumps */


/* --------------------------------------------------------------------------- */
FARPORTB    (CK_TRIM, 0x2029)               /* bits [5:0] */


/* =============================================================================
 *  Product specific system ports are defined in
 *      /products/<PRODUCT>/include/ioports.h
 * =============================================================================
 */


/* =============================================================================
 *  User ports
 * =============================================================================
 */
/* --------------------------------------------------------------------------- */
PORTWBIT    (VARIOUS,   0x2800)
PORTBBIT    (VARIOUS_L, 0x2800)
PORTBBIT    (HW_VER,    0x2801)             /* read-only */

#define     EENV_DED            (1u << 7)   /* read-only: EE/NV double error detected  */
#define     EENV_SEC            (1u << 6)   /* read-only: EE/NV single error corrected */
#define     WKUP                (1u << 5)   /* read-only: LIN wake-up */
#define     PHISTAT1            (1u << 4)   /* read-only: LIN physical status (bit 1) */
#define     PHISTAT0            (1u << 3)   /* read-only: LIN physical status (bit 0) */
#define     EXTMEM              (1u << 2)   /* read-only: 0=Use internal ROM, 1=Use external memory */
#define     SCOPE_BIT           (1u << 1)   /* reserved */
#define     SWI                 (1u << 0)   /* write-only: Software interrupt request (automatically cleared) */

/* --------------------------------------------------------------------------- */
PORTB       (WD_T,      0x2802)     /* Core watchdog: Timeout register; system protected in MODE=3 */
PORTB       (WD_CTRL,   0x2803)     /* Core watchdog: Control register; system protected in MODE=3 */

#define     WD_ERR              (1u << 7)   /* read-only, read-and-clear; watchdog access error         */
#define     WD_WND              (1u << 6)   /* read-only: acknowledge window is open (for window mode)  */
#define     WD_MODE             (3u << 4)   /* Watchdog mode */
#define     WD_MODE_DISABLED    (0u << 4)   /* Watchdog diabled */
#define     WD_MODE_TIMER       (1u << 4)   /* Timer Watchdog mode */
#define     WD_MODE_WINDOW      (2u << 4)   /* Window Watchdog mode */
#define     WD_MODE_INTELLIGENT (3u << 4)   /* Intelligent Watchdog mode */
#define     WD_DIV              (3u << 0)   /* Predivider */
#define     WD_DIV_8            (0u << 0)   /* Division 8   */
#define     WD_DIV_32           (1u << 0)   /* Division 32  */
#define     WD_DIV_128          (2u << 0)   /* Division 128 */
#define     WD_DIV_512          (3u << 0)   /* Division 512 */

/* --------------------------------------------------------------------------- */
PORTB       (WD_TG,     0x2804)     /* Intelligent watchdog tag register    */
PORTB       (XIN,       0x2805)     /* read-only: External inputs register  */

/* --------------------------------------------------------------------------- */
PORTW       (TIMER,     0x2806)     /* Software timer (15-bit) */

#define     TMR_EN              (1u << 15)

/* --------------------------------------------------------------------------- */
PORTB       (MC_MARK,   0x280C)     /* Mlx16 MARK register (MICE) */
PORTB       (MC_STAT,   0x280D)     /* read-only MC_STAT[3:0] MICE status */
PORTW       (MC_EXCHG,  0x280E)     /* MICE register */

/* --------------------------------------------------------------------------- */
/* ADC control register */
PORTW       (ADC_CTRL,  0x2810)

#define     ADC_EOC             (1u << 15)   /* read-only : End of single channel conversion */
/* bit 14 reserved */
/* bit 13 reserved */
/* bit 12 reserved */
/* bit 11 reserved */
/* bit 10 reserved */
/* bit  9 reserved */
#define     ADC_SOFT_TRIG       (1u << 8)   /* Trigger next channel conversion (cleared when conversion is started) */
#define     ADC_SYNC_SOC        (1u << 7)   /* Discard (0) or not (1) first hard triggering pulse */
/* bit 6 reserved */
#define     ADC_OVFM            (1u << 5)   /* ADC overflow memorized                             */
#define     ADC_OVF             (1u << 4)   /* ADC overflow of last converted channel */
/* bit 3 reserved */
#define     ADC_LOOP            (1u << 2)   /* 0: one conversion cycle through all channels; 1: permament conversion */
#define     ADC_TRIG_SRC        (1u << 1)   /* 0: Soft trigger mode, 1: Hard trigger mode   */
#define     ADC_START           (1u << 0)   /* Start of conversion sequence                 */

/* --------------------------------------------------------------------------- */
/* ADC Mux, Reference and Trigger source selection
 *      
 *  ADC_SEL[15:8]  -- channel selection
 *
 *  ADC_REF[7:0]
 *      HW trigger source [7:4]
 *      unused [3:2]
 *      ADC reference [1:0]
 *
 */
PORTW       (ADC_SBASE, 0x2812)

/* --------------------------------------------------------------------------- */
/* ADC Data base pointer */
PORTW       (ADC_DBASE, 0x2814)

/* =============================================================================
 *  Product specific user ports are defined in
 *      /products/<PRODUCT>/include/ioports.h
 * =============================================================================
 */


#endif /* MULAN2_IO_H_ */
