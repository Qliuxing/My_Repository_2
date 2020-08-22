#
# Copyright (C) 2011-2015 Melexis N.V.
#
# Software Platform
#

# Target file name (without extension)

PLTF_DIR = ../../../..

PRODUCT=81310
PROFILE=81310-lin20-9600-loader

include $(PLTF_DIR)/config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c lin2b_romtbl.S vectors.S traps.c

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:%.o=%.d)

LDFLAGS := -N $(INSTSET) -L $(LIBDIR) -l$(PROFILE)
LDFLAGS += -T $(PROFILE).ld
LDFLAGS += -Wl,-Map,$(TARGET).map
LDFLAGS += -Wl,--discard-none

APP_OPTIONS = $(EXTRA_DEFINES)

# Rules
.PHONY: all
all: $(TARGET).elf $(TARGET).hex $(TARGET).lss

$(TARGET).elf : $(OBJS) $(LIBDIR)/lib$(PROFILE).a
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# build the platform library - if not yet build
$(LIBDIR)/lib$(PROFILE).a:
	'$(MAKE)' libs

.PHONY: clean
clean:
	-$(RM) $(OBJDIR) *.MDS *.cod
	-$(RM) $(TARGET).map $(TARGET).elf $(TARGET).hex $(TARGET).lss

# Default rules
include $(PLTF_DIR)/config/Rules.mk

# Override default rule for hex
%.hex: %.elf
	$(OCP) -O ihex --gap-fill=0xAA --pad-to=0xC000 $< $@

# Include automatic dependencies
ifneq (${MAKECMDGOALS}, $(filter ${MAKECMDGOALS}, clean clean_libs libs))
-include $(DEPS)
endif
