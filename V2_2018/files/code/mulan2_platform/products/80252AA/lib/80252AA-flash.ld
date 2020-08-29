/*
 * Software Platform
 *
 * Copyright (C) 2009-2011 Melexis N.V.
 * Linker script file
 *
 * Project: 80252AA
 *          Flash :   12 Kbytes (or OTP ROM : 16 KBytes)
 *          RAM :    512 bytes
 *          EEPROM : 192 bytes
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

/* --------------------------------------------------------------------------
 * Memory region sizes (project specific)
 */
__eeprom_size  = 192;   /* size in bytes */

/* Memory region position (core specific) */
__eeprom_start = 0x1000;                           /* EEPROM start address */
__eeprom_end   = __eeprom_start + __eeprom_size;

MEMORY
{
    ram             (w)     : ORIGIN = 0x0000, LENGTH = 0x0200

    eeprom          (w)     : ORIGIN = 0x1000, LENGTH = 0xC0
    
    system_io       (rw!x)  : ORIGIN = 0x2000, LENGTH = 0xFF
    user_io         (rw!x)  : ORIGIN = 0x2800, LENGTH = 0xFF

    vectors         (rx)    : ORIGIN = 0x4000, LENGTH = 0x98    /* fp0: page */
    system_services (rx)    : ORIGIN = 0x4098, LENGTH = 0x08    /* shall be in fp:0 page to be used as "call fp0:label"     */
                                                                /* Address of the label must be aligned on 8 bytes boundary */

    /* System sections located before program memory since total length
     * is variable (could be 12K or 16K)
     */
    product_no      (r)     : ORIGIN = 0x40A0, LENGTH = 0x08    /* Product number */
    app_version     (r)     : ORIGIN = 0x40A8, LENGTH = 0x04    /* Application version */
    pltf_version    (r)     : ORIGIN = 0x40AC, LENGTH = 0x04    /* SW platform version */
    rom_checksum    (rx)    : ORIGIN = 0x40B0, LENGTH = 0x08

    rom             (rx)    : ORIGIN = 0x40B8, LENGTH = 0x2F48  /* LENGTH = 0x3F50 in case of OTP/ROM */

}


/* Define direct, extra and io page addresses */
__dp__ = 0x00000;
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
        *(.system_services)
    } > system_services

    .product_no :
    {
        *(.product_no)
    } > product_no

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
		*(.rom_checksum)
	} > rom_checksum

    .mlx16_text :
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

    } > rom


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
    } > ram AT > rom
    __data_dp_size = __data_dp_end - __data_dp_start;
    __data_load_start = LOADADDR(.dp.data);

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
    } > ram AT > rom
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


    /* The stack starts immediately after the global variables */
    PROVIDE (_stack = .);
    
    /* This is the starting address of the heap.
     * It is located at the end of RAM and grows downward to the end of stack.
     * In a typical embedded application the heap is not used, but this
     * symbol has to be defined since some library functions need it.
     */
    PROVIDE (__end = 0x0200);  /* for 512 bytes RAM */

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
