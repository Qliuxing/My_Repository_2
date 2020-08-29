#
# Copyright (C) 2005-2011 Melexis N.V.
#
# Software Platform
#

# Default rules

$(OBJDIR)/%.o:%.c $(OBJDIR_MARK)
	$(CC) -c -MMD $(INSTSET) $(CFLAGS)  $(CPPFLAGS) $< -o $@ 

$(OBJDIR)/%.o:%.S $(OBJDIR_MARK)
	$(CC) -c -MMD $(INSTSET) $(ASFLAGS) $(CPPFLAGS) $< -o $@ 

$(OBJDIR)/%.d:%.c $(OBJDIR_MARK)
	$(CC) -MM -P $(INSTSET) $(CFLAGS) $(CPPFLAGS) $< -MF $@

$(OBJDIR)/%.d:%.S $(OBJDIR_MARK)
	$(CC) -MM -P $(INSTSET) $(ASFLAGS) $(CPPFLAGS) $< -MF $@

$(OBJDIR_MARK):
	-$(MKDIR) $(OBJDIR)
	$(ECHO) > $(OBJDIR_MARK)


# Execute 'make clean' to clean up obj files from the previos build
# before doing library build for other profile
.PHONY: libs
libs:
	@$(MAKE) --directory=$(PLTF_DIR)/libsrc clean PRODUCT=$(PRODUCT) PROFILE=$(PROFILE)
	@$(MAKE) --directory=$(PLTF_DIR)/libsrc all   PRODUCT=$(PRODUCT) PROFILE=$(PROFILE)

.PHONY: clean_libs
clean_libs:
	@$(MAKE) --directory=$(PLTF_DIR)/libsrc clean PRODUCT=$(PRODUCT) PROFILE=$(PROFILE)

# Create hex / bin files from ELF output file
%.hex: %.elf
	$(OCP) -O ihex $< $@

%.bin: %.elf
	$(OCP) -O binary $< $@

# Create extended listing file from ELF output file
%.lss: %.elf
	$(CC) --version > $@
	$(ODP) -h -S -z $< >> $@

# Create a symbol table from ELF output file
%.sym: %.elf
	$(NM) -n $< > $@

# Create assembler files from C source files.
%.s : %.c
	$(CC) -S $(INSTSET) $(CFLAGS) $(CPPFLAGS) $< -o $@

# Create preprocessed source
%.i : %.c
	$(CC) -E $(INSTSET) $(CFLAGS) $(CPPFLAGS) $< -o $@ 
