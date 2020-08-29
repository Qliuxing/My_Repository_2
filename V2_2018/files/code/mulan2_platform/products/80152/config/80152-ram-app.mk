#
# Copyright (C) 2010 Melexis N.V.
#
# Software Platform Configuration File
#

# EEPROM ECC usage : ECC is used (1) or not used (0)
EEPROM_USE_ECC := 0

# Select linker script
LD_SCRIPT := 80152-ram-app.ld

# Notify the Platform that we're linking to RAM
CPPFLAGS += -DRAM_APPLICATION
