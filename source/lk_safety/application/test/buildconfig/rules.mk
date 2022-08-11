LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/build_config.c

MODULE_DEPS += lib/res_loader
include make/module.mk
