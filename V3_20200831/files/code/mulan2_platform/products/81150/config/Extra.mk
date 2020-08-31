#
# Copyright (C) 2008-2013 Melexis N.V.
#
# Software Platform
#


#--- Product specific defines -------------------------------------------------

CORE_TYPE := 82050
INSTSET := -mlx16-x8
MCU_OSC_FREQ_KHZ := 250ul

CFLAGS += -ffunction-sections -fdata-sections

# Use -mdpage-explicit to have global variables in nodp by default
CFLAGS  += -mdpage-explicit

# Interrupt controller bug MMC16-26 / MULAN-5
#
# CPU instructions "callf" and "pop M" might be aborted in the middle.
# Workaround: insert mov r,#0 instruction before the "callf" and "pop M".
# It is done automatically by C-compiler with -mwith-itc-bug-workaround option
# for "pop M" instruction while compiling C-sources (compiler never uses callf
# instruction). Note, for assembly code this workaround shall be done by the
# programmer.

CFLAGS  += -mwith-itc-bug-workaround
ASFLAGS += -mwith-itc-bug-workaround

# --- LIN API options ---------------------------------------------------------
# CPPFLAGS += -DHAS_LIN_AUTOADDRESSING  # auto-adressing is not supported by 81150 HW
CPPFLAGS += -DHAS_SET_LOADER_NAD        # reports new NAD to the LIN loader

CPPFLAGS += -DHAS_MLX4_CMD_ACK_TIMEOUT  # requires application's handlers to check that API commands are not timed-out 

# --- LIN loader options ------------------------------------------------------

# -- Loader flash protection options
#CPPFLAGS += -DLDR_HAS_PROTECTION_KEY                  # Loader protection mechanism enabling
CPPFLAGS += -DLDR_PROTECTION_KEY_LENGTH=32            # key length can be whether 32 or 64 bits
CPPFLAGS += -DLDR_PROTECTION_KEY_ADDR_FLASH=0xBF46    # protection key location into Flash
CPPFLAGS += -DLDR_UNLOCKING_KEY_ADDR_NVRAM=0x1080     # unlocking key location into NVRAM

# CPPFLAGS += -DHAS_FLASH_WRITE_BUFFER_IN_NVRAM_SRAM
# CPPFLAGS += -DFLASH_WRITE_BUFFER_ADDR=0x1080

# --- Product specific options ------------------------------------------------
CPPFLAGS += -I $(ROOTDIR)/libsrc/LIN    # LIN component includes

#--- List of modules for this configuration -----------------------------------
LIBSUBMODULE := startup ../products/$(PRODUCT)/src nvram math
 

#--- Include profile specific configuration -----------------------------------
include $(PRODUCT_DIR)/config/$(PROFILE_NAME).mk

# FPLL is visible in all source files
FPLL := "($(MCU_OSC_FREQ_KHZ) * $(MCU_PLL_MULT))"

# Extra defines from profile
CPPFLAGS += -DFPLL=$(FPLL)
CPPFLAGS += -DMCU_PLL_MULT=$(MCU_PLL_MULT)

#------------------------------------------------------------------------------
# Set internal compile time switches based on configuration
# from specified profile
#
# STANDALONE_LOADER - indicates presence of standalone LIN-pin loader; otherwise
#                     
# LIN_PIN_LOADER  � indicates presence of LIN-pin loader, i.e. loader which
#					uses LIN-pin as a communication link. This can be standard
#					LIN loader w/ fast mode) or standalone loader (fast mode
#					only)
# HAS_MLX4_CODE   � indicates presence of the code for MLX4. 
# LD_SCRIPT       � specifies appropriate linker script file (if not defined in
#					the profile)
#
ifeq ($(APP_TYPE), NOLIN_APP)

    # --- Check if loader is requested (based on HAS_LOADER) ------------------
    ifeq ($(HAS_LOADER), 0)
        STANDALONE_LOADER := 0
        HAS_MLX4_CODE := 0
        CPPFLAGS += -DSTANDALONE_LOADER=0 -DLIN_PIN_LOADER=0 -DHAS_MLX4_CODE=0
        ifndef LD_SCRIPT
            LD_SCRIPT := $(PRODUCT)-nolin.ld
        endif
    else
    ifeq ($(HAS_LOADER), 1)

        LIBSUBMODULE += LIN ../products/LINLoader # Standalone loader

        STANDALONE_LOADER := 1
        HAS_MLX4_CODE := 1
        CPPFLAGS += -DSTANDALONE_LOADER=1 -DLIN_PIN_LOADER=1 -DHAS_MLX4_CODE=1
        CPPFLAGS += -DML_FAST_BAUDRATE=$(ML_FAST_BAUDRATE)UL
        CPPFLAGS += -DMLX4_FW_STANDALONE_LOADER
        
        CPPFLAGS += -DSUPPORT_LINNETWORK_LOADER
        
        ifndef LD_SCRIPT
            LD_SCRIPT := $(PRODUCT)-fast.ld
        endif	        
        ifndef LD_SCRIPT_LOADER
            LD_SCRIPT_LOADER := $(PRODUCT)-fast-loaderB.ld
        endif
    else
        $(error Incorrect HAS_LOADER value (should be either 0 or 1))
    endif
    endif


else  # LINxx_APP

    #--- Add LIN module  ---------------------------------------------------------
    LIBSUBMODULE += LIN

    CPPFLAGS += -DML_BAUDRATE=$(ML_BAUDRATE)

    # --- Select MLX4 FW image based on LIN version and baud rate settings ---------------------------
    ifeq ($(APP_TYPE), LIN21_APP)
        $(warning Use LIN20_APP configuration for LIN2.1 application)
        CPPFLAGS += -DMLX4_FW_LIN2X
    else
    ifeq ($(APP_TYPE), LIN20_APP)
        CPPFLAGS += -DMLX4_FW_LIN2X
    else
    ifeq ($(APP_TYPE), LIN13_APP)
        ifeq ($(ML_BAUDRATE), 9600)
            CPPFLAGS += -DMLX4_FW_LIN13_9600
        else
        ifeq ($(ML_BAUDRATE), 19200)
            CPPFLAGS += -DMLX4_FW_LIN13_19200
        else
            $(error There is no MLX4 LIN image for specified baud rate $(ML_BAUDRATE). Please contact Melexis)
        endif
        endif
    else
        $(error Incorrect APP_TYPE value)
    endif
    endif
    endif

    # --- Check if loader is requested (based on HAS_LOADER) -----------------------
    ifeq ($(HAS_LOADER), 0)
        STANDALONE_LOADER := 0
        HAS_MLX4_CODE := 1
        CPPFLAGS += -DSTANDALONE_LOADER=0 -DLIN_PIN_LOADER=0 -DHAS_MLX4_CODE=1
        ifndef LD_SCRIPT
            LD_SCRIPT := $(PRODUCT)-lin.ld
        endif	
    else
    ifeq ($(HAS_LOADER), 1)
        LIBSUBMODULE += ../products/LINLoader

        STANDALONE_LOADER := 0
        HAS_MLX4_CODE := 1
        CPPFLAGS += -DSTANDALONE_LOADER=0 -DLIN_PIN_LOADER=1 -DHAS_MLX4_CODE=1
        CPPFLAGS += -DML_FAST_BAUDRATE=$(ML_FAST_BAUDRATE)UL
        
        CPPFLAGS += -DSUPPORT_LINNETWORK_LOADER
        
        ifndef LD_SCRIPT
            LD_SCRIPT := $(PRODUCT)-lin.ld
        endif	
        ifndef LD_SCRIPT_LOADER
            LD_SCRIPT_LOADER := $(PRODUCT)-lin-loaderB.ld
        endif
    else
        $(error Incorrect HAS_LOADER value (should be either 0 or 1))
    endif
    endif

endif  # LINxx_APP
