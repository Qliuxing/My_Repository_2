#
# Copyright (C) 2008-2013 Melexis N.V.
#
# Software Platform Configuration File
#

# PLL Multiplier value (1..80)
# Fpll = Fosc * multiplier = 250 kHz * 80 = 20 MHz
MCU_PLL_MULT := 80

# Application type: NOLIN_APP, LIN13_APP or LIN20_APP
APP_TYPE := LIN20_APP

# Default LIN baudrate
ML_BAUDRATE := 10417

# Loader usage: loader is used (1) or not used (0) 
HAS_LOADER := 1

# Fast loader baudrate (125000, 100000, 75000 or 50000)
ML_FAST_BAUDRATE := 50000
