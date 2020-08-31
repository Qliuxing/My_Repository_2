/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef MMC16_IO_H_
#define MMC16_IO_H_

/* This file should be only included from <ioports.h>, never directly */
#ifndef IOPORTS_H_
#error "Include <ioports.h> instead of this file."
#endif


/*
 * IO ports common to all MMC16 based products
 *
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
#define     WD_BOOT     (1u << 7)   /* read-only: (0) POR reset, (1) watchdog reset */
#define     EE_WE       (1u << 6)   /* EEPROM Write Enable bit */
#define     OUTC_WE     (1u << 5)
#define     OUTB_WE     (1u << 4)
#define     OUTA_WE     (1u << 3)
/* bit 2 reserved */
#define     HALT        (1u << 1)   /* write-only */
/* bit 0 reserved */

/* --------------------------------------------------------------------------- */
FARPORTB    (EEPROM, 0x2001)
#define     EE_BUSY     (1u << 7)   /* read-only */
#define     EE_CPTEST   (1u << 6)   /* only for test-mode */
#define     EE_VEE2     (1u << 5)   /* only for test-mode */
#define     EE_VEE1     (1u << 4)   /* only for test-mode */
#define     EE_TEST     (1u << 3)   /* only for test-mode */
#define     EE_DMA      (1u << 2)   /* only for test-mode */

#define     EE_CTL_MASK         (3u << 0)
#define     EE_CTL_WRITE        (0u << 0)
#define     EE_CTL_ERASE        (1u << 0)
#define     EE_CTL_BLOCK_WRITE  (2u << 0)   /* only for test-mode */
#define     EE_CTL_BLOCK_ERASE  (3u << 0)   /* only for test-mode */

/* --------------------------------------------------------------------------- */
FARPORTW    (CONTROL2, 0x2002)
/* bits [15:1] reserved */
#define     EE_ACTIVE   (1u << 0)   /* EERPOM control (for MMC16 version >= 1.3) 1: EEPROM active, 0: EEPROM standby */

/* --------------------------------------------------------------------------- */
/* Interrupt priorities */
FARPORTW    (PRIO, 0x2004)
/* PRIO[15:14]  EXT4_IT
 * PRIO[13:12]  EXT3_IT
 * PRIO[11:10]  EXT2_IT
 * PRIO[9:8]    EXT1_IT
 * PRIO[7:6]    EXT0_IT
 * PRIO[5:4]    EE_IT
 * PRIO[3:2]    ADC_IT
 * PRIO[1:0]    TIMER_IT
 */

/* --------------------------------------------------------------------------- */
/* Interrupt masking
 * 0: Disabled (Masked), 1: Enabled
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
#define     EN_EXT7_IT          (1u << 4)
#define     EN_EXT6_IT          (1u << 3)
#define     EN_WD_ATT_IT        (1u << 2)
#define     EN_EXT5_IT          (1u << 1)
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
#define     CLR_EXT7_IT         (1u << 4)
#define     CLR_EXT6_IT         (1u << 3)
#define     CLR_WD_ATT_IT       (1u << 2)
#define     CLR_EXT5_IT         (1u << 1)
#define     CLR_EXCHANGE_IT     (1u << 0)
    
/* --------------------------------------------------------------------------- */
/* PORTW 0x200A reserved */

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

/* --------------------------------------------------------------------------- */
FARPORTW    (ANA_OUTB,   0x201E)    /* NB! write access to ANA_OUTB port is protected by OUTB_WE bit in CONTROL port */
FARPORTB    (ANA_OUTB_L, 0x201E)
FARPORTB    (ANA_OUTB_H, 0x201F)

/* --------------------------------------------------------------------------- */
FARPORTW    (ANA_OUTC,   0x2020)    /* NB! write access to ANA_OUTC port is protected by OUTC_WE bit in CONTROL port */
FARPORTB    (ANA_OUTC_L, 0x2020)
FARPORTB    (ANA_OUTC_H, 0x2021)

/* --------------------------------------------------------------------------- */
FARPORTW    (ANA_TEST,   0x2022)
FARPORTB    (ANA_TEST_L, 0x2022)
FARPORTB    (ANA_TEST_H, 0x2023)

/* --------------------------------------------------------------------------- */
FARPORTB    (CK_TRIM, 0x2024)

/* --------------------------------------------------------------------------- */
/* PORTB 0x2025  reserved */

/* --------------------------------------------------------------------------- */
/* PORTW 0x2026  reserved */

/* --------------------------------------------------------------------------- */
/* PORTW 0x2028  reserved */

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
PORTBBIT    (HW_VER,    0x2801)             /* MMC16-core version */
#define     EENV_DED            (1u << 7)   /* MMC16_1.2: read-only, EEPROM double error detected; cleared if next reading is Ok    */
                                            /* MMC16_1.3: EEPROM double error detected; sticky bit (see MMC16-38)                   */
#define     EENV_SEC            (1u << 6)   /* MMC16_1.2: read-only, EEPROM single error corrected; cleared if next reading is Ok   */
                                            /* MMC16_1.3: EEPROM single error corrected; sticky bit (see MMC16-38)                  */
/* bit 5 reserved */
/* bit 4 reserved */
/* bit 3 reserved */
#define     EXTMEM              (1u << 2)   /* read-only: 0=Use internal ROM, 1=Use external memory */
#define     SCOPE_BIT           (1u << 1)   /* reserved */
#define     SWI                 (1u << 0)   /* write-only: Software interrupt request (automatically cleared) */

/* --------------------------------------------------------------------------- */
PORTB       (WD_T,      0x2802)     /* Core watchdog: Timeout register */
PORTB       (WD_CTRL,   0x2803)     /* Core watchdog: Control register */
#define     WD_ERR              (1u << 7)   /* read-and-clear bit: indicates watchdog access error  */
#define     WD_WND              (1u << 6)   /* read-only: indicates open acknowledgment window      */
#define     WD_MODE_MASK        (3u << 4)   /* watchdog mode bits */
#define     WD_MODE_DISABLED    (0u << 4)   /* Watchdog diabled */
#define     WD_MODE_TIMER       (1u << 4)   /* Timer Watchdog mode */
#define     WD_MODE_WINDOW      (2u << 4)   /* Window Watchdog mode */
#define     WD_MODE_INTELLIGENT (3u << 4)   /* Intelligent Watchdog mode */

#define     WD_DIV_MASK         (3u << 0)   /* predivider   */
#define     WD_DIV_8            (0u << 0)   /* Division 8   */
#define     WD_DIV_32           (1u << 0)   /* Division 32  */
#define     WD_DIV_128          (2u << 0)   /* Division 128 */
#define     WD_DIV_512          (3u << 0)   /* Division 512 */


/* --------------------------------------------------------------------------- */
PORTB       (WD_TG,     0x2804)     /* Intelligent watchdog tag register  */
PORTB       (XIN,       0x2805)     /* External inputs register           */

/* --------------------------------------------------------------------------- */
PORTW       (TIMER,     0x2806)
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
#define     ADC_SYNC_SOC        (1u << 7)   /* Discard (0) or not (1) first hard triggering pulse [MMC16 v1.1] */
/* bit 6 reserved */
#define     ADC_OVFM            (1u << 5)   /* ADC overflow memorized                  */
#define     ADC_OVF             (1u << 4)   /* ADC overflow of last converted channel  */
/* bit 3 reserved */
#define     ADC_LOOP            (1u << 2)   /* 0: one conversion cycle through all channels; 1: permanent conversion */
#define     ADC_TRIG_SRC        (1u << 1)   /* 0: Soft trigger mode, 1: Hard trigger mode (alias)   */
#define     ADC_HW_TRIG         (1u << 1)   /* 0: Soft trigger mode, 1: Hard trigger mode           */
#define     ADC_START           (1u << 0)   /* Start of conversion sequence                         */


/* ---------------------------------------------------------------------------
 * ADC Settings Pointer : points to area (ROM or RAM) where ADC settings
 *                        are stored (e.g. selects channel, reference,
 *                        trigger source etc)
 *      
 *  ADC_SEL[15:8]  -- channel selection
 *  ADC_REF[7:0]   -- reference, trigger source etc (product specific)
 *
 */
PORTW       (ADC_SBASE, 0x2812) /* write: stores new SBASE address
                                 * read: returns current value of S pointer (not SBASE address) [MMC16 v1.1]
                                 */

/* ---------------------------------------------------------------------------
 * ADC Data Pointer : points to area in RAM where conversion results are saved 
 */
PORTW       (ADC_DBASE, 0x2814) /* write: stores new DBASE address
                                 * read: returns current value of D pointer (not SBASE address) [MMC16 v1.1]
                                 */

/* =============================================================================
 *  Product specific user ports are defined in
 *      /products/<PRODUCT>/include/ioports.h
 * =============================================================================
 */


#endif /* MMC16_IO_H_ */
