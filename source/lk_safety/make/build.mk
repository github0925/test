# use linker garbage collection, if requested
ifeq ($(WITH_LINKER_GC),1)
GLOBAL_COMPILEFLAGS += -ffunction-sections -fdata-sections
GLOBAL_LDFLAGS += --gc-sections
endif

BOARD_NAME ?= $(CHIPVERSION)_ref
ifneq ($(PROJECT), default)
BOARD_NAME := $(CHIPVERSION)_$(PROJECT)
endif

ifneq (,$(EXTRA_BUILDRULES))
-include $(EXTRA_BUILDRULES)
endif

$(EXTRA_LINKER_SCRIPTS):

$(OUTBIN): $(OUTELF)
	@echo generating image: $@
	$(NOECHO)$(SIZE) $<
	$(NOECHO)$(OBJCOPY) -O binary $< $@


ifeq ($(EXECUTION_PLACE),norflash)

ifeq ($(RAPID_STUDIO),1)
	$(AT) tools/sign_tool/run_sign_safe_windows $(OUTBIN) $(ROMBASE)
else
	$(AT) tools/sign_tool/run_sign_safe $(OUTBIN) $(ROMBASE)
endif

else
ifeq ($(RAPID_STUDIO),1)
ifneq ($(META_SEMIDRIVE),)
	@echo "boardname:"$(BOARD_NAME)
	$(AT) cp  $(META_SEMIDRIVE)/scripts/$(BOARD_NAME)/t_loader.bin $(BUILDDIR)/t_loader_safety_merged.bin
	$(AT) cat $(OUTBIN) >> $(BUILDDIR)/t_loader_safety_merged.bin
	$(AT) tools/sign_tool/run_sign_safe_windows $(BUILDDIR)/t_loader_safety_merged.bin $(MEMBASE)
else
	$(AT) tools/sign_tool/run_sign_safe_windows $(OUTBIN) $(MEMBASE)
endif
else
	$(AT) tools/sign_tool/run_sign_safe $(OUTBIN) $(MEMBASE)
endif
endif


$(OUTELF).hex: $(OUTELF)
	@echo generating hex file: $@
	$(NOECHO)$(OBJCOPY) -O ihex $< $@

$(OUTELF): $(ALLMODULE_OBJS) $(EXTRA_OBJS) $(LINKER_SCRIPT) $(EXTRA_LINKER_SCRIPTS)
	@echo linking $@
	$(NOECHO)$(SIZE) -t --common $(sort $(ALLMODULE_OBJS)) $(EXTRA_OBJS)
	$(NOECHO)$(LD) $(GLOBAL_LDFLAGS) -dT $(LINKER_SCRIPT) $(addprefix -T,$(EXTRA_LINKER_SCRIPTS)) \
		--start-group $(ALLMODULE_OBJS) $(EXTRA_OBJS) --end-group $(LIBGCC) -Map=$(OUTELF).map -o $@

$(OUTELF).sym: $(OUTELF)
	@echo generating symbols: $@
	$(NOECHO)$(OBJDUMP) -t $< | $(CPPFILT) > $@

$(OUTELF).sym.sorted: $(OUTELF)
	@echo generating sorted symbols: $@
	$(NOECHO)$(OBJDUMP) -t $< | $(CPPFILT) | sort > $@

$(OUTELF).lst: $(OUTELF)
	@echo generating listing: $@
	$(NOECHO)$(OBJDUMP) -Mreg-names-raw -d $< | $(CPPFILT) > $@

$(OUTELF).debug.lst: $(OUTELF)
	@echo generating listing: $@
	$(NOECHO)$(OBJDUMP) -Mreg-names-raw -S $< | $(CPPFILT) > $@

$(OUTELF).dump: $(OUTELF)
	@echo generating objdump: $@
	$(NOECHO)$(OBJDUMP) -x $< > $@

$(OUTELF).size: $(OUTELF)
	@echo generating size map: $@
	$(NOECHO)$(NM) -S --size-sort $< > $@

# print some information about the build
$(BUILDDIR)/srcfiles.txt: $(OUTELF)
	@echo generating $@
	$(NOECHO)echo $(sort $(ALLSRCS)) | tr ' ' '\n' > $@

$(BUILDDIR)/include_paths.txt: $(OUTELF)
	@echo generating $@
	$(NOECHO)echo $(subst -I,,$(sort $(GLOBAL_INCLUDES))) | tr ' ' '\n' > $@

#include arch/$(ARCH)/compile.mk

