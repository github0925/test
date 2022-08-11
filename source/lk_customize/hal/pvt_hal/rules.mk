LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc/

ifeq ($(SUPPORT_PVT_SDDRV),true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/pvt_hal.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/pvt_hal_weak.c
endif
include make/module.mk
