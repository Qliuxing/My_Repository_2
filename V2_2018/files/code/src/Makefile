#
# Copyright (C) 2005-2015 Melexis N.V.
#
# Software Platform
#
# Revision $Name:  $
#
# File $RCSfile: Makefile,v $
#

# target dependent configurable tools
include Makefile.configure.mk

ifndef MLX4_TYPE
$(error 'Error: Variable MLX4_TYPE is not defined')
endif

ifndef CHIPREV
$(error 'Error: Variable CHIPREV is not defined')
endif

ifndef PRODUCT
$(error 'Error: Variable PRODUCT is not defined')
endif

PROFILE ?= $(PRODUCT)-$(MLX4_TYPE)

UNCRUSTIFY ?= uncrustify

PROJ_DIR = ../..
CODE_DIR = $(PROJ_DIR)/code
PLTF_DIR = $(CODE_DIR)/mulan2_platform
LINT_DIR = $(PROJ_DIR)/verification/lint
UNCR_DIR = $(PROJ_DIR)/verification/uncrustify

LIBRARIES = 

TARGET := MLX$(PRODUCT)$(CHIPREV)_S03_VALVE

LDF_FILE = 

include $(PLTF_DIR)/config/Config.mk

# search path for ld script
vpath
vpath %.ld $(PRODUCT_LIBDIR)

#
# LIBRARIES
#
LIBS_DIRS = $(realpath $(patsubst %,$(CODE_DIR)/libraries/%,$(LIBRARIES)))
VPATH   = $(LIBS_DIRS)
INCDIRS = $(LIBS_DIRS)
include $(patsubst %,%/srclist.mk,$(LIBS_DIRS))

#
# SOURCE FILES LIST
#
include Makefile.srcs.mk
SRCS += $(LIBRARIES_SRC)
SRCS += $(LIN_SRCS)

OBJS  = $(patsubst %.S, $(OBJDIR)/%.o, $(filter %.S, $(SRCS)))
OBJS += $(patsubst %.c, $(OBJDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:%.o=%.d)

LDFLAGS := -N -L $(LIBDIR) -l$(PROFILE)
LDFLAGS += -L $(PRODUCT_LIBDIR) -T $(LD_SCRIPT)
LDFLAGS += $(INSTSET)
LDFLAGS += -Wl,-Map,$(TARGET).map
# Remove deadcode
##LDFLAGS += -Wl,--gc-sections -Wl,-s

CPPFLAGS += $(patsubst %,-I%,$(INCDIRS))
#CPPFLAGS += -D__MATHLIB_NON_INLINE__

# Main rules
.PHONY: all
all: $(TARGET).elf temp.hex $(TARGET).hex $(TARGET).lss $(TARGET).sym

# Link : Create ELF file from object files
$(TARGET).elf: $(OBJS) $(LIBDIR)/lib$(PROFILE).a Makefile
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# build the platform library - if not yet build
$(LIBDIR)/lib$(PROFILE).a:
	'$(MAKE)' libs

# Create temporary hex file from ELF output file
temp.hex: $(TARGET).elf
	$(OCP) -O ihex  $< $@

# Create final hex file based on temp.hex:
# 1. Generate fill pattern (0x07FF) for range [0x4000..0xBFFF] excluding locations
#    occupied by temp.hex
# 2. Join output from (1) with temp.hex file
# 3. Swap bytes big endian to little endian for checksum calculation
# 4. Crop result to leave only ROM area including checksum, i.e. [0x4000..0xBFFF]
# 5. Exclude the checksum, i.e. [0xBF4E..0xBF4F]
# 6. Calculate the checksum
# 7. Swap bytes little endian to big endian to make a standard output
$(TARGET).hex: temp.hex
	$(SREC_CAT) -disable-sequence-warning '(' '(' --generate '(' 0x4000 0xC000 -minus -within temp.hex -intel ')' -repeat-data 0xFF 0x07 ')' temp.hex --intel ')' \
        --byte-swap 2 --crop 0x4000 0xC000 --exclude 0xBF4E 0xBF50 --b-e-crc16 0xBF4E --CCITT --NoAUG \
        --byte-swap 2 --output $(TARGET).hex --intel --address-length=2 --line-length=44

# Remove Temp hex file
	-$(RM) temp.hex

.PHONY: release
release : all
	$(CP) $(TARGET).elf $(TARGET).hex $(TARGET).lss $(PROJ_DIR)/release

.PHONY: clean
clean:
	-$(RM) $(OBJDIR) 
	-$(RM) $(TARGET).cod $(TARGET).elf.MDS $(TARGET).map $(TARGET).elf $(TARGET).hex
	-$(RM) $(TARGET).lss $(TARGET).sym
	-$(RM) $(TARGET)_lint.txt
	-$(RM) $(TARGET)_lint_misra.txt

.PHONY: lint
lint: $(SRCS)
# get correct gcc include directories
	@$(ECHO) $(subst /cygdrive/c,C:,$(foreach v,$(shell echo | mlx16-cpp -Wp,-v 2>&1),$(if $(findstring "/,$(v)), ,$(if $(findstring /,$(v)),-i\"$(v)\",)))) > $(LINT_DIR)/gcc-include-path.lnt
# get all include directories
	@$(ECHO) $(subst /cygdrive/c,C:,$(subst -I,,$(foreach v,$(CPPFLAGS),$(if $(findstring /,$(v):0:1),-i\"$(v)\",)))) > $(LINT_DIR)/project_include_path.lnt
	@$(ECHO) | $(CC) -E -dM $(INSTSET) $(CFLAGS) $(CPPFLAGS) - > $(LINT_DIR)/lint_cmac.h
	$(LINT) -i"$(LINT_DIR)" common-options.lnt co-gcc.lnt project-options.lnt temp.lnt $(filter %.c, $(SRCS)) > $(TARGET)_lint.txt
	$(LINT) -i"$(LINT_DIR)" common-options.lnt co-gcc.lnt au-misra2.lnt project-options.lnt temp.lnt $(filter %.c, $(SRCS)) > $(TARGET)_lint_misra.txt

# get all application c and h files to be uncrustified
SRCS_UNCRUSTIFY := $(wildcard *.c) $(wildcard *.h)

.PHONY: uncrustify
uncrustify:
	$(UNCRUSTIFY) -c $(UNCR_DIR)/uncrustify.cfg --replace --no-backup -l C $(SRCS_UNCRUSTIFY)

.PHONY: uncrustify-check
uncrustify-check:
	$(UNCRUSTIFY) -c $(UNCR_DIR)/uncrustify.cfg --check -l C $(SRCS_UNCRUSTIFY)

include $(PLTF_DIR)/config/Rules.mk

# Include automatic dependencies
ifneq (${MAKECMDGOALS},clean)
ifneq (${MAKECMDGOALS},lint)
ifneq (${MAKECMDGOALS},uncrustify)
ifneq (${MAKECMDGOALS},uncrustify-check)
-include $(DEPS)
endif
endif
endif
endif
