#
# Copyright (C) 2011-2013 Melexis N.V.
#
# Software Platform
#

# Target file name (without extension)
TARGET = example
PRODUCT=81108_9
PROFILE=81108_9C-nolin

PLTF_DIR = ../../../..

include $(PLTF_DIR)/config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c vectors.S

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:%.o=%.d)

LDFLAGS := -N -L $(LIBDIR) -l$(PROFILE) -T $(PROFILE).ld $(INSTSET) -Wl,-Map,$(TARGET).map
LDFLAGS += -Wl,--discard-none

# Main rules
all: $(TARGET).elf $(TARGET).hex $(TARGET).lss

$(TARGET).elf : $(OBJS) 
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clean:
	-$(RM) $(OBJDIR) *.MDS *.cod
	-$(RM) $(TARGET).map $(TARGET).elf $(TARGET).hex $(TARGET).lss

.PHONY: all clean

include $(PLTF_DIR)/config/Rules.mk

# Include automatic dependencies

ifneq (${MAKECMDGOALS}, $(filter ${MAKECMDGOALS}, clean clean_libs libs))
-include $(DEPS)
endif
