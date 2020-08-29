#
# Copyright (C) 2011-2015 Melexis N.V.
#
# Software Platform
#

PRODUCT=81315
PROFILE=81315-standalone-loader

PLTF_DIR = ../../../..

# Application name
TARGET := $(PRODUCT)_$(notdir $(CURDIR))
TARGET_TEST := $(TARGET)_hi_flash

include $(PLTF_DIR)/config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:%.o=%.d)

LDFLAGS_COMMON := -N $(INSTSET) -L $(LIBDIR) -l$(PROFILE)
LDFLAGS_COMMON += -Wl,--discard-none

LDFLAGS_APP  := $(LDFLAGS_COMMON) -T $(PROFILE).ld -Wl,-Map,$(TARGET).map
LDFLAGS_TEST := $(LDFLAGS_COMMON) -T $(PRODUCT)-fast-hi-flash-test.ld -Wl,-Map,$(TARGET_TEST).map

# Rules
.PHONY: all
all: app test

.PHONY: app test
app: $(TARGET).elf $(TARGET).hex $(TARGET).lss
test: $(TARGET_TEST).elf $(TARGET_TEST).hex $(TARGET_TEST).lss

$(TARGET).elf : $(OBJS) $(LIBDIR)/lib$(PROFILE).a
	$(CC) $(OBJS) -o $@ $(LDFLAGS_APP)

$(TARGET_TEST).elf : $(OBJS) $(LIBDIR)/lib$(PROFILE).a
	$(CC) $(OBJS) -o $@ $(LDFLAGS_TEST)

# build the platform library - if not yet build
$(LIBDIR)/lib$(PROFILE).a:
	'$(MAKE)' libs

.PHONY: clean
clean:
	-$(RM) $(OBJDIR) *.MDS *.cod
	-$(RM) $(TARGET).map $(TARGET).elf $(TARGET).hex $(TARGET).lss
	-$(RM) $(TARGET_TEST).map $(TARGET_TEST).elf $(TARGET_TEST).hex $(TARGET_TEST).lss

# Default rules
include $(PLTF_DIR)/config/Rules.mk

# Override default rule for hex
%.hex: %.elf
	$(OCP) -O ihex --gap-fill=0xAA --pad-to=0xC000 $< $@

# Include automatic dependencies
ifneq (${MAKECMDGOALS}, $(filter ${MAKECMDGOALS}, clean clean_libs libs))
-include $(DEPS)
endif
