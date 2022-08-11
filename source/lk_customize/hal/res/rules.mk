LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/inc

ifeq ($(RES_EPU), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/res_epu.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/res.c
endif
include make/module.mk
