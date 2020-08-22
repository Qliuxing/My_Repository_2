/*
 * Software Platform
 *
 * Copyright (C) 2009-2015 Melexis N.V.
 * Linker script file
 *
 * Project: MULAN2
 *          Flash :   32 Kbytes
 *          RAM :      2 KBytes
 *          EEPROM1 : 256 bytes
 *          EEPROM2 : 256 bytes
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
EXTERN(__vectors mlx4_prog)
EXTERN(mlx_pltf_version mlx_enter_protected_mode)

MEMORY
{
    shared_ram      (rw!x)  : ORIGIN = 0x0000, LENGTH = 0x10    /* MLX16/MLX14 shared RAM, 16 bytes */

    ram_lin_fixed   (rw)    : ORIGIN = 0x0010, LENGTH = 0x0008  /* Used by LIN library only to store NAD.
                                                                 * Address must be fixed for support 
                                                                 * compatibility with LINLoader. */

    ram             (rwx)   : ORIGIN = 0x0018, LENGTH = 0x0728  /* MLX16 private RAM */
    dynram          (rwx)   : ORIGIN = 0x0740, LENGTH = 0x0080  /* scratch RAM on the top of stack; normally occupied by stack */

    mlx4_ram        (rw!x)  : ORIGIN = 0x07C0, LENGTH = 0x40    /* MLX4 private RAM, 64 bytes */

    eeprom          (rwx)   : ORIGIN = 0x1000, LENGTH = 0x100   /* EEPROM/NVRAM, 256 bytes */
    eeprom2         (rwx)   : ORIGIN = 0x1100, LENGTH = 0x100   /* EEPROM/NVRAM, 256 bytes */

    system_io       (rw!x)  : ORIGIN = 0x2000, LENGTH = 0x100   /* System ports */
    user_io         (rw!x)  : ORIGIN = 0x2800, LENGTH = 0x100   /* User ports */

    mlx4_flash      (rx)    : ORIGIN = 0x4000, LENGTH = 0x1540  /* MLX4 program memory */
    mlx4_table      (r)     : ORIGIN = 0x5540, LENGTH = 0x88    /* LIN tables (must be aligned on 64-byte boundary) */

    rom             (rx)    : ORIGIN = 0x55C8, LENGTH = 0x68b8  /* MLX16 program memory */

    loader_flags    (r)     : ORIGIN = 0xBE80, LENGTH = 0x0080  /* Bootloader flags page (app_enable, app_disable flags etc) */

    protection_key  (r)     : ORIGIN = 0xBF46, LENGTH = 0x08    /* Protection 64-bit key */
    flash_crc       (r)     : ORIGIN = 0xBF4E, LENGTH = 0x02    /* Flash CRC16 */
    product_no      (r)     : ORIGIN = 0xBF50, LENGTH = 0x08    /* Product number */
    app_version     (r)     : ORIGIN = 0xBF58, LENGTH = 0x04    /* Application version */
    pltf_version    (r)     : ORIGIN = 0xBF5C, LENGTH = 0x04    /* SW platform version */

    system_services (rx)    : ORIGIN = 0xBF60, LENGTH = 0x06    /* shall be in fp:0 page to be used as "call fp0:label" */
                                                                /* Address of the label must be aligned on 8 bytes boundary */

    loader_rst_state (r)    : ORIGIN = 0xBF66, LENGTH = 0x02    /* Reset vector state (variable loader_rst_state)
                                                                 * 3: reset vector points to the low part of the flash
                                                                 * 2: reset vector points to the high part of the flash (loaderB is active)
                                                                 */

    vectors         (rx)    : ORIGIN = 0xBF68, LENGTH = 0x98    /* Interrupt vectors (fp0:68, i.e. last ROM/FLASH page) */
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
    .mlx4_flash :
    {
        __mlx4_flash_start = .;
        KEEP(*(.mlx4_prog))
        __mlx4_flash_end = .;
    } > mlx4_flash

    .mlx4_table :
    {
        __mlx4_table_start = .;
        KEEP(*(.mlx4_table))
        __mlx4_table_end = .;
    } > mlx4_table

    .mlx16_vectors :
    {
        KEEP(*(.vectors))
    } > vectors

    .loader_flags :
    {
        KEEP(*(.loader_flags))
    } > loader_flags

    .loader_rst_state :
    {
        KEEP(*(.loader_rst_state))
    } > loader_rst_state

    .system_services :
    {
        KEEP(*(.system_services))
    } > system_services

    .protection_key :
    {
        KEEP(*(.protection_key))
    } > protection_key

    .flash_crc :
    {
        KEEP(*(.flash_crc))
    } > flash_crc

    .product_no :
    {
        KEEP(*(.product_no))
    } > product_no

    .app_version :
    {
        KEEP(*(.app_version))
    } > app_version

    .pltf_version :
    {
        KEEP(*(.pltf_version))
    } > pltf_version

    .ramfunc :
    {
        . = ALIGN (2);
        __ramfunc_start = . ;
        *(.ramfunc)
        . = ALIGN (2);
        __ramfunc_end = . ;
    } > dynram AT > rom

    __ramfunc_size = __ramfunc_end - __ramfunc_start;
    __ramfunc_load_start = LOADADDR(.ramfunc);

    .text :
    {
        _stext = .;

        /*
         * Link system functions by name (in case .mlx_text attribute is not
         * specified for them)
         */
        KEEP(*(.text.start))
        KEEP(*(.text._low_level_init))
        KEEP(*(.text._ram_section_init))
        KEEP(*(.text._premain))
        KEEP(*(.text._fatal))

        KEEP(*(.mlx_text))

        *(.rodata)          /* mostly jumptables */
        *(.rodata._mlx_*)   /* ROM arrays */
        . = ALIGN (0x2);

        *(.text)
        *(.text.*)
        *(.rodata.*)
        . = ALIGN (0x2);

        _etext = .;

        /*
         * this should be last linking to rom
         * the rest of the flash will be used to store load data
         * (see "AT > rom" below)
         */

    } > rom

    /* Shared area between Mlx4 and Mlx16
     * The beginning and the end of this area must be defined by
     * writing in the port SHRAML and SHRAMH
     */
    .shared_ram :
    {
        *(.special)
    } > shared_ram

    /* This section is only used by LIN library to store NAD (Node Address
       Description). Application, System and LINLoder program use this NAD in
       during firmware updating. Must be placed in fixed address area to be
       compatible with LINLoder.
     */
    .ram_lin_fixed (NOLOAD):
    {
        *(.ram_lin_fixed)
    } > ram_lin_fixed

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

    /* Global data not cleared after reset with .dp attribute */
    .dp.noinit (NOLOAD) :
    {
        . = ALIGN (0x2);
        __dp_noinit_start = .;
        *(.dp.noinit)
        . = ALIGN (0x2);
        __dp_noinit_end = .;
    } > ram
    __dp_noinit_size = __dp_noinit_end - __dp_noinit_start;

    /* Global initialized variables with nodp attribute
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

    .bist_stat (NOLOAD) :
    {
        . = ALIGN (0x2);
        __bist_stat_start = .;
        *(.bist_stat)
        . = ALIGN (0x2);
        __bist_stat_end = .;
    } > ram
    __bist_stat_size = __bist_stat_end - __bist_stat_start; 

    /* The stack starts immediately after the global variables */
    PROVIDE (_stack = .);
    
    /* This is the starting address of the heap.
     * It is located at the end of RAM and grows downward to the end of stack.
     * In a typical embedded application the heap is not used, but this
     * symbol has to be defined since some library functions need it.
     */
    PROVIDE (__end = 0x0800);  /* for 2K RAM */

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
