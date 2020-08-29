#
# Copyright (C) 2008-2011 Melexis N.V.
#
# Software Platform Configuration File
#

# PLL Multiplier value (1..80)
# Fpll = Fosc * multiplier = 250 kHz * 80 = 20 MHz
MCU_PLL_MULT := 80

# Application type: NOLIN_APP, LIN13_APP or LIN20_APP
APP_TYPE := NOLIN_APP

# Loader usage: loader is used (1) or not used (0) 
HAS_LOADER := 0
