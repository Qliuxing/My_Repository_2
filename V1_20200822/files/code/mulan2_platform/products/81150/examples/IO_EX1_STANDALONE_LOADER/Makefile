#
# Copyright (C) 2014 Melexis N.V.
#
# Software Platform
#

# Target file name (without extension)

PLTF_DIR = ../../../..

TARGET = example
PRODUCT=81150
PROFILE=81150-standalone-loader

include $(PLTF_DIR)/config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:%.o=%.d)

LDFLAGS := -N -L $(LIBDIR) -l$(PROFILE) -T $(PROFILE).ld $(INSTSET) -Wl,-Map,$(TARGET).map

# Main rules
all: $(TARGET).elf $(TARGET).hex $(TARGET).lss

# Link : Create ELF file from object files
$(TARGET).elf : $(OBJS) 
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clean:
	-$(RM) $(OBJDIR) $(TARGET).map $(TARGET).elf $(TARGET).hex $(TARGET).lss *.MDS *.cod

.PHONY: all clean

include $(PLTF_DIR)/config/Rules.mk

# Override default rule for hex
%.hex: %.elf
	$(OCP) -O ihex --gap-fill=0xAA --pad-to=0xC000 $< $@

# Include automatic dependencies

ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif
