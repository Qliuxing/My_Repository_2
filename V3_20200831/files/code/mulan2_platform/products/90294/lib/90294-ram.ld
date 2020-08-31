/*
 * Software Platform
 *
 * Copyright (C) 2010-2012 Melexis N.V.
 * Linker script file
 *
 * Project: 90294
 *          ROM    :  10 Kbytes
 *          RAM    : 384  bytes
 *          EEPROM : 128  bytes
 */

OUTPUT_FORMAT("elf32-mlx16-8")
OUTPUT_ARCH(mlx16_8)

/*
 * Default libraries which will be used for linking
 * Any additional libraries could be provided either through command
 * line option or in linker script
 */

/* Standard library */
GROUP(-lc -lgcc)

/* Force searching symbol in libraries if not defined */
EXTERN(__vectors)


/* --------------------------------------------------------------------------
 * Memory region sizes (project specific)
 */
__ram_size     = 384;  /* size in bytes */
__eeprom_size  = 128;  /* size in bytes */

__system_io_size = 256;
__user_io_size   = 256;


/* Memory region position (core specific) */
__ram_start    = 0x0000;                           /* RAM start address    */
__ram_end      = __ram_start + __ram_size;

__eeprom_start = 0x1000;                           /* EEPROM start address */
__eeprom_end   = __eeprom_start + __eeprom_size;

__system_io_start = 0x2000;
__system_io_end   = __system_io_start + __system_io_size;

__user_io_start = 0x2800;
__user_io_end   = __user_io_start + __user_io_size;



MEMORY
{
    vectors         (rwx)   : ORIGIN = 0x0000, LENGTH = 0x0C    /* MLX16 RAM               */    
    ram             (rwx)   : ORIGIN = 0x0016, LENGTH = 0x0180 - 0x16 /* MLX16 RAM               */
    
    eeprom          (rwx)   : ORIGIN = 0x1000, LENGTH = 0x080   /* EEPROM/NVRAM, 256 bytes */
    
    system_io       (rw!x)  : ORIGIN = 0x2000, LENGTH = 0x100   /* System ports */
    user_io         (rw!x)  : ORIGIN = 0x2800, LENGTH = 0x100   /* User ports   */



}


/* Define direct, extra and io page addresses */
__dp__ = __ram_start;
__io__ = __user_io_start;
__ep__ = __eeprom_start;

/*__fp0__ = 0xBF00;	*//* interrupt vector table */
/*__fp1__ = 0xBE00; */
/*__fp2__ = 0xBD00; */
/*__fp3__ = 0xBC00; */
/*__fp4__ = 0xBB00; */
/*__fp5__ = 0xBA00; */
/*__fp6__ = 0xB900; */
/*__fp7__ = 0x1000; */
 


SECTIONS
{
    .vectors :
    {
        *(.vectors)
    } > vectors

/*
    .ext_var :
    {
        . = ALIGN (0x2);
        __ext_var_start = .;
        *(.ext_var)
        . = ALIGN (0x2);
        __ext_var_end = .;
    } > 
    __ext_var_size = __ext_var_end - __ext_var_start;
*/

    /* Initialized variables (data) with .dp attribute
     * The initial values are copied by crt from ROM to RAM
     *
     * NOTES: Output section name shoud be '.dp.data' otherwise binutils
     * complain about "Dwarf Error"
     */
    .dp.data :
    {
        . = ALIGN (0x2);
        __data_dp_start = . ;
        *(.dp.data)
        . = ALIGN (0x2);
        __data_dp_end = . ;
    } > ram
    __data_dp_size = __data_dp_end - __data_dp_start;

    /* Uninitialized variables (bss) with .dp attribute
     * This memory area is zeroed by crt
     *
     * NOTES: Output section name shoud be '.dp' otherwise binutils
     * complain about "Dwarf Error"
     */
    .dp :
    {
        . = ALIGN (0x2);
        __bss_dp_start = .;
        *(.dp)
        . = ALIGN (0x2);
        __bss_dp_end = . ;
    } > ram
    __bss_dp_size = __bss_dp_end - __bss_dp_start;
    
    /* Global initialized variables with nodp attribute
     * The initial values are copied by crt from ROM
     */
    .data :
    {
        . = ALIGN (2);
        __data_start = . ;
        *(.ramfunc)
        *(.data)
        *(.data.*)
        . = ALIGN (2);
        __data_end = . ;
    } > ram
    __data_size = __data_end - __data_start ;
    
    /* Global variables with nodp attribute
     * This memory area is zeroed by crt
     */
    .bss :
    {
        . = ALIGN (0x2);
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN (0x2);
        __bss_end = . ;
    } > ram
    __bss_size = __bss_end - __bss_start;
    

    /* Global data not cleared after reset  */
    .noinit (NOLOAD) :
    {
        . = ALIGN (0x2);
        __noinit_start = .;
        *(.noinit)
        . = ALIGN (0x2);
        __noinit_end = .;
    } > ram
    __noinit_size = __noinit_end - __noinit_start;
    
    .text :
    {
        _stext = .;
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN (0x2);
        _etext = .;

        /*
         * this should be last linking to rom
         * the rest of the flash will be used to store load data
         * (see "AT > rom" below)
         */

    } > ram

    /* The stack starts immediately after the global variables */
    PROVIDE (_stack = .);
    
    /* This is the starting address of the heap.
     * It is located at the end of RAM and grows downward to the end of stack.
     * In a typical embedded application the heap is not used, but this
     * symbol has to be defined since some library functions need it.
     */
    PROVIDE (__end = __ram_end);
}
/********************** Extra Page Section Definitions ***********************/

SECTIONS
{
    /* Global variables in extra page with no values specified. */
    .ep :
    {
        . = ALIGN (2);        
        __bss_ep_start = .;
        *(.ep)
        . = ALIGN (2);        
        __bss_ep_end = . ;
    } > eeprom 

    .ep.data :
    {
        . = ALIGN (2);        
        __data_ep_start = . ;
        *(.ep.data)
        . = ALIGN (2);
        __data_ep_end = . ;
    } > eeprom 
}

/* EOF */
