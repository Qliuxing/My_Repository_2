#
# Copyright (C) 2005-2015 Melexis N.V.
#
# Software Platform
#

PLTF_DIR := ../../..
PRODUCT ?= 81300
PROFILE ?= 81300-lin13-9600-loader

# Application name
TARGET := $(PRODUCT)_$(notdir $(CURDIR))

include $(PLTF_DIR)/config/Config.mk

#
# SOURCE FILES LIST
#
SRCS = main.c lin2b_romtbl.S traps.c



OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS  = $(OBJS:%.o=%.d)


LDFLAGS := -N -L $(LIBDIR) -l$(PROFILE) -T $(PROFILE).ld $(INSTSET) -Wl,-Map,$(TARGET).map
LDFLAGS += -Wl,--discard-none

# Main rules
.PHONY: all
all: $(TARGET).elf $(TARGET).hex $(TARGET).lss

.PHONY: clean
clean:
	-$(RM) $(OBJDIR)
	-$(RM) $(TARGET).map $(TARGET).elf $(TARGET).hex $(TARGET).lss
	-$(RM) *.MDS *.COD

$(TARGET).elf : $(OBJS) $(LIBDIR)/lib$(PROFILE).a
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# build the platform library - if not yet build
$(LIBDIR)/lib$(PROFILE).a:
	'$(MAKE)' libs

# Common rules
include $(PLTF_DIR)/config/Rules.mk

# Override default rule for hex
%.hex: %.elf
	$(OCP) -O ihex --gap-fill=0xAA --pad-to=0xC000 $< $@

# Include automatic dependencies
ifneq (${MAKECMDGOALS}, $(filter ${MAKECMDGOALS}, clean clean_libs libs))
-include $(DEPS)
endif
