#
# Copyright (C) 2011 Melexis N.V.
#
# Software Platform
#

# Main rules

all: $(SUBMODULE)

install: $(SUBMODULE)

clean: $(SUBMODULE)

# Sub rules

$(SUBMODULE):
	$(MAKE) -C $@ $(MAKECMDGOALS)
	@ECHO --- Processing: $@

.PHONY: all install clean $(SUBMODULE)
