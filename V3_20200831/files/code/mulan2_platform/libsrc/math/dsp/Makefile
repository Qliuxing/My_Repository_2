#
# Copyright (C) 2005-2013 Melexis N.V.
#
# Math library - dsp module
#

# Common definitions

include ../../../config/Config.mk


ifeq ($(LIB_SIMULATION),1)
SRCS := \
	gcc_dsp.c
else
SRCS := \
	vecsumU8_U8.S \
	vecsumU16_U8.S \
	vecsumU16_U16.S \
	vecsumI8_I8.S \
	vecsumI16_I8.S \
	vecsumI16_I16.S \
	vecsumU32_U16.S \
	vecsumU32_U32.S \
	vecsumU48_U32.S \
	vecsumI32_I16.S \
	vecsumI32_I32.S \
	vecsumI48_I32.S \
	vecmaxU16_U16.S \
	vecmaxU32_U32.S \
	normmaxvectorU16_I16.S \
	normmaxvectorU32_I32.S \
	norm1vectorU32_I16.S \
	norm1vectorU32_I32.S \
	norm1vectorU48_I32.S \
	norm2U32_U16byU16.S \
	norm2U48_U16byU16.S \
	norm2U32_I16byI16.S \
	norm2U48_I16byI16.S \
	norm2vectorU32_U16byU16.S \
	norm2vectorU48_U16byU16.S \
	norm2vectorU32_I16byI16.S \
	norm2vectorU48_I16byI16.S \
	dotproductU32_U16byU16.S \
	dotproductU48_U16byU16.S \
	dotproductI32_I16byI16.S \
	dotproductI48_I16byI16.S
endif

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS  = $(OBJS:.o=.d)

# --- Main rules --------------------------------------------------------------

all: $(LIBDIR)/lib$(PROFILE_NAME).a

$(LIBDIR)/lib$(PROFILE_NAME).a: $(OBJS)
	$(AR) rcs $(LIBDIR)/lib$(PROFILE_NAME).a $(OBJS)

clean:
	-$(RM) $(OBJDIR)

install: all

uninstall:
# Uninstall will be done on upper layer


.PHONY: all clean install uninstall

# Default rules

include ../../../config/Rules.mk

ifneq (${MAKECMDGOALS},uninstall)
ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif
endif
