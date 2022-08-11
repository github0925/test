LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/host \
	$(LOCAL_DIR)/device

MODULE_DEPS += exdev/norflash

MODULE_SRCS += \
	$(LOCAL_DIR)/host/cadence_ospi.c \

include make/module.mk
