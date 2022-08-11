LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/virtio.c

MODULE_COMPILEFLAGS += -Wno-format

include make/module.mk
