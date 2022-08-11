LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/src/boot_ss.c \
	$(LOCAL_DIR)/src/boot_ss_cfg.c

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc \

include make/module.mk

