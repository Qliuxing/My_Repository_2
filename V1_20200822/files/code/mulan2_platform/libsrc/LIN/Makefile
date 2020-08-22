#
# Copyright (C) 2005-2013 Melexis N.V.
#
# Software Platform
#

# Common definitions

include ../../config/Config.mk


SRCS := mlx4_prog.S api.S lin.c ml_LinInterruptHandler.c
SRCS += ml_GetBaudRate.c ml_InitLinModule.c

ifeq ($(HAS_LOADER), 1)
SRCS += flashupload.c ml_LinInit.c loader_state.c loader_get_state.c
SRCS += flashfunctions_c_cmp.c ml_SetFastBaudRate.c
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

include ../../config/Rules.mk

ifneq (${MAKECMDGOALS},uninstall)
ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif
endif
