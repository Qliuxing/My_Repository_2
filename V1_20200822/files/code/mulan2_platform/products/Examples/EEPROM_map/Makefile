#
# Copyright (C) 2012 Melexis N.V.
#
# Software Platform
#

# Target file name (without extension)
TARGET = app

include ../../../config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c eeprom.c # vectors.S

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:%.o=%.d)

LDFLAGS := -N -L $(LIBDIR) -l$(PROFILE) -T $(PROFILE).ld $(INSTSET) -Wl,-Map,$(TARGET).map,--cref

# Main rules
all: $(TARGET).elf $(TARGET).hex $(TARGET).eep $(TARGET).lss $(TARGET).sym

# Link : Create ELF file from object files
$(TARGET).elf : $(OBJS) 
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clean:
	-$(RM) $(OBJDIR) $(TARGET).map $(TARGET).elf $(TARGET).hex $(TARGET).eep 
	-$(RM) $(TARGET).lss $(TARGET).sym

.PHONY: all clean

include $(ROOTDIR)/config/Rules.mk

# Create EEPROM hex file
%.eep: %.elf
	-$(OCP) -j .ep.data -j .ep \
        --set-section-flags=.ep="alloc,load" --no-change-warnings -O ihex $< $@ || exit 0

# Include automatic dependencies

ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif
