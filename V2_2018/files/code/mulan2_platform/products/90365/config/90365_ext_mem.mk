#
# Copyright (C) 2010 Melexis N.V.
#
# Software Platform Configuration File
#

# EEPROM ECC usage : ECC is used (1) or not used (0)
EEPROM_USE_ECC := 1

# Link to extended ROM memory (_ext_mem)
LD_SCRIPT := 90365_ext_mem.ld
CPPFLAGS += -DEXT_MEM_DEBUG

