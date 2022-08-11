LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/include/ $(GLOBAL_INCLUDES) \

MODULE_SRCS += \
	$(LOCAL_DIR)/core.c

include make/module.mk
