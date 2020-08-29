#
# Copyright (C) 2005-2013 Melexis N.V.
#
# Software Platform
#

# Common definitions

include ../../config/Config.mk


SRCS = nvram.c

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
