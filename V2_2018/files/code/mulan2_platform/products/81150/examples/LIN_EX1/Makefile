#
# Copyright (C) 2011-2013 Melexis N.V.
#
# Software Platform
#

# Target file name (without extension)

TARGET = example
TARGET_TEST = example_hi_flash

PRODUCT=81150
PROFILE=81150-lin20-9600-loader

PLTF_DIR = ../../../..

include $(PLTF_DIR)/config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c mlu.c lin2b_romtbl.S vectors.S traps.c

OBJS =  $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))

DEPS = $(OBJS:%.o=%.d)

LDFLAGS_COMMON := -N -L $(LIBDIR) -l$(PROFILE) $(INSTSET)
LDFLAGS_COMMON += -Wl,--discard-none

LDFLAGS := $(LDFLAGS_COMMON) -T $(PROFILE).ld -Wl,-Map,$(TARGET).map

# Main rules
all: $(TARGET).elf $(TARGET).hex $(TARGET).lss

$(TARGET).elf : $(OBJS) 
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clean:
	-$(RM) $(OBJDIR) *.MDS *.cod
	-$(RM) $(TARGET).map $(TARGET).elf $(TARGET).hex $(TARGET).lss

.PHONY: all clean

include $(PLTF_DIR)/config/Rules.mk

# Override default rule for hex
%.hex: %.elf
	$(OCP) -O ihex --gap-fill=0xAA --pad-to=0xC000 $< $@

# Include automatic dependencies

ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif
