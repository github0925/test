LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include/lib

MODULE_SRCS += \
	$(LOCAL_DIR)/cbuf.c

include make/module.mk
