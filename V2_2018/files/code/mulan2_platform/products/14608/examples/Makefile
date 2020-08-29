#
# Copyright (C) 2008-2009 Melexis N.V.
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

.PHONY: all install clean $(SUBMODULE)