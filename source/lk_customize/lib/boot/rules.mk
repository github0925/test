LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/boot.c

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include \

MODULE_DEPS += hal/scr/

include make/module.mk

