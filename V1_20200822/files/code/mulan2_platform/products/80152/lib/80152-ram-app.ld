/*
 * Software Platform
 *
 * Copyright (C) 2010-2011 Melexis N.V.
 * Linker script file
 *
 * Project: 80152
 *          Flash : 8 Kbytes (overlapped with OTP ROM : 8 or 16 KBytes)
 *          RAM : 256 bytes
 *          EEPROM : 64 bytes
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
EXTERN(_start)


/* --------------------------------------------------------------------------
 * Memory region sizes (project specific)
 */
__eeprom_size  = 64;    /* size in bytes */


/* Memory region position (core specific) */
__eeprom_start = 0x1000;                           /* EEPROM start address */
__eeprom_end   = __eeprom_start + __eeprom_size;

MEMORY
{
    ram             (w)     : ORIGIN = 0x0000, LENGTH = 0x0100

    eeprom          (w)     : ORIGIN = 0x1000, LENGTH = 0x40
    
    system_io       (rw!x)  : ORIGIN = 0x2000, LENGTH = 0xFF
    user_io         (rw!x)  : ORIGIN = 0x2800, LENGTH = 0xFF
}


/* Define direct, extra and io page addresses */
__dp__ = 0x00000;
__io__ = 0x02800;
__ep__ = 0x01000;


SECTIONS
{

    .vectors :
    {
        *(.vectors)
    } > ram


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


    .mlx16_text :
    {
        _stext = .;
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN (0x2);
        _etext = .;
    } > ram


    /* The stack starts immediately after the global variables */
    PROVIDE (_stack = .);
    
    /* This is the starting address of the heap.
     * It is located at the end of RAM and grows downward to the end of stack.
     * In a typical embedded application the heap is not used, but this
     * symbol has to be defined since some library functions need it.
     */
    PROVIDE (__end = 0x0200);  /* for 512 bytes RAM */

}


/* EOF */
