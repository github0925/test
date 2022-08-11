LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

ifeq ($(SUPPORT_SCR_SDDRV),true)

GLOBAL_DEFINES += ENABLE_SD_SCR=1
GLOBAL_INCLUDES += $(LOCAL_DIR)/inc
MODULE_SRCS += \
	$(LOCAL_DIR)/src/scr_hal.c

include make/module.mk

endif
