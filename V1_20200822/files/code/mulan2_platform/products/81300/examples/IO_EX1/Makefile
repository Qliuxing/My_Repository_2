#
# Copyright (C) 2011-2015 Melexis N.V.
#
# Software Platform
#

PRODUCT=81300
PROFILE=81300-nolin

PLTF_DIR = ../../../..

# Application name
TARGET := $(PRODUCT)_$(notdir $(CURDIR))

include $(PLTF_DIR)/config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:%.o=%.d)

LDFLAGS := -N $(INSTSET) -L $(LIBDIR) -l$(PROFILE)
LDFLAGS += -T $(PROFILE).ld
LDFLAGS += -Wl,-Map,$(TARGET).map

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

# Include automatic dependencies
ifneq (${MAKECMDGOALS}, $(filter ${MAKECMDGOALS}, clean clean_libs libs))
-include $(DEPS)
endif
