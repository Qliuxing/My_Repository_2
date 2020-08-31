/*
 * Software Platform
 *
 * Copyright (C) 2009-2013 Melexis N.V.
 * Linker script file
 *
 * Project: MLX81108 / MLX81109
 *          Flash :   32 Kbytes
 *          RAM :     1 Kbyte
 *          EEPROM1 :  256 bytes
 *          EEPROM2 :  256 bytes
 */

OUTPUT_FORMAT("elf32-mlx16-x8")
OUTPUT_ARCH(mlx16x)

/*
 * Default libraries which will be used for linking
 * Any additional libraries could be provided either through command
 * line option or in linker script
 */

/* Standard library */
GROUP(-lc -lgcc)

/* Force searching symbol in libraries if not defined */
EXTERN(__vectors)

MEMORY
{
    ram             (rwx)   : ORIGIN = 0x0000, LENGTH = 0x0400  /* MLX16 RAM               */

    eeprom          (rwx)   : ORIGIN = 0x1000, LENGTH = 0x100   /* EEPROM/NVRAM, 256 bytes */
    eeprom2         (rwx)   : ORIGIN = 0x1100, LENGTH = 0x100   /* EEPROM/NVRAM, 256 bytes */
    
    system_io       (rw!x)  : ORIGIN = 0x2000, LENGTH = 0x100   /* System ports */
    user_io         (rw!x)  : ORIGIN = 0x2800, LENGTH = 0x100   /* User ports   */
}


/* Define direct, extra and io page addresses */
__dp__ = 0x0000;
__io__ = 0x2800;
__ep__ = 0x1000;

__fp0__ = 0xBF00;	/* interrupt vector table */
__fp1__ = 0xBE00;
__fp2__ = 0xBD00;
__fp3__ = 0xBC00;
__fp4__ = 0xBB00;
__fp5__ = 0xBA00;
__fp6__ = 0xB900;
__fp7__ = 0x1000;
 


SECTIONS
{
    .vectors :
    {
        *(.vectors)
    } > ram

    /* Initialized variables (data) with .dp attribute
     * The initial values are copied by crt from ROM to RAM
     *
     * NOTES: Output section name should be '.dp.data' otherwise binutils
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

    /* Global initialised variables with nodp attribute
     * The initial values are copied by crt from ROM
     */
    .data :
    {
        . = ALIGN (2);
        __data_start = . ;
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

    .ramfunc :
    {
        . = ALIGN (2);
        __ramfunc_start = . ;
        *(.ramfunc)
        . = ALIGN (2);
        __ramfunc_end = . ;
    } > ram

    __ramfunc_size = __ramfunc_end - __ramfunc_start;
    __ramfunc_load_start = LOADADDR(.ramfunc);

    .text :
    {
        _stext = .;
        *(.text)
        *(.text.*)

        *(.mlx_text)

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
    PROVIDE (__end = 0x0400);  /* for 1 Kbyte RAM */

}

/* EOF */
