LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc/
include $(LOCAL_DIR)/lib/rules.mk

ifeq ($(SUPPORT_G2DLITE_SDDRV), true)
GLOBAL_DEFINES += ENABLE_SD_G2DLITE=1
MODULE_SRCS += \
	$(LOCAL_DIR)/src/g2dlite_hal.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/g2dlite_hal_weak.c
endif

include make/module.mk
