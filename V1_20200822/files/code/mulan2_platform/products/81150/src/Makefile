#
# Copyright (C) 2008-2013 Melexis N.V.
#
# Software Platform
#

# Common definitions

include ../../../config/Config.mk

SRCS := fatal.c low_level_init.c premain.c
SRCS += it_ext0.c it_ext1.c it_ext2.c it_ext3.c
SRCS += sys_mlx16.c
SRCS += mlx_ispowerok.c

ifeq ($(HAS_MLX4_CODE), 1)
SRCS += vectors-lin.S
else
SRCS += vectors-nolin.S
endif


OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS  = $(OBJS:.o=.d)

# --- Main rules --------------------------------------------------------------

all: $(LIBDIR)/lib$(PROFILE_NAME).a
	$(CP) $(PRODUCT_LIBDIR)/$(LD_SCRIPT) $(LIBDIR)/$(PROFILE_NAME).ld

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

ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif
