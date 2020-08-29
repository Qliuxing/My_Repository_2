/*
 * Copyright (C) 2008-2009 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <ioports.h>
#include <syslib.h>
#include <string.h> /* memcpy */
#include <wdlib.h>  /* WD_BOOT_CHECK() */

/*
 * Calibration of 1MHz internal clock (depends on CPU clock)
 *
 *  CK_TRIM[5:0] => 0..63
 *  CK_TRIM = 64 - FPLL_MHz
 */
#define DEF_CK_TRIM  ((64000UL - FPLL + 500) / 1000)

#if (DEF_CK_TRIM > 63)
#error "Incorrect DEF_CK_TRIM value"
#endif

/* Module's variables */
static  uint8 wd_bist_active     __attribute__((section(".noinit")));

/* ------------------------------------------------------------------------- */
static inline __attribute__ ((always_inline))
void* MCU_getStackPtr (void) 
{
    void *sp;

    __asm__ __volatile__ (
        "mov %0, s" "\n\t"
        : "=r"(sp)
        :
    );
    
    return sp;
}

/* ------------------------------------------------------------------------- */
static inline __attribute__ ((always_inline))
void MCU_setStackPtr (void *sp)
{
    __asm__ __volatile__ (
        "mov s, %[arg1]" "\n\t"
        :
        : [arg1] "r" (sp)
        : "S" /* do we need "S" register in clobbered list ? */
    );
}

/* Forward declaration */
int do_segment_check (uint16* start, uint16* end); /* return value not used ..*/
void do_memory_test (void);
void _fatal(void);

static void _wd_bist_clear (void);
void _wd_bist (void);
static uint16 _wd_bist_is_active (void);


/* ----------------------------------------------------------------------------
 * Function low_level_init() is invoked by start-up code before C runtime
 * initialization. Thus function cannot rely on uninitialized data being
 * cleared and cannot use any initialized data, because the .bss and .data
 * sections have not been initialized yet.
 *
 * NOTE:
 *  1. Function with the same name (i.e. low_level_init) linked from
 *     application directory overrides this function
 *  2. Before first EEPROM access delay might be needed (see issue MLX12123-22)
 */
void _low_level_init (void)
{
    /* --- Trimming --------------------------------------------------------- */
    CONTROL = OUTA_WE | OUTB_WE | OUTC_WE;	/* grant access to ANA_OUTx registers */

    /* Application specific configuration:
     * - trimming
     * - patching
     * 
     * ANA_OUTA = ...
     *  ...
     */

    CONTROL &= ~(OUTA_WE | OUTB_WE | OUTC_WE);

    /*
     * Calibrate 1MHz internal clock using CK_TRIM divider
     * This 1MHz clock is used by 15-bit core timer, watchdog and EEPROM
     */
    CK_TRIM = DEF_CK_TRIM;


    /* check WD reset and BIST flags to determine WD BIST reset */
    if ( WD_BOOT_CHECK() && _wd_bist_is_active() ) {
        /* It was WD BIST reset */
        _wd_bist_clear();
    } else {
        /* It was POR or WD reset (not WD BIST) */
        _wd_bist_clear(); /* should be called as soon as possible after reset */
        do_memory_test();
        /* _low_level_bists(); */ /* other BISTs */
        _wd_bist(); /* wd_bist should be last bist */   
    }
} 

/* ----------------------------------------------------------------------------
 * Clears flag which indicates ongoing WD BIST 
 */
static void _wd_bist_clear (void)
{
    wd_bist_active = 0;
}

/* ----------------------------------------------------------------------------
 * Executes WD BIST 
 */
void _wd_bist (void)
{
    wd_bist_active = 1;
    MSEC_DELAY(10);   /* wait for WD triggering */
    /* test failed otherwise */
}

/* ----------------------------------------------------------------------------
 * Returns state of WD BIST
 */
static uint16 _wd_bist_is_active (void)
{
    return wd_bist_active;
}

/* ---------------------------------------------------------------------------------------------------------------
 * RAM tests
 * Function do_memory_test () should NOT be static and/or inlined otherwise 
 * beware of stack pointer when called in C w/wo optimization levels, i.e. 
 * need to restore stack at the end
 * beware of the no_init section, and duplicate copy this area for full test
 */
/* alt: do_ram_check */

/* linker symbols
   should all be word aligned
*/
extern uint16 stack;
extern uint16 _end;
extern uint16 _dp__;
extern uint16 _noinit_start;
extern uint16 _noinit_end;
extern uint16 _noinit_size;
#define STACK_FRAME_SIZE 24    /* stack frame size in bytes
				  should be word aligned size
			       */
			       /* 24 bytes ok also in case of -O0 
				  only *IF* not an error in do_segment_check
				  to add 4 bytes to allow for call wd_bist_error
			       */
void do_memory_test (void)
{
    /*
     *  Memory sections layout (in increasing address order) :
     *  - global/static variables
     *  - noinit
     *  - stack
     */

     
    /*
     * - check whether we have enough free RAM above or below noinit section
     *   to relocate noinit/stack
     * - test this RAM area
     * - relocate noinit/stack there
     * - switch to new stack
     */
    if (((uint16)&_end - (uint16)&stack - STACK_FRAME_SIZE) > ((uint16)&_noinit_size + STACK_FRAME_SIZE) ) {
        /* there is a free RAM above noinit */

        /* check memory where we will save noinit/stack frame */
      do_segment_check((uint16 *) ((uint16)&_end - (uint16)&_noinit_size - STACK_FRAME_SIZE), 
		       &_end);

	/* relocate noinit/stack to new area */
        memcpy((uint16 *)((uint16)&_end - (uint16)&_noinit_size - STACK_FRAME_SIZE),
	       &_noinit_start,
	       (uint16)&_noinit_size + STACK_FRAME_SIZE);

	/* switch to relocated stack */
        MCU_setStackPtr(MCU_getStackPtr() + ((uint16)&_end - (uint16)&stack - STACK_FRAME_SIZE));

        /* check remaining RAM */
        do_segment_check(&_dp__, 
			 (uint16 *) ((uint16)&_end - (uint16)&_noinit_size - STACK_FRAME_SIZE));

	/* relocate noinit/stack to original area */
        memcpy(&_noinit_start,
	       (uint16 *) ((uint16)&_end - (uint16)&_noinit_size - STACK_FRAME_SIZE),
	       (uint16)&_noinit_size + STACK_FRAME_SIZE);

	/* switch to old stack */
        MCU_setStackPtr(MCU_getStackPtr() - ((uint16)&_end - (uint16)&stack - STACK_FRAME_SIZE));

    }
    else if (((uint16)&_noinit_start - (uint16)&_dp__) > ((uint16)&_noinit_size + STACK_FRAME_SIZE) ) {
        /* there is a free RAM below noinit */

        /* check memory where we will save noinit/stack frame */
        do_segment_check(&_dp__, 
			 (uint16 *) ((uint16)&_dp__ + (uint16)&_noinit_size + STACK_FRAME_SIZE));

	/* relocate noinit/stack */
        memcpy(&_dp__, &_noinit_start, (uint16)&_noinit_size + STACK_FRAME_SIZE);
	/* switch to new stack */
        MCU_setStackPtr(MCU_getStackPtr() - ((uint16)&_noinit_start - (uint16)&_dp__));
	/* check remaining RAM */
        do_segment_check((uint16 *) ((uint16)&_dp__ + (uint16)&_noinit_size + STACK_FRAME_SIZE), 
			 &_end);

	/* relocate noinit/stack to original area */
        memcpy(&_noinit_start, &_dp__, (uint16)&_noinit_size + STACK_FRAME_SIZE);
	/* switch to new stack */
        MCU_setStackPtr(MCU_getStackPtr() + ((uint16)&_noinit_start - (uint16)&_dp__));

    }
    else {
        /* there is no free RAM to relocate noinit/stack */
        _fatal(); /* should also set BIST_ERROR? */
    }

    /* note: stack frame and no_init should also be restored also upon exceptions */

    /* initialize main memory to 0? : memset */
    /* in else case above, the low RAM memory will not be initialized 
       as from do_segment_check; this should be catered for in do_data_bss,
       however it will not be done in case this contains variables with fixed
       addresses (declared extern) like IO ports 
    */
    
    return; /* was: return 0; */
}

/* ---------------------------------------------------------------------------
 * Checks segment from 'start' (inclusive) to end (non-inclusive)
 * Note that start and end are pointers to uint16 thus segments must be
 * aligned to a word boundary
 */
/* function name do_xxx suggests void procedure
   instead of return value can call _wd_bist_error ()
*/
int do_segment_check (uint16* start, uint16* end)
{
  uint16 *w;

  /* clear memory */
  for (w = start; w < end; ) {
    *w++ = 0;
  }

  /* check zero pattern and write 0x5555 pattern */
  for (w = start; w < end; ) {
    if (*w != 0) {
      return -1;
    }
    *w++ = 0x5555;
  }

  /* check 0x555 pattern, write and check 0xAAAA pattern */
  for (w = end; w > start; ) {
    if (*--w != 0x5555) {
      return -2;
    }
    *w = 0xAAAA;
    /* beware of compiler optim */
    if (0xAAAA != * (volatile uint16*) w) {
      return -3;
    }
  }

  return 0;
}

/* EOF */
