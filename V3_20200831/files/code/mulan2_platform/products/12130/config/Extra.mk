#
# Copyright (C) 2012 Melexis N.V.
#
# Software Platform
#
#


#--- Product specific defines -------------------------------------------------
CORE_TYPE := mmc16
INSTSET := -mlx16-x8
MCU_PLL_FREQ_KHZ := 13000ul

# Use -mdpage-explicit to have global variables in nodp by default
CFLAGS  += -mdpage-explicit

# Interrupt controller bug fixed (see MMC16-26 and MULAN-5 on Jira)
#CFLAGS  += -mwith-itc-bug-workaround
#ASFLAGS += -mwith-itc-bug-workaround

LD_SCRIPT := $(PRODUCT).ld


#--- List of modules for this product -----------------------------------------
LIBSUBMODULE := startup ../products/$(PRODUCT)/src eeprom math


#--- Include profile specific configuration -----------------------------------
include $(PRODUCT_DIR)/config/$(PROFILE_NAME).mk

# FPLL is visible in all source files
FPLL := $(MCU_PLL_FREQ_KHZ)

# Extra defines from profile
CPPFLAGS += -DEEPROM_USE_ECC=$(EEPROM_USE_ECC)
CPPFLAGS += -DFPLL=$(FPLL)
