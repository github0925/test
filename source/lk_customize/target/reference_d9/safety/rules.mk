LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

PLATFORM:= kunlun
SUB_PLATFORM:= safety

ifeq ($(SUPPORT_DISP_SDDRV),true)
MODULE_DEPS +=	exdev/display
GLOBAL_DEFINES += SUPPORT_DISP_SDDRV=1
ifeq ($(SUPPORT_LVGL_GUI), true)
GLOBAL_DEFINES += SUPPORT_LVGL_GUI=1
endif
endif
##SYSTEM_TIMER:synopsys generic	sdrv_timer##
SYSTEM_TIMER ?= sdrv_timer

ifeq ($(SYSTEM_TIMER),sdrv_timer)
GLOBAL_DEFINES += SDRV_TIMER=1
endif

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/ $(GLOBAL_INCLUDES)

MODULE_SRCS += \
	$(LOCAL_DIR)/target_init.c \
	$(LOCAL_DIR)/port.c

ifeq ($(SUPPORT_INPUT_SVC),true)
MODULE_SRCS += \
	$(LOCAL_DIR)/touch_device.c
endif

ifeq ($(SUPPORT_CSI_SVC),true)
MODULE_SRCS += \
	$(LOCAL_DIR)/cam_dev.c
endif

CSI_BOARDS := \
    CSI_BOARD_507 CSI_BOARD_507_A02 CSI_BOARD_507_A02P \
    CSI_BOARD_508 CSI_BOARD_ICL02

CSI_BOARD_VER ?= CSI_BOARD_507_A02

ifneq ($(findstring $(CSI_BOARD_VER),$(CSI_BOARDS)),)
    GLOBAL_DEFINES += $(CSI_BOARD_VER)=1
endif

ifeq ($(EXECUTION_PLACE),norflash)
TARGET_LINKER_SCRIPT := system-twosegment
else
ifneq ($(EXECUTION_PLACE), ddr)
override PRELOAD_RES_SIZE := 0
else
ifeq ($(PRELOAD_RES_SIZE),)
override PRELOAD_RES_SIZE := 0
endif
endif
TARGET_LINKER_SCRIPT := system-onesegment
endif

# rules for generating the linker script
$(BUILDDIR)/$(TARGET_LINKER_SCRIPT)-sd.ld: $(LOCAL_DIR)/$(TARGET_LINKER_SCRIPT).ld $(wildcard arch/*.ld) linkerscript.phony
	@echo generating $@
	@$(MKDIR)
	$(NOECHO) sed "s/%RELOCATED_OBJS%/$(RELOCATED_OBJS)/;s/%RELOCATED_OBJS_LINK_CMD%/$(RELOCATED_OBJS_LINK_CMD)/;s/%MEMBASE%/$(MEMBASE)/;s/%MEMSIZE%/$(MEMSIZE)/;s/%ROMBASE%/$(ROMBASE)/;s/%PRELOAD_RES_SIZE%/$(PRELOAD_RES_SIZE)/;s/%KERNEL_BASE%/$(KERNEL_BASE)/;s/%KERNEL_LOAD_OFFSET%/$(KERNEL_LOAD_OFFSET)/" < $< > $@.tmp
	@$(call TESTANDREPLACEFILE,$@.tmp,$@)

LINKER_SCRIPT += $(BUILDDIR)/$(TARGET_LINKER_SCRIPT)-sd.ld

include make/module.mk

