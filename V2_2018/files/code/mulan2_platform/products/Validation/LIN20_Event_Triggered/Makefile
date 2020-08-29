#
# Copyright (C) 2011-2015 Melexis N.V.
#
# Software Platform
#

PLTF_DIR := ../../..
PRODUCT ?= 81300
PROFILE ?= 81300-lin20-9600-loader

# Application name
TARGET := $(PRODUCT)_$(notdir $(CURDIR))

include $(PLTF_DIR)/config/Config.mk

#
# SOURCE FILES LIST
#
LDF_FILE = ldf/etf.ldf
LDF_NODE_NAME = Node_doorFrontLeft

LIN_HDRS = lin_api.h lin_cfg.h lin_core.h lin_diag.h lin_signals.h
LIN_SRCS = lin_cfg.c lin_core.c lin_diag.c lin_signals.c mlu30.c lin2b_romtbl.S

SRCS = main.c traps.c $(LIN_SRCS)


OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS  = $(OBJS:%.o=%.d)

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

# Generate driver from LDF file
.PHONY: drv
drv:
	$(LDF_NODEGEN) -n$(LDF_NODE_NAME) -bf -v $(LDF_FILE)

.PHONY: clean_drv
clean_drv: 
	-$(RM) $(OBJDIR) $(LIN_SRCS) $(LIN_HDRS)

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
ifneq (${MAKECMDGOALS}, $(filter ${MAKECMDGOALS}, clean clean_libs libs drv clean_drv))
-include $(DEPS)
endif
