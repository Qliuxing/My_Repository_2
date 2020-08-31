#
# Copyright (C) 2011-2015 Melexis N.V.
#
# Software Platform
#

.PHONY: all
all: $(EXAMPLE)

.PHONY: install
install: $(EXAMPLE)

.PHONY: clean
clean: $(EXAMPLE)

.PHONY: clean_libs
clean_libs: $(EXAMPLE)

# Sub rules
.PHONY: $(EXAMPLE)
$(EXAMPLE):
	@ECHO --- Processing: $@
	$(MAKE) -C $@ $(MAKECMDGOALS)
