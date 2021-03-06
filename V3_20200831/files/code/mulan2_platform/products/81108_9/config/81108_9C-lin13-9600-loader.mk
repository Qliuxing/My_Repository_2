#
# Copyright (C) 2008-2015 Melexis N.V.
#
# Software Platform Configuration File
#

# Application type: NOLIN_APP, LIN13_APP or LIN20_APP
APP_TYPE := LIN13_APP

# Default LIN baudrate
ML_BAUDRATE := 9600

# Loader usage: loader is used (1) or not used (0) 
HAS_LOADER := 1

# Fast loader baudrate (75000, 50000 or 25000)
ML_FAST_BAUDRATE := 75000

# High CPU clock is defined by design and SHALL NOT be changed
MCU_PLL_FREQ_KHZ := 24000ul

# Fpll = Fosc * multiplier = 250 kHz * 96 = 24 MHz
MCU_PLL_MULT := 96

# RAM Size of the device
RAM_SIZE := 1024

# target device identification
CPPFLAGS += -D__MLX81108_9C__
