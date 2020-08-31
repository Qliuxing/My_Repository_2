#
# Copyright (C) 2011-2015 Melexis N.V.
#
# Software Platform
#

.PHONY: all
all: 
	-$(MAKE) -f build.mk clean TARGET=LIN_NAD_02h EXTRA_DEFINES="-DUNIQUE_NAD=0x02" 
	-$(MAKE) -f build.mk all   TARGET=LIN_NAD_02h EXTRA_DEFINES="-DUNIQUE_NAD=0x02" 
	-$(MAKE) -f build.mk clean TARGET=LIN_NAD_0Bh EXTRA_DEFINES="-DUNIQUE_NAD=0x0B"
	-$(MAKE) -f build.mk all   TARGET=LIN_NAD_0Bh EXTRA_DEFINES="-DUNIQUE_NAD=0x0B" 

.PHONY: clean
clean:
	-$(MAKE) -f build.mk clean TARGET=LIN_NAD_02h EXTRA_DEFINES="-DUNIQUE_NAD=0x02" 
	-$(MAKE) -f build.mk clean TARGET=LIN_NAD_0Bh EXTRA_DEFINES="-DUNIQUE_NAD=0x0B" 

.PHONY: clean_libs
clean_libs:
	-$(MAKE) -f build.mk clean_libs

