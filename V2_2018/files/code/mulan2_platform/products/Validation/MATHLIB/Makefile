#
# Copyright (C) 2005-2015 Melexis N.V.
#
# Software Platform
#

PLTF_DIR := ../../..
PRODUCT ?= 81300
PROFILE ?= 81300-nolin


# Target file name (without extension)
TARGET  := $(PRODUCT)_$(notdir $(CURDIR))_basic_test
TARGET2 := $(PRODUCT)_$(notdir $(CURDIR))_trig_test
TARGET3 := $(PRODUCT)_$(notdir $(CURDIR))_dsp_test

include $(PLTF_DIR)/config/Config.mk

# This flag could be used to turn off INLINE ASM functions realization (INLINE ASM is used by default)
#CPPFLAGS += -D__MATHLIB_NON_INLINE__

#
# SOURCE FILES LIST
#
SRCS = main.c mul_test.c div_test.c rand_test.c lfsr_test.c parity_test.c bitrev_test.c interleave_test.c isqrt_test.c ilog_test.c
SRCS += vectors.S
#SRCS += stub.c

SRCS2 = trig_main.c sincos_test.c tan_test.c atan_test.c
SRCS2 += vectors.S
#SRCS2 += stub.c

SRCS3 = dsp_main.c dsp_test.c dsp2_test.c
SRCS3 += vectors.S traps.c
#SRCS3 += stub.c

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS  = $(OBJS:%.o=%.d)

OBJS2  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS2)))
OBJS2 += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS2)))
DEPS2  = $(OBJS2:%.o=%.d)

OBJS3  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS3)))
OBJS3 += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS3)))
DEPS3  = $(OBJS3:%.o=%.d)

LDFLAGS = -N -L $(LIBDIR) -l$(PROFILE) -T $(PROFILE).ld $(INSTSET) -Wl,-N,-Map,$(basename $@).map

# Main rules
.PHONY: all
all: target target2 target3

.PHONY: target target2 target3
target  : $(TARGET).elf  $(TARGET).hex  $(TARGET).lss
target2 : $(TARGET2).elf $(TARGET2).hex $(TARGET2).lss
target3 : $(TARGET3).elf $(TARGET3).hex $(TARGET3).lss

.PHONY: clean
clean:
	-$(RM) $(OBJDIR)
	-$(RM) $(TARGET).map $(TARGET).elf $(TARGET).hex $(TARGET).lss
	-$(RM) $(TARGET2).map $(TARGET2).elf $(TARGET2).hex $(TARGET2).lss
	-$(RM) $(TARGET3).map $(TARGET3).elf $(TARGET3).hex $(TARGET3).lss

$(TARGET).elf : $(OBJS) $(LIBDIR)/lib$(PROFILE).a
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
$(TARGET2).elf : $(OBJS2) $(LIBDIR)/lib$(PROFILE).a
	$(CC) $(OBJS2) -o $@ $(LDFLAGS)
$(TARGET3).elf : $(OBJS3) $(LIBDIR)/lib$(PROFILE).a
	$(CC) $(OBJS3) -o $@ $(LDFLAGS)



# build the platform library - if not yet build
$(LIBDIR)/lib$(PROFILE).a:
	'$(MAKE)' libs

# Common rules
include $(PLTF_DIR)/config/Rules.mk

# Include automatic dependencies
ifneq (${MAKECMDGOALS}, $(filter ${MAKECMDGOALS}, clean clean_libs libs))
-include $(DEPS)
-include $(DEPS2)
-include $(DEPS3)
endif
