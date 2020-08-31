#
# Copyright (C) 2011 Melexis N.V.
#
# Software Platform
#

PROJ_NAME := RAM_application_naked

include ../../../config/Config.mk

#
# SOURCE FILES LIST
#

SRCS = main.c vectors.S

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:%.o=%.d)

LDFLAGS := -N -L $(LIBDIR) -l$(PROFILE) -T $(PROFILE).ld $(INSTSET) -Wl,-Map,$(PROJ_NAME).map

# Main rules
all: $(PROJ_NAME).elf $(PROJ_NAME).hex $(PROJ_NAME).bin $(PROJ_NAME).lss

$(PROJ_NAME).elf : $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clean:
	-$(RM) $(OBJDIR) $(PROJ_NAME).map $(PROJ_NAME).elf $(PROJ_NAME).hex $(PROJ_NAME).bin $(PROJ_NAME).lss *.MDS *.COD

.PHONY: all clean

include $(ROOTDIR)/config/Rules.mk

# Include automatic dependencies

ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif
