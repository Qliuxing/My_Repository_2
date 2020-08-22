#
# Copyright (C) 2015 Melexis N.V.
#
# Makefile sources
#

#
# SOURCE FILES LIST
#
SRCS  = main.c app_version.c
SRCS += LIN_Communication.c LIN_2x.c LIN_SAE_J2602.c LIN_Diagnostics.c
SRCS += ADC.c Diagnostic.c ErrorCodes.c MotorDriver.c MotorDriverTables.c MotorStall.c 
SRCS += NVRAM_UserPage.c PID_Control.c Timer.c 
SRCS += SPI_Debug.c

# Platform/src overruled sources:
SRCS += fatal.c premain.c
SRCS += muldiv16_16by16by16.S

#ifne ($(PROFILE), 81300-nolin)
    SRCS += lin2b_romtbl.S vectors-lin.S
#else
    #SRCS += vectors-nolin.S
#endif
