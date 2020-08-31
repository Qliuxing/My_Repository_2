/*
 * Software Platform
 *
 * Copyright (C) 2010-2012 Melexis N.V.
 * Linker script file
 *
 * Project: 90294
 *          ROM    :  12 Kbytes
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

/* Force searching symbol in libraries if not defined in user files */
EXTERN(__vectors)
EXTERN(mlx_pltf_version mlx_enter_protected_mode)


/* --------------------------------------------------------------------------
 * Memory region sizes (project specific)
 */
__ram_size     = 384;  /* size in bytes */
__rom_size     = 12K;  /* size in bytes */
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

/*----------------------------------------------------------------------------*/
/* Additional: For EEPROM test CRC-8 */
__eeprom_non_tested_size_in_bytes = 4; /* last 4 bytes not covered by CRC */
__eeprom_nr_parts = 5;

__eeprom_test_end_addr = (__eeprom_end - __eeprom_non_tested_size_in_bytes); /* CRC8 is in 2nd last word */
__eeprom_test_size_in_bytes = (__eeprom_size - __eeprom_non_tested_size_in_bytes);
__eeprom_test_part_size = ((__eeprom_test_size_in_bytes +1) / __eeprom_nr_parts);
__eeprom_end_addr_bypass = (__eeprom_end - 2); /* BYPASS is in 2nd last word */

/*----------------------------------------------------------------------------*/
/* Additional: For ROM test checksum */
__rom_non_tested_size_in_word = 1; /* length of romCheckSum word */
__rom_nr_parts = 80;

__rom_full_size_in_words = __rom_size / 2;
__rom_tested_size_in_words = __rom_full_size_in_words - __rom_non_tested_size_in_word;

__rom_test_end_addr = __rom_end - (__rom_non_tested_size_in_word * 2);
__rom_test_part_size_byte = (__rom_tested_size_in_words * 2) / __rom_nr_parts;


MEMORY
{
    noinit2         (w)     : ORIGIN = 0x0000, LENGTH = 0x0018  /* fail-safe data (failSafePCatError, failSafeSource) */

    ram             (w)     : ORIGIN = 0x0018, LENGTH = 0x0168

    eeprom          (w)     : ORIGIN = 0x1000, LENGTH = 0x080	/* various reserved regions */
    
    system_io       (rw!x)  : ORIGIN = 0x2000, LENGTH = 0x100
    user_io         (rw!x)  : ORIGIN = 0x2800, LENGTH = 0x100

    vectors         (rx)    : ORIGIN = 0x4000, LENGTH = 0x98    /* fp0: page */
    system_services (rx)    : ORIGIN = 0x4098, LENGTH = 0x08    /* shall be in fp:0 page to be used as "call fp0:label"     */
                                                                /* Address of the label must be aligned on 8 bytes boundary */

    product_no      (r)     : ORIGIN = 0x40A0, LENGTH = 0x08    /* Product number */
    app_version     (r)     : ORIGIN = 0x40A8, LENGTH = 0x04    /* Application version */
    pltf_version    (r)     : ORIGIN = 0x40AC, LENGTH = 0x04    /* SW platform version */


    rom             (rx)    : ORIGIN = 0x40B0, LENGTH = 0x2F4E
    rom_checksum    (rx)    : ORIGIN = 0x6FFE, LENGTH = 0x02    /* checksum should be last word of the ROM */
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


    /* Global data not cleared after reset  */
    .noinit2 (NOLOAD) :
    {
        . = ALIGN (0x2);
        __noinit2_start = .;
        *(.noinit2)
        . = ALIGN (0x2);
        __noinit2_end = .;
    } > noinit2
    __noinit2_size = __noinit2_end - __noinit2_start;
    
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
