#!/bin/sh
#

# Build for Standalone loader
export TARGET='fast2b'
export COMPILE_STANDALONE_LOADER='1'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 1'\
' -d coFastStdAlne cvON'\
' -d coFASTFLASH cvON'
gmake clean
gmake install

# --- 12 MHz images --------------------------

# LIN2.0 @ all baudrates
export TARGET='lin2b_12mhz'
export COMPILE_STANDALONE_LOADER='0'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_12MHZ'\
' -d coTIMEOUT cvTO2x'\
' -d CFG_SLEEP_TO coLIN20_ANY'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 9600 bps
export TARGET='lin2b_v13_9600_12mhz'
export COMPILE_STANDALONE_LOADER='0'
export  MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_12MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_9600'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 19200 bps
export TARGET='lin2b_v13_19200_12mhz'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_12MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_19200'\
' -d coFASTFLASH cvON'
gmake clean
gmake install

# --- 18 MHz images --------------------------

# LIN2.0 @ all baudrates
export TARGET='lin2b_18mhz'
export COMPILE_STANDALONE_LOADER='0'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_18MHZ'\
' -d coTIMEOUT cvTO2x'\
' -d CFG_SLEEP_TO coLIN20_ANY'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 9600 bps
export TARGET='lin2b_v13_9600_18mhz'
export COMPILE_STANDALONE_LOADER='0'
export  MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_18MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_9600'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 19200 bps
export TARGET='lin2b_v13_19200_18mhz'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_18MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_19200'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# --- 20 MHz images --------------------------

# LIN2.0 @ all baudrates
export TARGET='lin2b_20mhz'
export COMPILE_STANDALONE_LOADER='0'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_20MHZ'\
' -d coTIMEOUT cvTO2x'\
' -d CFG_SLEEP_TO coLIN20_ANY'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 9600 bps
export TARGET='lin2b_v13_9600_20mhz'
export COMPILE_STANDALONE_LOADER='0'
export  MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_20MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_9600'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 19200 bps
export TARGET='lin2b_v13_19200_20mhz'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_20MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_19200'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# --- 24 MHz images --------------------------

# LIN2.0 @ all baudrates
export TARGET='lin2b_24mhz'
export COMPILE_STANDALONE_LOADER='0'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_24MHZ'\
' -d coTIMEOUT cvTO2x'\
' -d CFG_SLEEP_TO coLIN20_ANY'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 9600 bps
export TARGET='lin2b_v13_9600_24mhz'
export COMPILE_STANDALONE_LOADER='0'
export  MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_24MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_9600'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 19200 bps
export TARGET='lin2b_v13_19200_24mhz'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_24MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_19200'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# --- 25 MHz images --------------------------

# LIN2.0 @ all baudrates
export TARGET='lin2b_25mhz'
export COMPILE_STANDALONE_LOADER='0'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_25MHZ'\
' -d coTIMEOUT cvTO2x'\
' -d CFG_SLEEP_TO coLIN20_ANY'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 9600 bps
export TARGET='lin2b_v13_9600_25mhz'
export COMPILE_STANDALONE_LOADER='0'
export  MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_25MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_9600'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 19200 bps
export TARGET='lin2b_v13_19200_25mhz'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_25MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_19200'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# --- 28 MHz images --------------------------

# LIN2.0 @ all baudrates
export TARGET='lin2b_28mhz'
export COMPILE_STANDALONE_LOADER='0'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_28MHZ'\
' -d coTIMEOUT cvTO2x'\
' -d CFG_SLEEP_TO coLIN20_ANY'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 9600 bps
export TARGET='lin2b_v13_9600_28mhz'
export COMPILE_STANDALONE_LOADER='0'
export  MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_28MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_9600'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 19200 bps
export TARGET='lin2b_v13_19200_28mhz'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_28MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_19200'\
' -d coFASTFLASH cvON'
gmake clean
gmake install

# --- 30 MHz images --------------------------

# LIN2.0 @ all baudrates
export TARGET='lin2b_30mhz'
export COMPILE_STANDALONE_LOADER='0'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_30MHZ'\
' -d coTIMEOUT cvTO2x'\
' -d CFG_SLEEP_TO coLIN20_ANY'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 9600 bps
export TARGET='lin2b_v13_9600_30mhz'
export COMPILE_STANDALONE_LOADER='0'
export  MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_30MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_9600'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 19200 bps
export TARGET='lin2b_v13_19200_30mhz'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_30MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_19200'\
' -d coFASTFLASH cvON'
gmake clean
gmake install

# --- 32 MHz images --------------------------

# LIN2.0 @ all baudrates
export TARGET='lin2b_32mhz'
export COMPILE_STANDALONE_LOADER='0'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_32MHZ'\
' -d coTIMEOUT cvTO2x'\
' -d CFG_SLEEP_TO coLIN20_ANY'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 9600 bps
export TARGET='lin2b_v13_9600_32mhz'
export COMPILE_STANDALONE_LOADER='0'
export  MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_32MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_9600'\
' -d coFASTFLASH cvON'
gmake clean
gmake install


# LIN1.3 @ 19200 bps
export TARGET='lin2b_v13_19200_32mhz'
export MLX4_DEFS=\
' -d COMPILE_STANDALONE_LOADER 0'\
' -d coFastStdAlne cvOFF'\
' -d CFG_PLL_FREQ coPLL_32MHZ'\
' -d coTIMEOUT cvTO13'\
' -d CFG_SLEEP_TO coLIN13_19200'\
' -d coFASTFLASH cvON'
gmake clean
gmake install

# EOF
