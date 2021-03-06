#
# Copyright (C) 2008-2013 Melexis N.V.
#
# Software Platform Configuration File
#

# Application type: NOLIN_APP, LIN13_APP or LIN20_APP
APP_TYPE := LIN20_APP

# Default LIN baudrate
ML_BAUDRATE := 9600

# Loader usage: loader is used (1) or not used (0) 
HAS_LOADER := 1

# Fast loader baudrate (50000 or 25000)
ML_FAST_BAUDRATE := 25000

# High CPU clock is defined by design and SHALL NOT be changed
MCU_PLL_FREQ_KHZ := 24000ul

# Fpll = Fosc * multiplier = 250 kHz * 96 = 24 MHz
MCU_PLL_MULT := 96

# RAM Size of the device
RAM_SIZE := 1024

# target device identification
CPPFLAGS += -D__MLX81106_7C__

