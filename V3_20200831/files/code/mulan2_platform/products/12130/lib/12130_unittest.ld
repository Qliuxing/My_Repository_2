/*
 * Software Platform
 *
 * Copyright (C) 2012 Melexis N.V.
 * Linker script file
 *
 * Project: 12130
 *          ROM :    15 Kbytes
 *          RAM :    448 bytes
 *          EEPROM : 192 bytes
 *
 */

OUTPUT_FORMAT("elf32-mlx16-x8")
OUTPUT_ARCH(mlx16x)

/*
 * Default libraries which will be used for linking
 * Any additional libraries could be provided either through command
 * line option or in linker script
 */

GROUP(-lc -lgcc)            /* Standard library */
GROUP(task1.a task2.a)      /* Task libraries   */

/* Force searching symbol in libraries if not defined in user files */
EXTERN(__vectors)
EXTERN(mlx_pltf_version mlx_enter_protected_mode)


/* --------------------------------------------------------------------------
 * Memory region sizes (project specific)
 */
__ram_size     = 0x800;   /* size in bytes */
__rom_size     = 15K;   /* size in bytes */
__eeprom_size  = 192;   /* size in bytes */

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
    sys_ram         (w)     : ORIGIN = 0x0000, LENGTH = 0x10        /* BIST result and bit accessible ram   */
    task1_ram       (w)     : ORIGIN = 0x0010, LENGTH = 0x60        /* 96 bytes                             */
    task2_ram       (w)     : ORIGIN = 0x0070, LENGTH = 0x60        /* 96 bytes                             */
    ram             (w)     : ORIGIN = 0x00D0, LENGTH = 0x800 - 0xD0

    task1_eeprom    (w)     : ORIGIN = 0x1000, LENGTH = 0x10        /* 16 bytes */
    task2_eeprom    (w)     : ORIGIN = 0x1010, LENGTH = 0x10        /* 16 bytes */
    eeprom          (w)     : ORIGIN = 0x1020, LENGTH = 192 - 0x20
    
    system_io       (rw!x)  : ORIGIN = 0x2000, LENGTH = 0x100
    user_io         (rw!x)  : ORIGIN = 0x2800, LENGTH = 0x100

    vectors         (rx)    : ORIGIN = 0x4000, LENGTH = 0x98    /* fp0: page */
    system_services (rx)    : ORIGIN = 0x4098, LENGTH = 0x08    /* shall be in fp:0 page to be used as "call fp0:label"     */
                                                                /* Address of the label must be aligned on 8 bytes boundary */
    product_no      (r)     : ORIGIN = 0x40A0, LENGTH = 0x08    /* Product number */
    app_version     (r)     : ORIGIN = 0x40A8, LENGTH = 0x04    /* Application version */
    pltf_version    (r)     : ORIGIN = 0x40AC, LENGTH = 0x04    /* SW platform version */

    rom             (rx)    : ORIGIN = 0x40B0, LENGTH = 0x8D50
    task1_rom       (rx)    : ORIGIN = 0xCE00, LENGTH = 0x0700  /* 1792 bytes */
    task2_rom       (rx)    : ORIGIN = 0xD500, LENGTH = 0x0700  /* 1792 bytes */

    assert_mem      (r)     : ORIGIN = 0x0000, LENGTH = 0x00    /* for assertions */
}


/* Define direct, extra and io page addresses */
__dp__ = __ram_start;
__io__ = __user_io_start;
__ep__ = __eeprom_start;


SECTIONS
{
    /* ------------------------------------------------------------------------
     * Task 1 allocations
     */
    .task1_text :
    {
        . = ALIGN (0x2);
        __task1_text_start = .;

        task1.a(.text)
        task1.a(.text.*)

        task1.a(.rodata)
        task1.a(.rodata.*)

        . = ALIGN (0x2);
        __task1_text_end = .;

    } > task1_rom

    .task1_bss_dp :
    {
        . = ALIGN (0x2);
        __task1_bss_dp_start = .;
        task1.a(.dp)
        . = ALIGN (0x2);
        __task1_bss_dp_end = . ;
    } > task1_ram

    /*
     * Startup initialization routine can not access private RAM directly, so
     * initialized variables are not allowed. Run-time initalization function
     * linked into task1_text area shall be used for variable initialization.
     */    
    .initialized_variables_are_not_allowed_in_task1 :
    {
        task1.a(.dp.data)
    } > assert_mem


    /*
     *  Private RAM of the task is located in the dp section of the RAM (first 256 bytes of RAM).
     *  The full address access is not efficient for the dp section and thus SHALL NOT be used.
     *  So, variables w/o dp attribute are not allowed in the private RAM of the task.
     */
    .variable_without_dp_attribute_is_not_allowed_in_task1 :
    {
        task1.a (.data)
        task1.a (.data.*)
        task1.a (.bss)
        task1.a (.bss.*)
        task1.a (COMMON)

    } > assert_mem

    .task1_bss_ep :
    {
        . = ALIGN (2);        
        __task1_bss_ep_start = .;
        task1.a(.ep)
        . = ALIGN (2);        
        __task1_bss_ep_end = . ;
    } > task1_eeprom 

    .task1_data_ep :
    {
        . = ALIGN (2);        
        __task1_data_ep_start = . ;
        task1.a(.ep.data)
        . = ALIGN (2);
        __task1_data_ep_end = . ;
    } > task1_eeprom


    /* ------------------------------------------------------------------------
     * Task 2 allocations
     */
    .task2_text :
    {
        . = ALIGN (0x2);
        __task2_text_start = .;

        task2.a(.text)
        task2.a(.text.*)

        task2.a(.rodata)
        task2.a(.rodata.*)

        . = ALIGN (0x2);
        __task2_text_end = .;

    } > task2_rom

    .task2_bss_dp :
    {
        . = ALIGN (0x2);
        __task2_bss_dp_start = .;
        task2.a(.dp)
        . = ALIGN (0x2);
        __task2_bss_dp_end = . ;
    } > task2_ram


    /*
     * Startup initialization routine can not access private RAM directly, so
     * initialized variables are not allowed. Run-time initalization function
     * linked into task1_text area shall be used for variable initialization.
     */    
    .initialized_variables_are_not_allowed_in_task2 :
    {
        task2.a(.dp.data)
    } > assert_mem


    /*
     *  Private RAM of the task is located in the dp section of the RAM (first 256 bytes of RAM).
     *  The full address access is not efficient for the dp section and thus SHALL NOT be used.
     *  So, variables w/o dp attribute are not allowed in the private RAM of the task.
     */
    .variable_without_dp_attribute_is_not_allowed_in_task2 :
    {
        task2.a (.data)
        task2.a (.data.*)
        task2.a (.bss)
        task2.a (.bss.*)
        task2.a (COMMON)

    } > assert_mem

    .task2_bss_ep :
    {
        . = ALIGN (2);        
        __task2_bss_ep_start = .;
        task2.a(.ep)
        . = ALIGN (2);        
        __task2_bss_ep_end = . ;
    } > task2_eeprom 

    .task2_data_ep :
    {
        . = ALIGN (2);        
        __task2_data_ep_start = . ;
        task2.a(.ep.data)
        . = ALIGN (2);
        __task2_data_ep_end = . ;
    } > task2_eeprom


    /* ------------------------------------------------------------------------
     * Common tasks allocations
     */
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
