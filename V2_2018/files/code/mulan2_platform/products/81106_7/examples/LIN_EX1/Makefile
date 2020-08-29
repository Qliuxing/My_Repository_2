#
# Copyright (C) 2011-2013 Melexis N.V.
#
# Software Platform
#

# Target file name (without extension)
TARGET = example
TARGET_TEST = example_hi_flash

PRODUCT=81106_7
PROFILE=81106_7C-lin20-19200-loader

PLTF_DIR = ../../../..

include $(PLTF_DIR)/config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c mlu.c lin2b_romtbl.S vectors.S traps.c
SRCS += led.c

OBJS =  $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))

DEPS = $(OBJS:%.o=%.d)

CFLAGS += -DDEBUG_IO

LDFLAGS_COMMON := -N -L $(LIBDIR) -l$(PROFILE) $(INSTSET)
LDFLAGS_COMMON += -Wl,--discard-none

LDFLAGS_APP  := $(LDFLAGS_COMMON) -T $(PROFILE).ld -Wl,-Map,$(TARGET).map
LDFLAGS_TEST := $(LDFLAGS_COMMON) -T 81106_7-hi-flash-test.ld -Wl,-Map,$(TARGET_TEST).map

# Main rules
all: $(TARGET).elf $(TARGET).hex $(TARGET).lss $(TARGET_TEST).elf $(TARGET_TEST).hex $(TARGET_TEST).lss

$(TARGET).elf : $(OBJS) 
	$(CC) $(OBJS) -o $@ $(LDFLAGS_APP)

$(TARGET_TEST).elf : $(OBJS) 
	$(CC) $(OBJS) -o $@ $(LDFLAGS_TEST)

clean:
	-$(RM) $(OBJDIR) *.MDS *.cod
	-$(RM) $(TARGET).map $(TARGET).elf $(TARGET).hex $(TARGET).lss
	-$(RM) $(TARGET_TEST).map $(TARGET_TEST).elf $(TARGET_TEST).hex $(TARGET_TEST).lss

.PHONY: all clean

include $(PLTF_DIR)/config/Rules.mk

# Override default rule for hex
%.hex: %.elf
	$(OCP) -O ihex --gap-fill=0xAA --pad-to=0xC000 $< $@

# Include automatic dependencies

ifneq (${MAKECMDGOALS}, $(filter ${MAKECMDGOALS}, clean clean_libs libs))
-include $(DEPS)
endif
