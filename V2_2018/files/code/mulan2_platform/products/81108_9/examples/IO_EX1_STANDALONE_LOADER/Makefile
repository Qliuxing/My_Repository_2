#
# Copyright (C) 2011-2013 Melexis N.V.
#
# Software Platform
#

# Target file name (without extension)
TARGET = example
PRODUCT=81108_9
PROFILE=81108_9C-standalone-loader

PLTF_DIR = ../../../..

include $(PLTF_DIR)/config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c vectors.S

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:%.o=%.d)

LDFLAGS_COMMON := -N -L $(LIBDIR) -l$(PROFILE) $(INSTSET)
LDFLAGS_COMMON += -Wl,--discard-none

LDFLAGS_APP  := $(LDFLAGS_COMMON) -T $(PROFILE).ld -Wl,-Map,$(TARGET).map

# Main rules
all: $(TARGET).elf $(TARGET).hex $(TARGET).lss

$(TARGET).elf : $(OBJS) 
	$(CC) $(OBJS) -o $@ $(LDFLAGS_APP)


clean:
	-$(RM) $(OBJDIR) *.MDS *.cod
	-$(RM) $(TARGET).map $(TARGET).elf $(TARGET).hex $(TARGET).lss

.PHONY: all clean

include $(PLTF_DIR)/config/Rules.mk

# Override default rule for hex
%.hex: %.elf
	$(OCP) -O ihex --gap-fill=0xAA --pad-to=0xC000 $< $@

# Include automatic dependencies

ifneq (${MAKECMDGOALS}, $(filter ${MAKECMDGOALS}, clean clean_libs libs))
-include $(DEPS)
endif
