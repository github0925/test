LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/sdpe_mgr.c

MODULE_DEPS += \
	lib/storage_device \
	lib/partition

include make/module.mk
