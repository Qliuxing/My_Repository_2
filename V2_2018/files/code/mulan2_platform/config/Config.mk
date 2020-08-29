#
# Copyright (C) 2009-2013 Melexis N.V.
#
# Software Platform
#


#--- Common directories -------------------------------------------------------
# Identify the root directory of the SW Platform (escape spaces in the path)
empty :=
space := $(empty) $(empty)
ROOTDIR := $(subst $(space),\$(space),$(realpath $(dir $(filter %/Config.mk, $(MAKEFILE_LIST)))..))
LIBDIR  := $(ROOTDIR)/lib
INCDIR  := $(ROOTDIR)/include
OBJDIR := obj
OBJDIR_MARK := $(OBJDIR)/create

#--- Common flags -------------------------------------------------------------
CPPFLAGS  = -I . -I $(INCDIR)

OPTIMIZATION = -Os

CFLAGS  = $(OPTIMIZATION)
CFLAGS += -g
CFLAGS += -std=gnu99
CFLAGS += -mram-align-word
#CFLAGS += -fshort-enums 
CFLAGS += -Wall -Wextra
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wundef
#CFLAGS += -Wlogical-op
#CFLAGS += -Winline

ASFLAGS  = $(OPTIMIZATION) -gdwarf-2


#--- Product specific configuration -------------------------------------------

ifndef PRODUCT
$(error 'Error: Variable PRODUCT is not defined')
endif

ifndef PROFILE
$(error Error: Variable PROFILE is not defined)
endif

# Strip file extension and spaces (if any)
PROFILE_NAME := $(notdir $(basename $(strip $(PROFILE))))

PRODUCT_DIR    := $(ROOTDIR)/products/$(PRODUCT)
PRODUCT_LIBDIR := $(ROOTDIR)/products/$(PRODUCT)/lib

CPPFLAGS += -I $(INCDIR)/$(CORE_TYPE)

CPPFLAGS += -I $(PRODUCT_DIR)/include
CPPFLAGS += -D__MLX$(PRODUCT)__
#CPPFLAGS += -DPRODUCT_STR=\"$(PRODUCT)\"
#
# Placeholder for command-line defines:
#   gmake APP_OPTIONS="-Dxxx"
#
CPPFLAGS +=  $(APP_OPTIONS)

# Include product specific settings
include $(PRODUCT_DIR)/config/Extra.mk


#--- Toolchain and utilities --------------------------------------------------

# GCC programs
AR   = mlx16-ar
CC   = mlx16-gcc
ODP  = mlx16-objdump
OCP  = mlx16-objcopy
NM   = mlx16-nm
SIZE = mlx16-size

# Standard utilities (MUST be in the environment PATH)
ECHO  = echo
RM    = rm -fr
CP    = cp -f
MKDIR = mkdir -p

# Additional tools
ifeq ($(OS),Windows_NT)
	TRHEX    = $(ROOTDIR)/bin/TrHex.exe
	SREC_CAT = $(ROOTDIR)/bin/srec_cat.exe
	LINT     = lint-nt.exe
	LDF_NODEGEN = $(ROOTDIR)/bin/ldf_nodegen.exe
else
	TRHEX    = trhex
	SREC_CAT = srec_cat
	LINT     = lint-nt.exe
	LDF_NODEGEN = ldf_nodegen
endif
