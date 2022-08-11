LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/app_boot_ss.c \

MODULE_DEPS += lib/reboot

include make/module.mk
