/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <ioports.h>
#include <syslib.h>
#include <itc2.h>


extern void _fatal (void);
static void _unexpected_interrupt (void);

/*
 * Link all 2nd level interrupts to unexpected interrupt handler by default
 */
void ITC2_PWMA (void)           __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_PWMI (void)           __attribute__((weak,alias("_unexpected_interrupt")));

void ITC2_T1_IRQ1 (void)        __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_T1_IRQ2 (void)        __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_T1_IRQ3 (void)        __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_T1_IRQ4 (void)        __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_T1_IRQ5 (void)        __attribute__((weak,alias("_unexpected_interrupt")));

void ITC2_T2_IRQ1 (void)        __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_T2_IRQ2 (void)        __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_T2_IRQ3 (void)        __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_T2_IRQ4 (void)        __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_T2_IRQ5 (void)        __attribute__((weak,alias("_unexpected_interrupt")));

void ITC2_CURREG (void)         __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_SPI_OVF (void)        __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_SPI_REC_WORD (void)   __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_SPI_TR_WORD (void)    __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_EXTIO_IRQ0 (void)     __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_EXTIO_IRQ1 (void)     __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_EXTIO_IRQ2 (void)     __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_EXTIO_IRQ3 (void)     __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_EXTIO_IRQ4 (void)     __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_EXTIO_IRQ5 (void)     __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_EXTIO_IRQ6 (void)     __attribute__((weak,alias("_unexpected_interrupt")));

void ITC2_VSUP_OV (void)        __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_VSUP_UV (void)        __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_VREG_UV (void)        __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_VBOOST_UV (void)      __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_OVER_TEMP (void)      __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_VDS_ERROR (void)      __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_VGS_ERROR (void)      __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_OVER_CURRENT (void)   __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_HVIO0_SHORT_DET (void)    __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_HVIO1_SHORT_DET (void)    __attribute__((weak,alias("_unexpected_interrupt")));
void ITC2_HVIO2_SHORT_DET (void)    __attribute__((weak,alias("_unexpected_interrupt")));

/*
 * Vector table (2nd level) to be linked to predefined address
 */
const L2_HandlerType L2_vector_table[] __attribute__((used, section(".L2_vectors"))) =
{
    &ITC2_CURREG,           /* connected to EXT3_IT */
    &ITC2_SPI_OVF,
    &ITC2_SPI_REC_WORD,
    &ITC2_SPI_TR_WORD,
    &ITC2_EXTIO_IRQ0,
    &ITC2_EXTIO_IRQ1,
    &ITC2_EXTIO_IRQ2,
    &ITC2_EXTIO_IRQ3,
    &ITC2_EXTIO_IRQ4,
    &ITC2_EXTIO_IRQ5,
    &ITC2_EXTIO_IRQ6,

    &ITC2_VSUP_OV,          /* connected to EXT4_IT */
    &ITC2_VSUP_UV,
    &ITC2_VREG_UV,
    &ITC2_VBOOST_UV,
    &ITC2_OVER_TEMP,
    &ITC2_VDS_ERROR,
    &ITC2_VGS_ERROR,
    &ITC2_OVER_CURRENT,
    &ITC2_HVIO0_SHORT_DET,
    &ITC2_HVIO1_SHORT_DET,
    &ITC2_HVIO2_SHORT_DET,

    &ITC2_PWMA,             /* connected to EXT5_IT */
    &ITC2_PWMI,

    &ITC2_T1_IRQ1,          /* connected to EXT6_IT */
    &ITC2_T1_IRQ2,
    &ITC2_T1_IRQ3,
    &ITC2_T1_IRQ4,
    &ITC2_T1_IRQ5,

    &ITC2_T2_IRQ1,          /* connected to EXT7_IT */
    &ITC2_T2_IRQ2,
    &ITC2_T2_IRQ3,
    &ITC2_T2_IRQ4,
    &ITC2_T2_IRQ5
};



/*-----------------------------------------------------------------------------
 * Handler of unexpected interrupt
 */
static void _unexpected_interrupt (void)
{
    _fatal();
}




/* EOF */
