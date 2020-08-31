#
# Copyright (C) 2005-2011 Melexis N.V.
#
# Software Platform
#

# Default target image name
TARGET ?= mlx4_loop

# Source files
SRCS = mlx4_loop.asm

# Object files
OBJS = $(patsubst %.asm, %.bin, $(SRCS))


# --- Tools and utilities ---------------------------------------------------------

ifeq ($(OS),Windows_NT)

# MLX4 tools on Windows host
# Notes: MLX4 tools MUST be in environment PATH. The tools are usually
#        located at: C:\Program Files\Melexis\Bin

MLX4_AS     = as
MLX4_LD     = lyn
MLX4_OBSEND = obsend
TRHEX = ../../bin/TrHex

else

MLX4_AS     = MlxAs
MLX4_LD     = MlxLyn
MLX4_OBSEND = MlxObsend
TRHEX = TrHex

endif



# Standard utilities (MUST be in the environment PATH)
ECHO  = echo
RM    = rm -fr
CP    = cp -f
MKDIR = mkdir -p


# Rules
all: $(TARGET).bin

install: all
	$(CP) $(TARGET).bin ../../../products/81300/src

%.bin: $(SRCS)
	$(MLX4_AS) $<,$*.lst, -p -o=$* $(MLX4_DEFS)
	$(MLX4_LD) $*.obj, $*.cod, -u
	$(MLX4_OBSEND) $*.cod,f,$@,
#	$(MLX4_OBSEND) $*.cod,f,$*.hex,intel
	-$(RM) $*.cod $*.grp $*.map $*.obj

# macro clean : remove obj executable, obj files...
clean:
	-$(RM) $(TARGET).lst $(TARGET).bin

uninstall:

.PHONY: all clean install uninstall
