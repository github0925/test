LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

PLATFORM:= kunlun
SUB_PLATFORM:= ap1

GLOBAL_INCLUDES := \
		$(LOCAL_DIR)/ $(GLOBAL_INCLUDES)

MODULE_SRCS += \
	$(LOCAL_DIR)/target_init.c

# rules for generating the linker script
$(BUILDDIR)/system-onesegment-sd.ld: $(LOCAL_DIR)/system-onesegment.ld $(wildcard arch/*.ld) linkerscript.phony
	@echo generating $@
	@$(MKDIR)
	$(NOECHO)sed "s/%MEMBASE%/$(MEMBASE)/;s/%MEMSIZE%/$(MEMSIZE)/;s/%KERNEL_BASE%/$(KERNEL_BASE)/;s/%KERNEL_LOAD_OFFSET%/$(KERNEL_LOAD_OFFSET)/" < $< > $@.tmp
	@$(call TESTANDREPLACEFILE,$@.tmp,$@)
ifeq ($(WITH_KERNEL_VM),1)
LINKER_SCRIPT += $(BUILDDIR)/system-onesegment-sd.ld
else
LINKER_SCRIPT += $(BUILDDIR)/system-onesegment.ld
endif
include make/module.mk

