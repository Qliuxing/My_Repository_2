MUPET module is integrated from design repository:
http://cvs.kiev.elex.be/cgi-bin/cvsweb/cvseseg/projects/feasibility/MUPeT_module/


MUPET module integration instructions
-------------------------------------

1.  Copy files mupet_cfg.h, mupet_cbk.h and mupet_cbk.c (if any)
    from \libsrc\MUPET\TEMPLATE to application directory.

2.  Configure MUPET module by changing mupet_cfg.h in application directory.
    Update mupet_cbk.h and mupet_cbk.c (if any) to add application specific functionality.

3.  Include mupet.h into application source files which require functions from MUPET API.

    #include <mupet.h>


4. Update Makefile of the application to compile the MUPET module.

    If some other SW modules are already used by application, just specify MUPET module
    to the list of modules:

        MODULES = SENT MUPET

    Otherwise,
        - Create MODULES variable and specify MUPET module there:
            
            MODULES = MUPET

        - Add MODULES_DIRS variable which contains relative pathes to all modules
          specified in $(MODULES) list:

            MODULES_DIRS = $(patsubst %,$(PLTF_TOP_DIR)/libsrc/%,$(MODULES))

        - Specify $(MODULES_DIRS) in VAPTH and INCDIRS:

            VPATH   = $(MODULES_DIRS)
            INCDIRS = $(MODULES_DIRS)

        - Include srclist.mk file from module's directory which has a list of all
          sources files of the module in $(MODULES_SRC) variable:

            include $(patsubst %,%/srclist.mk,$(MODULES_DIRS))

        - Add $(MODULES_SRC) to the list of application's source files:

            SRCS += $(MODULES_SRC)


        - Add module's include directory to the include searching path:
            CPPFLAGS += $(patsubst %,-I%,$(INCDIRS))



=== Makefile (example) ========================================================
[...]
PRODUCT = 90294
PROFILE = 90294

PLTF_TOP_DIR = ../../../../../
MODULES = MUPET

#
# List of source files
#
# @note
#   1. The $(MODULES_SRC) is created from srclist.mk of the modules
#

SRCS = $(MODULES_SRC)
SRCS += main.c interrupts.c debug.c analog.c wd.c sys_modes.c 
SRCS += adc.c analogbuff.c dsp.c minmax.c output.c supervisor.c 
SRCS += cam_crank.c learning.c crank_learning.c cam_learning.c dynamic.c standstill.c
SRCS += tCompens.c map_eeprom.c globals.c info.S ampli_min.c eps_correct.c
SRCS += start_up.c bist_ram.c bist_wd.c diag.c


# --------------------------------------------------------------------------------

include $(PLTF_TOP_DIR)/config/Config.mk

MODULES_DIRS = $(patsubst %,$(PLTF_TOP_DIR)/libsrc/%,$(MODULES))

#vpath
#vpath %.c $(MODULES_DIRS)

VPATH   = $(MODULES_DIRS)
INCDIRS = $(MODULES_DIRS)

include $(patsubst %,%/srclist.mk,$(MODULES_DIRS))

CPPFLAGS += $(patsubst %,-I%,$(INCDIRS))

[...]
===============================================================================


See usage example at:
http://cvs.kiev.elex.be/cgi-bin/cvsweb/cvseseg/projects/90294/soft/src/Makefile?only_with_tag=MUPET_intgeration_example

