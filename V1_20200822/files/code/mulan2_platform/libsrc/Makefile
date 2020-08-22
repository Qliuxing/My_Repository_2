#
# Copyright (C) 2005-2013 Melexis N.V.
#
# Software Platform
#

# Common definitions
include ../config/Config.mk

SUBMODULE := $(LIBSUBMODULE)

# Main rules
.PHONY: all
all: start_profile $(SUBMODULE) end_profile

.PHONY: clean
clean:  $(SUBMODULE)
	@$(ECHO) --- Clean profile : $(PROFILE_NAME)
	$(RM) $(LIBDIR)/lib$(PROFILE_NAME).a
	$(RM) $(LIBDIR)/$(PROFILE_NAME).ld

# 'install' target is an alias for 'all' target
.PHONY: install
install: all

.PHONY: uninstall
uninstall:
# no action on uninstall target

# Sub rules
.PHONY: $(SUBMODULE)
$(SUBMODULE):
	'$(MAKE)' -C $@ $(MAKECMDGOALS)

.PHONY: start_profile
start_profile :
	@$(ECHO) --- Creating profile : $(PROFILE_NAME)

.PHONY: end_profile
end_profile :
	@$(ECHO) --- Profile $(PROFILE_NAME) successfully created

# EOF
