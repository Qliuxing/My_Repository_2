#
# Copyright (C) 2005-2014 Melexis N.V.
#
# Math library
#

# Common definitions

include ../../config/Config.mk




LIB_SIMULATION := 0

SUBMODULE = dsp


ifeq ($(LIB_SIMULATION),1)
SRCS := \
	gcc_math.c \
	crc16.c crc8.c crc_ccitt.c \
	lfsr16.c lfsr32.c \
	parity4.c parity8.c parity16.c parity32.c \
	ilog.c ilog32.c iexp.c iexp32.c \
	isqrt.c isqrt32.c gcc_isqrt.c \
	bitrev4.c bitrev_t16.c \
	bitrev8.c \
	bitrev16.c \
	interleave4.c \
	interleave8.c \
	interleave16.c \
	gcc_norm.c \
	sincos.c gcc_sincos.c \
	tan.c gcc_tan.c \
	atan.c
else
SRCS := \
	div32_32by16.S \
	div16_32by16.S \
	div16_16by16.S \
	div8hi_8by8.S \
	div8_8by8.S \
	mul32_32by16.S \
	mul32_16by16.S \
	mul32_16by16_copr.S \
	mulI32_I16byI16_copr.S \
	mul32hi_32by16.S \
	mul24_16by8.S \
	mul16hi_16by8.S \
	mul16_16by16.S \
	mul16_16by16_copr.S \
	mulI16_I16byI16_copr.S \
	mulq15_q15byq15.S \
	mul16_8by8.S \
	ilog.c ilog32.c iexp.c iexp32.c \
	isqrt.c isqrt32.c isqrt_helper.S \
	rand32.S \
	crc16.c crc8.c crc_ccitt.c \
	lfsr16.c lfsr32.c \
	parity4.c parity8.c parity16.c parity32.c \
	bitrev4.c bitrev_t16.c \
	bitrev8.c \
	bitrev16.c \
	interleave4.c \
	interleave8.c \
	interleave16.c \
	sincos.c sincos_helper.S \
	tan.c tan_helper.S \
	atan.c
endif

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS  = $(OBJS:.o=.d)

# --- Main rules --------------------------------------------------------------

all: $(LIBDIR)/lib$(PROFILE_NAME).a $(SUBMODULE)

$(LIBDIR)/lib$(PROFILE_NAME).a: $(OBJS)
	$(AR) rcs $(LIBDIR)/lib$(PROFILE_NAME).a $(OBJS)

clean: $(SUBMODULE)
	-$(RM) $(OBJDIR)

$(SUBMODULE):
	$(MAKE) -C $@ LIB_SIMULATION=$(LIB_SIMULATION) $(MAKECMDGOALS)

install: all

uninstall:
# Uninstall will be done on upper layer


.PHONY: all clean install uninstall $(SUBMODULE)

# Default rules

include ../../config/Rules.mk

ifneq (${MAKECMDGOALS},uninstall)
ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif
endif
