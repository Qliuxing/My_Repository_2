/*
 * Software Platform
 *
 * Copyright (C) 2009-2011 Melexis N.V.
 * Linker script file
 *
 * Project: 12127
 *          ROM :    8 Kbytes
 *          RAM :    320 bytes
 *          EEPROM : 128 bytes
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

/* Force searching symbol in libraries if not defined in user files */
EXTERN(__vectors)
EXTERN(mlx_pltf_version mlx_enter_protected_mode)


/* --------------------------------------------------------------------------
 * Memory region sizes (project specific)
 */
__ram_size     = 320;  /* size in bytes */
__rom_size     = 8K;   /* size in bytes */
__eeprom_size  = 128;  /* size in bytes */

__system_io_size = 256;
__user_io_size   = 256;


/* Memory region position (core specific) */
__ram_start    = 0x0000;                           /* RAM start address    */
__ram_end      = __ram_start + __ram_size;

__rom_start    = 0x4000;                           /* ROM start address    */
__rom_end      = __rom_start + __rom_size;

__eeprom_start = 0x1000;                           /* EEPROM start address */
__eeprom_end   = __eeprom_start + __eeprom_size;

__system_io_start = 0x2000;
__system_io_end   = __system_io_start + __system_io_size;

__user_io_start = 0x2800;
__user_io_end   = __user_io_start + __user_io_size;



MEMORY
{
    sys_ram         (w)     : ORIGIN = 0x0000, LENGTH = 0x10    /* BIST result and bit accessible ram   */
    ram             (w)     : ORIGIN = 0x0010, LENGTH = 0x130   /* (320 - 16) bytes                     */

    eeprom          (w)     : ORIGIN = 0x1000, LENGTH = 0x80    /* 128 bytes: various reserved regions */
    
    system_io       (rw!x)  : ORIGIN = 0x2000, LENGTH = 0x100
    user_io         (rw!x)  : ORIGIN = 0x2800, LENGTH = 0x100

    vectors         (rx)    : ORIGIN = 0x4000, LENGTH = 0x98    /* fp0: page */
    system_services (rx)    : ORIGIN = 0x4098, LENGTH = 0x08    /* shall be in fp:0 page to be used as "call fp0:label"     */
                                                                /* Address of the label must be aligned on 8 bytes boundary */

    product_no      (r)     : ORIGIN = 0x40A0, LENGTH = 0x08    /* Product number */
    app_version     (r)     : ORIGIN = 0x40A8, LENGTH = 0x04    /* Application version */
    pltf_version    (r)     : ORIGIN = 0x40AC, LENGTH = 0x04    /* SW platform version */

    rom             (rx)    : ORIGIN = 0x40B0, LENGTH = 0x1F4E
    rom_checksum    (rx)    : ORIGIN = 0x5FFE, LENGTH = 0x02    /* checksum should be last word of the ROM */
}


/* Define direct, extra and io page addresses */
__dp__ = __ram_start;
__io__ = __user_io_start;
__ep__ = __eeprom_start;


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


    .sys_ram :
    {
        . = ALIGN (0x2);
        *(.sys_ram)
        . = ALIGN (0x2);
    } > sys_ram


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
