#
# Copyright (C) 2013 Melexis N.V.
#
# Software Platform
#

# Target file name (without extension)
TARGET = app
PRODUCT = 90330
PROFILE = 90330

include ../../../../config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c vectors.S prestart.S

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:%.o=%.d)

LDFLAGS := -N -L $(LIBDIR) -l$(PROFILE) -T $(PROFILE).ld $(INSTSET) -Wl,-Map,$(TARGET).map

# Main rules
all: $(TARGET).elf $(TARGET).hex $(TARGET).lss

# Link : Create ELF file from object files
$(TARGET).elf : $(OBJS) 
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clean:
	-$(RM) $(OBJDIR) $(TARGET).map $(TARGET).elf $(TARGET).hex $(TARGET).lss

.PHONY: all clean

include $(ROOTDIR)/config/Rules.mk

# Include automatic dependencies

ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif
