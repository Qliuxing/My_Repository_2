#
# Copyright (C) 2008-2009 Melexis N.V.
#
# Software Platform
#

# Target file name (without extension)
TARGET = LDF_Nodegenerator_Template_LIN2_1
PRODUCT=81108_9
PROFILE=81108_9C-lin20-19200-loader
PLTF_DIR = ../../../..

include $(PLTF_DIR)/config/Config.mk


#
# SOURCE FILES LIST
#

# Special LIN definitions

LDF_FILE = LDF/Melexis_Example_LIN.LDF
LIN_NODE = MLX_Node

LIN_HDRS = lin_api.h lin_cfg.h lin_core.h lin_diag.h lin_signals.h
LIN_SRCS = lin_cfg.c lin_core.c lin_diag.c lin_signals.c mlu30.c lin2b_romtbl.S 


SRCS = main.c adc.c interrupts.c $(LIN_SRCS) 



OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:%.o=%.d)

LDFLAGS := -N -L $(LIBDIR) -l$(PROFILE) -T $(PROFILE).ld $(INSTSET) -Wl,-Map,$(TARGET).map
CFLAGS  += -O0  # disable code size optimization


# Main rules
all: $(TARGET).elf $(TARGET).hex $(TARGET).lss
#	$(SIZE) -Ax $(TARGET).elf
	$(SIZE) -B $(TARGET).elf
	@$(ECHO) --- Project : $(TARGET) successfully created

# Generate driver from LDF file
drv:
	$(LDF_NODEGEN) -f -pt -ps -pf -Mi:ID_FILTERING_MODE -n$(LIN_NODE) -v $(LDF_FILE)

# Link : Create ELF file from object files
$(TARGET).elf : $(OBJS) $(LIBDIR)/lib$(PROFILE).a 
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clean:
	-$(RM) $(OBJDIR) $(TARGET).map $(TARGET).elf $(TARGET).hex $(TARGET).lss *.MDS
	@$(ECHO) --- Clean project $(TARGET) finished

clean_drv: 
	-$(RM) $(OBJDIR) $(LIN_SRCS) $(LIN_HDRS)
	@$(ECHO) --- Clean LIN driver $(TARGET) finished


.PHONY: all clean clean_drv drv
.SILENT:

include $(ROOTDIR)/config/Rules.mk

# Include automatic dependencies

ifneq (${MAKECMDGOALS},clean)
ifneq (${MAKECMDGOALS},drv)
-include $(DEPS)
endif
endif

