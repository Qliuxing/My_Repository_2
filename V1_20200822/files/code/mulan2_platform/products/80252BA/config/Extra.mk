#
# Copyright (C) 2008-2011 Melexis N.V.
#
# Software Platform
#


#--- Product specific defines -------------------------------------------------
CORE_TYPE := mmc16
INSTSET := -mlx16-8
MCU_PLL_FREQ_KHZ := 16000ul

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

#--- List of modules for this product -----------------------------------------
LIBSUBMODULE := startup ../products/$(PRODUCT)/src eeprom math


#--- Include profile specific configuration -----------------------------------
include $(PRODUCT_DIR)/config/$(PROFILE_NAME).mk

# FPLL is visible in all source files
FPLL := $(MCU_PLL_FREQ_KHZ)

# Extra defines from profile
CPPFLAGS += -DEEPROM_USE_ECC=$(EEPROM_USE_ECC)
CPPFLAGS += -DFPLL=$(FPLL)
