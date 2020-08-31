#
# Copyright (C) 2005-2014 Melexis N.V.
#
# Software Platform
#

# Default target image name
TARGET ?= lin2b

# Source files
ifeq ($(COMPILE_STANDALONE_LOADER), 1)
SRCS = fast2b.asm
DEPS = lincst.asm
else
SRCS = lin2b.asm
DEPS = lincst.asm
endif

# Object files
OBJS = $(patsubst %.asm, %.bin, $(SRCS))


# Address of LIN Configuration table
#
# N.B. !!!
#     1. LIN_CONFIG_TABLE_ADDR must be aligned on 64-byte boundary
#     2. If you need to change LIN_CONFIG_TABLE_ADDR, please adjust "mlx4_table"
#        memory region accordingly in product's linker script
#
# LIN_CONFIG_TABLE_ADDR = 5440 # 0x1540 = 0x5540 - 0x4000 for MULAN2
#
LIN_CONFIG_TABLE_ADDR = 5440

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
	$(CP) $(TARGET).bin ../LIN

%.bin: $(SRCS)
	$(MLX4_AS) $<,$*.lst, -p $(MLX4_DEFS) -d ROM_TABLE $(LIN_CONFIG_TABLE_ADDR)
	$(MLX4_LD) $(<:.asm=.obj), $*.cod, -u
	$(MLX4_OBSEND) $*.cod,f,$@,
#	$(MLX4_OBSEND) $*.cod,f,$*.hex,intel
	-$(RM) $*.cod $*.grp $*.map $(<:.asm=.obj)

# macro clean : remove obj executable, obj files...
clean:
	-$(RM) $(TARGET).lst $(TARGET).bin
	-$(RM) $(TARGET).hex

uninstall:

.PHONY: all clean install uninstall
