/*
 * Software Platform
 *
 * Copyright (C) 2009 Melexis N.V.
 * Linker script file
 *
 * Project: 14608
 *          Flash :    5 Kbytes
 *          RAM :    192 bytes
 *          EEPROM :  64 bytes
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
EXTERN(mlx_pltf_version mlx_enter_protected_mode)

MEMORY
{
    ram             (w)     : ORIGIN = 0x0000, LENGTH = 0x1000

    eeprom          (w)     : ORIGIN = 0x1000, LENGTH = 0x7F

    system_io       (rw!x)  : ORIGIN = 0x2000, LENGTH = 0xFF
    user_io         (rw!x)  : ORIGIN = 0x2800, LENGTH = 0xFF

    vectors         (rx)    : ORIGIN = 0x3000, LENGTH = 0x98    /* fp0: page */
    system_services (rx)    : ORIGIN = 0x3098, LENGTH = 0x08    /* shall be in fp:0 page to be used as "call fp0:0x98" */

    sbase           (rx)    : ORIGIN = 0x30A0, LENGTH = 0x22

    rom             (rx)    : ORIGIN = 0x30C2, LENGTH = 0xCF2E

    app_version     (r)     : ORIGIN = 0xFFF0, LENGTH = 0x04    /* Application version */
    pltf_version    (r)     : ORIGIN = 0xFFF4, LENGTH = 0x04    /* SW platform version */
    rom_checksum    (rx)    : ORIGIN = 0xFFF8, LENGTH = 0x04
}


/* Define direct, extra and io page addresses */
__dp__ = 0x00000;
__dpstart__ = 0x00028; /* start for global variables, first addresses reserved for addr assigned variables */
__io__ = 0x02800;
__ep__ = 0x01000;


SECTIONS
{
    .mlx16_vectors :
    {
        *(.vectors)
    } > vectors

    .system_services :
    {
        *(.system_services*)
    } > system_services
	
    .sbase :
    {
        *(.sbase)
    } > sbase
	
    .mlx16_text :
    {
        _stext = .;
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN (0x2);
        _etext = .;
        __data_load_start = .;
    } > rom

    .app_version :
    {
        *(.app_version)
    } > app_version

    .pltf_version :
    {
        *(.pltf_version)
    } > pltf_version

    .rom_checksum :
    {
        *(.rom_checksum+)
    } > rom_checksum


    /* Global data not cleared after reset  */
    .noinit :
    {
        . = ALIGN (0x2);
        __noinit_start = .;
        *(.noinit.BISTf)
        . = ALIGN (0x2);
        *(.noinit.BISTr)
        . = ALIGN (0x2);
        *(.noinit.*)
        __noinit_end = .;
    } > ram
    __noinit_size = __noinit_end - __noinit_start;

    /* Initialized variables (data) with .dp attribute
     * The initial valies are copied by crt from ROM to RAM
     *
     * NOTES: Output section name shoud be '.dp.data' otherwise binutils
     * complain about "Dwarf Error"
     */
    .dp.data : AT (__data_load_start)
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
    .dp __dpstart__ :
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
    .data : AT (__data_load_start + __data_dp_size)
    {
        . = ALIGN (2);
        __data_start = . ;
        *(.data)
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
        *(COMMON)
        . = ALIGN (0x2);
        __bss_end = . ;
    } > ram
    __bss_size = __bss_end - __bss_start;


    /* The stack starts immediately after the global variables */
    PROVIDE (_stack = .);
    
    /* This is the starting address of the heap.
     * It is located at the end of RAM and grows downward to the end of stack.
     * In a typical embedded application the heap is not used, but this
     * symbol has to be defined since some library functions need it.
     */
    PROVIDE (__end = 0x1000);  /* for 4096 bytes RAM */
}

/********************** Extra Page Section Definitions ***********************/

SECTIONS
{
    /* Global variables in extra page with no values specified. */
    .ep __ep__ :
    {
        __bss_ep_start = .;
        *(.ep)
        __bss_ep_end = . ;
    } > eeprom 
  
    /* Global variables in extra page with values. */
    __data_ep_load_start =  __data_load_start + __data_dp_size + __data_size ;
    .ep.data : AT ( __data_ep_load_start )
    {
        __data_ep_start = . ;
        *(.ep.data)
        . = ALIGN (2);
        __data_ep_end = . ;  
    } > eeprom 
}

/************************ IO Page Section Definitions ************************/

SECTIONS
{
    /* The io variables (i.e. ports) cannot be zeroed or have initial values */
    .io ( __io__ ) :
    {
        *(.io)
    }
}

/* EOF */
