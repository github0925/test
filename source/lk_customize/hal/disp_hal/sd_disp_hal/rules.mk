LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/inc

# MODULE_INCLUDES += $(GET_LOCAL_DIR)/lib/inc

ifeq ($(SUPPORT_DISP_SDDRV),true)
GLOBAL_DEFINES += ENABLE_SD_DISP=1
MODULE_SRCS += \
	$(LOCAL_DIR)/src/disp_hal.c
endif

include make/module.mk
