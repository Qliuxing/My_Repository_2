#
# Copyright (C) 2011 Melexis N.V.
#
# Software Platform Configuration File
#

# EEPROM ECC usage : ECC is used (1) or not used (0)
EEPROM_USE_ECC := 1

# Link to RAM
LD_SCRIPT := 90294-ram.ld
CPPFLAGS += -DRAM_APPLICATION
