#
# Copyright (C) 2011-2013 Melexis N.V.
#
# Software Platform
#

# Target file name (without extension)
# Notes:
#   1. The name loaderB is used for historical reason, since tools might
#      expect the file with such name by default.
TARGET = loaderB-$(PROFILE)

include ../../config/Config.mk

ifndef LD_SCRIPT_LOADER
$(error LD_SCRIPT_LOADER is not defined)
endif	

ifneq ($(HAS_LOADER), 1)
$(error LIN-pin loader image is NOT required for this configuration)
endif

SRCS = loader.c mlu30.c vectors.S loader_state.c
# Source files and flags
ifeq ($(STANDALONE_LOADER),1)
else
    SRCS += lin2b_romtbl.S 
endif

LDFLAGS := -N -L $(LIBDIR) -L $(PRODUCT_LIBDIR) -l$(PROFILE) -T $(LD_SCRIPT_LOADER) $(INSTSET) -Wl,-Map,$(TARGET).map

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS  = $(OBJS:.o=.d)

# --- Main rules --------------------------------------------------------------
all: $(TARGET).elf $(TARGET).hex $(TARGET).lss

$(TARGET).elf : $(OBJS) 
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clean:
	-$(RM) $(OBJDIR)
	-$(RM) $(TARGET).elf $(TARGET).map $(TARGET).lss $(TARGET).hex

install: all

uninstall:
# Uninstall will be done on upper layer


.PHONY: all clean install uninstall

# Default rules

include ../../config/Rules.mk

ifneq (${MAKECMDGOALS},uninstall)
ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif
endif
