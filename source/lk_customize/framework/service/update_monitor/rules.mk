LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/update_monitor.c  \

MODULE_DEPS += framework/communication
MODULE_DEPS += lib/cksum
MODULE_DEPS += lib/storage_device
include make/module.mk
