#
# Copyright (C) 2008-2010 Melexis N.V.
#
# Software Platform
#


#--- Product specific defines -------------------------------------------------
CORE_TYPE := mmc16
INSTSET := -mlx16-8
# FPLL = 12000 + 2000 (worst-case, needed for 1MHz timer/clock)
MCU_PLL_FREQ_KHZ := 14000ul

# Use -mdpage-explicit to have global variables in nodp by default
CFLAGS  += -mdpage-explicit

# Workaround for ITC bug (see MULAN-5 and MMC16-26 on Jira), aready fixed in HW
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
