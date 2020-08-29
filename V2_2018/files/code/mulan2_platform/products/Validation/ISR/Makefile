#
# Copyright (C) 2011-2013 Melexis N.V.
#
# Software Platform
#

PLTF_DIR := ../../..
PRODUCT ?= 81300
PROFILE ?= 81300-nolin

# Target file name (without extension)
TARGET = $(PRODUCT)_$(notdir $(CURDIR))

include $(PLTF_DIR)/config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c vectors.S

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:%.o=%.d)

LDFLAGS := -N $(INSTSET) -L $(LIBDIR) -l$(PROFILE)
LDFLAGS += -T $(PROFILE).ld
LDFLAGS += -Wl,-Map,$(TARGET).map

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