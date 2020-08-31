#
# Copyright (C) 2008-2015 Melexis N.V.
#
# Software Platform Configuration File
#

# PLL Multiplier value (1..128)
# Fpll = Fosc * multiplier = 250 kHz * 80 = 10 MHz
#MCU_PLL_MULT := 40
# Fpll = Fosc * multiplier = 250 kHz * 80 = 12 MHz
#MCU_PLL_MULT := 48
# Fpll = Fosc * multiplier = 250 kHz * 80 = 20 MHz
#MCU_PLL_MULT := 80
# Fpll = Fosc * multiplier = 250 kHz * 100 = 25 MHz
#MCU_PLL_MULT := 100
# Fpll = Fosc * multiplier = 250 kHz * 112 = 28 MHz
MCU_PLL_MULT := 112
# Fpll = Fosc * multiplier = 250 kHz * 120 = 30 MHz
#MCU_PLL_MULT := 120
# Fpll = Fosc * multiplier = 250 kHz * 128 = 32 MHz
#MCU_PLL_MULT := 128

# Application type: NOLIN_APP, LIN13_APP or LIN20_APP
APP_TYPE := LIN20_APP

# Default LIN baudrate
ML_BAUDRATE := 19200

# Loader usage: loader is used (1) or not used (0) 
HAS_LOADER := 1

# Fast loader baudrate (125000, 100000, 75000 or 50000)
ML_FAST_BAUDRATE := 100000
