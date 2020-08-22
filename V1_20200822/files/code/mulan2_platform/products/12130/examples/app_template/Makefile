#
# Copyright (C) 2011-2012 Melexis N.V.
#
# Software Platform
#
#

# Target file name (without extension)
TARGET = app

PRODUCT = 12130
PROFILE = 12130

include ../../../../config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c adc_isr.c vectors.S 

TASK1_SRC = task1.c
TASK2_SRC = task2.c


# Task libraries
# \notes:
# 1. Names shall not be changed (linker script dependency)
TASK1_LIB = task1
TASK2_LIB = task2


OBJS  = $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
OBJS += $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))

TASK1_OBJS  = $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(TASK1_SRC)))
TASK1_OBJS += $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(TASK1_SRC)))

TASK2_OBJS  = $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(TASK2_SRC)))
TASK2_OBJS += $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(TASK2_SRC)))

DEPS = $(OBJS:%.o=%.d) $(TASK1_OBJS:%.o=%.d) $(TASK2_OBJS:%.o=%.d)

LDFLAGS := $(INSTSET) -N -L $(LIBDIR) -L $(OBJDIR) -l$(PROFILE)
LDFLAGS += -T $(PROFILE).ld
LDFLAGS += -Wl,-Map,$(TARGET).map,--cref
LDFLAGS += -Wl,--discard-none

# Main rules
all: $(TARGET).elf $(TARGET).hex $(TARGET).lss $(TARGET).sym

# Create task libraries from object files
$(OBJDIR)/$(TASK1_LIB).a: $(TASK1_OBJS)
	$(AR) rcs $@ $(TASK1_OBJS)

$(OBJDIR)/$(TASK2_LIB).a: $(TASK2_OBJS)
	$(AR) rcs $@ $(TASK2_OBJS)

# Link : Create ELF file from object files
$(TARGET).elf : $(OBJS) $(OBJDIR)/$(TASK1_LIB).a $(OBJDIR)/$(TASK2_LIB).a
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clean:
	-$(RM) $(OBJDIR) $(TARGET).elf $(TARGET).hex
	-$(RM) $(TARGET).map $(TARGET).lss $(TARGET).sym

.PHONY: all clean


include $(ROOTDIR)/config/Rules.mk

# Include automatic dependencies

ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif
