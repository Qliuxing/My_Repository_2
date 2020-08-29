#
# Copyright (C) 2008-2009 Melexis N.V.
#
# Software Platform
#

# Common definitions

include ../../../config/Config.mk

SRCS := vectors.S
SRCS += fatal.c low_level_init.c premain.c sys_mlx16.c
SRCS += itc2.c itc2_diag.c itc2_extio.c itc2_pwm.c itc2_tmr1.c itc2_tmr2.c



OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(patsubst %.o, %.d, $(OBJS))

# Main rules

all: $(OBJS)

install: all
#	$(CP) $(OBJS) $(LIBDIR)
	$(CP) $(PRODUCT_LIBDIR)/$(LD_SCRIPT) $(LIBDIR)/$(PROFILE_NAME).ld
	$(AR) rcs $(LIBDIR)/lib$(PROFILE_NAME).a $(OBJS)

clean:
	-$(RM) $(OBJDIR)

uninstall:
# Uninstall will be done on upper layer
#	$(RM) $(patsubst %, $(LIBDIR)/%, $(notdir $(OBJS)))


.PHONY: all clean install uninstall

# Default rules

include ../../../config/Rules.mk

ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif
