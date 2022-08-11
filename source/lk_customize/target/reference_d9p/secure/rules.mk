LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

PLATFORM:= kunlun
SUB_PLATFORM:= secure

##SYSTEM_TIMER:synopsys generic	sdrv_timer##
SYSTEM_TIMER ?= sdrv_timer

ifeq ($(SYSTEM_TIMER),sdrv_timer)
GLOBAL_DEFINES += SDRV_TIMER=1
endif

GLOBAL_INCLUDES := \
		$(LOCAL_DIR)/ $(GLOBAL_INCLUDES)

MODULE_SRCS += \
    $(LOCAL_DIR)/target_init.c \

ifeq ($(SUPPORT_DDR_INIT_n_TRAINING),true)
    MODULE_SRCS += $(LOCAL_DIR)/ddr_init.c
ifneq ($(DDR_TYPE),)
    DDR_SCRIPT_NAME := ddr_init_$(DDR_TYPE)_$(DDR_SIZE)_$(DDR_FREQ).c
    DDR_SCRIPT_PATH := \"ddr_init_script/$(DDR_TYPE)/$(DDR_SCRIPT_NAME)\"
    MODULE_CFLAGS += -DDDR_SCRIPT_PATH=$(DDR_SCRIPT_PATH)
endif
endif

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

