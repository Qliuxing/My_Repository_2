#
# Copyright (C) 2010 Melexis N.V.
#
# Software Platform
#

# Target file name (without extension)
TARGET = app
PRODUCT = 90294
PROFILE = 90294

include ../../../../config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c low_level_init.c vectors.S

# local vectors.S 
#SRCS += vectors.S

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:%.o=%.d)

LDFLAGS := -N -L $(LIBDIR) -l$(PROFILE) -T $(PROFILE).ld  $(INSTSET) -Wl,-Map,$(TARGET).map,--cref
LDFLAGS += -Wl,--discard-none

# Main rules
all: $(TARGET).elf $(TARGET).hex $(TARGET).lss $(TARGET).sym

# Link : Create ELF file from object files
$(TARGET).elf : $(OBJS) 
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clean:
	-$(RM) $(OBJDIR) $(TARGET).map $(TARGET).elf $(TARGET).hex $(TARGET).lss $(TARGET).sym

.PHONY: all clean

include $(ROOTDIR)/config/Rules.mk

# Include automatic dependencies

ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif
