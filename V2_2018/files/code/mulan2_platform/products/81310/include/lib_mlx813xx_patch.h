/*
 * Copyright (C) 2008-2015 Melexis N.V.
 *
 * Software Platform
 * Product/board specific functions
 *
 */

#ifndef LIB_MLX81310_PATCH_H_
    #define LIB_MLX81310_PATCH_H_

    #include <syslib.h>
    #include <lib_mlx813xx_nvram.h>

    extern volatile uint8  u16PatchSize __attribute((nodp, addr(0x10E8)));
    extern volatile uint8  u16PatchCRC  __attribute((nodp, addr(0x10E9)));
    extern volatile uint16 u16PatchVersion __attribute((nodp, addr(0x10EA)));

    #define C_NVRAM_PATCH_MASK          0xFFF8U
    #ifndef C_NVRAM_PATCH_ID
        #define C_NVRAM_PATCH_ID        ((0x04U << 8U) | (0xB8U))
    #endif


    /*
     *****************************************************************************
     * patch_Load
     * Load the patches in the patching mechanism
     *
     *****************************************************************************
     */
    __MLX_TEXT__  static INLINE void patch_Load (void)
    {
        /* Now the NV-contents is copied to (NV)RAM, the Patch can be activated
         * 0x1080 - 0x10E7: Patch-code.
         * 0x10E8: Size (0x1080-0x10E7) in 16-bits words.
         * 0x10E9: CRC8
         * 0x10EA: Bit 7:3: Project Software version ID, Bit 2:0: Patch Software version ID
         * 0x10EB: Project-ID
         * 0x10EC-0x10FB: Patch-table
         */
        if ((FL_CTRL0 & FL_DETECT) == 0)
        {
            /* This is a ROM chip */
            uint16 u16CRC = nvram_CalcCRC( ((uint16*) 0x10E8) - u16PatchSize,
                                           ((END_MLX_PATCH_ADDR + 1) - 0x10E8 - u16PatchSize)/2 );

            if ( u16CRC == 0xFFU )
            {
                /* Correct Checksum */
                if ( (u16PatchVersion & C_NVRAM_PATCH_MASK) == C_NVRAM_PATCH_ID )
                {
                    /* Correct Project-ID and ROM Firmware version */
                    uint16 *u16PatchAddr = (uint16*) BGN_MLX_PATCH_ADDR;
                    uint16 *u16PatchIo = (uint16*) &PATCH0_I;
                    do
                    {
                        *u16PatchIo++ = *u16PatchAddr++;
                    } while ( u16PatchIo <= (uint16*) &PATCH3_A);
                }
            }
        }
        else
        {
            /* No patching of flash chips */
        }
    }

#endif /* LIB_MLX81310_PATCH_H_ */
