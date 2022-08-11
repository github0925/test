LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/km_dump.c \
	$(LOCAL_DIR)/km.c

include make/module.mk

